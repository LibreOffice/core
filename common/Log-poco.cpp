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
 * Logging infrastructure and buffering implementation.
 * Classes: StreamLogger - Functions: initialize(), getLevel(), prefix()
 */

#include <config.h>

#include "Log.hpp"

#include <common/ProcUtil.hpp>
#include <common/StaticLogHelper.hpp>
#include <common/Util.hpp>

#include <Poco/AutoPtr.h>
#include <Poco/FileChannel.h>
#include <Poco/Logger.h>
#include <Poco/Version.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unistd.h>
#include <unordered_map>

namespace
{
/// Tracks the number of thread-local buffers (for debugging purposes).
std::atomic_int32_t ThreadLocalBufferCount(0);

#ifndef NDEBUG
// In debug builds, we track the thread-ids to list the ones still running at exit.
thread_local std::int32_t OwnThreadIdIndex = 0;
std::int32_t ThreadIdArray[256];
std::atomic_int32_t NextThreadIdIndex(0);
#endif // !NDEBUG

/// Which log areas should be disabled
bool AreasDisabled[Log::AreaMax] = { false, };

} // namespace

/// Wrapper to expose protected 'log' and genericise
class GenericLogger : public Poco::Logger
{
    GenericLogger(const std::string& name,
                  Poco::AutoPtr<Poco::Channel> chan, int lvl)
        : Poco::Logger(name, std::move(chan), lvl)
    {
    }

public:
    static GenericLogger& create(const std::string& name,
                                 Poco::AutoPtr<Poco::Channel> chan, int lvl)
    {
        // Expect no thread contention creating
        // loggers and we can't access the internal mutex.
        if (find(name))
            throw Poco::ExistsException();
        auto* log = new GenericLogger(name, std::move(chan), lvl);
        add(log);
        return *log;
    }

    void doLog(Log::Level l, const std::string& text)
    {
        Poco::Message::Priority prio = Poco::Message::Priority::PRIO_TRACE;
#define MAP(l,p) case Log::Level::l: prio = Poco::Message::Priority::p;break
        switch (l) {
            MAP(FTL, PRIO_FATAL);
            MAP(CTL, PRIO_CRITICAL);
            MAP(ERR, PRIO_ERROR);
            MAP(WRN, PRIO_WARNING);
            MAP(NTC, PRIO_NOTICE);
            MAP(INF, PRIO_INFORMATION);
            MAP(DBG, PRIO_DEBUG);
            MAP(TRC, PRIO_TRACE);
        default:
            break;
#undef MAP
        }

        if (getLevel() < prio)
            return;
#if POCO_VERSION >= 0x010C0501
        Poco::Channel* channel = getChannel().get();
#else
        auto channel = getChannel();
#endif
        if (!channel)
            return;
        channel->log(Poco::Message(name(), text, prio));
    }

    static Log::Level mapToLevel(Poco::Message::Priority prio)
    {
#define MAP(l,p) case Poco::Message::Priority::p: return Log::Level::l;
        switch (prio) {
            MAP(FTL, PRIO_FATAL);
            MAP(CTL, PRIO_CRITICAL);
            MAP(ERR, PRIO_ERROR);
            MAP(WRN, PRIO_WARNING);
            MAP(NTC, PRIO_NOTICE);
            MAP(INF, PRIO_INFORMATION);
            MAP(DBG, PRIO_DEBUG);
            MAP(TRC, PRIO_TRACE);
        default:
            return Log::Level::TRC;
        }
#undef MAP
    }
};

namespace Log
{
    extern StaticHelper Static;
    extern StaticUIHelper StaticUILog;

    using namespace Poco;

    class ConsoleChannel : public Poco::Channel
    {
    public:
        static constexpr std::size_t BufferSize = 64 * 1024;

        void close() override { flush(); }

        /// Write the given buffer to stderr directly.
        static std::size_t writeRaw(const char* data, std::size_t count)
        {
#if WASMAPP
            // In WASM, stdout works best.
            constexpr int LOG_FILE_FD = STDOUT_FILENO;
#else
            // By default, write to stderr.
            constexpr int LOG_FILE_FD = STDERR_FILENO;
#endif

            const char *ptr = data;
            while (count > 0)
            {
                ssize_t wrote;
                while ((wrote = ::write(LOG_FILE_FD, ptr, count)) < 0 && errno == EINTR)
                {
                }

                if (wrote < 0)
                {
                    break;
                }

                ptr += wrote;
                count -= wrote;
            }
            return ptr - data;
        }

