/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *    Copyright (C) 2011 Norbert Thiebaud
 *    License: GPLv3
 */

/* define to activate stats reporting on hash usage*/
/* #define HASH_STAT */

/* ===============================================
 * Set-up: defines to identify the system and system related properties
 * ===============================================
 */

#ifdef __APPLE__
#ifdef __x86_64__
#undef CORE_BIG_ENDIAN
#define CORE_LITTLE_ENDIAN
#else
#define CORE_BIG_ENDIAN
#undef CORE_LITTLE_ENDIAN
#endif

#endif

#ifdef _MSC_VER
#undef CORE_BIG_ENDIAN
#define CORE_LITTLE_ENDIAN
#endif /* Def _MSC_VER */

#if defined(__linux) || defined(__FreeBSD__)
#include <sys/param.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#undef CORE_BIG_ENDIAN
#define CORE_LITTLE_ENDIAN
#else /* !(__BYTE_ORDER == __LITTLE_ENDIAN) */
#if __BYTE_ORDER == __BIG_ENDIAN
#define CORE_BIG_ENDIAN
#undef CORE_LITTLE_ENDIAN
#endif /* __BYTE_ORDER == __BIG_ENDIAN */
#endif /* !(__BYTE_ORDER == __LITTLE_ENDIAN) */
#endif /* Def __linux */

#if defined(__OpenBSD__) || defined(__FreeBSD__) || \
    defined(__NetBSD__) || defined(__DragonFly__)
#include <machine/endian.h>
#if _BYTE_ORDER == _LITTLE_ENDIAN
#undef CORE_BIG_ENDIAN
#define CORE_LITTLE_ENDIAN
#else /* !(_BYTE_ORDER == _LITTLE_ENDIAN) */
#if _BYTE_ORDER == _BIG_ENDIAN
#define CORE_BIG_ENDIAN
#undef CORE_LITTLE_ENDIAN
#endif /* _BYTE_ORDER == _BIG_ENDIAN */
#endif /* !(_BYTE_ORDER == _LITTLE_ENDIAN) */
#endif /* Def *BSD */

#if defined(__HAIKU__)
#include <endian.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#undef CORE_BIG_ENDIAN
#define CORE_LITTLE_ENDIAN
#else /* !(__BYTE_ORDER == __LITTLE_ENDIAN) */
#if __BYTE_ORDER == __BIG_ENDIAN
#define CORE_BIG_ENDIAN
#undef CORE_LITTLE_ENDIAN
#endif /* __BYTE_ORDER == __BIG_ENDIAN */
#endif /* !(__BYTE_ORDER == __LITTLE_ENDIAN) */
#endif /* Def __HAIKU__ */

#ifdef __sun
#ifdef __sparc
#define CORE_BIG_ENDIAN
#undef CORE_LITTLE_ENDIAN
#else  /* Ndef __sparc */
#undef CORE_BIG_ENDIAN
#define CORE_LITTLE_ENDIAN
#endif /* Ndef __sparc */
#endif /* Def __sun */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

#include <config_options.h>

/* modes */
#ifdef _MSC_VER
#define FILE_O_RDONLY     _O_RDONLY
#define FILE_O_BINARY     _O_BINARY
#define PATHNCMP _strnicmp /* MSVC converts paths to lower-case sometimes? */
#define ssize_t long
#define S_ISREG(mode) (((mode) & _S_IFMT) == (_S_IFREG)) /* MSVC does not have this macro */
#else /* not windaube */
#define FILE_O_RDONLY     O_RDONLY
#define FILE_O_BINARY     0
#define PATHNCMP strncmp
#endif /* not windaube */

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

static int internal_boost = 0;
static char* base_dir;
static char* work_dir;
static size_t work_dir_len;

/* SRCDIR, BUILDDIR and WORKDIR with every backslash turned into a forward
   slash. cl /sourceDependencies emits forward-slash-normalised include paths,
   and these copies let a plain prefix compare decide whether an include lives
   inside the build tree. build_dir may be NULL if BUILDDIR is not set. */
static char* src_dir_fwd;
static size_t src_dir_fwd_len;
static char* build_dir_fwd;
static size_t build_dir_fwd_len;
static char* work_dir_fwd;
static size_t work_dir_fwd_len;

#ifdef __GNUC__
#define clz __builtin_clz
#else
static int clz(unsigned int value)
{
    int result = 32;

    while(value)
    {
        value >>= 1;
        result -= 1;
    }
    return result;
}
#endif

static unsigned int get_unaligned_uint(const unsigned char* cursor)
{
    unsigned int   result;

    memcpy(&result, cursor, sizeof(unsigned int));
    return result;
}

/* ===============================================
 * memory pool for fast fix-size allocation (non-thread-safe)
 * ===============================================
 */
struct pool
{
    void*    head_free;  /**< head of a linked list of freed element */
    char*    fresh;      /**< top of a memory block to dig new element */
    char*    tail;       /**< to detect end of extent... when fresh pass tail */
    void*    extent;     /**< pointer to the primary extent block */
    int      size_elem;  /**< size of an element. */
    int      primary;    /**< primary allocation in bytes */
    int      secondary;  /**< secondary allocation in bytes */
};
#define POOL_ALIGN_INCREMENT 8 /**< alignment, must be a power of 2 and of size > to sizeof(void*) */


