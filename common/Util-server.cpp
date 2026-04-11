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
 * Server-side utility implementations.
 * Functions: spawnProcess(), getMemoryUsage(), DirectoryCounter
 */

#include <config.h>

#include <common/Log.hpp>
#include <common/NumUtil.hpp>
#include <common/ProcUtil.hpp>
#include <common/StringVector.hpp>
#include <common/Util.hpp>

#include <Poco/Exception.h>

#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <spawn.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/time.h>
#include <sys/resource.h>
#elif defined __FreeBSD__
#include <sys/resource.h>
#include <sys/user.h>
extern char** environ;
#endif

// 'environ' is not directly available on macOS, but using _NSGetEnviron() should be good enough
#ifdef __APPLE__
#  include <crt_externs.h>
#  define environ (*_NSGetEnviron())
#endif

namespace
{
/// If line starts with tag, return a pointer to the first digit after the tag.
const char* startsWith(const char* line, std::string_view tag)
{
    if (!std::strncmp(line, tag.data(), tag.size()))
    {
        const char* p = line + tag.size();
        while (*p != '\0' && *p != '\n' && !std::isdigit(static_cast<unsigned char>(*p)))
        {
            ++p;
        }

        return (*p != '\0' && *p != '\n') ? p : nullptr;
    }

    return nullptr;
}

std::size_t getFromCGroup(const std::string& group, const std::string& key)
{
    std::size_t num = 0;

    std::string groupPath;
    FILE* cg = fopen("/proc/self/cgroup", "r");
    if (cg != nullptr)
    {
        char line[4096] = { 0 };
        while (fgets(line, sizeof(line), cg))
        {
            StringVector bits = StringVector::tokenize(line, strlen(line), ':');
            if (bits.size() > 2 && bits[1] == group)
            {
                groupPath = "/sys/fs/cgroup/" + group + bits[2];
                break;
            }
        }
        LOG_TRC("control group path for " << group << " is " << groupPath);
        fclose(cg);
    }

    if (groupPath.empty())
        return 0;

    std::string path = groupPath + "/" + key;
    LOG_TRC("Read from " << path);
    FILE* file = fopen(path.c_str(), "r");
    if (file != nullptr)
    {
        char line[4096] = { 0 };
        if (fgets(line, sizeof(line), file))
            num = atoll(line);
        fclose(file);
    }

    return num;
}

std::string getCurrentCGroupPath()
{
    std::ifstream file("/proc/self/cgroup");
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            // cgroup v2 format: 0::/path/to/cgroup
            if (line.substr(0, 3) == "0::")
            {
                return line.substr(3);
            }
        }
    }
    return "/";
}

bool isCGroupV2() { return std::ifstream("/sys/fs/cgroup/cgroup.controllers").good(); }

std::size_t getFromCGroupV2(const std::string& key)
{
    std::string cgroupPath = getCurrentCGroupPath();
    std::string fullPath = "/sys/fs/cgroup" + cgroupPath + "/" + key;
    std::size_t num = 0;

    LOG_TRC("Reading from: " << fullPath);

    std::ifstream file(fullPath);
    if (file.is_open())
    {
        std::string line;
        if (std::getline(file, line))
        {
            if (line == "max")
            {
                return 0; // Unlimited
            }
            else
            {
                num = std::stoull(line);
            }
        }
    }
    return num;
}
} // namespace

namespace ProcUtil
{

int spawnProcess(const std::string& cmd, const StringVector& args)
{
    // Create a vector of zero-terminated strings.
    std::vector<std::string> argStrings;
    for (const auto& arg : args)
        argStrings.push_back(args.getParam(arg));

    std::vector<char*> params;
    params.push_back(const_cast<char*>(cmd.c_str()));
    for (const auto& i : argStrings)
        params.push_back(const_cast<char*>(i.c_str()));
    params.push_back(nullptr);

    pid_t pid = -1;
    int status = posix_spawn(&pid, params[0], nullptr, nullptr, params.data(), environ);
    if (status < 0)
    {
        LOG_ERR("Failed to posix_spawn for command '" << cmd);
        throw Poco::SystemException("Failed to fork posix_spawn command ", cmd);
    }

    return pid;
}

} // namespace ProcUtil