        template <std::size_t N> void writeRaw(const char (&data)[N])
        {
            writeRaw(data, N - 1); // Minus the null.
        }

        void writeRaw(const std::string& string) { writeRaw(string.data(), string.size()); }

        /// Flush the stderr file data.
        static bool flush() { return ::fflush(stderr) == 0; }

        /// Overloaded log function that takes a naked data pointer to log.
        /// Appends new-line to the given data.
        void log(const char* data, std::size_t size)
        {
            char buffer[BufferSize];
            if (size < sizeof(buffer) - 1)
            {
                memcpy(buffer, data, size);
                buffer[size] = '\n';
                writeRaw(buffer, size + 1);
            }
            else
            {
                // The buffer is too small, we must split the write.
                writeRaw(data, size);
                writeRaw("\n", 1);
            }
        }

        /// Implement the Channel log virtual.
        void log(const Poco::Message& msg) override
        {
            const std::string& s = msg.getText();
            log(s.data(), s.size());
        }
    };

    void preFork() { flush(); }

    void postFork()
    {
        /// after forking we can end up with threads that
        /// logged in the parent confusing our counting.
        ThreadLocalBufferCount = 0;
#ifndef NDEBUG
        NextThreadIdIndex = 0;
        memset(ThreadIdArray, 0, sizeof(ThreadIdArray));
#endif // !NDEBUG

        // The PID has changed after the fork.
        std::ostringstream oss;
        oss << Static.getName();
        if constexpr (!Util::isMobileApp())
            oss << '-' << std::setw(5) << std::setfill('0') << ProcUtil::getProcessId();
        Static.setId(oss.str());
    }

    class BufferedConsoleChannel : public ConsoleChannel
    {
        class ThreadLocalBuffer
        {
            static constexpr std::size_t BufferSize = ConsoleChannel::BufferSize;
            static constexpr std::size_t FlushBufferSize =
                std::min<std::size_t>(512UL, ConsoleChannel::BufferSize / 4); // ~4-5 entries.
            static constexpr std::int64_t MaxDelayMicroseconds = 5 * 1000 * 1000; // 5 seconds.

        public:
            ThreadLocalBuffer()
                : _size(0)
                , _oldest_time_us(0)
            {
                ++ThreadLocalBufferCount;
#ifndef NDEBUG
                OwnThreadIdIndex = NextThreadIdIndex++;
                ThreadIdArray[OwnThreadIdIndex] = ProcUtil::getThreadId();
#endif // !NDEBUG
            }

            ~ThreadLocalBuffer()
            {
                flush();
                --ThreadLocalBufferCount;
#ifndef NDEBUG
                ThreadIdArray[OwnThreadIdIndex] = 0;
#endif // !NDEBUG
            }

            std::size_t size() const { return _size; }
            std::size_t available() const { return BufferSize - _size; }

            /// Flush internal buffers, if any.
            void flush()
            {
                if (_size)
                {
                    ConsoleChannel::writeRaw(_buffer, _size);
                    _size = 0;
                    ConsoleChannel::flush();
                }
            }

            void log(const char* data, std::size_t size, bool force, std::int64_t ts)
            {
                if (_size + size > BufferSize - 1)
                {
                    flush();
                    if (size > BufferSize - 1)
                    {
                        // The buffer is too small, we must split the write.
                        ConsoleChannel::writeRaw(data, size);
                        ConsoleChannel::writeRaw("\n", 1);
                        return;
                    }
                }

                // Fits.
                if (_size == 0)
                    _oldest_time_us = ts;
                buffer(data, size);
                _buffer[_size] = '\n';
                ++_size;

                // Flush important messages and large caches immediately.
                if (force || _size >= FlushBufferSize ||
                    (ts - _oldest_time_us) > MaxDelayMicroseconds)
                {
                    flush();
                }
            }

            void buffer(const char* data, std::size_t size)
            {
                assert(_size + size <= BufferSize && "Buffer overflow");

                memcpy(_buffer + _size, data, size);
                _size += size;

                assert(_size <= BufferSize && "Buffer overflow");
            }