static void* pool_take_extent(struct pool* pool, int allocate)
{
    unsigned int size = 0;
    void* extent;
    void* data = NULL;

    if(pool->extent)
    {
        /* we already have an extent, so this is a secondary */
        if(pool->secondary)
        {
            size = pool->secondary;
        }
    }
    else
    {
        assert(pool->primary);
        size = pool->primary;
    }
    if(size)
    {
        extent = malloc(size);
        if(extent)
        {
            *(void**)extent = pool->extent;
            pool->extent = extent;
            if(allocate)
            {
                data = ((char*)extent) + POOL_ALIGN_INCREMENT;
                pool->fresh = ((char*)data) + pool->size_elem;
                pool->tail = pool->fresh + (size - pool->size_elem);
            }
            else
            {
                pool->fresh = ((char*)extent) + POOL_ALIGN_INCREMENT;
                pool->tail = pool->fresh + (size - pool->size_elem);
            }
        }
    }
    return data;
}

/* Create a memory pool for fix size objects
 * this is a simplified implementation that
 * is _not_ thread safe.
 */
static struct pool* pool_create(int size_elem, int primary, int secondary)
{
    struct pool* pool;

    assert(primary > 0);
    assert(secondary >= 0);
    assert(size_elem > 0);

    pool = (struct pool*)calloc(1, sizeof(struct pool));
    if(!pool) return NULL;
    /* Adjust the element size so that it be aligned, and so that an element could
     * at least contain a void*
     */
    pool->size_elem = size_elem = (size_elem + POOL_ALIGN_INCREMENT - 1) & ~(POOL_ALIGN_INCREMENT - 1);

    pool->primary = (size_elem * primary) + POOL_ALIGN_INCREMENT;
    pool->secondary = secondary > 0 ? (size_elem * secondary) + POOL_ALIGN_INCREMENT : 0;
    pool_take_extent(pool, FALSE);

    return pool;

}

static void pool_destroy(struct pool* pool)
{
    void* extent;
    void* next;

    if(pool != NULL)
    {
        extent = pool->extent;
        while(extent)
        {
            next = *(void**)extent;
            free(extent);
            extent = next;
        }
        free(pool);
    }
}

static void* pool_alloc(struct pool* pool)
{
    void* data;

    data = pool->head_free;
    if(data == NULL)
    {
        /* we have no old-freed elem */
        if(pool->fresh <= pool->tail)
        {
            /* pick a slice of the current extent */
            data = (void*)pool->fresh;
            pool->fresh += pool->size_elem;
        }
        else
        {
            /* allocate a new extent */
            data = pool_take_extent(pool, TRUE);
        }
    }
    else
    {
        /* re-used old freed element by chopping the head of the free list */
        pool->head_free = *(void**)data;
    }

    return data;
}


/* ===============================================
 * Hash implementation customized to be just tracking
 * a unique list of string (i.e no data associated
 * with the key, no need for retrieval, etc...
 *
 * This is tuned for the particular use-case we have here
 * measures in tail_build showed that
 * we can get north of 4000 distinct values stored in a hash
 * the collision rate is at worse around 2%
 * the collision needing an expensive memcmp to resolve
 * have a rate typically at 1 per 1000
 * for tail_build we register 37229 unique key
 * with a total of 377 extra memcmp needed
 * which is completely negligible compared to the
 * number of memcmp required to eliminate duplicate
 * entry (north of 2.5 millions for tail_build)
 * ===============================================
 */

struct hash_elem
{
    struct hash_elem* next;
    const char*    key;
    int      key_len;
};

struct hash
{
    struct hash_elem** array;
    struct pool* elems_pool;
    unsigned int used;
    unsigned int size;
    unsigned int load_limit;
#ifdef HASH_STAT
    int stored;
    int collisions;
    int cost;
    int memcmp;
#endif
};

/* The following hash_compute function was adapted from :
 * lookup3.c, by Bob Jenkins, May 2006, Public Domain.
 *
 * The changes from the original are mostly cosmetic
 */
#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))


#if defined CORE_BIG_ENDIAN
#define MASK_C1 0xFFFFFF00
#define MASK_C2 0xFFFF0000
#define MASK_C3 0xFF000000
#elif defined CORE_LITTLE_ENDIAN
#define MASK_C1 0xFFFFFF
#define MASK_C2 0xFFFF
#define MASK_C3 0xFF
#else
#error "Missing Endianness definition"
#endif


#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}
#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}

