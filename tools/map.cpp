/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* A debugging tool to detect un-shared pages between
 * forkit and its children */

#include <config.h>

#include <vector>
#include <iostream>
#include <sstream>
#include <sysexits.h>
#include <unordered_map>
#include <unordered_set>

#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#if defined(__GLIBC__)
#include <error.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <locale.h>
#include <math.h>

#include <common/HexUtil.hpp>
#include <common/NumUtil.hpp>
#include <common/Util.hpp>

#if !defined(__GLIBC__)
void error(int status, int errnum, const char *format, ...)
{
    va_list args;
    (void)status;
    (void)errnum;

    va_start(args, format);
    printf(format, args);
    va_end(args);
}
#endif

typedef unsigned long long addr_t;

static bool DumpHex = false;
static bool DumpAll = false;
static bool DumpMap = false;
static bool DumpStrings = false;
static int  DumpWidth = 32;

#define MAP_SIZE 21
#define PATH_SIZE 1000 // No harm in having it much larger than strictly necessary. Avoids compiler warning.
#define BUFFER_SIZE 9600

static size_t read_buffer(char *buffer, size_t size,
                          const char *file, char sep)
{
    int file_desc = open(file, O_RDONLY);
    if (file_desc == -1)
        return 0;

    size_t total_bytes = 0;
    size_t count = size;
    char *ptr = buffer;

    while (count)
    {
        ssize_t number_bytes;

        do {
            number_bytes = read(file_desc, ptr, count);
        } while (number_bytes < 0 && errno == EINTR);

        if (number_bytes < 0)
            break;

        ptr += number_bytes;
        total_bytes = ptr - buffer;

        if (total_bytes == size)
        {
            --total_bytes;
            break;
        }

        if (number_bytes==0)
            break;  // EOF

        count -= number_bytes;
    }

    close(file_desc);

    if (total_bytes)
    {
        size_t i = total_bytes;

        while (i--)
            if (buffer[i]=='\n' || buffer[i]=='\0')
                buffer[i]=sep;

        if (buffer[total_bytes-1]==' ')
            buffer[total_bytes-1]='\0';
    }

    buffer[total_bytes] = '\0';
    return total_bytes;
}

static int openPid(unsigned proc_id, const char *name)
{
    char path_proc[PATH_SIZE];
    snprintf(path_proc, sizeof(path_proc), "/proc/%d/%s", proc_id, name);
    int fd = open(path_proc, 0);
    if (fd < 0)
        error(EXIT_FAILURE, errno, "Failed to open %s", path_proc);
    return fd;
}

namespace {

struct Map {
private:
    addr_t _start;
    addr_t _end;
    std::string _name;
    mutable size_t _ptrCount;
public:
    Map(addr_t start, addr_t end, const std::string& name)
        : _start(start)
        , _end(end)
        , _name(name)
        , _ptrCount(0)
    {
    }

    addr_t getStart() const { return _start; }

    addr_t getEnd() const { return _end; }

    const std::string& getName() const { return _name; }

    std::string getShortName() const
    {
        auto lastSlash = _name.rfind('/');
        if (lastSlash != std::string::npos)
            return _name.substr(lastSlash+1, _name.length()-lastSlash);
        else
            return _name;
    }

    void bumpPointerCountInUnshared() const
    {
        _ptrCount++;
    }
    size_t getPtrCount() const { return _ptrCount; }

    size_t size() const { return _end - _start; }
};

struct StringData {
private:
    size_t _count;
    size_t _chars;

public:
    StringData() :
        _count(0),
        _chars(0)
    {}

    void setCount(size_t count) { _count = count; }

    size_t getCount() const { return _count; }

    void setChars(size_t chars) { _chars = chars; }

    size_t getChars() const { return _chars; }
};

struct AddrSpace {
private:
    unsigned _proc_id;
    std::vector<Map> _maps;
    std::unordered_map<addr_t, std::string> _addrToStr;
    StringData _strings[3];

public:
    AddrSpace(unsigned proc_id) :
        _proc_id(proc_id)
    {
    }
    void printStats()
    {
        char prefixes[] = { 'S', 'U', 'C' };
        for (int i = 0; i < 3; ++i)
        {
            printf("%cStrings      :%20lld, %lld chars\n",
                   prefixes[i], static_cast<addr_t>(_strings[i].getCount()),
                   static_cast<addr_t>(_strings[i].getChars()));
        }
    }

