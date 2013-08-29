/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/** Cheap and cheesy replacement code for liblangtag on systems that do not
    allow / want LGPL code or dependencies on glib.

    XXX NOTE: This code does not check language tags for validity or if they
    are registered with IANA, does not canonicalize or strip default script
    tags if included nor does it do any other fancy stuff that liblangtag is
    capable of. It just makes depending code work without.
 */

#include <cstdlib>
#include <cstring>
#include <cstdio>

namespace {

typedef int lt_bool_t;
typedef void* lt_pointer_t;

struct lt_error_t {
    void *something;
    lt_error_t() : something(NULL) {}
};

static void* g_malloc(size_t s)
{
    return malloc(s);
}

static void g_free(void* p)
{
    if (p)
        free(p);
}

static void lt_error_unref(lt_error_t *error)
{
    if (error)
    {
        g_free( error->something);
        g_free( error);
    }
}

struct my_ref
{
    sal_uInt32 mnRef;
    explicit my_ref() : mnRef(1) {}
    virtual ~my_ref() {}
    void incRef() { ++mnRef; }
    void decRef() { if (--mnRef == 0) delete this; }
};

struct my_t_impl : public my_ref
{
    char* mpStr;
    explicit my_t_impl() : my_ref(), mpStr(NULL) {}
    virtual ~my_t_impl() { g_free( mpStr); }
    explicit my_t_impl( const my_t_impl& r )
        :
            my_ref(),
            mpStr(r.mpStr ? strdup( r.mpStr) : NULL)
    {
    }
    my_t_impl& operator=( const my_t_impl& r )
    {
        if (this == &r)
            return *this;
        g_free( mpStr);
        mpStr = (r.mpStr ? strdup( r.mpStr) : NULL);
        return *this;
    }
    void assign( const char* str )
    {
        g_free( mpStr);
        mpStr = (str ? strdup( str) : NULL);
    }
    void assign( const char* str, const char* stop )
    {
        g_free( mpStr);
        if (str && str < stop)
        {
            mpStr = static_cast<char*>(g_malloc( stop - str + 1));
            memcpy( mpStr, str, stop - str);
            mpStr[stop - str] = 0;
        }
        else
            mpStr = NULL;
    }
    virtual void append( const char* str, const char* stop )
    {
        if (str && str < stop)
        {
            size_t nOld = mpStr ? strlen( mpStr) : 0;
            size_t nNew = nOld + (stop - str) + 1;
            char* p = static_cast<char*>(g_malloc( nNew));
            if (nOld)
                memcpy( p, mpStr, nOld);
            memcpy( p + nOld, str, stop - str);
            p[nNew-1] = 0;
            g_free( mpStr);
            mpStr = p;
        }
    }
    virtual void zero()
    {
        g_free( mpStr);
        mpStr = NULL;
    }
};

struct lt_lang_t : public my_t_impl
{
    explicit lt_lang_t() : my_t_impl() {}
    virtual ~lt_lang_t() {}
};

struct lt_script_t : public my_t_impl
{
    explicit lt_script_t() : my_t_impl() {}
    virtual ~lt_script_t() {}
};

struct lt_region_t : public my_t_impl
{
    explicit lt_region_t() : my_t_impl() {}
    virtual ~lt_region_t() {}
};

struct lt_variant_t : public my_t_impl
{
    explicit lt_variant_t() : my_t_impl() {}
    virtual ~lt_variant_t() {}
};

struct lt_list_t : public my_t_impl
{
    lt_list_t* mpPrev;
    lt_list_t* mpNext;
    explicit lt_list_t() : my_t_impl(), mpPrev(NULL), mpNext(NULL) {}
    explicit lt_list_t( const lt_list_t& r )
        :
            my_t_impl( r), mpPrev(NULL), mpNext(NULL)
    {
    }
    virtual ~lt_list_t()
    {
        if (mpPrev)
            mpPrev->mpNext = mpNext;
        if (mpNext)
            mpNext->mpPrev = mpPrev;
    }
};

static lt_pointer_t lt_list_value( const lt_list_t* p )
{
    // This may look odd, but in this implementation the list element itself
    // holds the char* mpStr to be obtained with lt_variant_get_tag()
    return static_cast<lt_pointer_t>(const_cast<lt_list_t*>(p));
}

static const lt_list_t* lt_list_next( const lt_list_t* p )
{
    return p ? p->mpNext : NULL;
}

static lt_list_t* my_copyList( const lt_list_t * pList )
{
    lt_list_t* pNewList = NULL;
    lt_list_t* pLast = NULL;
    while (pList)
    {
        lt_list_t* p = new lt_list_t( *pList);
        if (!pNewList)
            pNewList = p;
        if (pLast)
        {
            pLast->mpNext = p;
            p->mpPrev = pLast;
        }
        pLast = p;
        pList = pList->mpNext;
    }
    return pNewList;
}

static void my_unrefList( lt_list_t* pList )
{
    while (pList)
    {
        lt_list_t* pNext = pList->mpNext;
        pList->decRef();
        pList = pNext;
    }
}

static void my_appendToList( lt_list_t** ppList, lt_list_t* pEntry )
{
    if (ppList)
    {
        if (!*ppList)
            *ppList = pEntry;
        else
        {
            lt_list_t* pThat = *ppList;
            for (lt_list_t* pNext = pThat->mpNext; pNext; pNext = pThat->mpNext)
                pThat = pNext;
            pThat->mpNext = pEntry;
            pEntry->mpPrev = pThat;
        }
    }
}

// my_t_impl has a superfluous mpStr here, but simplifies things much in the
// parser.
struct my_t_list : public my_t_impl
{
    lt_list_t* mpList;
    explicit my_t_list() : my_t_impl(), mpList(NULL) {}
    explicit my_t_list( const my_t_list& r ) : my_t_impl( r), mpList( my_copyList( r.mpList)) {}
    virtual ~my_t_list()
    {
        my_unrefList( mpList);
    }
    my_t_list& operator=( const my_t_list& r )
    {
        if (this == &r)
            return *this;
        my_t_impl::operator=( r);
        lt_list_t* pList = my_copyList( r.mpList);
        my_unrefList( mpList);
        mpList = pList;
    }
    virtual void append( const char* str, const char* stop )
    {
        lt_list_t* p = new lt_list_t;
        p->assign( str, stop);
        my_appendToList( &mpList, p);
    }
    virtual void zero()
    {
        my_t_impl::zero();
        my_unrefList( mpList);
        mpList = NULL;
    }
};

struct lt_tag_t : public my_t_impl
{
    lt_lang_t   maLanguage;
    lt_script_t maScript;
    lt_region_t maRegion;
    my_t_list   maVariants;
    explicit lt_tag_t() : my_t_impl(), maLanguage(), maScript(), maRegion(), maVariants() {}
    virtual ~lt_tag_t() {}
    explicit lt_tag_t( const lt_tag_t& r )
        :
            my_t_impl( r),
            maLanguage( r.maLanguage),
            maScript( r.maScript),
            maRegion( r.maRegion),
            maVariants( r.maVariants)
    {
    }
    lt_tag_t& operator=( const lt_tag_t& r )
    {
        if (this == &r)
            return *this;
        my_t_impl::operator=( r);
        maLanguage = r.maLanguage;
        maScript = r.maScript;
        maRegion = r.maRegion;
        maVariants = r.maVariants;
        return *this;
    }
    void assign( const char* str )
    {
        maLanguage.zero();
        maScript.zero();
        maRegion.zero();
        maVariants.zero();
        my_t_impl::assign( str);
    }
};

static void lt_db_initialize() { }
static void lt_db_finalize() { }
static void lt_db_set_datadir( const char* /* dir */ ) { }

static lt_tag_t* lt_tag_new(void)
{
    return new lt_tag_t;
}

static lt_tag_t* lt_tag_copy(lt_tag_t *tag)
{
    return (tag ? new lt_tag_t( *tag) : NULL);
}

static void lt_tag_unref(lt_tag_t *tag)
{
    if (tag)
        tag->decRef();
}

/** See http://tools.ietf.org/html/rfc5646

    We are simply ignorant of grandfathered (irregular and regular) subtags and
    may either bail out or accept them, sorry (or not). However, we do accept
    any i-* irregular and x-* privateuse. Subtags are not checked for validity
    (alpha, digit, registered, ...).
 */
static lt_bool_t lt_tag_parse(lt_tag_t *tag,
                              const char *tag_string,
                              lt_error_t **error)
{
    (void) error;
    if (!tag)
        return 0;
    tag->assign( tag_string);
    if (!tag_string)
        return 0;
    // In case we supported other subtags this would get more complicated.
    my_t_impl* aSubtags[] = { &tag->maLanguage, &tag->maScript, &tag->maRegion, &tag->maVariants, NULL };
    my_t_impl** ppSub = &aSubtags[0];
    const char* pStart = tag_string;
    const char* p = pStart;
    const char* pEnd = pStart + strlen( pStart);   // scanning includes \0
    bool bStartLang = true;
    bool bPrivate = false;
    for ( ; p <= pEnd && ppSub && *ppSub; ++p)
    {
        if (p == pEnd || *p == '-')
        {
            size_t nLen = p - pStart;
            if (*ppSub == &tag->maLanguage)
            {
                if (bStartLang)
                {
                    bStartLang = false;
                    switch (nLen)
                    {
                        case 1:     // irregular or privateuse
                            if (*pStart == 'i' || *pStart == 'x')
                            {
                                (*ppSub)->assign( pStart, p);
                                bPrivate = true;
                            }
                            else
                                return 0;   // bad
                            break;
                        case 2:     // ISO 639 alpha-2
                        case 3:     // ISO 639 alpha-3
                            (*ppSub)->assign( pStart, p);
                            break;
                        case 4:     // reserved for future use
                            return 0;   // bad
                            break;
                        case 5:
                        case 6:
                        case 7:
                        case 8:     // registered language subtag
                            (*ppSub++)->assign( pStart, p);
                            break;
                        default:
                            return 0;   // bad
                    }
                }
                else
                {
                    if (nLen > 8)
                        return 0;   // bad
                    if (bPrivate)
                    {
                        // Any combination of  "x" 1*("-" (2*8alphanum))
                        // allowed, store first as language and return ok.
                        // For i-* simply assume the same.
                        (*ppSub)->append( pStart-1, p);
                        return !0;  // ok
                    }
                    else if (nLen == 3)
                    {
                        // extlang subtag, 1 to 3 allowed we don't check that.
                        // But if it's numeric it's a region UN M.49 code
                        // instead and no script subtag is present, so advance.
                        if ('0' <= *pStart && *pStart <= '9')
                        {
                            ppSub += 2; // &tag->maRegion XXX watch this when inserting fields
                            --p;
                            continue;   // for
                        }
                        else
                            (*ppSub)->append( pStart-1, p);
                    }
                    else
                    {
                        // Not part of language subtag, advance.
                        ++ppSub;
                        --p;
                        continue;   // for
                    }
                }
            }
            else if (*ppSub == &tag->maScript)
            {
                switch (nLen)
                {
                    case 4:
                        // script subtag, or a (DIGIT 3alphanum) variant with
                        // no script and no region
                        if ('0' <= *pStart && *pStart <= '9')
                        {
                            ppSub += 2; // &tag->maVariants XXX watch this when inserting fields
                            --p;
                            continue;   // for
                        }
                        else
                            (*ppSub++)->assign( pStart, p);
                        break;
                    case 3:
                        // This may be a region UN M.49 code if 3DIGIT and no
                        // script code present. Just check first character and
                        // advance.
                        if ('0' <= *pStart && *pStart <= '9')
                        {
                            ++ppSub;
                            --p;
                            continue;   // for
                        }
                        else
                            return 0;   // bad
                        break;
                    case 2:
                        // script omitted, region subtag, advance.
                        ++ppSub;
                        --p;
                        continue;   // for
                        break;
                    case 1:
                        // script omitted, region omitted, extension subtag
                        // with singleton, stop parsing
                        ppSub = NULL;
                        break;
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                        // script omitted, region omitted, variant subtag
                        ppSub += 2; // &tag->maVariants XXX watch this when inserting fields
                        --p;
                        continue;   // for
                        break;
                    default:
                        return 0;   // bad
                }
            }
            else if (*ppSub == &tag->maRegion)
            {
                if (nLen == 2 || nLen == 3)
                    (*ppSub++)->assign( pStart, p);
                else
                {
                    // advance to variants
                    ++ppSub;
                    --p;
                    continue;   // for
                }
            }
            else if (*ppSub == &tag->maVariants)
            {
                // Stuff the remainder into variants, might not be correct, but ...
                switch (nLen)
                {
                    case 4:
                        // a (DIGIT 3alphanum) variant
                        if ('0' <= *pStart && *pStart <= '9')
                            ;   // nothing
                        else
                            return 0;   // bad
                        break;
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                        ;   // nothing, variant
                        break;
                    default:
                        return 0;   // bad
                }
                (*ppSub)->append( pStart, p);
            }
            pStart = p+1;
        }
    }
    return !0;
}

static char* lt_tag_canonicalize(lt_tag_t *tag,
                                 lt_error_t **error)
{
    (void) error;
    return tag && tag->mpStr ? strdup( tag->mpStr) : NULL;
}

static const lt_lang_t* lt_tag_get_language(const lt_tag_t  *tag)
{
    return tag && tag->maLanguage.mpStr ? &tag->maLanguage : NULL;
}

static const lt_script_t *lt_tag_get_script(const lt_tag_t  *tag)
{
    return tag && tag->maScript.mpStr ? &tag->maScript : NULL;
}

static const lt_region_t *lt_tag_get_region(const lt_tag_t  *tag)
{
    return tag && tag->maRegion.mpStr ? &tag->maRegion : NULL;
}

static const lt_list_t *lt_tag_get_variants(const lt_tag_t  *tag)
{
    return tag ? tag->maVariants.mpList : NULL;
}

static const char *lt_lang_get_tag(const lt_lang_t *lang)
{
    return lang ? lang->mpStr : NULL;
}

static const char *lt_script_get_tag(const lt_script_t *script)
{
    return script ? script->mpStr : NULL;
}

static const char *lt_region_get_tag(const lt_region_t *region)
{
    return region ? region->mpStr : NULL;
}

static const char *lt_variant_get_tag(const lt_variant_t *variant)
{
    return variant ? variant->mpStr : NULL;
}

#ifdef erDEBUG
static void lt_tag_dump(const lt_tag_t *tag)
{
    fprintf( stderr, "\n");
    fprintf( stderr, "SimpleLangtag  langtag: %s\n", tag->mpStr);
    fprintf( stderr, "SimpleLangtag language: %s\n", tag->maLanguage.mpStr);
    fprintf( stderr, "SimpleLangtag   script: %s\n", tag->maScript.mpStr);
    fprintf( stderr, "SimpleLangtag   region: %s\n", tag->maRegion.mpStr);
}
#endif

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