static unsigned int hash_compute( struct hash const * hash, const char* key, int length)
{
    unsigned int a;
    unsigned int b;
    unsigned int c;                                          /* internal state */
    const unsigned char* uk = (const unsigned char*)key;

    /* Set up the internal state */
    a = b = c = 0xdeadbeef + (length << 2);

    /* we use this to 'hash' full path with mostly a common root
     * let's now waste too much cycles hashing mostly constant stuff
     */
    if(length > 36)
    {
        uk += length - 36;
        length = 36;
    }
    /*------ all but last block: aligned reads and affect 32 bits of (a,b,c) */
    while (length > 12)
    {
        a += get_unaligned_uint(uk);
        b += get_unaligned_uint(uk+4);
        c += get_unaligned_uint(uk+8);
        mix(a,b,c);
        length -= 12;
        uk += 12;
    }

    /*----------------------------- handle the last (probably partial) block */
    /* Note: we possibly over-read, which would trigger complaint from VALGRIND
     * but we mask the undefined stuff if any, so we are still good, thanks
     * to alignment of memory allocation and tail-memory management overhead
     * we always can read 3 bytes past the official end without triggering
     * a segfault -- if you find a platform/compiler couple for which that postulate
     * is false, then you just need to over-allocate by 2 more bytes in file_load()
     * file_load already over-allocate by 1 to stick a \0 at the end of the buffer.
     */
    switch(length)
    {
    case 12: c+=get_unaligned_uint(uk+8); b+=get_unaligned_uint(uk+4); a+=get_unaligned_uint(uk); break;
    case 11: c+=get_unaligned_uint(uk+8) & MASK_C1; b+=get_unaligned_uint(uk+4); a+=get_unaligned_uint(uk); break;
    case 10: c+=get_unaligned_uint(uk+8) & MASK_C2; b+=get_unaligned_uint(uk+4); a+=get_unaligned_uint(uk); break;
    case 9 : c+=get_unaligned_uint(uk+8) & MASK_C3; b+=get_unaligned_uint(uk+4); a+=get_unaligned_uint(uk); break;
    case 8 : b+=get_unaligned_uint(uk+4); a+=get_unaligned_uint(uk); break;
    case 7 : b+=get_unaligned_uint(uk+4) & MASK_C1; a+=get_unaligned_uint(uk); break;
    case 6 : b+=get_unaligned_uint(uk+4) & MASK_C2; a+=get_unaligned_uint(uk); break;
    case 5 : b+=get_unaligned_uint(uk+4) & MASK_C3; a+=get_unaligned_uint(uk); break;
    case 4 : a+=get_unaligned_uint(uk); break;
    case 3 : a+=get_unaligned_uint(uk) & MASK_C1; break;
    case 2 : a+=get_unaligned_uint(uk) & MASK_C2; break;
    case 1 : a+=get_unaligned_uint(uk) & MASK_C3; break;
    case 0 : return c & hash->size;              /* zero length strings require no mixing */
    }

    final(a,b,c);
    return c & hash->size;
}

static void hash_destroy(struct hash* hash)
{
    if(hash)
    {
        if(hash->array)
        {
            free(hash->array);
        }
        if(hash->elems_pool)
        {
            pool_destroy(hash->elems_pool);
        }
        free(hash);
    }
}

static struct hash* hash_create(unsigned int size)
{
    struct hash* hash;

    assert(size > 0);
    hash = (struct hash*)(calloc(1, sizeof(struct hash)));
    if(hash)
    {
        size += (size >> 2) + 1; /* ~ 75% load factor */
        if(size >= 15)
        {
            hash->size = (((unsigned int)0xFFFFFFFF) >> clz((unsigned int)size));
        }
        else
        {
            hash->size = size = 15;
        }
        hash->load_limit = hash->size - (hash->size >> 2);
        hash->used = 0;
        hash->array = (struct hash_elem**)calloc(hash->size + 1, sizeof(struct hash_elem*));
        if(hash->array == NULL)
        {
            hash_destroy(hash);
            hash = NULL;
        }
    }
    if(hash)
    {
        hash->elems_pool = pool_create(sizeof(struct hash_elem),
                                       size, size << 1);
        if(!hash->elems_pool)
        {
            hash_destroy(hash);
            hash = NULL;
        }
    }
    return hash;
}

static void hash_resize(struct hash* hash)
{
    unsigned int old_size = hash->size;
    unsigned int hashed;
    struct hash_elem* hash_elem;
    struct hash_elem* next;
    struct hash_elem** array;
    unsigned int i;

    hash->size = (old_size << 1) + 1;
    /* we really should avoid to get there... so print a message to alert of the condition */
    fprintf(stderr, "resize hash %u -> %u\n", old_size, hash->size);
    if(hash->size == old_size)
    {
        return;
    }
    array = (struct hash_elem**)calloc(hash->size + 1, sizeof(struct hash_elem*));
    if(array)
    {
        hash->load_limit = hash->size - (hash->size >> 2);
        for(i=0; i <= old_size; i++)
        {
            hash_elem = (struct hash_elem*)hash->array[i];
            while(hash_elem)
            {
                next = hash_elem->next;

                hashed = hash_compute(hash, hash_elem->key, hash_elem->key_len);
                hash_elem->next = array[hashed];
                array[hashed] = hash_elem;
                hash_elem = next;
            }
        }
        free(hash->array);
        hash->array = (struct hash_elem**)array;
    }
    else
    {
        hash->size = old_size;
    }
}

static int compare_key(struct hash const * hash, const char* a, const char* b, int len, int const * cost)
{
#ifdef HASH_STAT
    *cost += 1;
    hash->memcmp += 1;
#else
    (void) hash;
    (void) cost;
#endif
    return memcmp(a,b, len);
}

/* a customized hash_store function that just store the key and return
 * TRUE if the key was effectively stored, or FALSE if the key was already there
 */
static int hash_store(struct hash* hash, const char* key, int key_len)
{
    unsigned int hashed;
    struct hash_elem* hash_elem;
    int cost = 0;

    (void) cost;
    hashed = hash_compute(hash, key, key_len);
#ifdef HASH_STAT
    hash->stored += 1;
#endif
    hash_elem = (struct hash_elem*)hash->array[hashed];
    while(hash_elem && (hash_elem->key_len != key_len || compare_key(hash, hash_elem->key, key, key_len, &cost)))
    {
        hash_elem = hash_elem->next;
    }

    if(!hash_elem)
    {
        hash_elem = (struct hash_elem*)pool_alloc(hash->elems_pool);
        if(hash_elem)
        {
            hash_elem->key = key;
            hash_elem->key_len = key_len;
            hash_elem->next = hash->array[hashed];

#ifdef HASH_STAT
            if(hash_elem->next)
            {
                hash->collisions += 1;
                hash->cost += cost;
            }
#endif
            hash->array[hashed] = hash_elem;
            hash->used += 1;
            if(hash->used > hash->load_limit)
            {
                hash_resize(hash);
            }
        }
        return TRUE;
    }
    return FALSE;
}