    const Map *findMap(addr_t addr) const
    {
        for (const Map &i : _maps)
        {
            if (i.getStart() <= addr && i.getEnd() > addr)
                return &i;
        }
        return nullptr;
    }

    std::string findName(addr_t page) const
    {
        auto map = findMap(page);
        return map ? map->getName() : std::string("");
    }

    std::vector<Map> getSortedMaps() const
    {
        std::vector<Map> sortedMaps = _maps;
        std::sort(sortedMaps.begin(), sortedMaps.end(),
                  [](const Map &a, const Map &b) { return a.getPtrCount() > b.getPtrCount(); });
        return sortedMaps;
    }

    void insert(addr_t start, addr_t end, const char *name)
    {
        _maps.emplace_back(start, end, std::string(name, 0, strlen(name) - 1));
    }

    // Normal OUString:
    // 20 00 00 00 00 00 00 00  02 00 00 00 05 00 00 00  4b 00 45 00 59 00 5f 00  55 00 00 00 00 00 00 00  |  ...............K.E.Y._.U......
    // 20 00 00 00 00 00 00 00  02 00 00 00 05 00 00 00  4b 45 59 5f 55 00 00 00  00 00 00 00 00 00 00 00  |  ...............KEY_U..........

    static bool isStringAtOffset(const std::vector<unsigned char> &data, size_t i,
                          uint32_t len, bool isUnicode, std::string &str)
    {
        str = std::string(isUnicode ? "U_" : "S_");
        int step = isUnicode ? 2 : 1;
        for (size_t j = i; j < i + len*step && j < data.size(); j += step)
        {
            if (isascii(data[j]) && !iscntrl(data[j]) &&
                (step == 1 || data[j+1] == 0))
                str += static_cast<char>(data[j]);
            else
                return false;
        }
        return true;
    }

    static bool isCStringAtOffset(const std::vector<unsigned char> &data, size_t i,
                           std::string &str, size_t &skip)
    {
        str = "C_";
        size_t j;
        for (j = i; j < data.size(); j++)
        {
            if (isascii(data[j]) && !iscntrl(data[j]))
                str += static_cast<char>(data[j]);
            else
                break;
        }
        if (data[j] == '\0' && str.length() > 7)
            return true;

        // avoid large chunks of non-null-terminated ASCII creating work.
        if (j > i + 4)
            skip = j - i - 4;

        return false;
    }

    void scanForSalStrings(Map &map, const std::vector<unsigned char> &data)
    {
        for (size_t i = 0; i < data.size() - 24; i += 4)
        {
            const uint32_t *p = reinterpret_cast<const uint32_t *>(&data[i]);
            uint32_t len;
            std::string str;
            if ((p[0] & 0xffffff) < 0x1000 && // plausible max ref-count
                (len = p[1]) < 0x100 &&     // plausible max string length
                len <= (data.size() - i) &&
                len > 2)
            {
                bool isUnicode = data[i+1] == 0 && data[i+3] == 0;
                if (isStringAtOffset(data, i + 8, len, isUnicode, str))
                {
                    StringData &sdata = _strings[isUnicode ? 1 : 0];
                    sdata.setCount(sdata.getCount() + 1);
                    sdata.setChars(sdata.getChars() + len);
                    _addrToStr[map.getStart() + i] = str;
                    i += ((4 + str.length() * (isUnicode ? 2 : 1)) >>2 ) * 4;
                }
            }
            size_t skip = 0;
            if ((i%8 == 0) && isCStringAtOffset(data, i, str, skip))
            {
                StringData &sdata = _strings[2];
                sdata.setCount(sdata.getCount() + 1);
                sdata.setChars(sdata.getChars() + str.length());
                _addrToStr[map.getStart() + i] = str;
                i += (str.length() >> 2) * 4;
            }
            else
                i += skip;
        }
    }

