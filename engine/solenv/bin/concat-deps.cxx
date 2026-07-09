/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *    Copyright (C) 2011 Norbert Thiebaud
 *    License: GPLv3
 */

#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

/* On Windows cl lower-cases some paths, so include prefixes are compared
   case-insensitively there. */
#ifdef _MSC_VER
#define PATHNCMP _strnicmp
#else
#define PATHNCMP strncmp
#endif

/* True unless SYSTEM_BOOST is "TRUE": when the bundled boost is in use, its
   many headers are collapsed to a single unpacked target. */
static bool internal_boost = false;

/* WORKDIR. */
static std::string work_dir;

/* SRCDIR, BUILDDIR and WORKDIR with every backslash turned into a forward
   slash. cl /sourceDependencies emits forward-slash-normalised include paths,
   and these copies let a plain prefix compare decide whether an include lives
   inside the build tree. build_dir_fwd stays empty when BUILDDIR is not set. */
static std::string src_dir_fwd;
static std::string build_dir_fwd;
static std::string work_dir_fwd;

/* Return a copy of s with every backslash turned into a forward slash. An empty
   string comes back for a null pointer, so an unset environment variable and an
   empty one are treated alike. */
static std::string dup_forward_slashes(const char* s)
{
    std::string result;
    if (s)
    {
        result = s;
        for (char& c : result)
        {
            if (c == '\\')
                c = '/';
        }
    }
    return result;
}

/* Load the whole regular file name into out. Returns false when the file is
   missing, is not a regular file, or cannot be read in full. */
static bool file_load(const std::string& name, std::string& out)
{
    std::error_code ec;
    const std::filesystem::file_status status = std::filesystem::status(name, ec);
    if (ec || !std::filesystem::is_regular_file(status))
        return false;
    const std::uintmax_t size = std::filesystem::file_size(name, ec);
    if (ec)
        return false;

    std::ifstream stream(name, std::ios::binary);
    if (!stream.is_open())
        return false;

    out.resize(static_cast<size_t>(size));
    if (size > 0)
    {
        stream.read(&out[0], static_cast<std::streamsize>(size));
        if (static_cast<std::uintmax_t>(stream.gcount()) != size)
        {
            out.clear();
            return false;
        }
    }
    return true;
}

static void cancel_relative(const char* base, char** ref_cursor, char** ref_cursor_out,
                            const char* end)
{
    char* cursor = *ref_cursor;
    char* cursor_out = *ref_cursor_out;

    do
    {
        cursor += 3;
        while (cursor_out > base && cursor_out[-1] == '/')
            cursor_out--;
        while (cursor_out > base && *--cursor_out != '/')
            ;
    } while (cursor + 3 < end && std::string_view(cursor, 4) == "/../");
    *ref_cursor = cursor;
    *ref_cursor_out = cursor_out;
}

static void eat_space(const char** token)
{
    while (' ' == **token || '\t' == **token)
        ++(*token);
}

/*
 * Prune LibreOffice specific duplicate dependencies to improve
 * gnumake startup time, and shrink the disk-space footprint.
 */
static int elide_dependency(const char* key, int key_len, const char** unpacked_end)
{
    /* boost brings a plague of header files */
    int unpacked = 0;
    /* walk down path elements */
    for (int i = 0; i < key_len - 1; i++)
    {
        if (key[i] == '/')
        {
            if (0 == unpacked)
            {
                if (!PATHNCMP(key + i + 1, "workdir/", 8))
                {
                    unpacked = 1;
                    continue;
                }
            }
            else
            {
                if (!PATHNCMP(key + i + 1, "UnpackedTarball/", 16))
                {
                    if (unpacked_end)
                        *unpacked_end = strchr(key + i + 17, '/');
                    return 1;
                }
            }
        }
    }

    return 0;
}

/*
 * We collapse tens of internal boost headers to the unpacked target, such
 * that you can re-compile / install boost and all is well.
 */