static int file_stat(const char* name, struct stat* buffer_stat, int* rc)
{
    int rc_local = stat(name, buffer_stat);
    if (rc_local  < 0)
    {
        *rc = errno;
    }
    return rc_local;
}

static off_t file_get_size(const char* name, int* rc)
{
    struct stat buffer_stat;
    off_t       size = -1;

    if (!file_stat(name, &buffer_stat, rc))
    {
        if(S_ISREG(buffer_stat.st_mode))
        {
            size = buffer_stat.st_size;
        }
        else
        {
            *rc = EINVAL;
        }
    }
    return size;
}

#if !ENABLE_RUNTIME_OPTIMIZATIONS
static void * file_load_buffers[100000];
static size_t file_load_buffer_count = 0;
#endif

static char* file_load(const char* name, off_t* size, int* return_rc)
{
    off_t local_size = 0;
    int rc = 0;
    char* buffer = NULL;
    int fd;

    assert(name != NULL);

    if(!size)
    {
        size = &local_size;
    }
    *size = file_get_size(name, &rc);
    if (!rc && *size >= 0)
    {
        fd = open(name, FILE_O_RDONLY | FILE_O_BINARY);
        if (!(fd == -1))
        {
            buffer = (char*)malloc((size_t)(*size + 1));
#if !ENABLE_RUNTIME_OPTIMIZATIONS
            if (buffer != NULL)
            {
                if (file_load_buffer_count == 100000)
                {
                    free(buffer);
                    buffer = NULL;
                }
                else
                {
                    file_load_buffers[file_load_buffer_count++] = buffer;
                }
            }
#endif
            if (buffer == NULL)
            {
                rc = ENOMEM;
            }
            else
            {
                ssize_t i;

              REDO:
                i = read(fd, buffer, (size_t)(*size));
                if(i == -1)
                {
                    if(errno == EINTR)
                    {
                        goto REDO;
                    }
                    else
                    {
                        rc = errno;
                    }
                }
                else
                {
                    if (i != *size)
                    {
                        rc = EIO;
                    }
                }
                buffer[*size] = 0;
            }
            close(fd);
        }
    }

    if(rc && buffer)
    {
        free(buffer);
        buffer = NULL;
    }
    if(return_rc)
    {
        *return_rc = rc;
    }
    return buffer;
}

static void cancel_relative(char const * base, char** ref_cursor, char** ref_cursor_out, char const * end)
{
    char* cursor = *ref_cursor;
    char* cursor_out = *ref_cursor_out;

    do
    {
        cursor += 3;
        while(cursor_out > base && cursor_out[-1] == '/')
            cursor_out--;
        while(cursor_out > base && *--cursor_out != '/');
    }
    while(cursor + 3 < end && !memcmp(cursor, "/../", 4));
    *ref_cursor = cursor;
    *ref_cursor_out = cursor_out;
}

static void eat_space(char ** token)
{
    while ((' ' == **token) || ('\t' == **token)) {
        ++(*token);
    }
}

/*
 * Prune LibreOffice specific duplicate dependencies to improve
 * gnumake startup time, and shrink the disk-space footprint.
 */
static int
elide_dependency(const char* key, int key_len, const char **unpacked_end)
{
#if 0
    {
        int i;
        fprintf (stderr, "elide?%d!: '", internal_boost);
        for (i = 0; i < key_len; i++) {
            fprintf (stderr, "%c", key[i]);
        }
        fprintf (stderr, "'\n");
    }
#endif

    /* boost brings a plague of header files */
    int i;
    int unpacked = 0;
    /* walk down path elements */
    for (i = 0; i < key_len - 1; i++)
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
#define BOOST_TARGET "/UnpackedTarball/boost.done"
    fprintf(stdout, "%s" BOOST_TARGET " ", work_dir);
}

static void emit_unpacked_target(const char* token, const char* end)
{
    fwrite(token, 1, end-token, stdout);
    fputs(".done ", stdout);
}

/* prefix paths to absolute */
static void print_fullpaths(char* line)
{
    char* token;
    char* end;
    int boost_count = 0;
    int token_len;
    const char * unpacked_end = NULL; /* end of UnpackedTarget match (if any) */
    /* for UnpackedTarget the target is GenC{,xx}Object, don't mangle! */
    int target_seen = 0;

    token = line;
    eat_space(&token);
    while (*token)
    {
        end = token;
        /* hard to believe that in this day and age drive letters still exist */
        if (*end && (':' == *(end+1)) &&
            (('\\' == *(end+2)) || ('/' == *(end+2))) &&
            isalpha((unsigned char)*end))
        {
            end = end + 3; /* only one cross, err drive letter per filename */
        }
        while (*end && (' ' != *end) && ('\t' != *end) && (':' != *end)) {
            ++end;
        }
        token_len = end - token;
        if (target_seen &&
            elide_dependency(token, token_len, &unpacked_end))
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
                unpacked_end = NULL;
            }
        }
        else
        {
            if (fwrite(token, token_len, 1, stdout) != 1)
                abort();
            fputc(' ', stdout);
        }
        token = end;
        eat_space(&token);
        if (!target_seen && ':' == *token)
        {
            target_seen = 1;
            fputc(':', stdout);
            ++token;
            eat_space(&token);
        }
    }
}

