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

#include <common/StateEnum.hpp>

#include <array>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <source_location>
#include <sstream>
#include <string>
#include <thread>

#ifdef __ANDROID__
#include <android/log.h>
#endif

#if defined __EMSCRIPTEN__
#include <emscripten/console.h>
#endif

namespace Log
{
    enum Level : std::uint8_t
    {
        FTL = 1, // Fatal
        CTL,     // Critical
        ERR,     // Error
        WRN,     // Warning
        NTC,     // Notice
        INF,     // Information
        DBG,     // Debug
        TRC,     // Trace
        MAX
    };

    // Different logging domains
    STATE_ENUM(Area,
               Generic,
               Pixel,
               Socket,
               WebSocket,
               Http,
               WebServer,
               Storage,
               WOPI,
               Admin,
               Javascript);

    // Types of phases for unit test
    STATE_ENUM(Phase,
               Setup,
               Load,
               Edit);

    /// Initialize the logging system.
    void initialize(const std::string& name,
                    const std::string& logLevel,
                    bool withColor = false,
                    bool logToFile = false,
                    const std::map<std::string, std::string>& config = {},
                    bool logToFileUICmd = false,
                    const std::map<std::string, std::string>& configUICmd = {});

    /// Shutdown and release the logging system.
    void shutdown();

    /// Flush buffered console logs, if used.
    void flush();

    /// Prepare for forking.
    void preFork();

    /// Cleanup state after forking
    void postFork();

    /// Post renaming a thread.
    void reset();

    void setThreadLocalLogLevel(const std::string& logLevel);

    /// Per-thread log prefix that is both safe and efficient.
    /// Generates log entry prefix. Example follows (without the vertical bars).
    /// |wsd-07272-07298 2020-04-25 17:29:28.928697 -0400 [ websrv_poll ] TRC  |
    class Prefix
    {
        Prefix(const Prefix&) = delete;
        Prefix& operator=(const Prefix&) = delete;

    public:
        Prefix();

        /// Get the log prefix, without updating it.
        std::string_view prefix() const { return _prefix; }

        /// Reset the prefix, re-generating it again.
        void reset();

        /// Update and return the log prefix.
        std::string_view update(std::string_view level,
                                std::chrono::time_point<std::chrono::system_clock> tp =
                                    std::chrono::system_clock::now());

        static thread_local Prefix Instance;

    private:
        static constexpr std::size_t BufferSize = 128;
        std::array<char, BufferSize> _buffer;
        std::string_view _prefix;
        std::tm _last_tm; ///< The local timestamp in the prefix.
        std::time_t _last_time; ///< The system timestamp in the prefix.
        char* _year_pos;
        char* _level_pos;
    };

    /// Per-thread log prefix that is both safe and efficient.
    /// Generates log entry prefix. Example follows (without the vertical bars).
    /// |wsd-07272-07298 2020-04-25 17:29:28.928697 -0400 [ websrv_poll ] TRC  |
    std::string_view prefix(
        const std::string_view level,
        std::chrono::time_point<std::chrono::system_clock> tp = std::chrono::system_clock::now());

#ifdef BUILDING_TESTS
    /// Generates the reference log entry prefix. Do *not* use, except for tests.
    char* prefixReference(const std::chrono::time_point<std::chrono::system_clock>& tp,
                          char* buffer, const std::string_view level);
#endif

    /// is a certain level of logging enabled ?
    bool isEnabled(Level l, Area a = Area::Generic);

    inline bool traceEnabled()
    {
        return isEnabled(Log::Level::TRC);
    }

    /// Main entry function for all logging
    void log(Level l, const std::string &text);
    bool isLogUIEnabled();
    void logUI(Level l, const std::string &text);
    bool isLogUIMerged();
    bool isLogUITimeEnd();
    void setUILogMergeInfo(bool mergeCmd, bool logTimeEndOfMergedCmd);

    /// Setting the logging level
    void setLevel(const std::string &l);

    /// Set disabled areas
    void setDisabledAreas(const std::string &areas);

    /// Getting the logging level
    Level getLevel();

    const std::string& getLogLevelName(const std::string& channel);
    void setLogLevelByName(const std::string &channel,
                           const std::string &level);