static void emit_single_boost_header(void)
{
    std::cout << work_dir << "/UnpackedTarball/boost.done ";
}

static void emit_unpacked_target(const char* token, const char* end)
{
    std::cout.write(token, end - token);
    std::cout << ".done ";
}

/* prefix paths to absolute */
static void print_fullpaths(const char* line)
{
    int boost_count = 0;
    const char* unpacked_end = nullptr; /* end of UnpackedTarget match (if any) */
    /* for UnpackedTarget the target is GenC{,xx}Object, don't mangle! */
    int target_seen = 0;

    const char* token = line;
    eat_space(&token);
    while (*token)
    {
        const char* end = token;
        /* hard to believe that in this day and age drive letters still exist */
        if (*end && (':' == *(end + 1)) && (('\\' == *(end + 2)) || ('/' == *(end + 2)))
            && isalpha(static_cast<unsigned char>(*end)))
        {
            end = end + 3; /* only one cross, err drive letter per filename */
        }
        while (*end && (' ' != *end) && ('\t' != *end) && (':' != *end))
            ++end;
        const int token_len = static_cast<int>(end - token);
        if (target_seen && elide_dependency(token, token_len, &unpacked_end))
        {
            if (unpacked_end)
            {
                if (internal_boost && !PATHNCMP(unpacked_end - 5, "boost", 5))
                {
                    ++boost_count;
                    if (boost_count == 1)
                        emit_single_boost_header();
                    else
                    {
                        /* don't output, and swallow trailing \\\n if any */
                        token = end;
                        eat_space(&token);
                        if (token[0] == '\\' && token[1] == '\n')
                            end = token + 2;
                    }
                }
                else
                {
                    emit_unpacked_target(token, unpacked_end);
                }
                unpacked_end = nullptr;
            }
        }
        else
        {
            std::cout.write(token, token_len);
            if (!std::cout)
                abort();
            std::cout.put(' ');
        }
        token = end;
        eat_space(&token);
        if (!target_seen && ':' == *token)
        {
            target_seen = 1;
            std::cout.put(':');
            ++token;
            eat_space(&token);
        }
    }
}

/* end points at the terminating NUL of a rewritten rule. Return a pointer to
   the last character that is not trailing whitespace or a colon, stopping at
   the start of the rule so an all-space rule cannot walk before base. */
static const char* eat_space_at_end(const char* end, const char* base)
{
    const char* real_end = end - 1;
    while (real_end > base
           && (' ' == *real_end || '\t' == *real_end || '\n' == *real_end || ':' == *real_end))
    { /* eat colon and whitespace at end, stopping at the start of the rule */
        --real_end;
    }
    return real_end;
}

/* Build workdir/<rel>, where rel is the path below workdir/Dep/, that is
   <Class>/<stem>.<ext>. Sets last_dot to the index of the last dot in the rel
   part so the caller can replace the extension, or npos when there is none. */
static std::string object_target_from_relative(const char* rel, size_t& last_dot)
{
    std::string dest = work_dir;
    dest += '/';
    last_dot = std::string::npos;
    for (const char* s = rel; *s != 0; ++s)
    {
        dest += *s;
        if (*s == '.')
            last_dot = dest.size() - 1;
    }
    return dest;
}

static std::string generate_phony_line(const char* phony_target, const char* extension)
{
    size_t last_dot;
    std::string line = object_target_from_relative(phony_target, last_dot);
    if (last_dot != std::string::npos)
        line.resize(last_dot + 1); /* keep through the dot, drop the old extension */
    else
        line += '.';
    line += extension;
    line += ": $(gb_Helper_PHONY)\n";
    return line;
}