static char * eat_space_at_end(char * end)
{
    char * real_end;
    assert('\0' == *end);
    real_end = end - 1;
    while (' ' == *real_end || '\t' == *real_end || '\n' == *real_end
                || ':' == *real_end)
    {    /* eat colon and whitespace at end */
         --real_end;
    }
    return real_end;
}

static char* phony_content_buffer;
/* Write workdir/<rel> into dest and NUL-terminate it, where rel is the path
   below workdir/Dep/, that is <Class>/<stem>.<ext>. Return a pointer to the
   last dot in what was written, so the caller can replace the extension, or
   NULL if there is none. */
static char* object_target_from_relative(char* dest, const char* rel)
{
    char* d;
    const char* s;
    char* last_dot = NULL;

    memcpy(dest, work_dir, work_dir_len);
    d = dest + work_dir_len;
    *d++ = '/';
    for(s = rel; *s != 0; ++s, ++d)
    {
        *d = *s;
        if(*d == '.')
            last_dot = d;
    }
    *d = 0;
    return last_dot;
}

static char* generate_phony_line(char const * phony_target, char const * extension)
{
    //fprintf(stderr, "generate_phony_line called with phony_target %s and extension %s\n", phony_target, extension);
    char* last_dot = object_target_from_relative(phony_content_buffer, phony_target);
    char* dest = last_dot + 1;
    char const * src;

    //fprintf(stderr, "generate_phony_line after phony_target copy: %s\n", phony_content_buffer);
    for(src = extension; *src != 0; ++src, ++dest)
        *dest = *src;
    //fprintf(stderr, "generate_phony_line after extension add: %s\n", phony_content_buffer);
    strcpy(dest, ": $(gb_Helper_PHONY)\n");
    //fprintf(stderr, "generate_phony_line after phony add: %s\n", phony_content_buffer);
    return phony_content_buffer;
}

static int generate_phony_file(char* fn, char const * content)
{
    FILE* depfile;
    depfile = fopen(fn, "w");
    if(!depfile)
    {
        fprintf(stderr, "Could not open '%s' for writing: %s\n", fn, strerror(errno));
    }
    else
    {
        fputs(content, depfile);
        fclose(depfile);
    }
    return !depfile;
}

/* Return a malloc'd copy of s with every backslash turned into a forward
   slash, or NULL if s is NULL. */
static char* dup_forward_slashes(const char* s)
{
    char* out;
    char* p;
    size_t n;

    if (!s)
        return NULL;
    n = strlen(s);
    out = (char*)malloc(n + 1);
    if (out)
    {
        memcpy(out, s, n + 1);
        for (p = out; *p; ++p)
        {
            if (*p == '\\')
                *p = '/';
        }
    }
    return out;
}

/* Decide whether a forward-slash include path lies inside the source tree, the
   build tree or the work directory. System headers (for instance under Program
   Files) live outside all three and are dropped, matching the allowlist that
   filter-showIncludes.awk applies. On Windows PATHNCMP is case-insensitive, so
   the lower-cased paths cl emits still match the mixed-case prefixes. */
static int include_in_build_tree(const char* path, int len)
{
    if (src_dir_fwd_len && (size_t)len >= src_dir_fwd_len
        && PATHNCMP(path, src_dir_fwd, src_dir_fwd_len) == 0)
        return 1;
    if (build_dir_fwd_len && (size_t)len >= build_dir_fwd_len
        && PATHNCMP(path, build_dir_fwd, build_dir_fwd_len) == 0)
        return 1;
    if (work_dir_fwd_len && (size_t)len >= work_dir_fwd_len
        && PATHNCMP(path, work_dir_fwd, work_dir_fwd_len) == 0)
        return 1;
    return 0;
}

/* Map a dependency fragment path workdir/Dep/<Class>/<stem>.d to the object it
   describes, workdir/<Class>/<stem>.o. Returns a malloc'd string, or NULL if fn
   is not under workdir/Dep. */
static char* object_from_dep_path(const char* fn)
{
    const char* rel;
    char* out;
    char* dot;
    size_t rel_len;

    if (strncmp(fn, work_dir, work_dir_len) != 0
        || strncmp(fn + work_dir_len, "/Dep/", 5) != 0)
        return NULL;
    rel = fn + work_dir_len + 5;
    rel_len = strlen(rel);
    out = (char*)malloc(work_dir_len + 1 + rel_len + 2);
    if (!out)
        return NULL;
    dot = object_target_from_relative(out, rel);
    if (dot)
        strcpy(dot, ".o");
    return out;
}

static const char* json_skip_ws(const char* p, const char* end)
{
    while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'))
        ++p;
    return p;
}

/* Copy the JSON string that begins at the opening quote *pp into buf (capacity
   cap), undoing JSON escaping and turning backslashes into forward slashes.
   NUL-terminates buf and advances *pp past the closing quote. Returns the
   length written, or -1 if the string is malformed or does not fit. */