namespace Util
{

std::string getHumanizedBytes(unsigned long bytes)
{
    constexpr unsigned factor = 1024;
    short count = 0;
    float val = bytes;
    while (val >= factor && count < 4)
    {
        val /= factor;
        count++;
    }
    std::string unit;
    switch (count)
    {
        case 0:
            unit.clear();
            break;
        case 1:
            unit = "ki";
            break;
        case 2:
            unit = "Mi";
            break;
        case 3:
            unit = "Gi";
            break;
        case 4:
            unit = "Ti";
            break;
        default:
            assert(false);
    }

    unit += 'B';
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << val << ' ' << unit;
    return ss.str();
}

std::size_t getTotalSystemMemoryKb()
{
    std::size_t totalMemKb = 0;
    FILE* file = fopen("/proc/meminfo", "r");
    if (file != nullptr)
    {
        char line[4096] = { 0 };
        // coverity[tainted_data_argument : FALSE] - we trust the kernel-provided data
        while (fgets(line, sizeof(line), file))
        {
            if (const char* value = startsWith(line, "MemTotal:"))
            {
                totalMemKb = atoll(value);
                break;
            }
        }
        fclose(file);
    }

    return totalMemKb;
}

std::size_t getCGroupMemLimit()
{
#ifdef __linux__
    if (isCGroupV2())
    {
        return getFromCGroupV2("memory.max");
    }
    else
    {
        return getFromCGroup("memory", "memory.limit_in_bytes");
    }
#else
    return 0;
#endif
}

std::size_t getCGroupMemSoftLimit()
{
#ifdef __linux__
    if (isCGroupV2())
    {
        return getFromCGroupV2("memory.high");
    }
    else
    {
        return getFromCGroup("memory", "memory.soft_limit_in_bytes");
    }
#else
    return 0;
#endif
}

} // namespace Util

namespace ProcUtil
{

std::pair<std::size_t, std::size_t> getPssAndDirtyFromSMaps(FILE* file)
{
    std::size_t numPSSKb = 0;
    std::size_t numDirtyKb = 0;
    if (file)
    {
        const int fd = fileno(file);

        // Read the whole file in one go to minimize kernel page-table walks,
        // since /proc/pid/smaps[_rollup] is regenerated on each read() call.
        std::string buf(64 * 1024, '\0');
        std::size_t total = 0;
        for (;;)
        {
            if (total == buf.size())
                buf.resize(buf.size() * 2);

            const ssize_t n = pread(fd, buf.data() + total, buf.size() - total, total);
            if (n <= 0)
                break;
            total += n;
        }

        buf[total] = '\0'; // Ensure null-termination for strncmp/atoi.

        const char* pos = buf.data();
        const char* const end = pos + total;
        while (pos < end)
        {
            if (pos[0] == 'P')
            {
                const char* value;

                // Shared_Dirty is accounted for by forkit's RSS
                if ((value = startsWith(pos, "Private_Dirty:")))
                {
                    numDirtyKb += atoi(value);
                    pos = value;
                }
                else if ((value = startsWith(pos, "Pss:")))
                {
                    pos = value;
                    numPSSKb += atoi(value);
                }
            }

            do
            {
                // Skip the rest of the line.
                ++pos;
            } while (pos < end && *pos != '\0' && *pos != '\n');
            if (pos < end)
                ++pos; // Skip the end-of-line.
        }
    }

    return std::make_pair(numPSSKb, numDirtyKb);
}

std::string getMemoryStats(FILE* file)
{
    const std::pair<std::size_t, std::size_t> pssAndDirtyKb = getPssAndDirtyFromSMaps(file);
    std::ostringstream oss;
    oss << "procmemstats: pid=" << getpid() << " pss=" << pssAndDirtyKb.first
        << " dirty=" << pssAndDirtyKb.second;
    LOG_TRC("Collected " << oss.str());
    return oss.str();
}

std::size_t getMemoryUsagePSS(const pid_t pid)
{
    if (pid > 0)
    {
        // beautifully aggregated data in a single entry:
        const auto cmd_rollup = "/proc/" + std::to_string(pid) + "/smaps_rollup";
        FILE* fp = fopen(cmd_rollup.c_str(), "r");
        if (!fp)
        {
            const auto cmd = "/proc/" + std::to_string(pid) + "/smaps";
            fp = fopen(cmd.c_str(), "r");
        }

        if (fp != nullptr)
        {
            const std::size_t pss = getPssAndDirtyFromSMaps(fp).first;
            fclose(fp);
            return pss;
        }
    }

    return 0;
}

std::size_t getProcessTreePss(pid_t pid)
{
    try
    {
        std::size_t pss = getMemoryUsagePSS(pid);

        const std::string root = "/proc/" + std::to_string(pid) + "/task/";

        std::vector<std::string> tids;
        Poco::File(root).list(tids);

        for (const std::string& tid : tids)
        {
            std::ifstream children(root + tid + "/children");
            std::string child;
            while (children >> child)
            {
                const pid_t childPid = NumUtil::i32FromString(child, 0);
                if (childPid > 0)
                {
                    pss += getProcessTreePss(childPid);
                }
            }
        }

        return pss;
    }
    catch (const std::exception& exc)
    {
        LOG_DBG("Exception while getting TreePss for PID [" << pid << "]: " << exc.what());
    }

    return 0;
}

std::size_t getMemoryUsageRSS(const pid_t pid)
{
    static const int pageSizeBytes = getpagesize();
    std::size_t rss = 0;

    if (pid > 0)
    {
        rss = getStatFromPid(pid, 23);
        rss *= pageSizeBytes;
        rss /= 1024;
        return rss;
    }
    return 0;
}

size_t getCurrentThreadCount()
{
    DIR* dir = opendir("/proc/self/task");
    if (!dir)
    {
        LOG_TRC("Failed to open /proc/self/task");
        return 0;
    }

    size_t threads = 0;
    struct dirent* it;
    while ((it = readdir(dir)) != nullptr)
    {
        if (it->d_name[0] == '.')
            continue;
        threads++;
    }
    closedir(dir);
    LOG_TRC("We have " << threads << " threads");
    return threads;
}

std::size_t getCpuUsage(const pid_t pid)
{
    if (pid > 0)
    {
        std::size_t totalJiffies = 0;
        totalJiffies += getStatFromPid(pid, 13);
        totalJiffies += getStatFromPid(pid, 14);
        return totalJiffies;
    }
    return 0;
}

std::size_t getStatFromPid(const pid_t pid, int ind)
{
    if (pid > 0)
    {
        const auto cmd = "/proc/" + std::to_string(pid) + "/stat";
        FILE* fp = fopen(cmd.c_str(), "r");
        if (fp != nullptr)
        {
            char line[4096] = { 0 };
            if (fgets(line, sizeof(line), fp))
            {
                const std::string s(line);
                int index = 1;
                std::size_t pos = s.find(' ');
                while (pos != std::string::npos)
                {
                    if (index == ind)
                    {
                        fclose(fp);
                        return NumUtil::u64FromString(&s[pos], 0);
                    }
                    ++index;
                    pos = s.find(' ', pos + 1);
                }
            }
            fclose(fp);
        }
    }
    return 0;
}

void setProcessAndThreadPriorities(const pid_t pid, int prio)
{
    int res = setpriority(PRIO_PROCESS, pid, prio);
    LOG_TRC("Lowered kit [" << (int)pid << "] priority: " << prio << " with result: " << res);

#ifdef __linux__
    // rely on Linux thread-id priority setting to drop this thread' priority
    pid_t tid = getThreadId();
    res = setpriority(PRIO_PROCESS, tid, prio);
    LOG_TRC("Lowered own thread [" << (int)tid << "] priority: " << prio
                                   << " with result: " << res);
#endif
}

} // namespace ProcUtil