static bool generate_phony_file(const char* fn, const std::string& content)
{
    std::ofstream depfile(fn, std::ios::binary);
    if (!depfile.is_open())
    {
        std::cerr << "Could not open '" << fn << "' for writing: " << strerror(errno) << '\n';
        return false;
    }
    depfile.write(content.data(), static_cast<std::streamsize>(content.size()));
    return true;
}

/* Write the per-object dep-file as a single phony rule, picking the object
   class from the path. Returns the phony line, or an empty string when the path
   is not a known per-object dep-file or the file could not be written. */
static std::string write_phony_dep_file(const char* fn)
{
    if (strncmp(fn, work_dir.c_str(), work_dir.size()) != 0)
        return std::string();
    if (strncmp(fn + work_dir.size(), "/Dep/", 5) != 0)
        return std::string();
    const char* src_relative = fn + work_dir.size() + 5;
    /* cases ordered by frequency */
    if (strncmp(src_relative, "CxxObject/", 10) == 0
        || strncmp(src_relative, "GenCxxObject/", 13) == 0
        || strncmp(src_relative, "CObject/", 8) == 0
        || strncmp(src_relative, "GenCObject/", 11) == 0
        || strncmp(src_relative, "SdiObject/", 10) == 0
        || strncmp(src_relative, "AsmObject/", 10) == 0
        || strncmp(src_relative, "ObjCxxObject/", 13) == 0
        || strncmp(src_relative, "ObjCObject/", 11) == 0
        || strncmp(src_relative, "GenObjCxxObject/", 16) == 0
        || strncmp(src_relative, "GenObjCObject/", 14) == 0
        || strncmp(src_relative, "GenAsmObject/", 13) == 0
        || strncmp(src_relative, "GenNasmObject/", 14) == 0
        || strncmp(src_relative, "CxxClrObject/", 13) == 0
        || strncmp(src_relative, "GenCxxClrObject/", 16) == 0)
    {
        std::string created_line = generate_phony_line(src_relative, "o");
        if (!generate_phony_file(fn, created_line))
            return std::string();
        return created_line;
    }
    std::cerr << "no magic for " << fn << "(" << src_relative << ") in " << work_dir << '\n';
    return std::string();
}

/* Decide whether a forward-slash include path lies inside the source tree, the
   build tree or the work directory. System headers (for instance under Program
   Files) live outside all three and are dropped, matching the allowlist that
   filter-showIncludes.awk applies. On Windows PATHNCMP is case-insensitive, so
   the lower-cased paths cl emits still match the mixed-case prefixes. */
static bool include_in_build_tree(const std::string& path)
{
    const std::string* prefixes[] = { &src_dir_fwd, &build_dir_fwd, &work_dir_fwd };
    for (const std::string* prefix : prefixes)
    {
        if (!prefix->empty() && path.size() >= prefix->size()
            && PATHNCMP(path.c_str(), prefix->c_str(), prefix->size()) == 0)
            return true;
    }
    return false;
}

/* Map a dependency fragment path workdir/Dep/<Class>/<stem>.d to the object it
   describes, workdir/<Class>/<stem>.o, in out. Returns false when fn is not
   under workdir/Dep. */
static bool object_from_dep_path(const char* fn, std::string& out)
{
    if (strncmp(fn, work_dir.c_str(), work_dir.size()) != 0
        || strncmp(fn + work_dir.size(), "/Dep/", 5) != 0)
        return false;
    const char* rel = fn + work_dir.size() + 5;
    size_t last_dot;
    out = object_target_from_relative(rel, last_dot);
    if (last_dot != std::string::npos)
    {
        out.resize(last_dot);
        out += ".o";
    }
    return true;
}

static const char* json_skip_ws(const char* p, const char* end)
{
    while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'))
        ++p;
    return p;
}

/* Read the JSON string that begins at the opening quote at p into out, undoing
   JSON escaping and turning backslashes into forward slashes. Advances p past
   the closing quote. Returns false when the string is malformed. */