static int json_read_string(const char** pp, const char* end, char* buf, int cap)
{
    const char* p = *pp;
    int n = 0;

    if (p >= end || *p != '"')
        return -1;
    ++p;
    while (p < end && *p != '"')
    {
        char c = *p++;
        if (c == '\\' && p < end)
        {
            char e = *p++;
            switch (e)
            {
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                case 'r': c = '\r'; break;
                case 'b': c = '\b'; break;
                case 'f': c = '\f'; break;
                case 'u':
                    /* cl does not put \u escapes in paths. Skip the four hex
                       digits so the rest of the string stays in step. */
                    if (p + 4 <= end)
                        p += 4;
                    continue;
                default:  c = e; break; /* covers \\ \" \/ */
            }
        }
        if (c == '\\')
            c = '/';
        if (n + 1 >= cap)
            return -1;
        buf[n++] = c;
    }
    if (p >= end)
        return -1;
    ++p;
    buf[n] = 0;
    *pp = p;
    return n;
}

/* Append path to w, escaping every space with a backslash so make treats the
   path as a single prerequisite. Returns the new write cursor. */
static char* emit_path_escaped(char* w, const char* path)
{
    const char* s;

    for (s = path; *s; ++s)
    {
        if (*s == ' ')
            *w++ = '\\';
        *w++ = *s;
    }
    return w;
}

/* Include paths are read into a fixed stack buffer of this size. It is assumed
   larger than any real path. One that does not fit makes the conversion fail
   and the build stop, rather than writing a dependency file that is missing
   headers. */
#define INCLUDE_PATH_MAX 8192

/* Walk the "Includes" array in json once. For every header inside the build
   tree, write its make-escaped path into the buffer at *pw, terminate it with a
   NUL, and advance *pw past it. Return the number of paths written, or -1 if
   the JSON is malformed or an include path does not fit. */
static int collect_includes(const char* json, const char* end, char** pw)
{
    const char* p = strstr(json, "\"Includes\"");
    char* w = *pw;
    char path[INCLUDE_PATH_MAX];
    int len;
    int count = 0;

    if (!p)
        return 0;
    p += 10;
    p = json_skip_ws(p, end);
    if (p >= end || *p != ':')
        return -1;
    ++p;
    p = json_skip_ws(p, end);
    if (p >= end || *p != '[')
        return -1;
    ++p;
    for (;;)
    {
        p = json_skip_ws(p, end);
        if (p >= end || *p != '"')
            break;
        len = json_read_string(&p, end, path, sizeof(path));
        if (len < 0)
            return -1;
        if (len > 0 && include_in_build_tree(path, len))
        {
            w = emit_path_escaped(w, path);
            *w++ = 0;
            ++count;
        }
        p = json_skip_ws(p, end);
        if (p < end && *p == ',')
            ++p;
    }
    *pw = w;
    return count;
}

/* Worst-case growth of the generated make text over the JSON input. Each
   include path can appear both as a prerequisite and as its own no-dependency
   rule, and every space in a path becomes two bytes, so budget seven output
   bytes per input byte plus room for the object line. */
#define DEP_OUTPUT_GROWTH 7
/* The scratch buffer holds each escaped include path only once, so it needs
   only the space-doubling and a NUL after each path. */
#define DEP_SCRATCH_GROWTH 3

/* Turn a cl /sourceDependencies JSON fragment into the make-syntax dependency
   text that filter-showIncludes.awk used to produce, so the rest of process()
   can treat it exactly like a classic .d file. Returns a malloc'd,
   NUL-terminated buffer and sets *out_size, or NULL on failure. */
static char* convert_source_deps_json(const char* json, off_t json_size,
                                      const char* fn, off_t* out_size)
{
    const char* end = json + json_size;
    const char* p;
    const char* s;
    char* object;
    char* out;
    char* w;
    char* scratch;
    char* sw;
    size_t cap;
    char path[INCLUDE_PATH_MAX];
    int len;
    int n;
    int i;

    object = object_from_dep_path(fn);
    if (!object)
        return NULL;

    cap = (size_t)json_size * DEP_OUTPUT_GROWTH + strlen(object) * 2 + 4096;
    out = (char*)malloc(cap);
    if (!out)
    {
        free(object);
        return NULL;
    }
    w = out;

    /* target line: "<object> : \" */
    w = emit_path_escaped(w, object);
    free(object);
    *w++ = ' '; *w++ = ':'; *w++ = ' '; *w++ = '\\'; *w++ = '\n';

    /* source as the first prerequisite */
    p = strstr(json, "\"Source\"");
    if (p)
    {
        p += 8;
        p = json_skip_ws(p, end);
        if (p < end && *p == ':')
        {
            ++p;
            p = json_skip_ws(p, end);
            len = json_read_string(&p, end, path, sizeof(path));
            if (len < 0)
            {
                free(out);
                return NULL;
            }
            if (len > 0)
            {
                *w++ = ' ';
                w = emit_path_escaped(w, path);
                *w++ = ' '; *w++ = '\\'; *w++ = '\n';
            }
        }
    }

    /* Each in-tree header appears twice: once as a prerequisite of the object
       and once as its own no-dependency rule (fdo#40099, so a deleted header
       does not stop make). Collect them once into scratch, escaped and
       NUL-separated, then write both sections from that instead of walking the
       JSON twice. */
    scratch = (char*)malloc((size_t)json_size * DEP_SCRATCH_GROWTH + 64);
    if (!scratch)
    {
        free(out);
        return NULL;
    }
    sw = scratch;
    n = collect_includes(json, end, &sw);
    if (n < 0)
    {
        free(scratch);
        free(out);
        return NULL;
    }

    /* the rest of the prerequisites, one " <header> \" continuation line each */
    s = scratch;
    for (i = 0; i < n; ++i)
    {
        *w++ = ' ';
        while (*s)
            *w++ = *s++;
        ++s;
        *w++ = ' '; *w++ = '\\'; *w++ = '\n';
    }

    /* a blank line ends the object's rule, then a "<header> :" rule per header */
    *w++ = '\n';
    s = scratch;
    for (i = 0; i < n; ++i)
    {
        while (*s)
            *w++ = *s++;
        ++s;
        *w++ = ' '; *w++ = ':'; *w++ = '\n';
    }
    free(scratch);

    *w = 0;
    *out_size = (off_t)(w - out);
    return out;
}

