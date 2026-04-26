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

/*
 * General utility functions and helpers.
 * Functions: String manipulation, encoding, process management, system info
 */

#include <config.h>
#include <config_version.h>

#include <common/Common.hpp>
#include <common/Log.hpp>
#include <common/NumUtil.hpp>
#include <common/Protocol.hpp>
#include <common/Rectangle.hpp>
#include <common/SigUtil.hpp>
#include <common/TraceEvent.hpp>
#include <common/Util.hpp>
#include <common/base64.hpp>

#include <Poco/HexBinaryEncoder.h>
#include <Poco/URI.h>
#include <Poco/Util/Application.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>

#ifndef COOLWSD_BUILDCONFIG
#define COOLWSD_BUILDCONFIG
#endif

#if !MOBILEAPP
#include <common/SigHandlerTrap.hpp>
#endif

#if defined(__GLIBC__)
#  include <execinfo.h>
#  include <cxxabi.h>
#endif
#ifdef __linux__
#  include <sys/prctl.h>
#  include <sys/syscall.h>
#  include <sys/vfs.h>
#  include <sys/resource.h>
#  include <dlfcn.h>
#elif defined __FreeBSD__
#  include <sys/resource.h>
#  include <sys/thr.h>
#elif defined IOS
#import <Foundation/Foundation.h>
#endif

#ifndef _WIN32
#include <sys/uio.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <spawn.h>
#endif

#if defined __GLIBC__
#include <malloc.h>
#if defined(M_TRIM_THRESHOLD)
#include <dlfcn.h>
#endif
#endif

#if defined __EMSCRIPTEN__
#include <emscripten/console.h>
#endif

#ifdef _WIN32
#include <processthreadsapi.h>
#endif

// for version info
#include <Poco/Version.h>
#if ENABLE_SSL
#include <openssl/opensslv.h>
#endif
#include <zstd.h>
#define PNG_VERSION_INFO_ONLY
#include <png.h>
#undef PNG_VERSION_INFO_ONLY


namespace Util
{
    namespace rng
    {
        static std::mutex _rngMutex;

        // Create the prng with a random-device for seed.
        // If we don't have a hardware random-device, we will get the same seed.
        // In that case we are better off with an arbitrary, but changing, seed.
        static std::mt19937_64 _rng = std::mt19937_64(rng::getSeed());

        uint_fast64_t getSeed()
        {
            std::vector<char> hardRandom = getBytes(16);
            uint_fast64_t seed = *reinterpret_cast<uint_fast64_t *>(hardRandom.data());
            return seed;
        }

        // A new seed is used to shuffle the sequence.
        // N.B. Always reseed after getting forked!
        void reseed()
        {
            std::unique_lock<std::mutex> lock(_rngMutex);
            _rng.seed(rng::getSeed());
        }

        void seedForTesting(uint_fast64_t seed)
        {
            std::unique_lock<std::mutex> lock(_rngMutex);
            _rng.seed(seed);
        }

        // Returns a new random number.
        unsigned getNext()
        {
            std::unique_lock<std::mutex> lock(_rngMutex);
            return _rng();
        }

        /// Generate a string of random characters.
        std::string getHexString(const std::size_t length)
        {
            std::stringstream ss;
            Poco::HexBinaryEncoder hex(ss);
            hex.rdbuf()->setLineLength(0); // Don't insert line breaks.
            hex.write(getBytes(length).data(), length);
            hex.close(); // Flush.
            return ss.str().substr(0, length);
        }

        /// Generates a random string in Base64.
        /// Note: May contain '/' characters.
        std::string getB64String(const std::size_t length)
        {
            auto bytes = getBytes(length);
            return macaron::Base64::Encode(
                std::string_view(bytes.data(), length)).substr(0, length);
        }

        std::string getFilename(const std::size_t length)
        {
            std::string s = getB64String(length * 2);
            s.erase(std::remove_if(s.begin(), s.end(),
                                   [](const std::string::value_type& c)
                                   {
                                       // Remove undesirable characters in a filename.
                                       return c == '/' || c == ' ' || c == '+';
                                   }),
                     s.end());
            return s.substr(0, length);
        }
    } // namespace rng

    bool windowingAvailable()
    {
        return std::getenv("DISPLAY") != nullptr;
    }