static bool json_read_string(const char*& p, const char* end, std::string& out)
{
    out.clear();
    if (p >= end || *p != '"')
        return false;
    ++p;
    while (p < end && *p != '"')
    {
        char c = *p++;
        if (c == '\\' && p < end)
        {
            char e = *p++;
            switch (e)
            {
                case 'n':
                    c = '\n';
                    break;
                case 't':
                    c = '\t';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 'b':
                    c = '\b';
                    break;
                case 'f':
                    c = '\f';
                    break;
                case 'u':
                    /* cl does not put \u escapes in paths. Skip the four hex
                       digits so the rest of the string stays in step. */
                    if (p + 4 <= end)
                        p += 4;
                    continue;
                default:
                    c = e;
                    break; /* covers \\ \" \/ */
            }
        }
        if (c == '\\')
            c = '/';
        out.push_back(c);
    }
    if (p >= end)
        return false;
    ++p;
    return true;
}

/* Append path to w, escaping every space with a backslash so make treats the
   path as a single prerequisite. */
static void append_escaped(std::string& w, const std::string& path)
{
    for (char c : path)
    {
        if (c == ' ')
            w.push_back('\\');
        w.push_back(c);
    }
}

/* Walk the "Includes" array in json once. Every header inside the build tree is
   appended to includes, already make-escaped. Returns false when the JSON is
   malformed. */
static bool collect_includes(const char* json, const char* end, std::vector<std::string>& includes)
{
    const char* p = strstr(json, "\"Includes\"");
    if (!p)
        return true;
    p += 10;
    p = json_skip_ws(p, end);
    if (p >= end || *p != ':')
        return false;
    ++p;
    p = json_skip_ws(p, end);
    if (p >= end || *p != '[')
        return false;
    ++p;
    std::string path;
    for (;;)
    {
        p = json_skip_ws(p, end);
        if (p >= end || *p != '"')
            break;
        if (!json_read_string(p, end, path))
            return false;
        if (!path.empty() && include_in_build_tree(path))
        {
            std::string escaped;
            append_escaped(escaped, path);
            includes.push_back(std::move(escaped));
        }
        p = json_skip_ws(p, end);
        if (p < end && *p == ',')
            ++p;
    }
    return true;
}

/* Turn a cl /sourceDependencies JSON fragment into the make-syntax dependency
   text that filter-showIncludes.awk used to produce, so the rest of process()
   can treat it exactly like a classic .d file. Fills out and returns true, or
   returns false on failure. */
static bool convert_source_deps_json(const char* json, size_t json_size, const char* fn,
                                     std::string& out)
{
    const char* const end = json + json_size;

    std::string object;
    if (!object_from_dep_path(fn, object))
        return false;

    out.clear();

    /* target line: "<object> : \" */
    append_escaped(out, object);
    out += " : \\\n";

    /* source as the first prerequisite */
    const char* p = strstr(json, "\"Source\"");
    if (p)
    {
        p += 8;
        p = json_skip_ws(p, end);
        if (p < end && *p == ':')
        {
            ++p;
            p = json_skip_ws(p, end);
            std::string source;
            if (!json_read_string(p, end, source))
                return false;
            if (!source.empty())
            {
                out += ' ';
                append_escaped(out, source);
                out += " \\\n";
            }
        }
    }

    /* Each in-tree header appears twice: once as a prerequisite of the object
       and once as its own no-dependency rule (fdo#40099, so a deleted header
       does not stop make). Collect them once, then write both sections. */
    std::vector<std::string> includes;
    if (!collect_includes(json, end, includes))
        return false;

    /* the rest of the prerequisites, one " <header> \" continuation line each */
    for (const std::string& inc : includes)
    {
        out += ' ';
        out += inc;
        out += " \\\n";
    }

    /* a blank line ends the object's rule, then a "<header> :" rule per header */
    out += '\n';
    for (const std::string& inc : includes)
    {
        out += inc;
        out += " :\n";
    }
    return true;
}