    void scanMapsForStrings()
    {
        int mem_fd = openPid(_proc_id, "mem");
        for (auto &map : _maps)
        {
            std::vector<unsigned char> data;
            data.resize (map.size());
            if (lseek(mem_fd, map.getStart(), SEEK_SET) < 0 ||
                read(mem_fd, data.data(), map.size()) != static_cast<int>(map.size()))
                error(EXIT_FAILURE, errno, "Failed to seek in /proc/%d/mem to %lld",
                      _proc_id, map.getStart());

            scanForSalStrings(map, data);
        }
        close (mem_fd);
    }

    const std::unordered_map<addr_t, std::string>& getAddrToStr() const { return _addrToStr; }
};

}


static void dumpDiff(const AddrSpace &space,
                     const std::vector<char> &pageData,
                     const std::vector<char> &parentData)
{
    assert(pageData.size() == parentData.size());

    const unsigned int width = DumpWidth;

    for (std::size_t i = 0; i < pageData.size(); i += width)
    {
        std::string page = HexUtil::stringifyHexLine(pageData, i, width);
        std::string parent = HexUtil::stringifyHexLine(parentData, i, width);

        // page
        printf("0x%.4zx  %s\n", i, page.c_str());

        // strings
        const addr_t *ptrs = reinterpret_cast<const addr_t *>(&pageData[i]);
        std::stringstream annots;
        bool haveAnnots = false;
        for (unsigned int j = 0; j < width/8; j++)
        {
            std::string str;
            const Map *map;
            auto it = space.getAddrToStr().find(ptrs[j]);
            if (it != space.getAddrToStr().end())
            {
                str = it->second;
                haveAnnots = true;
            }
            else if (DumpMap && (map = space.findMap(ptrs[j])))
            {
                str = map->getShortName();
                haveAnnots = (str != "[heap]") &&
                             (str != "[anon]") &&
                             (str != "[stack]");
                if (!haveAnnots)
                    str.clear();
                else
                    map->bumpPointerCountInUnshared();
            }

            str.resize(24, ' ');
            annots << str << ' ';
        }
        if (haveAnnots)
            printf ("annot:  %s\n", annots.str().c_str());

        // parent
        if (page != parent)
        {
            printf ("-par't- ");
            assert(page.length() == parent.length());
            for (size_t j = 0; j < page.length(); ++j)
            {
                if (page[j] == parent[j] && page[j] != '|')
                    printf(" ");
                else
                    printf("%c", parent[j]);
            }
            printf("\n");
        }
    }
}

static void dumpPages(unsigned proc_id, unsigned parent_id, const char *type, const std::vector<addr_t> &pages, const AddrSpace &space)
{
    int mem_fd = openPid(proc_id, "mem");
    int parent_fd = openPid(parent_id, "mem");

    if (DumpHex)
        printf ("\nUn-shared data dump\n");

    size_t cnt = 0;
    addr_t sameButUnshared = 0;
    addr_t bytesTouched = 0;
    for (addr_t page : pages)
    {
        std::vector<char> pageData, parentData;
        pageData.resize(0x1000);
        parentData.resize(0x1000);

        if (lseek(mem_fd, page, SEEK_SET) < 0)
            error(EXIT_FAILURE, errno, "Failed to seek in /proc/<pid>/mem to %lld", page);
        if (read(mem_fd, pageData.data(), 0x1000) != 0x1000)
            error(EXIT_FAILURE, errno, "Failed to read page %lld from /proc/<pid>/mem", page);

        if (lseek(parent_fd, page, SEEK_SET) < 0)
            parentData.resize(0);
        else if (read(parent_fd, parentData.data(), 0x1000) != 0x1000)
            parentData.resize(0); // missing equivalent page.

        int touched = 0;
        const char *style;
        if (parentData.size() > 0)
        {
            bool zeroParent = true;
            for (size_t i = 0; i < pageData.size(); ++i)
            {
                if (pageData[i] != parentData[i])
                    touched++;
                if (parentData[i] != 0)
                    zeroParent = false;
            }
            if (zeroParent)
            {
                style = "zero parent page";
                touched = 0; // ignore tedious diff.
            }
            else if (touched == 0)
            {
                if (DumpAll)
                    style = "dump unchanged";
                else
                {
                    style = "un-shared but matches parent";
                    sameButUnshared++;
                }
            }
            else
                style = "was shared";
        }
        else
            style = "unique";

        if (DumpHex || DumpMap)
        {
            printf ("\n%s page: 0x%.8llx (%d/%d) - touched: %d - %s - from %s\n",
                    type, page, static_cast<int>(++cnt), static_cast<int>(pages.size()), touched,
                    style, space.findName(page).c_str());

            if (touched == 0) // not present in parent
            {
                std::stringstream pageStr;
                HexUtil::dumpHex(pageStr, pageData, "", "", false, DumpWidth);
                printf("%s", pageStr.str().c_str());
            }
            else
                dumpDiff(space, pageData, parentData);
        }

        bytesTouched += touched;
    }

    close (mem_fd);
    close (parent_fd);

    printf ("\tsame but unshared     %6lld (%lldkB)\n", sameButUnshared, sameButUnshared * 4);
    printf ("\tdirtied bytes touched %6lld per page %.2f\n\n",
            bytesTouched, static_cast<double>(bytesTouched) / pages.size());

    if (DumpMap)
    {
        printf("number of shared library pointers in touched pages\n");
        int count = 0;
        for (const auto &i : space.getSortedMaps())
        {
            printf("\t%ld\t%s\n", i.getPtrCount(), i.getName().c_str());
            if (count++ > 16)
                break;
        }
    }
}

