/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <common/StringVector.hpp>
#include <common/Log.hpp>

#define KIT_USE_UNSTABLE_API
#include <COKit/COKitEnums.h>

#include <typeinfo>

#include <Poco/File.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Path.h>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <limits>
#include <map>
#include <memory.h>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#define STRINGIFY(X) #X

#if CODE_COVERAGE
extern "C"
{
    void __gcov_reset(void);
    void __gcov_flush(void);
    void __gcov_dump(void);
}
#endif

#if defined(__COVERITY__)
#define THREAD_UNSAFE_DUMP_BEGIN _Pragma("coverity compliance block deviate MISSING_LOCK \"Intentionally thread-unsafe dumping\"")
#define THREAD_UNSAFE_DUMP_END _Pragma("coverity compliance end_block MISSING_LOCK")
#else
#define THREAD_UNSAFE_DUMP_BEGIN
#define THREAD_UNSAFE_DUMP_END
#endif

#ifdef __linux__
#define FDCOUNTER_USABLE 1
#endif

/// Format minutes with the units suffix until we migrate to C++20.
inline std::ostream& operator<<(std::ostream& os, const std::chrono::minutes& s)
{
    os << s.count() << "m";
    return os;
}

/// Format seconds with the units suffix until we migrate to C++20.
inline std::ostream& operator<<(std::ostream& os, const std::chrono::seconds& s)
{
    os << s.count() << 's';
    return os;
}

/// Format milliseconds with the units suffix until we migrate to C++20.
inline std::ostream& operator<<(std::ostream& os, const std::chrono::milliseconds& ms)
{
    os << ms.count() << "ms";
    return os;
}

/// Format microseconds with the units suffix until we migrate to C++20.
inline std::ostream& operator<<(std::ostream& os, const std::chrono::microseconds& ms)
{
    os << ms.count() << "us";
    return os;
}

template <typename S, typename U, typename V>
inline S& operator<<(S& stream, const std::pair<U, V>& pair)
{
    stream << pair.first << ": " << pair.second;
    return stream;
}

namespace Util
{
    namespace rng
    {
        /// Returns a global handle to /dev/urandom - do not close it.
        int getURandom();

        uint_fast64_t getSeed();
        void reseed();
        void seedForTesting(uint_fast64_t seed);
        unsigned getNext();

        /// Generate an array of random characters.
        std::vector<char> getBytes(size_t length);

        /// Generate a string of random characters.
        std::string getHexString(size_t length);

        /// Generates a random string suitable for
        /// file/directory names.
        std::string getFilename(size_t length);
    }

    /// A utility class to track relative time from some arbitrary
    /// origin, and to check if a certain amount has elapsed or not.
    class Stopwatch
    {
    public:
        Stopwatch()
            : _startTime(std::chrono::steady_clock::now())
        {
        }

        /// Returns the start-time.
        std::chrono::steady_clock::time_point startTime() const { return _startTime; }

        /// Resets the start-time to now.
        void restart() { _startTime = std::chrono::steady_clock::now(); }

        /// Returns the time that has elapsed since starting, in the units required.
        /// Units defaults to milliseconds.
        template <typename T = std::chrono::milliseconds>
        [[nodiscard]] T
        elapsed(std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now()) const
        {
            return std::chrono::duration_cast<T>(now - _startTime);
        }

        /// Returns true iff at least the given amount of time has elapsed.
        template <typename T>
        bool
        elapsed(T duration,
                std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now()) const
        {
            return elapsed<std::chrono::nanoseconds>(now) >= duration;
        }

    private:
        std::chrono::steady_clock::time_point _startTime;
    };

    /// A utility class to time using system metrics
    class SysStopwatch
    {
    public:
        SysStopwatch();
        void restart();
        [[nodiscard]] std::chrono::microseconds elapsedTime() const;

    private:
        static void readTime(uint64_t &cpu, uint64_t &sys);
        uint64_t _startCPU;
        uint64_t _startSys;
    };

    class CounterImpl;

    /// Needs to open dirent before forking in Kit process
    class ThreadCounter
    {
        std::unique_ptr<CounterImpl> _impl;

    public:
        ThreadCounter();
        ~ThreadCounter();
        /// Get number of items in this directory or -1 on error
        int count();
    };

    /// Needs to open dirent before forking in Kit process
    class FDCounter
    {
        std::unique_ptr<CounterImpl> _impl;
    public:
        FDCounter();
        ~FDCounter();
        /// Get number of items in this directory or -1 on error
        int count();
    };

    /// Exception safe scope count/guard
    struct ReferenceHolder
    {
        int &_count;
        ReferenceHolder(int &count) : _count(count) { _count++; }
        ~ReferenceHolder() { _count--; }
    };