    std::string replace(std::string result, const std::string_view from, const std::string_view to)
    {
        const std::size_t fromSize = from.size();
        if (fromSize > 0)
        {
            const std::size_t toSize = to.size();
            std::string::size_type pos = 0;
            while ((pos = result.find(from, pos)) != std::string::npos)
            {
                result.replace(pos, fromSize, to);
                pos += toSize; // Skip the replace to avoid endless recursion.
            }
        }

        return result;
    }

    std::string replaceAllOf(std::string_view str, std::string_view match, std::string_view repl)
    {
        std::ostringstream os;

        assert(match.size() == repl.size());
        if (match.size() != repl.size())
            return std::string("replaceAllOf failed");

        const std::size_t strSize = str.size();
        for (size_t i = 0; i < strSize; ++i)
        {
            auto pos = match.find(str[i]);
            if (pos != std::string::npos)
                os << repl[pos];
            else
                os << str[i];
        }

        return os.str();
    }

    void replaceAllSubStr(std::string& input, const std::string& target, const std::string& replacement)
    {
        if (target.empty())
            return;

        std::size_t pos = 0;
        while ((pos = input.find(target, pos)) != std::string::npos)
        {
            input.replace(pos, target.length(), replacement);
            pos += replacement.length();
        }
    }

    std::string cleanupFilename(const std::string &filename)
    {
        constexpr std::string_view mtch(",/?:@&=+$#'\"");
        constexpr std::string_view repl("------------");
        return replaceAllOf(filename, mtch, repl);
    }

    std::string formatLinesForLog(const std::string_view s)
    {
        std::string r;
        std::string_view::size_type n = s.size();
        if (n > 0 && s.back() == '\n')
            r = s.substr(0, n-1);
        else
            r = s;
        return replace(std::move(r), "\n", " / ");
    }

    std::string getCoolVersion() { return std::string(COOLWSD_VERSION); }

    std::string getCoolVersionHash() { return std::string(COOLWSD_VERSION_HASH); }

    void getVersionInfo(std::string& version, std::string& hash)
    {
        version = getCoolVersion();
        hash = getCoolVersionHash();
    }

    const std::string& getProcessIdentifier()
    {
        static std::string id = Util::rng::getHexString(8);

        return id;
    }

    std::string getVersionJSON(bool enableExperimental, const std::string& timezone)
    {
        std::string version, hash;
        Util::getVersionInfo(version, hash);

        std::string pocoVersion;
        pocoVersion += std::to_string((POCO_VERSION & 0xff000000) >> 24) + ".";
        pocoVersion += std::to_string((POCO_VERSION & 0x00ff0000) >> 16) + ".";
        pocoVersion += std::to_string((POCO_VERSION & 0x0000ff00) >> 8);
        std::string zstdVersion;
        zstdVersion += std::to_string(ZSTD_VERSION_MAJOR) + ".";
        zstdVersion += std::to_string(ZSTD_VERSION_MINOR) + ".";
        zstdVersion += std::to_string(ZSTD_VERSION_RELEASE);

        std::string json = R"({ "Version":     ")" + version +
                           "\", "
                           "\"Hash\":        \"" +
                           hash +
                           "\", "
                           "\"BuildConfig\": \"" +
                           std::string(COOLWSD_BUILDCONFIG) +
                           "\", "
                           "\"PocoVersion\": \"" +
                           pocoVersion +
                           "\", "
#if ENABLE_SSL
                           "\"OpenSSLVersion\": \"" +
                           std::string(OPENSSL_VERSION_STR) +
                           "\", "
#endif
                           "\"ZstdVersion\": \"" +
                           zstdVersion +
                           "\", "
                           "\"LibPngVersion\": \"" +
                           std::string(PNG_LIBPNG_VER_STRING) +
                           "\", "
                           "\"Protocol\":    \"" +
                           COOLProtocol::GetProtocolVersion() +
                           "\", "
                           "\"Id\":          \"" +
                           Util::getProcessIdentifier() + "\", ";