static std::vector<char> compressBitmap(const std::vector<char> &bitmap)
{
    std::vector<char> output;
    for (size_t i = 0, count = bitmap.size(); i < count; ++i)
    {
        char cur;
        int cnt = 0;
        size_t j = i;
        for (cur = bitmap[j]; j < count && bitmap[j] == cur; ++j)
            ++cnt;
        output.push_back(cur);
        if (cnt > 3)
        {
            char num[16];
            output.push_back('[');
            snprintf(num, sizeof(num), "%d", cnt);
            for (int cpy = 0; num[cpy] != '\0'; ++cpy)
                output.push_back(num[cpy]);
            output.push_back(']');
            i += cnt - 1;
        }
    }

    output.push_back('\0');
    return output;
}

static void dump_unshared(unsigned proc_id, unsigned parent_id,
                          const char *type, const std::vector<addr_t> &vaddrs,
                          const AddrSpace & space)
{
    int fd = openPid(proc_id, "pagemap");

    std::vector<char> bitmap;
    std::vector<addr_t> vunshared;
    addr_t numShared = 0, numOwn = 0;
    for (addr_t p : vaddrs)
    {
        if (lseek(fd, (p / 0x1000 * 8), SEEK_SET) < 0)
            error(EXIT_FAILURE, errno, "Failed to seek in pagemap");
        addr_t vaddrData;
        if (read(fd, &vaddrData, sizeof(addr_t)) != sizeof(addr_t))
            error(EXIT_FAILURE, errno, "Failed to read vaddrdata");
        {
            // https://patchwork.kernel.org/patch/6787921/
//            fprintf(stderr, "addr: 0x%8llx bits: 0x%8llx : %s\n", p, vaddrData,
//                    (vaddrData & ((addr_t)1 << 56)) ? "unshared" : "shared");
            if (vaddrData & (static_cast<addr_t>(1) << 56))
            {
                numOwn++;
                bitmap.push_back('*');
                vunshared.push_back(p);
            }
            else
            {
                numShared++;
                bitmap.push_back('.');
                if (DumpAll)
                    vunshared.push_back(p);
            }
        }
    }
    close (fd);

    printf ("Totals for %s\n", type);
    printf ("\tshared   %5lld (%lldkB)\n", numShared, numShared * 4);
    printf ("\tunshared %5lld (%lldkB)\n", numOwn, numOwn * 4);

    std::vector<char> compressed = compressBitmap(bitmap);
    printf ("\tRLE sharing bitmap:\n%s\n", compressed.data());

    dumpPages(proc_id, parent_id, type, vunshared, space);

    std::unordered_set<addr_t> unShared;
    for (addr_t addr : vunshared)
        unShared.insert(addr);

    if (DumpStrings)
    {
        printf("String dump:\n");
        for (const auto& addr : space.getAddrToStr())
        {
            if (DumpAll ||
                unShared.find(addr.first & ~0x1fff) != unShared.end())
                printf("0x%.16llx %s\n", addr.first, addr.second.c_str());
        }

        printf("String dump ends.\n");
    }
}