    /// Hex-encode an integral ID into a buffer, with padding support.
    inline std::string_view encodeId(char* buffer, std::size_t size, const std::uint64_t number,
                                     int width, char pad = '0')
    {
        // Skip leading (high-order) zeros, if any.
        int highNibble = (2 * sizeof(number) - 1) * 4;
        while ((number & (std::uint64_t(0xf) << highNibble)) == 0)
        {
            highNibble -= 4;
            if (highNibble <= 0)
                break;
        }

        // Pad, if necessary.
        highNibble = std::min<int>(size - 1, highNibble / 4) * 4;
        width = std::min<int>(size, width);
        int outIndex = 0;
        const int hexBytes = (highNibble / 4) + 1;
        for (; width > hexBytes; --width)
        {
            buffer[outIndex++] = pad;
        }

        // Hexify the remaining, if any.
        constexpr const char* const Hex = "0123456789abcdef";
        while (highNibble >= 0)
        {
            const auto byte = static_cast<unsigned char>((number >> highNibble) & 0xf);
            buffer[outIndex++] = (Hex[byte >> 4] << 8) | Hex[byte & 0xf];
            highNibble -= 4;
        }

        // Return a reference to the given buffer.
        return std::string_view(buffer, outIndex);
    }

    /// Hex-encode an integral ID into a string, with padding support.
    inline std::string encodeId(const std::uint64_t number, int width = 5, char pad = '0')
    {
        char buffer[32];
        return std::string(encodeId(buffer, sizeof(buffer), number, width, pad));
    }

    /// Hex-encode an integral ID into a stream, with padding support.
    inline std::ostringstream& encodeId(std::ostringstream& oss, const std::uint64_t number,
                                        int width = 5, char pad = '0')
    {
        char buffer[32];
        oss << encodeId(buffer, sizeof(buffer), number, width, pad);
        return oss;
    }

    /// Decode the hex-string into an ID. The reverse of encodeId().
    inline std::uint64_t decodeId(const std::string_view str)
    {
        std::uint64_t id = 0;
        std::stringstream ss;
        ss << std::hex << str;
        ss >> id;
        return id;
    }

    bool windowingAvailable();

    /// Send a message to all clients.
    void alertAllUsers(const std::string& msg);

    /// Send a 'error:' message with the specified cmd and kind parameters to all connected
    /// clients. This function can be called either in coolwsd or coolkit processes, even if only
    /// coolwsd obviously has contact with the actual clients; in coolkit it will be forwarded to
    /// coolwsd for redistribution. (This function must be implemented separately in each program
    /// that uses it, it is not in Util.cpp.)
    void alertAllUsers(const std::string& cmd, const std::string& kind);

    /// Assert that a lock is already taken.
    template <typename T> void assertIsLocked([[maybe_unused]] const T& lock)
    {
#ifndef NDEBUG
        assert(lock.owns_lock());
#endif
    }

    inline void assertIsLocked([[maybe_unused]] std::mutex& mtx)
    {
#ifndef NDEBUG
        assert(!mtx.try_lock());
#endif
    }

    /// Print given number of bytes in human-understandable form (KB,MB, etc.)
    std::string getHumanizedBytes(unsigned long bytes);

    /// Returns the total physical memory (in kB) available in the system
    size_t getTotalSystemMemoryKb();

    /// Returns the numerical content of a file at @path
    std::size_t getFromFile(const char *path);

    /// Returns the cgroup's memory limit, or 0 if not available in bytes
    std::size_t getCGroupMemLimit();

    /// Returns the cgroup's soft memory limit, or 0 if not available in bytes
    std::size_t getCGroupMemSoftLimit();

    /// Replace substring @a in string @s with string @b.
    std::string replace(std::string s, std::string_view a, std::string_view b);

    /// Replace character @a in string @s, in place, with character @b.
    inline std::string& replaceInPlace(std::string& s, char a, char b)
    {
        for (std::size_t i = 0; i < s.size(); ++i)
        {
            if (s[i] == a)
                s[i] = b;
        }

        return s;
    }

    /// Replace character @a in string @s with character @b.
    inline std::string replace(const std::string_view s, char a, char b)
    {
        std::string res(s);
        replaceInPlace(res, a, b);
        return res;
    }

    /// Replace any characters in @a matching characters in @b with replacement chars in @c and return
    std::string replaceAllOf(std::string_view str, std::string_view match, std::string_view repl);

    void replaceAllSubStr(std::string& input, const std::string& target, const std::string& replacement);

    std::string formatLinesForLog(std::string_view s);

    /// Returns the COOL Version number string.
    std::string getCoolVersion();

    /// Returns the COOL Version Hash string.
    std::string getCoolVersionHash();

    /// Get version information, that is, both version and hash.
    void getVersionInfo(std::string& version, std::string& hash);

    ///< A random hex string that identifies the current process.
    const std::string& getProcessIdentifier();

    std::string getVersionJSON(bool enableExperimental, const std::string& timezone);

#if ENABLE_DEBUG
    /// Returns the offset of the first invalid-UTF8 character.
    /// Otherwise, returns > len for all-valid UTF8 characters.
    /// for debugging validation only.
    inline size_t isValidUtf8(const unsigned char *data, size_t len)
    {
        for (size_t i = 0; i < len; ++i)
        {
            if (data[i] < 0x80)
                continue;
            if (data[i] >> 6 != 0x3)
                return i;
            int chunkLen = 1;
            for (; data[i] & (1 << (7-chunkLen)); chunkLen++)
                if (chunkLen > 4)
                    return i;

            // Allow equality as the lower limit of the loop below is not zero.
            if (i + chunkLen > len)
                return i;

            for (; chunkLen > 1; --chunkLen)
                if (data[++i] >> 6 != 0x2)
                    return i;
        }
        return len + 1;
    }