    /// Getting the logging level as a string
    const std::string& getLevelName();

#ifdef _WIN32
    static bool isDebuggerPresent;
#endif

    /// A helper class that captures an original location (the parent's) and a current location (actual).
    /// Supports comparing them via the same() member.
    class caller_location
    {
    public:
        constexpr caller_location(std::source_location orig_loc,
                                  std::source_location cur_loc = std::source_location::current())
            : _orig_loc(orig_loc)
            , _cur_loc(cur_loc)
        {
        }

        constexpr bool same() const
        {
            return _orig_loc.line() == _cur_loc.line() &&
                   (_orig_loc.file_name() == _cur_loc.file_name() ||
                    std::strcmp(_orig_loc.file_name(), _cur_loc.file_name()) == 0);
        }

        auto file_name() const { return _orig_loc.file_name(); }
        auto line() const { return _orig_loc.line(); }

    private:
        const std::source_location _orig_loc;
        const std::source_location _cur_loc;
    };

    /// Serialize a caller_location, if it has an original location that is different from the current.
    inline std::ostream& operator<<(std::ostream& lhs, const caller_location& loc)
    {
        if (!loc.same())
        {
            lhs << "(from " << loc.file_name() << ':' << loc.line() << ")|";
        }

        return lhs;
    }

    /// A callable object to mimick fetch_source_location but return that of the parent's.
    class SourceLocationGetter : public std::source_location
    {
    public:
        constexpr explicit SourceLocationGetter(
            std::source_location loc = std::source_location::current())
            : std::source_location(loc)
        {
        }
        constexpr std::source_location operator()() const { return *this; }
    };

} // namespace Log

/// A dummy fetch_source_location that returns the current location.
/// This is the no-op version. Must be replaced via LOG_CAPTURE_CALLER
/// to replace it with one that returns the parent's location.
constexpr std::source_location
fetch_source_location(std::source_location loc = std::source_location::current())
{
    return loc;
}

/// A default implementation that is a NOP.
/// Any context can implement this to prefix its log entries.
inline void logPrefix(std::ostream&) {}

inline std::ostream& operator<<(std::ostream& lhs, const std::function<void(std::ostream&)>& f)
{
    f(lhs);
    return lhs;
}

#ifdef IOS
// We know that when building with Xcode, __FILE__ will always be a full path, with several slashes,
// so this will always work. We want just the file name, they are unique anyway.
/// Strip the path and leave only the filename.
template <std::size_t N>
static constexpr std::size_t skipPathToFilename(const char (&s)[N], std::size_t n = N - 1)
{
    return n == 0 ? 0 : s[n] == '/' ? n + 1 : skipPathToFilename(s, n - 1);
}

#define LOG_FILE_NAME(f) (&f[skipPathToFilename(f)])

#elif defined _WIN32
/// Strip the path and leave only the filename.
template <std::size_t N>
static constexpr std::size_t skipPathToFilename(const char (&s)[N], std::size_t n = N - 1)
{
    return n == 0 ? 0 : (s[n] == '/' || s[n] == '\\') ? n + 1 : skipPathToFilename(s, n - 1);
}

#define LOG_FILE_NAME(f) (&f[skipPathToFilename(f)])

#else

/// Strip the path prefix ("./") that is noisy.
template <std::size_t N>
static constexpr std::size_t skipPathPrefix(const char (&s)[N], std::size_t n = 0)
{
    return s[n] == '.' || s[n] == '/' ? skipPathPrefix(s, n + 1) : n;
}

#define LOG_FILE_NAME(f) (&f[skipPathPrefix(f)])
#endif

#define UNIQUE_VAR(X) CONCATINATE(X, __LINE__)

#ifdef __ANDROID__