static void total_smaps(unsigned proc_id, unsigned parent_id,
                        const char *file, const char *cmdline)
{
    FILE *file_pointer;
    char buffer[BUFFER_SIZE];

    addr_t total_private_dirty = 0ull;
    addr_t total_private_clean = 0ull;
    addr_t total_shared_dirty = 0ull;
    addr_t total_shared_clean = 0ull;
    addr_t smap_value;
    char smap_key[MAP_SIZE];

    AddrSpace space(proc_id);

    std::vector<addr_t> heapVAddrs, anonVAddrs, fileVAddrs;
    std::vector<addr_t> *pushTo = nullptr;

    if ((file_pointer = fopen(file, "r")) == nullptr)
        error(EXIT_FAILURE, errno, "%s", file);

    while (fgets(buffer, sizeof(buffer), file_pointer))
    {
        // collect heap page details
        if (strstr(buffer, "[heap]"))
            pushTo = &heapVAddrs;
        else if (strstr(buffer, "/"))
            pushTo = &fileVAddrs;
        else
            pushTo = &anonVAddrs;

        if (strstr(buffer, " rw-p "))
        {
            addr_t start, end;
            // 012d0000-0372f000 rw-p 00000000 00:00 0  [heap]
            if (sscanf(buffer, "%llx-%llx rw-p", &start, &end) == 2)
            {
                const char *name = strchr(buffer, '[');
                if (!name)
                    name = strchr(buffer, '/');
                if (!name)
                    name = "[anon]\n";
                space.insert(start, end, name);
                // coverity[tainted_data : FALSE] - this is entirely intentional
                for (addr_t p = start; p < end; p += 0x1000)
                    pushTo->push_back(p);
            }
            else
                fprintf (stderr, "malformed heap line '%s'\n", buffer);
        }

        if (buffer[0] >= 'A' && buffer[0] <= 'Z')
        {
            if (sscanf(buffer, "%20[^:]: %llu", smap_key, &smap_value) == 2)
            {
                if (strncmp("Shared_Dirty", smap_key, 12) == 0)
                {
                    total_shared_dirty += smap_value;
                    continue;
                }
                if (strncmp("Shared_Clean", smap_key, 12) == 0)
                {
                    total_shared_clean += smap_value;
                    continue;
                }
                if (strncmp("Private_Dirty", smap_key, 13) == 0)
                {
                    total_private_dirty += smap_value;
                    continue;
                }
                if (strncmp("Private_Clean", smap_key, 13) == 0)
                {
                    total_private_clean += smap_value;
                    continue;
                }
            }
        }
    }
    fclose(file_pointer);
    space.scanMapsForStrings();

    printf("%s\n", cmdline);
    printf("Process ID    :%20d\n", proc_id);
    printf(" parent ID    :%20d\n", parent_id);
    printf("--------------------------------------\n");
    printf("Shared Clean  :%20lld kB\n", total_shared_clean);
    printf("Shared Dirty  :%20lld kB\n", total_shared_dirty);
    printf("Private Clean :%20lld kB\n", total_private_clean);
    printf("Private Dirty :%20lld kB\n", total_private_dirty);
    printf("--------------------------------------\n");
    printf("Shared        :%20lld kB\n", total_shared_clean + total_shared_dirty);
    printf("Private       :%20lld kB\n", total_private_clean + total_private_dirty);
    printf("--------------------------------------\n");
    printf("Heap page cnt :%20lld\n", static_cast<addr_t>(heapVAddrs.size()));
    printf("Anon page cnt :%20lld\n", static_cast<addr_t>(anonVAddrs.size()));
    printf("File page cnt :%20lld\n", static_cast<addr_t>(fileVAddrs.size()));
    printf("--------------------------------------\n");
    space.printStats();
    printf("\n");

    dump_unshared(proc_id, parent_id, "heap", heapVAddrs, space);
    dump_unshared(proc_id, parent_id, "anon", anonVAddrs, space);
    dump_unshared(proc_id, parent_id, "file", fileVAddrs, space);
}