    /// Returns the offset of the first invalid-UTF8 character.
    /// Otherwise, returns > len for all-valid UTF8 characters.
    /// for debugging validation only.
    inline size_t isValidUtf8(const std::string_view str)
    {
        return Util::isValidUtf8((unsigned char*)str.data(), str.size());
    }
#endif

    size_t findInVector(const std::vector<char>& tokens, const char *cstring, std::size_t offset = 0);

    /// Copy from @in to @out until @search is found.
    /// On a match return is true, the read position of @in will be at the
    /// start of @search and @out has a copy of @in appended as far as @search.
    /// On no match return is false, the read position of @in will be at eof
    /// and @out is appended to @in.
    bool copyToMatch(std::istream& in, std::ostream& out, std::string_view search);

    /// On a match return is true, the read position of @in will be at the
    /// start of @search.
    /// On no match return is false, the read position of @ will be at eof.
    bool seekToMatch(std::istream& in, std::string_view search);

    /// Trim trailing characters (on the right).
    inline std::string_view rtrim(const std::string_view s, const char ch)
    {
        const size_t last = s.find_last_not_of(ch);
        if (last != std::string::npos)
        {
            return s.substr(0, last + 1);
        }

        return std::string_view();
    }

    /// Trim spaces from both left and right. Just spaces.
    inline std::string& trim(std::string& s)
    {
        const size_t first = s.find_first_not_of(' ');
        const size_t last = s.find_last_not_of(' ');
        if (first != std::string::npos)
        {
            if (last != std::string::npos)
            {
                s = s.substr(first, last + 1 - first);
            }
            else
            {
                s = s.substr(first);
            }
        }
        else
        {
            if (last != std::string::npos)
            {
                s = s.substr(0, last + 1);
            }
            else
            {
                s.clear();
            }
        }

        return s;
    }

    /// Trim spaces from both left and right and copy. Just spaces.
    inline std::string_view trimmed(const std::string_view s)
    {
        const size_t first = s.find_first_not_of(' ');
        const size_t last = s.find_last_not_of(' ');
        if (first != std::string::npos)
        {
            if (last != std::string::npos)
            {
                return s.substr(first, last + 1 - first);
            }

            return s.substr(first);
        }

        if (last != std::string::npos)
        {
            return s.substr(0, last + 1);
        }

        return std::string_view();
    }

    /// Trim spaces from left and right. Just spaces.
    inline std::string trimmed(const std::string& s)
    {
        return std::string(trimmed(std::string_view(s)));
    }

    /// Trim spaces from left and right. Just spaces. FIXME: REMOVE!
    inline std::string trimmed(const char* s)
    {
        return trimmed(std::string(s));
    }

#if !HAVE_MEMRCHR

    inline void *memrchr(const void *s, int c, size_t n)
    {
        char *p = (char*)s + n - 1;
        while (p >= (char*)s)
        {
            if (*p == c)
                return p;
            p--;
        }
        return nullptr;
    }

#if 0

// Unit test for the above memrchr()

int main(int argc, char**argv)
{
  int success = 1;
  char *s;
  char *p;

#define TEST(s_,c,n,e) \
  s = s_; \
  printf("memrchr(\"%s\",'%c',%d)=",s,c,n); \
  p = memrchr(s, c, n); \
  if (p) \
    printf("\"%s\"", p); \
  else \
    printf("NULL"); \
  if (p == e) \
    printf(" OK\n"); \
  else \
    { \
      printf(" FAIL\n"); \
      success = 0; \
    }

  TEST("abc", 'x', 0, NULL);
  TEST("abc", 'x', 1, NULL);
  TEST("abc", 'x', 3, NULL);
  TEST("abc", 'a', 0, NULL);
  TEST("abc", 'a', 1, s);
  TEST("abc", 'a', 3, s);
  TEST("abc", 'b', 0, NULL);
  TEST("abc", 'b', 1, NULL);
  TEST("abc", 'b', 2, s+1);
  TEST("abc", 'b', 3, s+1);
  TEST("abc", 'c', 0, NULL);
  TEST("abc", 'c', 1, NULL);
  TEST("abc", 'c', 2, NULL);
  TEST("abc", 'c', 3, s+2);

  if (success)
    return 0;
  else
    return 1;
}

#endif

#endif

    inline size_t getLastDelimiterPosition(const char* message, const int length, const char delim)
    {
        if (message && length > 0)
        {
            const char *founddelim = static_cast<const char *>(memrchr(message, delim, length));
            const auto size = (founddelim == nullptr ? length : founddelim - message);
            return size;
        }

        return 0;
    }

