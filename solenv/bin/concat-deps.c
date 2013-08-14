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
#ifdef _AIX
#define CORE_BIG_ENDIAN
#undef CORE_LITTLE_ENDIAN
#endif /* Def _AIX */

#ifdef _MSC_VER
#define __windows
#undef CORE_BIG_ENDIAN
#define CORE_LITTLE_ENDIAN
#endif /* Def _MSC_VER */

#if defined(__linux) || defined(__OpenBSD__) || \
    defined(__FreeBSD__) || defined(__NetBSD__) || \
    defined(__DragonFly__) || defined(__FreeBSD_kernel__)
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
#endif /* Def __linux || Def *BSD */

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

#ifdef __windows
#include <io.h>
#else
#include <unistd.h>
#endif

/* modes */
#ifdef __windows
#define FILE_O_RDONLY     _O_RDONLY
#define FILE_O_BINARY     _O_BINARY
#define PATHNCMP _strnicmp /* MSVC converts paths to lower-case sometimes? */
#define inline __inline
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

int internal_boost = 0;
static char* base_dir;
static char* work_dir;
int work_dir_len;

#ifdef __GNUC__
#define clz __builtin_clz
#else
static inline int clz(unsigned int value)
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

static inline unsigned int get_unaligned_uint(const unsigned char* cursor)
{
unsigned int   result;

    memcpy(&result, cursor, sizeof(unsigned int));
    return result;
}

/* ===============================================
 * memory pool for fast fix-size allocation (non-tread-safe)
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
#define POOL_ALIGN_INCREMENT 8 /**< Alignement, must be a power of 2 and of size > to sizeof(void*) */


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
struct pool* pool_create(int size_elem, int primary, int secondary)
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

void pool_destroy(struct pool* pool)
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

static inline void* pool_alloc(struct pool* pool)
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
        /* re-used old freed element by chopipng the head of the free list */
        pool->head_free = *(void**)data;
    }

    return data;
}


static inline void pool_free(struct pool* pool, void* data)
{
    assert(pool && data);

    /* stack on top of the free list */
    *(void**)data = pool->head_free;
    pool->head_free = data;
}


/* ===============================================
 * Hash implementation custumized to be just tracking
 * a unique list of string (i.e no data associated
 * with the key, no need for retrieval, etc..
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
    int flags;
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
#define HASH_F_NO_RESIZE (1<<0)

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

static unsigned int hash_compute( struct hash* hash, const char* key, int length)
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
     * a segfault -- if you find a platform/compiler couple for which that postulat
     * is false, then you just need to over-allocate by 2 more bytes in file_load()
     * file_load already over-allocate by 1 to sitck a \0 at the end of the buffer.
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
    hash = calloc(1, sizeof(struct hash));
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
    fprintf(stderr, "resize hash %d -> %d\n", old_size, hash->size);
    if(hash->size == old_size)
    {
        hash->flags |= HASH_F_NO_RESIZE;
        return;
    }
    array = calloc(hash->size + 1, sizeof(struct hash_elem*));
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
        hash->flags |= HASH_F_NO_RESIZE;
    }
}

#ifdef HASH_STAT
static inline int compare_key(struct hash* hash, const char* a, const char* b, int len, int* cost)
{
    *cost += 1;
    hash->memcmp += 1;
    return memcmp(a,b, len);
}
#else
#define compare_key(h,a,b,l,c) memcmp(a,b,l)
#endif

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
        hash_elem = pool_alloc(hash->elems_pool);
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
int rc_local = 0;

    rc_local = stat(name, buffer_stat);
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
    if (!rc)
    {
        fd = open(name, FILE_O_RDONLY | FILE_O_BINARY);
        if (!(fd == -1))
        {
            buffer = malloc((size_t)(*size + 1));
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

static void _cancel_relative(char* base, char** ref_cursor, char** ref_cursor_out, char* end)
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

static inline void eat_space(char ** token)
{
    while ((' ' == **token) || ('\t' == **token)) {
        ++(*token);
    }
}

/*
 * Prune LibreOffice specific duplicate dependencies to improve
 * gnumake startup time, and shrink the disk-space footprint.
 */
static inline int
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
static inline void print_fullpaths(char* line)
{
    char* token;
    char* end;
    int boost_count = 0;
    int token_len;
    const char * unpacked_end = 0; /* end of UnpackedTarget match (if any) */
    /* for UnpackedTarget the target is GenC{,xx}Object, dont mangle! */
    int target_seen = 0;

    token = line;
    eat_space(&token);
    while (*token)
    {
        end = token;
        /* hard to believe that in this day and age drive letters still exist */
        if (*end && (':' == *(end+1)) &&
            (('\\' == *(end+2)) || ('/' == *(end+2))) && isalpha(*end))
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
                unpacked_end = 0;
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
        if (!target_seen)
        {
            if (':' == *token)
            {
                target_seen = 1;
                fputc(':', stdout);
                ++token;
                eat_space(&token);
            }
        }
    }
}