        if (!timezone.empty())
            json += R"("TimeZone":     ")" + timezone + "\", ";

        json += R"("Options":     ")" + std::string(enableExperimental ? " (E)" : "") + "\" }";
        return json;
    }

    std::string trimURI(const std::string &uriStr)
    {
        Poco::URI uri(uriStr);
        uri.setUserInfo("");
        uri.setPath("");
        uri.setQuery("");
        uri.setFragment("");
        return uri.toString();
    }

    /// Split a string in two at the delimiter and give the delimiter to the first.
    static
    std::pair<std::string, std::string> splitLast2(const std::string& str, const char delimiter = ' ')
    {
        if (!str.empty())
        {
            const char* s = str.c_str();
            const int length = str.size();
            const int pos = getLastDelimiterPosition(s, length, delimiter);
            if (pos < length)
                return std::make_pair(std::string(s, pos + 1), std::string(s + pos + 1));
        }

        // Not found; return in first.
        return std::make_pair(str, std::string());
    }

    std::tuple<std::string, std::string, std::string, std::string> splitUrl(const std::string& url)
    {
        // In case we have a URL that has parameters.
        std::string base;
        std::string params;
        std::tie(base, params) = Util::split(url, '?', false);

        std::string filename;
        std::tie(base, filename) = Util::splitLast2(base, '/');
        if (filename.empty())
        {
            // If no '/', then it's only filename.
            std::swap(base, filename);
        }

        std::string ext;
        std::tie(filename, ext) = Util::splitLast(filename, '.', false);

        return std::make_tuple(base, filename, ext, params);
    }

    std::string getTimeNow(const char* format)
    {
        char time_now[64];
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm;
        time_t_to_gmtime(now_c, now_tm);
        strftime(time_now, sizeof(time_now), format, &now_tm);

        return time_now;
    }

    std::string getHttpTimeNow()
    {
        return getTimeNow("%a, %d %b %Y %T");
    }

    std::string getHttpTime(std::chrono::system_clock::time_point time)
    {
        char http_time[64];
        std::time_t time_c = std::chrono::system_clock::to_time_t(time);
        std::tm time_tm;
        time_t_to_gmtime(time_c, time_tm);
        strftime(http_time, sizeof(http_time), "%a, %d %b %Y %T", &time_tm);

        return http_time;
    }

    std::size_t findInVector(const std::vector<char>& tokens, const char *cstring, std::size_t offset)
    {
        assert(cstring);
        for (std::size_t i = 0; i < tokens.size() - offset; ++i)
        {
            std::size_t j;
            for (j = 0; i + j < tokens.size() - offset && cstring[j] != '\0' && tokens[i + j + offset] == cstring[j]; ++j)
                ;
            if (cstring[j] == '\0')
                return i + offset;
        }
        return std::string::npos;
    }

    namespace
    {

    // For copyToMatch/seekToMatch
    bool processToMatch(std::istream& in, std::ostream* out, std::string_view search)
    {
        const size_t searchLen = search.length();
        assert(searchLen && "need to search for something");

        const std::streamsize overlap = searchLen - 1;
        std::streamsize carrySize = 0;

        std::vector<char> scratch(READ_BUFFER_SIZE + overlap);
        char* buffer = scratch.data();

        // Read READ_BUFFER_SIZE at a time, keep enough from last iteration to
        // match 'search' against what existed at the end of the last window (but
        // was too short to match) that might match now at the start of this
        // new window.
        while (in)
        {
            in.read(buffer + carrySize, READ_BUFFER_SIZE);
            std::streamsize bytesRead = in.gcount();
            if (!bytesRead)
                break;

            std::streamsize bytesInBuffer = carrySize + bytesRead;

            std::string_view view(buffer, bytesInBuffer);
            const auto match = view.find(search);

            if (match != std::string_view::npos)
            {
                // Copy as far as match
                if (out)
                    out->write(buffer, match);
                // Seek back to before match and return
                in.clear();
                in.seekg(-static_cast<std::streamoff>(bytesInBuffer - match), std::ios_base::cur);
                return true;
            }
            else
            {
                if (out)
                {
                    // Copy what definitely doesn't match so far to output.
                    std::streamsize bytesToWrite = bytesInBuffer > overlap ? bytesInBuffer - overlap : 0;
                    out->write(buffer, bytesToWrite);
                }
                // Rotate <= overlap to start of buffer for next iteration
                carrySize = std::min(overlap, bytesInBuffer);
                std::memmove(buffer, buffer + bytesInBuffer - carrySize, carrySize);
            }
        }

        // write left over
        if (carrySize > 0 && out)
            out->write(buffer, carrySize);
        return false;
    }

    } // namespace

    bool seekToMatch(std::istream& in, std::string_view search)
    {
        return processToMatch(in, nullptr, search);
    }

    bool copyToMatch(std::istream& in, std::ostream& out, std::string_view search)
    {
        return processToMatch(in, &out, search);
    }

    std::string getIso8601FracformatTime(std::chrono::system_clock::time_point time){
        char time_modified[64];
        std::time_t lastModified_us_t = std::chrono::system_clock::to_time_t(time);
        std::tm lastModified_tm;
        time_t_to_gmtime(lastModified_us_t, lastModified_tm);
        strftime(time_modified, sizeof(time_modified), "%FT%T.", &lastModified_tm);

        auto lastModified_s = std::chrono::time_point_cast<std::chrono::seconds>(time);

        std::ostringstream oss;
        oss << std::setfill('0')
            << time_modified
            << std::setw(6)
            << (time - lastModified_s).count() / (std::chrono::system_clock::period::den / std::chrono::system_clock::period::num / 1000000)
            << 'Z';

        return oss.str();
    }