static int process(struct hash* dep_hash, char* fn)
{
    int rc = 1;
    char* buffer;
    char* end;
    char* cursor;
    char* cursor_out;
    char* base;
    char* created_line = NULL;
    char* src_relative;
    int continuation = 0;
    char last_ns = 0;
    off_t size = 0;

    /* cl /sourceDependencies writes the include list as JSON to a file named
       like the dep-file with an extra .json suffix, and leaves the dep-file
       alone. Read that JSON when it is there. An older concat-deps opens only
       the dep-file, never the JSON, so an old and a new build can share one
       build directory. */
    int is_json = 0;
    buffer = NULL;
    {
        size_t fn_len = strlen(fn);
        char* json_name = (char*)malloc(fn_len + sizeof(".json"));
        if (json_name)
        {
            memcpy(json_name, fn, fn_len);
            memcpy(json_name + fn_len, ".json", sizeof(".json"));
            buffer = file_load(json_name, &size, &rc);
            free(json_name);
            if (!rc)
                is_json = 1;
        }
    }
    if (!is_json)
        buffer = file_load(fn, &size, &rc);
    if(!rc)
    {
        /* The first version of the /sourceDependencies path wrote the JSON into
           the dep-file itself, so a leading '{' in the dep-file is read as JSON
           too. A classic .d always starts with a path. */
        if (!is_json)
        {
            const char* probe = buffer;
            while (*probe == ' ' || *probe == '\t' || *probe == '\n' || *probe == '\r')
                ++probe;
            is_json = (*probe == '{');
        }
        if (is_json)
        {
            off_t converted_size = 0;
            char* converted = convert_source_deps_json(buffer, size, fn, &converted_size);
            if (!converted)
            {
                fprintf(stderr, "concat-deps: could not convert JSON dependencies in %s\n", fn);
                return 1;
            }
            buffer = converted;
            size = converted_size;
#if !ENABLE_RUNTIME_OPTIMIZATIONS
            /* hash_store keeps pointers into this buffer as keys, so it must
               live until the end of main. Add it to the file_load buffers,
               which are all freed there. */
            if (file_load_buffer_count != 100000)
                file_load_buffers[file_load_buffer_count++] = converted;
#endif
        }

        base = cursor_out = cursor = end = buffer;
        end += size;

        /* first eat unneeded space at the beginning of file
         */
        while(cursor < end && (*cursor == ' ' || *cursor == '\\'))
            ++cursor;

        while(cursor < end)
        {
            if(*cursor == '\\')
            {
                continuation = 1;
                *cursor_out++ = *cursor++;
            }
            else if(*cursor == '/')
            {
                if(cursor + 2 < end)
                {
                    if(!memcmp(cursor, "/./", 3))
                    {
                        cursor += 2;
                        continue;
                    }
                }
                if(cursor + 3 < end)
                {
                    if(!memcmp(cursor, "/../", 4))
                    {
                        cancel_relative(base, &cursor, &cursor_out, end);
                        // cancel_relative leaves cursor on the trailing '/' of
                        // the consumed "/../". Re-examine it so a following "/./"
                        // or "/../" is also collapsed. Without this a sequence
                        // like ".././../" loses one level and names a directory
                        // that does not exist.
                        continue;
                    }
                }
                *cursor_out++ = *cursor++;
            }
            else if(*cursor == '\n')
            {
                if(!continuation)
                {
                    *cursor_out = 0;
                    if(base < cursor)
                    {
                        /* here we have a complete rule */
                        if(last_ns == ':')
                        {
                            /* if the rule ended in ':' that is a no-dep rule
                             * these are the one for which we want to filter
                             * duplicate out
                             */
                            int key_len = eat_space_at_end(cursor_out) - base;
                            if (!elide_dependency(base,key_len + 1, NULL)
                                && hash_store(dep_hash, base, key_len))
                            {
                                /* DO NOT modify base after it has been added
                                   as key by hash_store */
                                print_fullpaths(base);
                                putc('\n', stdout);
                            }
                        }
                        else
                        {
                            /* rule with dep, just write it */
                            print_fullpaths(base);
                            putc('\n', stdout);
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
                if(*cursor != ' ' && *cursor != '\n' && *cursor != '\t' )
                {
                    last_ns = *cursor;
                }
                *cursor_out++ = *cursor++;
            }
        }

        /* just in case the file did not end with a \n, there may be a pending rule */
        if(base < cursor_out)
        {
            if(last_ns == ':')
            {
                int key_len = eat_space_at_end(cursor_out) - base;
                if (!elide_dependency(base,key_len + 1, NULL) &&
                    hash_store(dep_hash, base, key_len))
                {
                    puts(base);
                    putc('\n', stdout);
                }
            }
            else
            {
                puts(base);
                putc('\n', stdout);
            }
        }
    }
    else
    {
        if(strncmp(fn, work_dir, work_dir_len) == 0)
        {
            if(strncmp(fn+work_dir_len, "/Dep/", 5) == 0)
            {
                src_relative = fn+work_dir_len+5;
                // cases ordered by frequency
                if(strncmp(src_relative, "CxxObject/", 10) == 0)
                {
                    created_line = generate_phony_line(src_relative, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "GenCxxObject/", 13) == 0)
                {
                    created_line = generate_phony_line(src_relative, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "CObject/", 8) == 0)
                {
                    created_line = generate_phony_line(src_relative, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "GenCObject/", 11) == 0)
                {
                    created_line = generate_phony_line(src_relative, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "SdiObject/", 10) == 0)
                {
                    created_line = generate_phony_line(src_relative, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "AsmObject/", 10) == 0)
                {
                    created_line = generate_phony_line(src_relative, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "ObjCxxObject/", 13) == 0)
                {
                    created_line = generate_phony_line(src_relative, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "ObjCObject/", 11) == 0)
                {
                    created_line = generate_phony_line(src_relative, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "GenObjCxxObject/", 16) == 0)
                {
                    created_line = generate_phony_line(src_relative, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "GenObjCObject/", 14) == 0)
                {
                    created_line = generate_phony_line(src_relative, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "GenAsmObject/", 13) == 0)
                {
                    created_line = generate_phony_line(src_relative, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "GenNasmObject/", 14) == 0)
                {
                    created_line = generate_phony_line(src_relative, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "CxxClrObject/", 13) == 0)
                {
                    created_line = generate_phony_line(src_relative, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "GenCxxClrObject/", 16) == 0)
                {
                    created_line = generate_phony_line(src_relative, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else
                {
                    fprintf(stderr, "no magic for %s(%s) in %s\n", fn, src_relative, work_dir);
                }
            }
            if(!rc)
            {
                puts(created_line);
            }
        }
    }
    /* Note: yes we are going to leak 'buffer'
     * this is on purpose, to avoid cloning the 'key' out of it and our special
     * 'hash' just store the pointer to the key inside of buffer, hence it need
     * to remain allocated
     */
    // coverity[leaked_storage] - this is on purpose
    return rc;
}

static void usage(void)
{
    fputs("Usage: concat-deps <file that contains dep_files>\n", stderr);
}

#define kDEFAULT_HASH_SIZE 4096
#define PHONY_TARGET_BUFFER 4096

static int get_var(char **var, const char *name)
{
    *var = (char *)getenv(name);
    if(!*var)
    {
        fprintf(stderr,"Error: %s is missing in the environment\n", name);
        return 1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    int rc = 0;
    off_t in_list_size = 0;
    char* in_list;
    char* in_list_cursor;
    char* in_list_base;
    struct hash* dep_hash = NULL;
    const char *env_str;

    if(argc < 2)
    {
        usage();
        return 1;
    }
    if(get_var(&base_dir, "SRCDIR") || get_var(&work_dir, "WORKDIR"))
        return 1;
    work_dir_len = strlen(work_dir);

    /* BUILDDIR is optional here. It only helps keep cl /sourceDependencies deps
       trimmed to the build tree. The forward-slash copies feed the include
       allowlist. */
    build_dir_fwd = dup_forward_slashes(getenv("BUILDDIR"));
    build_dir_fwd_len = build_dir_fwd ? strlen(build_dir_fwd) : 0;
    src_dir_fwd = dup_forward_slashes(base_dir);
    src_dir_fwd_len = src_dir_fwd ? strlen(src_dir_fwd) : 0;
    work_dir_fwd = dup_forward_slashes(work_dir);
    work_dir_fwd_len = work_dir_fwd ? strlen(work_dir_fwd) : 0;
    phony_content_buffer = (char*)malloc(PHONY_TARGET_BUFFER);
    assert(phony_content_buffer); // Don't handle OOM conditions

    env_str = getenv("SYSTEM_BOOST");
    internal_boost = !env_str || strcmp(env_str,"TRUE");

    in_list = file_load(argv[1], &in_list_size, &rc);
    if(!rc)
    {
        dep_hash = hash_create( kDEFAULT_HASH_SIZE);
        in_list_base = in_list_cursor = in_list;

        /* extract filename of dep file from a 'space' separated list */
        while(*in_list_cursor)
        {
            /* the input here may contain Win32 \r\n EOL */
            if(*in_list_cursor == ' '
                || *in_list_cursor == '\n' || *in_list_cursor == '\r')
            {
                *in_list_cursor = 0;
                if(in_list_base < in_list_cursor)
                {
                    rc = process(dep_hash, in_list_base);
                    if(rc)
                    {
                        break;
                    }
                }
                in_list_cursor += 1;
                in_list_base = in_list_cursor;
            }
            else
            {
                in_list_cursor += 1;
            }
        }
        if(!rc)
        {
            /* catch the last entry in case the input did not terminate with a 'space' */
            if(in_list_base < in_list_cursor)
            {
                rc = process(dep_hash, in_list_base);
            }
        }
#ifdef HASH_STAT
        fprintf(stderr, "stats: u:%d s:%d l:%d t:%d c:%d m:%d $:%d\n",
                dep_hash->used, dep_hash->size, dep_hash->load_limit, dep_hash->stored,
                dep_hash->collisions, dep_hash->memcmp, dep_hash->cost);
#endif
    }
#if !ENABLE_RUNTIME_OPTIMIZATIONS
    {
        size_t i;
        hash_destroy(dep_hash);
        for (i = 0; i != file_load_buffer_count; ++i)
        {
            free(file_load_buffers[i]);
        }
    }
#endif
    return rc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