        private:
            char _buffer[BufferSize];
            std::size_t _size;
            std::int64_t _oldest_time_us; ///< The timestamp of the oldest buffered entry.
        };

    protected:
        std::size_t size() const { return _tlb.size(); }
        std::size_t available() const { return _tlb.available(); }

        void buffer(const char* data, std::size_t size) { _tlb.buffer(data, size); }

        void buffer(const std::string_view string) { buffer(string.data(), string.size()); }

    public:
        ~BufferedConsoleChannel() { flush(); }

        void close() override { flush(); }

        /// Flush buffers, if any.
        static void flush() { _tlb.flush(); }

        void log(const Poco::Message& msg) override
        {
            const std::string& s = msg.getText();
            _tlb.log(s.data(), s.size(), msg.getPriority() <= Message::PRIO_WARNING,
                     msg.getTime().raw());
        }

    private:
        static thread_local ThreadLocalBuffer _tlb;
    };

    thread_local BufferedConsoleChannel::ThreadLocalBuffer BufferedConsoleChannel::_tlb;

    /// Colored Console channel (needs to be buffered).
    class ColorConsoleChannel : public BufferedConsoleChannel
    {
    public:
        ColorConsoleChannel()
        {
            _colorByPriority.emplace(Message::PRIO_FATAL, "\033[1;31m"); // Bold Red
            _colorByPriority.emplace(Message::PRIO_CRITICAL, "\033[1;31m"); // Bold Red
            _colorByPriority.emplace(Message::PRIO_ERROR, "\033[1;35m"); // Bold Magenta
            _colorByPriority.emplace(Message::PRIO_WARNING, "\033[1;33m"); // Bold Yellow
            _colorByPriority.emplace(Message::PRIO_NOTICE, "\033[0;34m"); // Blue
            _colorByPriority.emplace(Message::PRIO_INFORMATION, "\033[0;34m"); // Blue
            _colorByPriority.emplace(Message::PRIO_DEBUG, "\033[0;36m"); // Teal
            _colorByPriority.emplace(Message::PRIO_TRACE, "\033[0;37m"); // Grey
        }

        void log(const Poco::Message& msg) override
        {
            const auto it = _colorByPriority.find(msg.getPriority());

            const std::string& s = msg.getText();
            const std::size_t need = s.size() + 12; // + Colors.

            if (available() < need)
            {
                flush();
                if (BufferSize < need)
                {
                    // Write directly, it will not fit.
                    if (it != _colorByPriority.end())
                    {
                        writeRaw(it->second);
                    }

                    writeRaw(s);
                    writeRaw("\033[0m\n"); // Restore default color.
                    return;
                }
            }

            // Fits.
            if (it != _colorByPriority.end())
            {
                buffer(it->second);
            }

            buffer(s);
            buffer("\033[0m\n"); // Restore default color.

            // Flush important messages and large caches immediately.
            if (msg.getPriority() <= Message::PRIO_WARNING || size() >= BufferSize / 2)
            {
                flush();
            }
        }

    private:
        std::unordered_map<Poco::Message::Priority, std::string> _colorByPriority;
    };