    /// Return the symbolic name for an errno value, or in decimal if not handled here.
    inline std::string symbolicErrno(int e)
    {
        // LCOV_EXCL_START Coverage for these is not very useful.
        // Errnos from <asm-generic/errno-base.h> and <asm-generic/errno.h> on Linux.
        switch (e)
        {
        case EPERM: return "EPERM";
        case ENOENT: return "ENOENT";
        case ESRCH: return "ESRCH";
        case EINTR: return "EINTR";
        case EIO: return "EIO";
        case ENXIO: return "ENXIO";
        case E2BIG: return "E2BIG";
        case ENOEXEC: return "ENOEXEC";
        case EBADF: return "EBADF";
        case ECHILD: return "ECHILD";
        case EAGAIN: return "EAGAIN";
        case ENOMEM: return "ENOMEM";
        case EACCES: return "EACCES";
        case EFAULT: return "EFAULT";
#ifdef ENOTBLK
        case ENOTBLK: return "ENOTBLK";
#endif
        case EBUSY: return "EBUSY";
        case EEXIST: return "EEXIST";
        case EXDEV: return "EXDEV";
        case ENODEV: return "ENODEV";
        case ENOTDIR: return "ENOTDIR";
        case EISDIR: return "EISDIR";
        case EINVAL: return "EINVAL";
        case ENFILE: return "ENFILE";
        case EMFILE: return "EMFILE";
        case ENOTTY: return "ENOTTY";
        case ETXTBSY: return "ETXTBSY";
        case EFBIG: return "EFBIG";
        case ENOSPC: return "ENOSPC";
        case ESPIPE: return "ESPIPE";
        case EROFS: return "EROFS";
        case EMLINK: return "EMLINK";
        case EPIPE: return "EPIPE";
        case EDOM: return "EDOM";
        case ERANGE: return "ERANGE";
        case EDEADLK: return "EDEADLK";
        case ENAMETOOLONG: return "ENAMETOOLONG";
        case ENOLCK: return "ENOLCK";
        case ENOSYS: return "ENOSYS";
        case ENOTEMPTY: return "ENOTEMPTY";
        case ELOOP: return "ELOOP";
        case ENOMSG: return "ENOMSG";
        case EIDRM: return "EIDRM";
#ifdef ECHRNG
        case ECHRNG: return "ECHRNG";
#endif
#ifdef EL2NSYNC
        case EL2NSYNC: return "EL2NSYNC";
#endif
#ifdef EL3HLT
        case EL3HLT: return "EL3HLT";
#endif
#ifdef EL3RST
        case EL3RST: return "EL3RST";
#endif
#ifdef ELNRNG
        case ELNRNG: return "ELNRNG";
#endif
#ifdef EUNATCH
        case EUNATCH: return "EUNATCH";
#endif
#ifdef ENOCSI
        case ENOCSI: return "ENOCSI";
#endif
#ifdef EL2HLT
        case EL2HLT: return "EL2HLT";
#endif
#ifdef EBADE
        case EBADE: return "EBADE";
#endif
#ifdef EBADR
        case EBADR: return "EBADR";
#endif
#ifdef EXFULL
        case EXFULL: return "EXFULL";
#endif
#ifdef ENOANO
        case ENOANO: return "ENOANO";
#endif
#ifdef EBADRQC
        case EBADRQC: return "EBADRQC";
#endif
#ifdef EBADSLT
        case EBADSLT: return "EBADSLT";
#endif
#ifdef EBFONT
        case EBFONT: return "EBFONT";
#endif
        case ENOSTR: return "ENOSTR";
        case ENODATA: return "ENODATA";
        case ETIME: return "ETIME";
        case ENOSR: return "ENOSR";
#ifdef ENONET
        case ENONET: return "ENONET";
#endif
#ifdef ENOPKG
        case ENOPKG: return "ENOPKG";
#endif
#ifdef EREMOTE
        case EREMOTE: return "EREMOTE";
#endif
        case ENOLINK: return "ENOLINK";
#ifdef EADV
        case EADV: return "EADV";
#endif
#ifdef ESRMNT
        case ESRMNT: return "ESRMNT";
#endif
#ifdef ECOMM
        case ECOMM: return "ECOMM";
#endif
        case EPROTO: return "EPROTO";
#ifdef EMULTIHOP
        case EMULTIHOP: return "EMULTIHOP";
#endif
#ifdef EDOTDOT
        case EDOTDOT: return "EDOTDOT";
#endif
        case EBADMSG: return "EBADMSG";
        case EOVERFLOW: return "EOVERFLOW";
#ifdef ENOTUNIQ
        case ENOTUNIQ: return "ENOTUNIQ";
#endif
#ifdef EBADFD
        case EBADFD: return "EBADFD";
#endif
#ifdef EREMCHG
        case EREMCHG: return "EREMCHG";
#endif
#ifdef ELIBACC
        case ELIBACC: return "ELIBACC";
#endif
#ifdef ELIBBAD
        case ELIBBAD: return "ELIBBAD";
#endif
#ifdef ELIBSCN
        case ELIBSCN: return "ELIBSCN";
#endif
#ifdef ELIBMAX
        case ELIBMAX: return "ELIBMAX";
#endif
#ifdef ELIBEXEC
        case ELIBEXEC: return "ELIBEXEC";
#endif
        case EILSEQ: return "EILSEQ";
#ifdef ERESTART
        case ERESTART: return "ERESTART";
#endif
#ifdef ESTRPIPE
        case ESTRPIPE: return "ESTRPIPE";
#endif
#ifdef EUSERS
        case EUSERS: return "EUSERS";
#endif
        case ENOTSOCK: return "ENOTSOCK";
        case EDESTADDRREQ: return "EDESTADDRREQ";
        case EMSGSIZE: return "EMSGSIZE";
        case EPROTOTYPE: return "EPROTOTYPE";
        case ENOPROTOOPT: return "ENOPROTOOPT";
        case EPROTONOSUPPORT: return "EPROTONOSUPPORT";
#ifdef ESOCKTNOSUPPORT
        case ESOCKTNOSUPPORT: return "ESOCKTNOSUPPORT";
#endif
        case EOPNOTSUPP: return "EOPNOTSUPP";
#ifdef EPFNOSUPPORT
        case EPFNOSUPPORT: return "EPFNOSUPPORT";
#endif
        case EAFNOSUPPORT: return "EAFNOSUPPORT";
        case EADDRINUSE: return "EADDRINUSE";
        case EADDRNOTAVAIL: return "EADDRNOTAVAIL";
        case ENETDOWN: return "ENETDOWN";
        case ENETUNREACH: return "ENETUNREACH";
        case ENETRESET: return "ENETRESET";
        case ECONNABORTED: return "ECONNABORTED";
        case ECONNRESET: return "ECONNRESET";
        case ENOBUFS: return "ENOBUFS";
#ifdef EISCONN
        case EISCONN: return "EISCONN";
#endif
#ifdef ENOTCONN
        case ENOTCONN: return "ENOTCONN";
#endif
#ifdef ESHUTDOWN
        case ESHUTDOWN: return "ESHUTDOWN";
#endif
#ifdef ETOOMANYREFS
        case ETOOMANYREFS: return "ETOOMANYREFS";
#endif
        case ETIMEDOUT: return "ETIMEDOUT";
        case ECONNREFUSED: return "ECONNREFUSED";
#ifdef EHOSTDOWN
        case EHOSTDOWN: return "EHOSTDOWN";
#endif
        case EHOSTUNREACH: return "EHOSTUNREACH";
        case EALREADY: return "EALREADY";
        case EINPROGRESS: return "EINPROGRESS";
#ifdef ESTALE
        case ESTALE: return "ESTALE";
#endif
#ifdef EUCLEAN
        case EUCLEAN: return "EUCLEAN";
#endif
#ifdef ENOTNAM
        case ENOTNAM: return "ENOTNAM";
#endif
#ifdef ENAVAIL
        case ENAVAIL: return "ENAVAIL";
#endif
#ifdef EISNAM
        case EISNAM: return "EISNAM";
#endif
#ifdef EREMOTEIO
        case EREMOTEIO: return "EREMOTEIO";
#endif
#ifdef EDQUOT
        case EDQUOT: return "EDQUOT";
#endif
#ifdef ENOMEDIUM
        case ENOMEDIUM: return "ENOMEDIUM";
#endif
#ifdef EMEDIUMTYPE
        case EMEDIUMTYPE: return "EMEDIUMTYPE";
#endif
        case ECANCELED: return "ECANCELED";
#ifdef ENOKEY
        case ENOKEY: return "ENOKEY";
#endif
#ifdef EKEYEXPIRED
        case EKEYEXPIRED: return "EKEYEXPIRED";
#endif
#ifdef EKEYREVOKED
        case EKEYREVOKED: return "EKEYREVOKED";
#endif
#ifdef EKEYREJECTED
        case EKEYREJECTED: return "EKEYREJECTED";
#endif
        case EOWNERDEAD: return "EOWNERDEAD";
        case ENOTRECOVERABLE: return "ENOTRECOVERABLE";
#ifdef ERFKILL
        case ERFKILL: return "ERFKILL";
#endif
#ifdef EHWPOISON
        case EHWPOISON: return "EHWPOISON";
#endif
        default: return std::to_string(e);
        }
        // LCOV_EXCL_STOP Coverage for these is not very useful.
    }