static unsigned getParent(int proc_id)
{
    int fd = openPid(proc_id, "stat");

    char buffer[4096];
    int len;
    if ((len = read(fd, buffer, sizeof (buffer) - 1)) < 0)
        error(EXIT_FAILURE, errno, "Failed to read /proc/%d/stat", proc_id);
    close (fd);
    buffer[len] = '\0';

    char state, cmd[4096];
    unsigned unused, ppid = 0;
    if (sscanf(buffer, "%d %s %c %d", &unused, cmd, &state, &ppid) != 4 || ppid == 0)
    {
        fprintf(stderr, "Failed to locate parent from "
                "/proc/%d/stat : '%s'\n", proc_id, buffer);
        exit (1);
    }

    return ppid;
}

int main(int argc, char **argv)
{
    DIR *root_proc;
    struct dirent *dir_proc;

    char path_proc[PATH_SIZE];
    char cmdline[BUFFER_SIZE];

    bool found = false;
    bool help = false;
    unsigned forPid = 0;
    const char *appOrPid = nullptr;

    setlocale (LC_ALL, "");

    for (int i = 1; i < argc; ++i)
    {
        const char *arg = argv[i];
        if (strstr(arg, "--help"))
            help = true;
        else if (strstr(arg, "--hex"))
            DumpHex = true;
        else if (strstr(arg, "--map"))
            DumpMap = true;
        else if (strstr(arg, "--all"))
            DumpAll = true;
        else if (strstr(arg, "--strings"))
            DumpStrings = true;
        else if (strstr(arg, "--width"))
        {
            DumpWidth = std::max(static_cast<int>(pow(2, round(log(atoi(argv[++i]))/log(2)))), 8);
        }
        else
            appOrPid = arg;
    }
    if (appOrPid == nullptr && forPid == 0)
        help = true;

    if (help)
    {
        fprintf(stderr, "Usage: coolmap --hex <name of process|pid>\n");
        fprintf(stderr, "Dump memory map information for a given process\n");
        fprintf(stderr, "    --hex           Hex dump relevant page contents and diff to parent process\n");
        fprintf(stderr, "    --map           Print address into their memory maps\n");
        fprintf(stderr, "    --strings       Print all detected strings\n");
        fprintf(stderr, "    --all           Hex dump all writable pages whether touched or not\n");
        fprintf(stderr, "    --width <bytes> Define width of hex dump in bytes, rounded to a power of 2\n");
        return 0;
    }

    forPid = atoi(appOrPid);

    root_proc = opendir("/proc");
    if (!root_proc)
        error(EXIT_FAILURE, errno, "%s", "/proc");

    while ((dir_proc = readdir(root_proc)))
    {
        if (!dir_proc && !dir_proc->d_name[0])
            error(EXIT_FAILURE, ENOTDIR, "bad dir");

        if (*dir_proc->d_name > '0' && *dir_proc->d_name <= '9')
        {
            const unsigned pid_proc = NumUtil::u64FromString(dir_proc->d_name, 0);

            snprintf(path_proc, sizeof(path_proc), "/proc/%s/%s", dir_proc->d_name, "cmdline");
            if (read_buffer(cmdline, sizeof(cmdline), path_proc, ' ') &&
                (forPid == pid_proc || (forPid == 0 && strstr(cmdline, appOrPid) && !strstr(cmdline, argv[0]))))
            {
                unsigned parent_id = getParent(pid_proc);
                snprintf(path_proc, sizeof(path_proc), "/proc/%s/%s", dir_proc->d_name, "smaps");
                total_smaps(pid_proc, parent_id, path_proc, cmdline);
                found = true;
            }
        }
    }

    if (!found)
        fprintf(stderr, "Failed to find process %s\n", appOrPid);

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