#if !MOBILEAPP
    // These are used in test/WhiteBoxTests.cpp and thus not needed in a mobile app.

    std::string time_point_to_iso8601(std::chrono::system_clock::time_point tp)
    {
        const std::time_t tt = std::chrono::system_clock::to_time_t(tp);
        std::tm tm;
        time_t_to_gmtime(tt, tm);

        std::ostringstream oss;
        oss << tm.tm_year + 1900 << '-' << std::setfill('0') << std::setw(2) << tm.tm_mon + 1 << '-'
            << std::setfill('0') << std::setw(2) << tm.tm_mday << 'T';
        oss << std::setfill('0') << std::setw(2) << tm.tm_hour << ':';
        oss << std::setfill('0') << std::setw(2) << tm.tm_min << ':';
        const std::chrono::duration<double> sec
            = tp - std::chrono::system_clock::from_time_t(tt) + std::chrono::seconds(tm.tm_sec);
        if (sec.count() < 10)
            oss << '0';
        oss << std::fixed << sec.count() << 'Z';

        return oss.str();
    }

    std::chrono::system_clock::time_point iso8601ToTimestamp(const std::string& iso8601Time,
                                                             const std::string& logName)
    {
        std::chrono::system_clock::time_point timestamp;
        std::tm tm;
        const char* cstr = iso8601Time.c_str();
        const char* trailing;
        if (!(trailing = strptime(cstr, "%Y-%m-%dT%H:%M:%S", &tm)))
        {
            LOG_WRN(logName << " [" << iso8601Time << "] is in invalid format."
                            << "Returning " << timestamp.time_since_epoch().count());
            return timestamp;
        }

        timestamp += std::chrono::seconds(timegm(&tm));
        if (trailing[0] == '\0')
            return timestamp;

        if (trailing[0] != '.')
        {
            LOG_WRN(logName << " [" << iso8601Time << "] is in invalid format."
                            << ". Returning " << timestamp.time_since_epoch().count());
            return timestamp;
        }

        char* end = nullptr;
        const std::size_t us = strtoul(trailing + 1, &end, 10); // Skip the '.' and read as integer.

        std::size_t denominator = 1;
        for (const char* i = trailing + 1; i != end; i++)
        {
            denominator *= 10;
        }

        const std::size_t seconds_us = us * std::chrono::system_clock::period::den
                                       / std::chrono::system_clock::period::num / denominator;

        timestamp += std::chrono::system_clock::duration(seconds_us);

        return timestamp;
    }