    void initialize(const std::string& name,
                    const std::string& logLevel,
                    const bool withColor,
                    const bool logToFile,
                    const std::map<std::string, std::string>& config,
                    const bool logToFileUICmd,
                    const std::map<std::string, std::string>& configUICmd)
    {
        Static.setName(name);
        std::ostringstream oss;
        oss << Static.getName();
        if constexpr (!Util::isMobileApp())
            oss << '-' << std::setw(5) << std::setfill('0') << ProcUtil::getProcessId();
        Static.setId(oss.str());

        // Configure the logger.
        AutoPtr<Channel> channel;

        if (logToFile)
        {
            channel = static_cast<Poco::Channel*>(new Poco::FileChannel("coolwsd.log"));
            for (const auto& pair : config)
            {
                channel->setProperty(pair.first, pair.second);
            }
        }
        else if (withColor)
        {
            channel = static_cast<Poco::Channel*>(new Log::ColorConsoleChannel());
        }
        else
        {
            const auto it = config.find("flush");
            if (it == config.end() || Util::toLower(it->second) != "false")
            {
                // Unbuffered (flushed) logging, directly writes each entry (to stderr).
                channel = static_cast<Poco::Channel*>(new Log::ConsoleChannel());
            }
            else
            {
                // Buffered logging, reduces number of write(2) syscalls.
                channel = static_cast<Poco::Channel*>(new Log::BufferedConsoleChannel());
            }
        }

        /**
         * Open the channel explicitly, instead of waiting for first log message
         * This is important especially for the kit process where opening the channel
         * after chroot can cause file creation inside the jail instead of outside
         * */
        channel->open();

        try
        {
            auto& logger = GenericLogger::create(Static.getName(), std::move(channel), Poco::Message::PRIO_TRACE);
            Static.setLogger(&logger);
        }
        catch (ExistsException&)
        {
            auto* logger = static_cast<GenericLogger*>(&Poco::Logger::get(Static.getName()));
            Static.setLogger(logger);
        }

        auto* logger = Static.getLogger();

        const std::string level = logLevel.empty() ? std::string("trace") : logLevel;
        logger->setLevel(level);
        Static.setLevel(level);

        const std::time_t t = std::time(nullptr);
        struct tm tm;
        LOG_INF("Initializing " << name << ". Log timestamps are in GMT time. Now: "
                                << std::put_time(Util::time_t_to_gmtime(t, tm), "%a %F %T %z")
                                << ". Log level is [" << logger->getLevel() << ']');

        StaticUILog.setName(name+"_ui");
        AutoPtr<Channel> channelUILog;
        if (logToFileUICmd)
        {
            channelUILog = static_cast<Poco::Channel*>(new Poco::FileChannel("coolwsd-ui-cmd.log"));
            for (const auto& pair : configUICmd)
            {
                channelUILog->setProperty(pair.first, pair.second);
            }

            channelUILog->open();
            try
            {
                auto& loggerUILog = GenericLogger::create(StaticUILog.getName(), std::move(channelUILog), Poco::Message::PRIO_TRACE);
                StaticUILog.setLogger(&loggerUILog);
            }
            catch (ExistsException&)
            {
                auto* loggerUILog =
                    static_cast<GenericLogger*>(&Poco::Logger::get(StaticUILog.getName()));
                StaticUILog.setLogger(loggerUILog);
            }
        }
    }

    namespace
    {
    bool IsShutdown = false;

    GenericLogger& logger()
    {
        GenericLogger* logger = Static.getThreadLocalLogger();
        if (logger != nullptr)
            return *logger;

        logger = Static.getLogger();
        return logger ? *logger
            : *static_cast<GenericLogger *>(
                &GenericLogger::get(Static.getInited() ? Static.getName() : std::string()));
    }

    GenericLogger& loggerUI()
    {
        GenericLogger* logger = StaticUILog.getThreadLocalLogger();
        if (logger != nullptr)
            return *logger;

        logger = StaticUILog.getLogger();
        return logger ? *logger
            : *static_cast<GenericLogger *>(
                &GenericLogger::get(StaticUILog.getInited() ? StaticUILog.getName() : std::string()));
    }

    } // namespace

    bool isLogUIEnabled()
    {
        return (StaticUILog.getThreadLocalLogger() != nullptr) ||
               (StaticUILog.getLogger() != nullptr);
    }

    void logUI(Level l, const std::string &text)
    {
        if (isLogUIEnabled())
            Log::loggerUI().doLog(l, text);
    }

    bool isLogUIMerged()
    {
        return StaticUILog.getMergeCmd();
    }

    bool isLogUITimeEnd()
    {
        return StaticUILog.getLogTimeEndOfMergedCmd();
    }

    void setUILogMergeInfo(bool mergeCmd, bool logTimeEndOfMergedCmd)
    {
        StaticUILog.setLogMergeInfo(mergeCmd, logTimeEndOfMergedCmd);
    }

    bool isEnabled(Level l, Area a)
    {
        if (IsShutdown)
            return false;

        // Check if logger is properly initialized before calling logger()
        // to avoid invalid downcast from Poco::Logger to GenericLogger.
        if (!Static.getThreadLocalLogger() && !Static.getLogger())
            return false;

        Log::Level logLevel = GenericLogger::mapToLevel(
            static_cast<Poco::Message::Priority>(logger().getLevel()));

        if (logLevel < static_cast<int>(l))
            return false;

        bool disabled = AreasDisabled[static_cast<size_t>(a)];

        // Areas shouldn't disable warnings & errors
        assert(!disabled || logLevel > static_cast<int>(Level::WRN));

        return !disabled;
    }