static int process(std::unordered_set<std::string>& dep_hash, const char* fn)
{
    /* cl /sourceDependencies writes the include list as JSON to a file named
       like the dep-file with an extra .json suffix, and leaves the dep-file
       alone. Read that JSON when it is there. An older concat-deps opens only
       the dep-file, never the JSON, so an old and a new build can share one
       build directory. */
    std::string buffer;
    bool is_json = false;

    if (file_load(std::string(fn) + ".json", buffer))
        is_json = true;
    else if (!file_load(fn, buffer))
    {
        /* Neither the JSON sidecar nor the dep-file itself could be read. */
        std::string created_line = write_phony_dep_file(fn);
        if (created_line.empty())
            return 1;
        std::cout << created_line << '\n';
        return 0;
    }

    /* The first version of the /sourceDependencies path wrote the JSON into the
       dep-file itself, so a leading '{' in the dep-file is read as JSON too. A
       classic .d always starts with a path. */
    if (!is_json)
    {
        const char* probe = buffer.c_str();
        while (*probe == ' ' || *probe == '\t' || *probe == '\n' || *probe == '\r')
            ++probe;
        is_json = (*probe == '{');
    }
    if (is_json)
    {
        std::string converted;
        if (!convert_source_deps_json(buffer.c_str(), buffer.size(), fn, converted))
        {
            std::cerr << "concat-deps: could not convert JSON dependencies in " << fn << '\n';
            return 1;
        }
        buffer.swap(converted);
        /* Leave a per-object dep-file behind so make finds the prerequisite and
           stops re-reading the included dep-files. Only a phony rule, and only
           when nothing is there yet: the real header list already goes to the
           aggregated dep-file this run writes, and a phony file makes an old or
           stale concat-deps rebuild the object rather than trust a list it
           cannot read. */
        std::error_code ec;
        if (!std::filesystem::exists(fn, ec))
            write_phony_dep_file(fn);
    }

    /* The parser rewrites the dependency text in place: it collapses "/./" and
       "/../", joins continuation lines, and emits each complete rule. Keys for
       the duplicate filter are copied into dep_hash, so buffer can be freed when
       we return. */
    const size_t size = buffer.size();
    buffer.push_back('\0'); /* a writable slot for the NUL the parser stores */
    char* const data = &buffer[0];
    char* const end = data + size;

    char* cursor = data;
    char* cursor_out = data;
    char* base = data;
    int continuation = 0;
    char last_ns = 0;

    /* first eat unneeded space at the beginning of file */
    while (cursor < end && (*cursor == ' ' || *cursor == '\\'))
        ++cursor;

    while (cursor < end)
    {
        if (*cursor == '\\')
        {
            continuation = 1;
            *cursor_out++ = *cursor++;
        }
        else if (*cursor == '/')
        {
            if (cursor + 2 < end && std::string_view(cursor, 3) == "/./")
            {
                cursor += 2;
                continue;
            }
            if (cursor + 3 < end && std::string_view(cursor, 4) == "/../")
            {
                cancel_relative(base, &cursor, &cursor_out, end);
                // cancel_relative leaves cursor on the trailing '/' of the
                // consumed "/../". Re-examine it so a following "/./" or "/../"
                // is also collapsed. Without this a sequence like ".././../"
                // loses one level and names a directory that does not exist.
                continue;
            }
            *cursor_out++ = *cursor++;
        }
        else if (*cursor == '\n')
        {
            if (!continuation)
            {
                *cursor_out = 0;
                if (base < cursor)
                {
                    /* here we have a complete rule */
                    if (last_ns == ':')
                    {
                        /* if the rule ended in ':' that is a no-dep rule
                         * these are the one for which we want to filter
                         * duplicate out
                         */
                        int key_len = static_cast<int>(eat_space_at_end(cursor_out, base) - base);
                        if (!elide_dependency(base, key_len + 1, nullptr)
                            && dep_hash.insert(std::string(base, static_cast<size_t>(key_len)))
                                   .second)
                        {
                            print_fullpaths(base);
                            std::cout.put('\n');
                        }
                    }
                    else
                    {
                        /* rule with dep, just write it */
                        print_fullpaths(base);
                        std::cout.put('\n');
                    }
                    last_ns = ' '; // cannot hurt to reset it
                }
                cursor += 1;
                base = cursor_out = cursor;
            }
            else
            {
                /* here we have a '\' followed by \n this is a continuation
                 * i.e not a complete rule yet
                 */
                *cursor_out++ = *cursor++;
                continuation = 0; // cancel current one (empty lines!)
            }
        }
        else
        {
            continuation = 0;
            /* not using isspace() here save 25% of I refs and 75% of D refs based on cachegrind */
            if (*cursor != ' ' && *cursor != '\n' && *cursor != '\t')
                last_ns = *cursor;
            *cursor_out++ = *cursor++;
        }
    }

    /* just in case the file did not end with a \n, there may be a pending rule */
    if (base < cursor_out)
    {
        *cursor_out = 0;
        if (last_ns == ':')
        {
            int key_len = static_cast<int>(eat_space_at_end(cursor_out, base) - base);
            if (!elide_dependency(base, key_len + 1, nullptr)
                && dep_hash.insert(std::string(base, static_cast<size_t>(key_len))).second)
            {
                std::cout << base << '\n';
                std::cout.put('\n');
            }
        }
        else
        {
            std::cout << base << '\n';
            std::cout.put('\n');
        }
    }
    return 0;
}