    inline size_t getDelimiterPosition(const char* message, const int length, const char delim)
    {
        if (message && length > 0)
        {
            const char *founddelim = static_cast<const char *>(std::memchr(message, delim, length));
            const size_t size = (founddelim == nullptr ? length : founddelim - message);
            return size;
        }

        return 0;
    }

    /// Return the position of sub-array @sub in array @data, if found, -1 otherwise.
    inline int findSubArray(const char* data, const std::size_t dataLen, const char* sub,
                            const std::size_t subLen)
    {
        assert(subLen < std::numeric_limits<unsigned int>::max() &&
               "Invalid sub-array length to find");
        if (sub && subLen && dataLen >= subLen)
        {
            for (std::size_t i = 0; i < dataLen; ++i)
            {
                std::size_t j;
                for (j = 0; j < subLen && i + j < dataLen && data[i + j] == sub[j]; ++j)
                {
                }

                if (j >= subLen)
                    return i;
            }
        }

        return -1;
    }

    inline
    std::string getDelimitedInitialSubstring(const char *message, const int length, const char delim)
    {
        const size_t size = getDelimiterPosition(message, length, delim);
        return std::string(message, size);
    }

    /// Eliminates the prefix from str (if present) and returns a string view.
    inline std::string_view eliminatePrefix(const std::string_view str,
                                            const std::string_view prefix)
    {
        if (str.starts_with(prefix))
        {
            return str.substr(prefix.size());
        }

        // Return the original string as-is.
        return str;
    }