    void shutdown()
    {
        if constexpr (Util::isMobileApp())
            return;

        if constexpr (!Util::isKitInProcess())
        {
            // Allow other threads time to exit.
            for (int i = 0; i < 10 && ThreadLocalBufferCount > 1; ++i)
            {
                std::this_thread::yield();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

#ifndef NDEBUG
            const auto currentThreadId = ProcUtil::getThreadId();
            for (int i = 0; i < NextThreadIdIndex; ++i)
            {
                if (ThreadIdArray[i] && ThreadIdArray[i] != currentThreadId)
                {
                    LOG_ERR(">>> Thread " << ThreadIdArray[i]
                                          << " is still running while shutting down logging");
                }
            }

            // Flush before we assert (no assertion in non-debug builds).
            flush();
            ::fflush(nullptr); // Flush all open output streams.
#endif // !NDEBUG

            assert(ThreadLocalBufferCount <= 1 &&
                   "Unstopped threads may have unflushed buffered log entries. "
                   "This is common if there has been an earlier fatal error. Check previous log entries");
        }

        // continue logging shutdown on mobile
        IsShutdown = !Util::isMobileApp();

        Poco::Logger::shutdown();

        flush();

        ::fflush(nullptr); // Flush all open output streams.
    }

    void flush() { BufferedConsoleChannel::flush(); }

    void setThreadLocalLogLevel(const std::string& logLevel)
    {
        if (!Static.getLogger())
        {
            return;
        }

        if constexpr (Util::isFuzzing())
        {
            // loggingleveloverride tries to increase log level, ignore.
            return;
        }

        // Use the same channel for all Poco loggers.
        auto channel = Static.getLogger()->getChannel();

        // The Poco loggers have to have names that are unique, but those aren't displayed anywhere.
        // So just use the name of the default logger for this process plus a counter.
        static int counter = 1;
        auto& logger = GenericLogger::create(Static.getName() + "." + std::to_string(counter++),
                                             std::move(channel),
                                             Poco::Logger::parseLevel(logLevel));

        Static.setThreadLocalLogger(&logger);
    }

    const std::string& getLevelName()
    {
        return Static.getLevel();
    }

    Level getLevel()
    {
        return GenericLogger::mapToLevel(
            static_cast<Poco::Message::Priority>(
                Log::logger().getLevel()));
    }

    void setLevel(const std::string &l)
    {
        Log::logger().setLevel(l);
        // Update our public flags in the array now ...
    }

    /// Set disabled areas
    void setDisabledAreas(const std::string &areaStr)
    {
        if (areaStr != "")
            LOG_INF("Setting disabled log areas to [" << areaStr << "]");
        StringVector areas = StringVector::tokenize(areaStr, ',');
        std::vector<bool> enabled(Log::AreaMax, true);
        for (size_t t = 0; t < areas.size(); ++t)
        {
            for (size_t i = 0; i < Log::AreaMax; ++i)
            {
                if (areas.equals(t, nameShort(static_cast<Log::Area>(i))))
                {
                    enabled[i] = false;
                    break;
                }
            }
        }
        for (size_t i = 0; i < Log::AreaMax; ++i)
            AreasDisabled[i] = !enabled[i];
    }

    void log(Level l, const std::string &text)
    {
        Log::logger().doLog(l, text);
    }

    static const std::string levelList[] = { "none",        "fatal",   "critical",
                                             "error",       "warning", "notice",
                                             "information", "debug",   "trace" };

    const std::string& getLogLevelName(const std::string& channel)
    {
        const int wsdLogLevel = GenericLogger::get(channel).getLevel();
        return levelList[wsdLogLevel];
    }

    void setLogLevelByName(const std::string &channel,
                           const std::string &level)
    {
        if constexpr (Util::isFuzzing())
        {
            // update-log-levels tries to increase log level, ignore.
            return;
        }

        // FIXME: seems redundant do we need that ?
        std::string lvl = level;

        // Get the list of channels..
        std::vector<std::string> nameList;
        GenericLogger::names(nameList);

        if (std::find(std::begin(levelList), std::end(levelList), level) == std::end(levelList))
            lvl = "debug";

        GenericLogger::get(channel).setLevel(lvl);
    }

} // namespace Log

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
