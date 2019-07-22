//
// Created by 苏畅 on 2019/6/22.
//

#ifndef REIMU_BUFFER_H
#define REIMU_BUFFER_H

#include <vector>
#include <string>
#include <cstring>
#include <mutex>

namespace reimu {
    class Buffer;

    typedef std::shared_ptr<Buffer> BufferPtr;

    class Slice {
    public:

        Slice() : pb_("") { pe_ = pb_; }

        Slice(const char *b, const char *e) : pb_(b), pe_(e) {}

        Slice(const char *d, size_t n) : pb_(d), pe_(d + n) {}

        Slice(const std::string &s) : pb_(s.data()), pe_(s.data() + s.size()) {}

        Slice(const char *s) : pb_(s), pe_(s + strlen(s)) {}



        const char *data() const { return pb_; }

        const char *begin() const { return pb_; }

        const char *end() const { return pe_; }

        char front() { return *pb_; }

        char back() { return pe_[-1]; }

        size_t size() const { return pe_ - pb_; }

        void resize(size_t sz) { pe_ = pb_ + sz; }

        inline bool empty() const { return pe_ == pb_; }

        void clear() { pe_ = pb_ = ""; }

        // return the eated data
        Slice eatWord();

        Slice eatLine();

        Slice eat(int sz) {
            Slice s( pb_, sz);
            pb_ += sz;
            return s;
        }

        Slice sub(int boff, int eoff = 0) const {
            Slice s(*this);
            s.pb_ += boff;
            s.pe_ += eoff;
            return s;
        }

        Slice &trimSpace();

        inline char operator[](size_t n) const { return pb_[n]; }

        std::string toString() const { return std::string(pb_, pe_); }

        // Three-way comparison.  Returns value:
        int compare(const Slice &b) const;

        // Return true if "x" is a prefix of "*this"
        bool starts_with(const Slice &x) const { return (size() >= x.size() && memcmp(pb_, x.pb_, x.size()) == 0); }

        bool end_with(const Slice &x) const {
            return (size() >= x.size() && memcmp(pe_ - x.size(), x.pb_, x.size()) == 0);
        }

        operator std::string() const { return std::string(pb_, pe_); }

        std::vector<Slice> split(char ch) const;


    private:
        const char *pb_;
        const char *pe_;

    };

    inline Slice Slice::eatWord() {
        const char *b = pb_;
        while (b < pe_ && isspace(*b)) {
            b++;
        }
        const char *e = b;
        while (e < pe_ && !isspace(*e)) {
            e++;
        }
        pb_ = e;
        return Slice(b, e - b);
    }

    inline Slice Slice::eatLine() {
        const char *p = pb_;
        while (pb_ < pe_ && *pb_ != '\n' && *pb_ != '\r') {
            pb_++;
        }
        return Slice(p, pb_ - p);
    }

    inline Slice &Slice::trimSpace() {
        while (pb_ < pe_ && isspace(*pb_))
            pb_++;
        while (pb_ < pe_ && isspace(pe_[-1]))
            pe_--;
        return *this;
    }

    inline bool operator<(const Slice &x, const Slice &y) {
        return x.compare(y) < 0;
    }

    inline bool operator==(const Slice &x, const Slice &y) {
        return ((x.size() == y.size()) && (memcmp(x.data(), y.data(), x.size()) == 0));
    }

    inline bool operator!=(const Slice &x, const Slice &y) {
        return !(x == y);
    }

    inline int Slice::compare(const Slice &b) const {
        size_t sz = size(), bsz = b.size();
        const int min_len = (sz < bsz) ? sz : bsz;
        int r = memcmp(pb_, b.pb_, min_len);
        if (r == 0) {
            if (sz < bsz)
                r = -1;
            else if (sz > bsz)
                r = +1;
        }
        return r;
    }

    inline std::vector<Slice> Slice::split(char ch) const {
        std::vector<Slice> r;
        const char *pb = pb_;
        for (const char *p = pb_; p < pe_; p++) {
            if (*p == ch) {
                r.push_back(Slice( pb, p));
                pb = p + 1;
            }
        }
        if (pe_ != pb_)
            r.push_back(Slice(pb, pe_));
        return r;
    }

    class Buffer : public std::enable_shared_from_this<Buffer> {
    private:
        const size_t MAX_PREPENND_LENGTH = 3000;
        std::vector<char> _v;
        size_t _write_idx = 0, _read_idx = 0;
        std::mutex _mutex;
    public:
        Slice Read(size_t n);

        Slice ReadAll();

        void PeekReadPos(size_t read_pos) {
            std::lock_guard<std::mutex> lock(_mutex);
            if (read_pos > 0) {
                _read_idx = read_pos;
            }
        };

        long GetReadPos() {
            std::lock_guard<std::mutex> lock(_mutex);
            return _read_idx;
        }

        Buffer &Write(const std::string &s);

        void Consume(size_t n) {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_read_idx + n > _write_idx) {
                _read_idx = _write_idx;
            } else {
                _read_idx = _read_idx + n;
            }
        }

        Slice ToSlice() {
            std::lock_guard<std::mutex> lock(_mutex);
            return Slice(&*(_v.begin() + _read_idx), &*(_v.begin() + _write_idx));
        }

    private:
        long prepend(); //缩减
    };
}


#endif //REIMU_BUFFER_H