#endif // !MOBILEAPP

    /// Returns the given system_clock time_point as string in GMT.
    /// Format: Thu Jan 27 03:45:27.123 2022
    std::string getSystemClockAsString(const std::chrono::system_clock::time_point time)
    {
        const auto ms = std::chrono::round<std::chrono::milliseconds>(time);
        const std::time_t t = std::chrono::system_clock::to_time_t(ms);
        // Round to nearest millisecond, rather than truncating with duration_cast().
        const auto msSinceEpoch =
            std::chrono::round<std::chrono::milliseconds>(time.time_since_epoch());
        const int msFraction = msSinceEpoch.count() % 1000;

        std::tm tm;
        time_t_to_gmtime(t, tm);

        char buffer[128] = { 0 };
        std::strftime(buffer, 80, "%a %b %d %H:%M:%S", &tm);
        std::stringstream ss;
        ss << buffer << '.' << std::setfill('0') << std::setw(3) << msFraction << ' '
           << tm.tm_year + 1900;
        return ss.str();
    }

    std::map<std::string, std::string> stringVectorToMap(const std::vector<std::string>& strvector, const char delimiter)
    {
        std::map<std::string, std::string> result;

        for (auto it = strvector.begin(); it != strvector.end(); ++it)
        {
            std::size_t delimiterPosition = 0;
            delimiterPosition = (*it).find(delimiter, 0);
            if (delimiterPosition != std::string::npos)
            {
                std::string key = (*it).substr(0, delimiterPosition);
                delimiterPosition++;
                result[key] = (*it).substr(delimiterPosition);
            }
            else
            {
                LOG_WRN("Util::stringVectorToMap => record is misformed: " << (*it));
            }
        }

        return result;
    }

    static std::string ApplicationPath;
    void setApplicationPath(const std::string& path)
    {
        ApplicationPath = Poco::Path(path).absolute().toString();
    }

    std::string getApplicationPath()
    {
        return ApplicationPath;
    }

    void forcedExit(int code)
    {
        if (code)
            LOG_FTL("Forced Exit with code: " << code);
        else
            LOG_INF("Forced Exit with code: " << code);

#if !MOBILEAPP
        /// Wait for the signal handler, if any,
        /// and prevent _Exit while collecting backtrace.
        SigUtil::SigHandlerTrap::wait();
#endif

        Log::shutdown();

#if CODE_COVERAGE
        __gcov_dump();
#endif

        std::_Exit(code);
    }

    std::string getMallocInfo()
    {
        std::string info;

#if defined __GLIBC__
        size_t size = 0;
        char* p = nullptr;
        FILE* f = open_memstream(&p, &size);
        if (f)
        {
            // Dump malloc internal structures.
            malloc_info(0, f);
            fclose(f);

            if (size)
                info = std::string(p, size);

            free(p);
        }
#endif // __GLIBC__

        return info;
    }

    void trimMalloc()
    {
#if defined(M_TRIM_THRESHOLD)
        // If platform supports glibc's malloc_trim, then attempt tcmalloc
        // equivalents if that's in use as an alternative.
        static auto releaseFreeMemory = [] {
            auto symbol = reinterpret_cast<void(*)(void)>(dlsym(RTLD_NEXT, "MallocExtension_ReleaseFreeMemory"));
            LOG_INF("Allocator is: " << (symbol ? "tcmalloc" : "glibc"));
            return symbol;
        }();

        if (releaseFreeMemory)
            releaseFreeMemory();
        else
            malloc_trim(0);
#endif
    }

    void sleepFromEnvIfSet(const char *domain, const char *envVar)
    {
        const char *value;
        if ((value = std::getenv(envVar)))
        {
            const size_t delaySecs = std::stoul(value);
            if (delaySecs > 0)
            {
                std::cerr << domain << ": Sleeping " << delaySecs
                          << " seconds to give you time to attach debugger to process "
                          << ProcUtil::getProcessId() << std::endl
                          << "sudo gdb --pid=" << ProcUtil::getProcessId() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(delaySecs));
            }
        }
    }

    Rectangle::Rectangle(const std::string &rectangle)
    {
        StringVector tokens(StringVector::tokenize(rectangle, ','));
        if (tokens.size() == 4)
        {
            _x1 = NumUtil::stoi(tokens[0]);
            _y1 = NumUtil::stoi(tokens[1]);
            _x2 = _x1 + NumUtil::stoi(tokens[2]);
            _y2 = _y1 + NumUtil::stoi(tokens[3]);
        }
        else
        {
            _x1 = _y1 = _x2 = _y2 = 0;
        }
    }

    std::string base64Encode(const std::string_view input)
    {
        return macaron::Base64::Encode(input);
    }

    std::string base64Decode(const std::string& input)
    {
        std::string decoded;
        macaron::Base64::Decode(input, decoded);
        return decoded;
    }

} // namespace Util

#if !MOBILEAPP
namespace SigUtil {
    std::atomic<int> SigHandlerTrap::SigHandling;
} // end namespace SigUtil
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