    /// Split a string in two at the delimiter, removing it.
    inline std::pair<std::string_view, std::string_view>
    split(const char* s, const int length, const char delimiter = ' ', bool removeDelim = true)
    {
        const size_t size = getDelimiterPosition(s, length, delimiter);

        std::string_view after;
        const int after_pos = size + (removeDelim ? 1 : 0);
        if (after_pos < length)
            after = std::string_view(s + after_pos, length - after_pos);

        return std::make_pair(std::string_view(s, size), after);
    }

    /// Split a string in two at the delimiter, removing it.
    inline std::pair<std::string, std::string>
    split(const std::string&& str, const char delimiter = ' ', bool removeDelim = true)
    {
        const auto& pair = split(str.data(), str.size(), delimiter, removeDelim);
        return std::make_pair(std::string(pair.first), std::string(pair.second));
    }

    /// Split a string in two at the delimiter, removing it.
    inline std::pair<std::string_view, std::string_view>
    split(const std::string& s, const char delimiter = ' ', bool removeDelim = true)
    {
        return split(s.c_str(), s.size(), delimiter, removeDelim);
    }

    /// Split a string in two at the delimiter.
    inline
    std::pair<std::string, std::string> splitLast(const char* s, const int length, const char delimiter = ' ', bool removeDelim = true)
    {
        const size_t size = getLastDelimiterPosition(s, length, delimiter);

        std::string after;
        int after_pos = size + (removeDelim? 1: 0);
        if (after_pos < length)
            after = std::string(s + after_pos, length - after_pos);

        return std::make_pair(std::string(s, size), after);
    }

    /// Split a string in two at the delimiter, removing it.
    inline
    std::pair<std::string, std::string> splitLast(const std::string& s, const char delimiter = ' ', bool removeDelim = true)
    {
        return splitLast(s.c_str(), s.size(), delimiter, removeDelim);
    }

    /// Append a length bytes to a vector, or strlen of data as a C string if not provided
    /// returns count of bytes appended.
    inline void vectorAppend(std::vector<char> &vector, const char *data, ssize_t length = -1)
    {
        size_t vlen = vector.size();

        if (!data)
        {
            return;
        }

        size_t dataLen = length;
        if (length < 0)
            dataLen = strlen(data);
        vector.resize(vlen+dataLen);
        std::memcpy(vector.data() + vlen, data, dataLen);
    }

    inline void vectorAppend(std::vector<char> &vector, const std::string &str)
    {
        vectorAppend(vector, str.c_str(), str.length());
    }

    /// Splits a URL into path (with protocol), filename, extension, parameters.
    /// All components are optional, depending on what the URL represents (can be a unix path).
    std::tuple<std::string, std::string, std::string, std::string> splitUrl(const std::string& url);

    /// Remove all but scheme://hostname:port/ from a URI.
    std::string trimURI(const std::string& uri);

    /// Cleanup a filename replacing anything potentially problematic
    /// either for a URL or for a file path
    std::string cleanupFilename(const std::string &filename);

    //// Return current time in HTTP format.
    std::string getHttpTimeNow();

    std::string getTimeNow(const char* format);

    //// Return time in HTTP format.
    std::string getHttpTime(std::chrono::system_clock::time_point time);

    //// Return time in ISO8061 fraction format
    std::string getIso8601FracformatTime(std::chrono::system_clock::time_point time);

    /// Convert a time_point to iso8601 formatted string.
    std::string time_point_to_iso8601(std::chrono::system_clock::time_point tp);

    /// Convert time from ISO8061 fraction format
    std::chrono::system_clock::time_point iso8601ToTimestamp(const std::string& iso8601Time, const std::string& logName);

    /// A null-converter between two identical clocks.
    template <typename Dst, typename Src, typename std::is_same<Src, Dst>::type>
    Dst convertChronoClock(const Src time)
    {
        return std::chrono::time_point_cast<Dst>(time);
    }

    /// Converter between two different clocks,
    /// such as system_clock and stead_clock.
    /// Note: by nature this has limited accuracy due to the latency
    /// between reading the Src and Dst clocks (typically a few nanos).
    template <typename Dst, typename Src, typename Enable = void>
    Dst convertChronoClock(const Src time)
    {
        [[maybe_unused]] const auto prime = Dst::clock::now();
        const auto before = Src::clock::now();
        const auto now = Dst::clock::now();
        const auto after = Src::clock::now();
        const auto diff = after - before;
        const auto correction = before + (diff / 2);
        return std::chrono::time_point_cast<typename Dst::duration>(now + (time - correction));
    }

    /// Converts from system_clock to string for debugging / tracing.
    /// Format (local time): Thu Jan 27 03:45:27.123 2022
    std::string getSystemClockAsString(std::chrono::system_clock::time_point time);

    /// conversion from steady_clock for debugging / tracing
    /// Format (local time): Thu Jan 27 03:45:27.123 2022
    inline std::string getSteadyClockAsString(const std::chrono::steady_clock::time_point time)
    {
        return getSystemClockAsString(
            convertChronoClock<std::chrono::system_clock::time_point>(time));
    }

    /// See getSystemClockAsString.
    inline std::string getClockAsString(const std::chrono::system_clock::time_point time)
    {
        return getSystemClockAsString(time);
    }