namespace Util
{

// If OS is not mobile, it must be Linux.
std::string getLinuxVersion()
{
    // Read operating system info. We can read "os-release" file, located in /etc.
    std::ifstream ifs("/etc/os-release");
    std::string str(std::istreambuf_iterator<char>{ ifs }, {});
    std::vector<std::string> infoList = Util::splitStringToVector(str, '\n');
    std::map<std::string, std::string> releaseInfo = Util::stringVectorToMap(infoList, '=');

    auto it = releaseInfo.find("PRETTY_NAME");
    if (it != releaseInfo.end())
    {
        std::string name = it->second;

        // See os-release(5). It says that the lines are "environment-like shell-compatible
        // variable assignments". What that means, *exactly*, is up for debate, but probably
        // of mainly academic interest. (It does say that variable expansion at least is not
        // supported, that is a relief.)

        // The value of PRETTY_NAME might be quoted with double-quotes or
        // single-quotes.

        // FIXME: In addition, it might contain backslash-escaped special
        // characters, but we ignore that possibility for now.

        // FIXME: In addition, if it really does support shell syntax (except variable
        // expansion), it could for instance consist of multiple concatenated quoted strings (with no
        // whitespace inbetween), as in:
        // PRETTY_NAME="Foo "'bar'" mumble"
        // But I guess that is a pretty remote possibility and surely no other code that
        // reads /etc/os-release handles that like a proper shell, either.

        if (name.length() >= 2 && ((name[0] == '"' && name[name.length() - 1] == '"') ||
                                   (name[0] == '\'' && name[name.length() - 1] == '\'')))
            name = name.substr(1, name.length() - 2);
        return name;
    }
    else
    {
        return "unknown";
    }
}

#if defined(BUILDING_TESTS)
/// No-op implementation in the test programs
void alertAllUsers(const std::string&) {}

/// No-op implementation in the test programs
void alertAllUsers(const std::string&, const std::string&) {}
#endif

SysStopwatch::SysStopwatch() { restart(); }

void SysStopwatch::restart() { readTime(_startCPU, _startSys); }

void SysStopwatch::readTime(uint64_t& cpu, uint64_t& sys)
{
    cpu = 0;
    sys = 0;
#if defined __linux__
    struct rusage times;
    if (!getrusage(RUSAGE_SELF, &times))
    {
        cpu = uint64_t(times.ru_utime.tv_sec) * 1000000 + times.ru_utime.tv_usec;
        sys = uint64_t(times.ru_stime.tv_sec) * 1000000 + times.ru_stime.tv_usec;
    }
#endif
}

std::chrono::microseconds SysStopwatch::elapsedTime() const
{
    uint64_t nowCPU, nowSys;
    readTime(nowCPU, nowSys);
    uint64_t totalUs = (nowCPU - _startCPU) + (nowSys - _startSys);
    return std::chrono::microseconds(totalUs);
}

} // namespace Util

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