static inline char * eat_space_at_end(char * end)
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
static inline char* generate_phony_line(char* phony_target, char* extension)
{
char* src;
char* dest;
char* last_dot = NULL;
    //fprintf(stderr, "generate_phony_line called with phony_target %s and extension %s\n", phony_target, extension);
    for(dest = phony_content_buffer+work_dir_len, src = phony_target; *src != 0; ++src, ++dest)
    {
        *dest = *src;
        if(*dest == '.')
        {
            last_dot = dest;
        }
    }
    //fprintf(stderr, "generate_phony_line after phony_target copy: %s\n", phony_content_buffer);
    for(dest = last_dot+1, src = extension; *src != 0; ++src, ++dest)
    {
        *dest = *src;
    }
    //fprintf(stderr, "generate_phony_line after extension add: %s\n", phony_content_buffer);
    strcpy(dest, ": $(gb_Helper_PHONY)\n");
    //fprintf(stderr, "generate_phony_line after phony add: %s\n", phony_content_buffer);
    return phony_content_buffer;
}

static inline int generate_phony_file(char* fn, char* content)
{
FILE* depfile;
    depfile = fopen(fn, "w");
    if(depfile)
    {
        fputs(content, depfile);
        fclose(depfile);
    }
    return !depfile;
}

static int _process(struct hash* dep_hash, char* fn)
{
int rc;
char* buffer;
char* end;
char* cursor;
char* cursor_out;
char* base;
char* created_line = NULL;
char* src_relative;
int continuation = 0;
char last_ns = 0;
off_t size;

    buffer = file_load(fn, &size, &rc);
    /* Note: yes we are going to leak 'buffer'
     * this is on purpose, to avoid cloning the 'key' out of it
     * and our special 'hash' just store the pointer to the key
     * inside of buffer, hence it need to remain allocated
     */
    if(!rc)
    {
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
                if(cursor + 3 < end)
                {
                    if(!memcmp(cursor, "/../", 4))
                    {
                        _cancel_relative(base, &cursor, &cursor_out, end);
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
                    created_line = generate_phony_line(src_relative+10, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(fn+work_dir_len+5, "UnoApiPartTarget/", 17) == 0)
                {
                    created_line = generate_phony_line(src_relative+17, "urd");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(fn+work_dir_len+5, "SrsPartTarget/", 14) == 0)
                {
                    created_line = generate_phony_line(src_relative+14, "");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "GenCxxObject/", 13) == 0)
                {
                    created_line = generate_phony_line(src_relative+13, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "CObject/", 8) == 0)
                {
                    created_line = generate_phony_line(src_relative+8, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "GenCObject/", 11) == 0)
                {
                    created_line = generate_phony_line(src_relative+11, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "SdiObject/", 10) == 0)
                {
                    created_line = generate_phony_line(src_relative+10, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "AsmObject/", 10) == 0)
                {
                    created_line = generate_phony_line(src_relative+10, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "ObjCxxObject/", 13) == 0)
                {
                    created_line = generate_phony_line(src_relative+13, "o");
                    rc = generate_phony_file(fn, created_line);
                }
                else if(strncmp(src_relative, "ObjCObject/", 11) == 0)
                {
                    created_line = generate_phony_line(src_relative+11, "o");
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
    return rc;
}

static void _usage(void)
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
        fprintf(stderr,"Error: %s is missing in the environement\n", name);
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
struct hash* dep_hash;
const char *env_str;

    if(argc < 2)
    {
        _usage();
        return 1;
    }
    if(get_var(&base_dir, "SRCDIR") || get_var(&work_dir, "WORKDIR"))
        return 1;
    work_dir_len = strlen(work_dir);
    phony_content_buffer = malloc(PHONY_TARGET_BUFFER);
    strcpy(phony_content_buffer, work_dir);
    phony_content_buffer[work_dir_len] = '/';

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
            if(*in_list_cursor == ' ' || *in_list_cursor == '\n')
            {
                *in_list_cursor = 0;
                if(in_list_base < in_list_cursor)
                {
                    rc = _process(dep_hash, in_list_base);
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
                rc = _process(dep_hash, in_list_base);
            }
        }
#ifdef HASH_STAT
        fprintf(stderr, "stats: u:%d s:%d l:%d t:%d c:%d m:%d $:%d\n",
                dep_hash->used, dep_hash->size, dep_hash->load_limit, dep_hash->stored,
                dep_hash->collisions, dep_hash->memcmp, dep_hash->cost);
#endif
    }
    return rc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