    /// See getSteadyClockAsString.
    inline std::string getClockAsString(const std::chrono::steady_clock::time_point time)
    {
        return getSteadyClockAsString(time);
    }

    /// Stringify the given time and print the difference from 'now' in
    /// a human-friendly format. E.g. Thu Oct 09 02:15:25.682 2025 (4h 43m 32s 211ms ago)
    template <typename U, typename T> std::string getTimeForLog(const U& now, const T& time)
    {
        const auto timeU = convertChronoClock<U>(time);
        // The above conversion will cause minor delays, so there will be
        // a difference even when now == time. For that, we ignore anything sub-micorsecond.
        const bool past = std::chrono::round<std::chrono::milliseconds>(now - timeU) >=
                          std::chrono::milliseconds::zero();
        const auto elapsed =
            std::chrono::round<std::chrono::milliseconds>(past ? now - timeU : timeU - now);

        const auto elapsedH = std::chrono::duration_cast<std::chrono::hours>(elapsed);
        const auto elapsedMin =
            std::chrono::duration_cast<std::chrono::minutes>(elapsed - elapsedH);
        const auto elapsedSec =
            std::chrono::duration_cast<std::chrono::seconds>(elapsed - elapsedH - elapsedMin);
        const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            elapsed - elapsedH - elapsedMin - elapsedSec);

        assert(elapsedH + elapsedMin + elapsedSec + elapsedMs ==
                   std::chrono::duration_cast<std::chrono::milliseconds>(elapsed) &&
               "Time-difference mismatch, likely a rounding error");

        std::stringstream ss;
        ss << getClockAsString(timeU) << " (";

        // Don't stringify 0 units, except for ms.
        if (elapsedH != std::chrono::hours::zero())
            ss << elapsedH << ' ';

        if (elapsedMin != std::chrono::minutes::zero())
            ss << elapsedMin << ' ';

        if (elapsedSec != std::chrono::seconds::zero())
            ss << elapsedSec << ' ';