#define LOG_LOG(LVL, STR) \
    ((void)__android_log_print(ANDROID_LOG_DEBUG, \
                               "coolwsd", "%s %s", #LVL, STR.c_str()))
#elif defined __EMSCRIPTEN__

// emscripten/console.h does not have emscripten_console_info (corresponding to JS console.info) nor
// emscripten_console_debug (corresponding to JS console.debug), so use emscripten_console_log
// (corresponding to JS console.log) instead:
#define LOG_LOG(LVL, STR) ( \
    Log::LVL <= Log::ERR ? emscripten_console_error((STR).c_str()) : \
    Log::LVL <= Log::WRN ? emscripten_console_warn((STR).c_str()) : \
                           emscripten_console_log((STR).c_str()))

#else

#define LOG_LOG(LVL, STR)  Log::log(Log::LVL, STR)
#endif

#define LOG_END_NOFILE(LOG) (void)0

/// Used to pass the current function's call site to capture the
/// grandparent's call location, rather than the parent's.
/// The current function must have LOG_CAPTURE_CALL(_DECLARATION).
/// Example: in function f(LOG_CAPTURE_CALL_DECLARATION) { child(LOG_PASS_PARENT_CALLER); }
#define LOG_PASS_PARENT_CALLER fetch_source_location
/// Used as the last parameter in the definition of a function that needs
/// to capture the caller's source location (filename and line number).
#define LOG_CAPTURE_CALLER [[maybe_unused]] const Log::SourceLocationGetter& LOG_PASS_PARENT_CALLER
/// Used as the last parameter in the declaration of a function that needs
/// to capture the caller's source location (filename and line number).
/// Logs from this function will include the caller's location,
/// which is helpful for generic helpers. Example:
/// WRN  WOPISrc validation error|(from wsd/FileServer.cpp:1416)|net/HttpHelper.hpp:79
#define LOG_CAPTURE_CALLER_DECLARATION LOG_CAPTURE_CALLER = Log::SourceLocationGetter()

/// Add the log-suffix.
/// Calls fetch_source_location(), which returns Log::caller_location. If this is different
/// from std::source_location::current(), we log that as the original local, like so:
/// INF  Copying 7750 bytes from empty.ods to /tmp/empty.ods|(from ./common/FileUtil.hpp:180)|common/FileUtil.cpp:89
#define LOG_END(LOG)                                                                               \
    LOG << '|' << Log::caller_location(fetch_source_location()) << LOG_FILE_NAME(__FILE__)         \
        << ":" STRING(__LINE__)

#define LOG_MESSAGE_(LVL, A, X, PREFIX, SUFFIX)  \
    do                                          \
    {                                           \
        if (LOG_CONDITIONAL(LVL, A))              \
        {                                       \
            LOG_BODY_(LVL, X, PREFIX, SUFFIX);    \
        }                                       \
    } while (false)

/// Logs this particular log entry only once in the process lifetime.
#define LOG_MESSAGE_ONCE_(LVL, A, X, PREFIX, SUFFIX)                                               \
    do                                                                                             \
    {                                                                                              \
        if (LOG_CONDITIONAL(LVL, A))                                                               \
        {                                                                                          \
            static std::once_flag UNIQUE_VAR(once);                                                \
            std::call_once(UNIQUE_VAR(once), [&]() { LOG_BODY_(LVL, X, PREFIX, SUFFIX); });        \
        }                                                                                          \
    } while (false)

#define LOG_BODY_IMPL_(LEVEL_DISPLAY_NAME, LEVEL, X, PREFIX, END)                                  \
    do                                                                                             \
    {                                                                                              \
        std::ostringstream oss_(Log::prefix(#LEVEL_DISPLAY_NAME).data(), std::ostringstream::ate); \
        PREFIX(oss_);                                                                              \
        oss_ << std::boolalpha << X;                                                               \
        END(oss_);                                                                                 \
        LOG_LOG(LEVEL, oss_.str());                                                                \
    } while (false)

#define LOG_BODY_(LVL, X, PREFIX, END) LOG_BODY_IMPL_(LVL, LVL, X, PREFIX, END)

/// Unconditionally log. LVL can be anything converted to string.
#define LOG_UNCONDITIONAL(LVL, X) LOG_BODY_IMPL_(LVL, FTL, X, logPrefix, LOG_END)

/// Unconditionally log at ANY level.
#define LOG_ANY(X) LOG_UNCONDITIONAL(ANY, X)
/// Unconditionally log at TST level. Used for tests only.
#define LOG_TST(X) LOG_UNCONDITIONAL(TST, X)

#if defined __GNUC__ || defined __clang__
#  define LOG_CONDITIONAL(type, area)  \
    __builtin_expect(Log::isEnabled(Log::Level::type, Log::Area::area), 0)
#else
#  define LOG_CONDITIONAL(type) Log::isEnabled(Log::Level::type, Log::Level::area)
#endif

#define LOG_TRC(X)        LOGA_TRC(Generic, X)
#define LOG_TRC_NOFILE(X) LOGA_TRC_NOFILE(Generic, X)
#define LOG_DBG(X)        LOGA_DBG(Generic, X)
#define LOG_INF(X)        LOGA_INF(Generic, X)
#define LOG_INF_NOFILE(X) LOGA_INF_NOFILE(Generic, X)
#define LOG_WRN(X)        LOG_MESSAGE_(WRN, Generic, X, logPrefix, LOG_END)
#define LOG_ERR(X)        LOG_MESSAGE_(ERR, Generic, X, logPrefix, LOG_END)

#define LOG_WRN_ONCE(X) LOG_MESSAGE_ONCE_(WRN, Generic, X, logPrefix, LOG_END)
#define LOG_WRN_ONCE_S(X) LOG_MESSAGE_ONCE_(WRN, Generic, X, ::logPrefix, LOG_END)
#define LOG_ERR_ONCE(X) LOG_MESSAGE_ONCE_(ERR, Generic, X, logPrefix, LOG_END)
#define LOG_ERR_ONCE_S(X) LOG_MESSAGE_ONCE_(ERR, Generic, X, ::logPrefix, LOG_END)

#define LOGA_TRC(A,X)        LOG_MESSAGE_(TRC, A, X, logPrefix, LOG_END)
#define LOGA_TRC_NOFILE(A,X) LOG_MESSAGE_(TRC, A, X, logPrefix, LOG_END_NOFILE)
#define LOGA_DBG(A,X)        LOG_MESSAGE_(DBG, A, X, logPrefix, LOG_END)
#define LOGA_INF(A,X)        LOG_MESSAGE_(INF, A, X, logPrefix, LOG_END)
#define LOGA_INF_NOFILE(A,X) LOG_MESSAGE_(INF, A, X, logPrefix, LOG_END_NOFILE)
// WRN and ERR should not be filtered by area

/// Internal: Log an entry with the given ERRNO appended using the given LOGGER.
#define LOG_ERRNO_(LOGGER, ERRNO, X)                                                               \
    do                                                                                             \
    {                                                                                              \
        const auto onrre = ERRNO; /* Save errno immediately while avoiding name clashes*/          \
        LOGGER(X << " (" << Util::symbolicErrno(onrre) << ": " << std::strerror(onrre) << ')');    \
    } while (false)

/// Log an ERR entry with the given errno appended.
#define LOG_ERR_ERRNO(ERRNO, X) LOG_ERRNO_(LOG_ERR, ERRNO, X)
/// Log an WRN entry with the given errno appended.
#define LOG_WRN_ERRNO(ERRNO, X) LOG_ERRNO_(LOG_WRN, ERRNO, X)
/// Log an INF entry with the given errno appended.
#define LOG_INF_ERRNO(ERRNO, X) LOG_ERRNO_(LOG_INF, ERRNO, X)
/// Log an DBG entry with the given errno appended.
#define LOG_DBG_ERRNO(ERRNO, X) LOG_ERRNO_(LOG_DBG, ERRNO, X)
/// Log an TRC entry with the given errno appended.
#define LOG_TRC_ERRNO(ERRNO, X) LOG_ERRNO_(LOG_TRC, ERRNO, X)

/// Log an ERR entry with errno appended.
/// NOTE: Must be called immediately after an API that sets errno.
/// Use LOG_ERR_ERRNO to pass errno explicitly.
#define LOG_SYS(X) LOG_ERR_ERRNO(errno, X)
/// Log an WRN entry with the given errno appended.
#define LOG_WRN_SYS(X) LOG_WRN_ERRNO(errno, X)
/// Log an INF entry with the given errno appended.
#define LOG_INF_SYS(X) LOG_INF_ERRNO(errno, X)
/// Log an DBG entry with the given errno appended.
#define LOG_DBG_SYS(X) LOG_DBG_ERRNO(errno, X)
/// Log an TRC entry with the given errno appended.
#define LOG_TRC_SYS(X) LOG_TRC_ERRNO(errno, X)

#define LOG_FTL(X)                                                                                 \
    do                                                                                             \
    {                                                                                              \
        std::cerr << X << std::endl;                                                               \
        if (LOG_CONDITIONAL(FTL, Generic))                                      \
        {                                                                                          \
            LOG_BODY_(FTL, X, logPrefix, LOG_END);  \
        }                                                                                          \
    } while (false)

/// Log an FTL (fatal) entry with errno appended.
/// NOTE: Must be called immediately after an API that sets errno.
#define LOG_SFL(X) LOG_ERRNO_(LOG_FTL, errno, X)

/// No-prefix versions:
#define LOG_TRC_S(X) LOG_MESSAGE_(TRC, Generic, X, ::logPrefix, LOG_END)
#define LOG_DBG_S(X) LOG_MESSAGE_(DBG, Generic, X, ::logPrefix, LOG_END)
#define LOG_INF_S(X) LOG_MESSAGE_(INF, Generic, X, ::logPrefix, LOG_END)
#define LOG_WRN_S(X) LOG_MESSAGE_(WRN, Generic, X, ::logPrefix, LOG_END)
#define LOG_ERR_S(X) LOG_MESSAGE_(ERR, Generic, X, ::logPrefix, LOG_END)

#define SERIALIZE(_, X) STRING(X) ": " << (X) << ", "

/// Logs the given variables and their values as LOG_ANY.
/// Example: LOG_VARS("Before xyz", size, name); =>
/// LOG_ANY('[' << __func__ << "] " << PREFIX << ": " << "size: " << size << ", name: " << name << ", ");
#define LOG_VARS(PREFIX, ...)                                                                      \
    LOG_ANY('[' << __func__ << "] " << PREFIX << ": " << FOR_EACH(SERIALIZE, NAME, __VA_ARGS__))

#define LOG_CHECK(X)                                                                               \
    do                                                                                             \
    {                                                                                              \
        if (!(X))                                                                                  \
        {                                                                                          \
            LOG_ERR("Check failed. Expected (" #X ").");                                           \
        }                                                                                          \
    } while (false)

#define LOG_CHECK_RET(X, RET)                                                                      \
    do                                                                                             \
    {                                                                                              \
        if (!(X))                                                                                  \
        {                                                                                          \
            LOG_ERR("Check failed. Expected (" #X ").");                                           \
            return RET;                                                                            \
        }                                                                                          \
    } while (false)

/// Assert the truth of a condition, with a custom message.
#define LOG_ASSERT_INTERNAL(condition, message, LOG)                                               \
    do                                                                                             \
    {                                                                                              \
        auto&& UNIQUE_VAR(cond) = !!(condition);                                                   \
        if (!UNIQUE_VAR(cond))                                                                     \
        {                                                                                          \
            std::ostringstream UNIQUE_VAR(oss);                                                    \
            UNIQUE_VAR(oss) << message;                                                            \
            const auto UNIQUE_VAR(msg) = UNIQUE_VAR(oss).str();                                    \
            LOG("ERROR: Assertion failure: "                                                       \
                << (UNIQUE_VAR(msg).empty() ? "" : UNIQUE_VAR(msg) + ". ")                         \
                << "Condition: " STRING(condition));                                               \
            assert(!STRING(condition)); /* NOLINT(misc-static-assert) */                           \
        }                                                                                          \
    } while (false)

/// Assert the truth of a condition.
#if defined(NDEBUG)
// In release mode assertions are logged as debug-level, since they are for developers.
#define LOG_ASSERT_MSG(condition, message)                                                         \
    LOG_ASSERT_INTERNAL(condition, "Precondition failed", LOG_DBG)
#define LOG_ASSERT(condition) LOG_ASSERT_MSG(condition, "Precondition failed")
#else
// In debug mode assertions are errors.
#define LOG_ASSERT_MSG(condition, message) LOG_ASSERT_INTERNAL(condition, message, LOG_ERR)
#define LOG_ASSERT(condition) LOG_ASSERT_MSG(condition, "Precondition failed")
#endif

// Must be included at the _end_ of this file, since it (Util.hpp)
// includes us (Log.hpp) and that would break the build.
#include <common/Util.hpp>

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