static void usage(void) { std::cerr << "Usage: concat-deps <file that contains dep_files>\n"; }

static const char* get_var(const char* name)
{
    const char* value = getenv(name);
    if (!value)
        std::cerr << "Error: " << name << " is missing in the environment\n";
    return value;
}

int main(int argc, char** argv)
{
    // All output goes through std::cout/std::cerr, so decouple them from the C
    // stdio streams to keep the per-token writes in the hot path cheap.
    std::ios_base::sync_with_stdio(false);

    if (argc < 2)
    {
        usage();
        return 1;
    }

    const char* srcdir = get_var("SRCDIR");
    if (!srcdir)
        return 1;
    const char* workdir = get_var("WORKDIR");
    if (!workdir)
        return 1;
    work_dir = workdir;

    /* BUILDDIR is optional here. It only helps keep cl /sourceDependencies deps
       trimmed to the build tree. The forward-slash copies feed the include
       allowlist. */
    build_dir_fwd = dup_forward_slashes(getenv("BUILDDIR"));
    src_dir_fwd = dup_forward_slashes(srcdir);
    work_dir_fwd = dup_forward_slashes(workdir);

    const char* env_str = getenv("SYSTEM_BOOST");
    internal_boost = !env_str || strcmp(env_str, "TRUE") != 0;

    std::string in_list;
    if (!file_load(argv[1], in_list))
        return 1;

    std::unordered_set<std::string> dep_hash;
    dep_hash.reserve(4096);

    int rc = 0;
    char* const data = in_list.empty() ? nullptr : &in_list[0];
    char* base = data;
    char* cursor = data;

    /* extract filename of dep file from a 'space' separated list */
    while (cursor && *cursor)
    {
        /* the input here may contain Win32 \r\n EOL */
        if (*cursor == ' ' || *cursor == '\n' || *cursor == '\r')
        {
            *cursor = 0;
            if (base < cursor)
            {
                rc = process(dep_hash, base);
                if (rc)
                    break;
            }
            ++cursor;
            base = cursor;
        }
        else
        {
            ++cursor;
        }
    }
    /* catch the last entry in case the input did not terminate with a 'space' */
    if (!rc && base && base < cursor)
        rc = process(dep_hash, base);

    return rc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