        ss << std::setprecision(3) << elapsedMs;
        ss << (past ? " ago)" : " later)");
        return ss.str();
    }

    /// Converts a unix-epoch time to steady_clock.
    /// Without timezone, this can be unreliable.
    inline std::chrono::steady_clock::time_point getSteadyClockFromEpoch(uint64_t epoch)
    {
        // Sun Sep 09 2001 01:46:40 GMT+0000 x 1000 (i.e. in milliseconds).
        if (epoch > 1'000'000'000'000)
        {
            epoch /= 1000; // Convert to seconds.
        }

        const auto sys = std::chrono::system_clock::from_time_t(epoch);
        return Util::convertChronoClock<std::chrono::steady_clock::time_point>(sys);
    }

    /**
     * Avoid using the configuration layer and rely on defaults which is only useful for special
     * test tool targets (typically fuzzing) where start-up speed is critical.
     */
    constexpr bool isFuzzing()
    {
#if LIBFUZZER
        return true;
#else
        return false;
#endif
    }

    constexpr bool isMobileApp()
    {
#ifdef MOBILEAPP
        return MOBILEAPP;
#else
        return false;
#endif
    }

    constexpr bool isDebugEnabled()
    {
#ifdef ENABLE_DEBUG
        return ENABLE_DEBUG;
#else
        return false;
#endif
    }

    constexpr bool isKitInProcess()
    {
#if defined(ENABLE_INPROC) && ENABLE_INPROC
        return true;
#else
        return isFuzzing() || isMobileApp();
#endif // ENABLE_INPROC
    }

    /**
     * Splits string into vector<string>. Does not accept referenced variables for easy
     * usage like (splitString("test", ..)) or (splitString(getStringOnTheFly(), ..))
     */
     //FIXME: merge with StringVector.
    inline std::vector<std::string> splitStringToVector(const std::string& str, const char delim)
    {
        size_t start;
        size_t end = 0;

        std::vector<std::string> result;

        while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
        {
            end = str.find(delim, start);
            result.emplace_back(str.substr(start, end - start));
        }
        return result;
    }

    void setApplicationPath(const std::string& path);
    std::string getApplicationPath();

    /**
     * Converts vector of strings to map. Strings should have formed like this: key + delimiter + value.
     * In case of a misformed string or zero length vector, passes that item and warns the developer.
     */
    std::map<std::string, std::string> stringVectorToMap(const std::vector<std::string>& strvector, char delimiter);

    // If OS is not mobile, it must be Linux.
    std::string getLinuxVersion();

    /// Converts in-place and returns the argument to lower-case.
    inline std::string& toLowerInplace(std::string& s)
    {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }
    /// Converts and returns a copy of the argument in lower-case.
    inline std::string toLower(std::string s) { return toLowerInplace(s); }

    /// Case insensitive comparison of two strings.
    /// Returns true iff the two strings are equal, regardless of case.
    inline bool iequal(const char* lhs, std::size_t lhs_len, const char* rhs, std::size_t rhs_len)
    {
        return ((lhs_len == rhs_len)
                && std::equal(lhs, lhs + lhs_len, rhs, [](const char lch, const char rch) {
                       return std::tolower(lch) == std::tolower(rch);
                   }));
    }

    /// Case insensitive comparison of two strings.
    template <std::size_t N> inline bool iequal(const std::string_view lhs, const char (&rhs)[N])
    {
        return iequal(lhs.data(), lhs.size(), rhs, N - 1); // Minus null termination.
    }

    /// Case insensitive comparison of two strings.
    inline bool iequal(const std::string_view lhs, const std::string_view rhs)
    {
        return iequal(lhs.data(), lhs.size(), rhs.data(), rhs.size());
    }

    /// Compare two containers (of the same type) for equality.
    template <typename Container> inline bool equal(const Container& c1, const Container& c2)
    {
        if (c1.size() == c2.size())
        {
            auto it1 = c1.begin();
            auto it2 = c2.begin();
            for (; it1 != c1.end() && it2 != c2.end(); ++it1, ++it2)
            {
                if (*it1 != *it2)
                    return false;
            }

            return true;
        }

        return false;
    }

    /// Convert a vector to a string. Useful for conversion in templates.
    template <typename T> inline std::string toString(const T& x)
    {
        std::ostringstream oss;
        oss << x;
        return oss.str();
    }

    /// Convert a vector to a string. Useful for conversion in templates.
    inline std::string toString(const std::vector<char>& x)
    {
        return std::string(x.data(), x.size());
    }

    /// No-op string conversion. Useful for conversion in templates.
    inline std::string toString(const std::string& s) { return s; }

    /// Create a string from a literal. Useful for conversion in templates.
    template <std::size_t N> inline std::string toString(const char (&s)[N])
    {
        return std::string(s);
    }

    // Create a ostringstream with desired ostream format set
    inline std::ostringstream makeDumpStateStream()
    {
        std::ostringstream os;
        os.setf(std::ios_base::boolalpha);
        return os;
    }

    /// Dump an object that supports .dumpState into a string.
    /// Helpful for logging.
    template <typename T> std::string dump(const T& object, const std::string& indent = ", ")
    {
        std::ostringstream oss(Util::makeDumpStateStream());
        object.dumpState(oss, indent);
        return oss.str().substr(indent.size());
    }

    /// Stringify elements from a container of pairs with a delimiter to a stream.
    template <std::ranges::forward_range T, typename... Delimiters>
    void joinPair(std::ostream& stream, T&& container, Delimiters&&... delimiters)
    {
        unsigned i = 0;
        for (const auto& pair : container)
        {
            if (i++)
            {
                (stream << ... << delimiters);
            }

            stream << pair;
        }
    }

    /// Stringify elements from a container of pairs with a delimiter to string.
    template <std::ranges::forward_range T, typename... Delimiters>
    std::string joinPair(T&& container, Delimiters&&... delimiters)
    {
        std::ostringstream oss;
        joinPair(oss, std::forward<T>(container), std::forward<Delimiters>(delimiters)...);
        return oss.str();
    }

    /// Stringify elements from a container of pairs with a delimiter to string.
    template <std::ranges::forward_range T> std::string joinPair(T&& container)
    {
        std::ostringstream oss;
        joinPair(oss, std::forward<T>(container), " / ");
        return oss.str();
    }

    /// Sleep based on count of seconds in env. var
    void sleepFromEnvIfSet(const char *domain, const char *envVar);

    /// Close logs and forcefully exit with the given exit code.
    /// This calls std::_Exit, which terminates the program without cleaning up
    /// static instances (i.e. anything registered with `atexit' or `on_exit').
    // coverity[+kill]
    void forcedExit(int code) __attribute__ ((__noreturn__));

    /// Returns the result of malloc_info, which is an XML string with all the arenas.
    std::string getMallocInfo();

    /// Call malloc_trim or alternative allocator equivalent
    void trimMalloc();

    // std::size isn't available on our android baseline so use this
    // solution as a workaround
    template <typename T, size_t S> char (&n_array_size( T(&)[S] ))[S];

#define N_ELEMENTS(arr)     (sizeof(Util::n_array_size(arr)))

    // Wrap gmtime_r() which is not portable
    std::tm *time_t_to_gmtime(std::time_t t, std::tm& tm);

    /// Base-64 encode the given input
    std::string base64Encode(std::string_view input);
    inline std::string base64Encode(const std::vector<unsigned char>& input)
    {
        return base64Encode(
            std::string_view(reinterpret_cast<const char*>(input.data()), input.size()));
    }

    /// Base-64 decode the given input.
    std::string base64Decode(const std::string& input);

#ifdef _WIN32
    std::wstring string_to_wide_string(const std::string& string);
    std::string wide_string_to_string(const std::wstring& wide_string);
#endif
} // end namespace Util

inline std::ostream& operator<<(std::ostream& os, const std::chrono::system_clock::time_point& ts)
{
    os << Util::getIso8601FracformatTime(ts);
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Poco::Net::HTTPRequest& request)
{
    os << request.getMethod() << ' ' << request.getVersion() << ' ' << request.getURI()
       << ", content-length: " << request.getContentLength64()
       << ", chunked: " << request.getChunkedTransferEncoding() << ", ";
    Util::joinPair(os, request, " / ");
    return os;
}

// std::to_underlying will be available in C++23
template <typename Enum> constexpr std::underlying_type_t<Enum> to_underlying(Enum e)
{
   return static_cast<std::underlying_type_t<Enum>>(e);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
