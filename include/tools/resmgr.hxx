/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_TOOLS_RESMGR_HXX
#define INCLUDED_TOOLS_RESMGR_HXX

#include <tools/toolsdllapi.h>
#include <i18nlangtag/languagetag.hxx>
#include <tools/resid.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <vector>

class SvStream;
class InternalResMgr;

/// Defines structure used to build resource
struct RSHEADER_TYPE
{
private:
    char nId[4];        ///< Identifier of resource
    char nRT[4];        ///< Resource type
    char nGlobOff[4];   ///< Global offset
    char nLocalOff[4];  ///< Local offset

public:
    inline sal_uInt32       GetId();    ///< Identifier of resource
    inline RESOURCE_TYPE    GetRT();    ///< Resource type
    inline sal_uInt32       GetGlobOff();  ///< Global offset
    inline sal_uInt32       GetLocalOff(); ///< Local offset
};

typedef OUString (*ResHookProc)( const OUString& rStr );

// Initialization
enum class RCFlags
{
    NONE                 = 0x00,
    GLOBAL               = 0x01,  // Global resource
    AUTORELEASE          = 0x02,
    NOTFOUND             = 0x04,
    FALLBACK_DOWN        = 0x08,
    FALLBACK_UP          = 0x10,
};
namespace o3tl
{
    template<> struct typed_flags<RCFlags> : is_typed_flags<RCFlags, 0x1f> {};
}

class Resource;
class ResMgr;

struct ImpRCStack
{
    // pResource and pClassRes equal NULL: resource was not loaded
    RSHEADER_TYPE * pResource;  ///< pointer to resource
    void          * pClassRes;  ///< pointer to class specified init data
    RCFlags         Flags;      ///< resource status
    void *          aResHandle; ///< Resource-Identifier from InternalResMgr
    const Resource* pResObj;    ///< pointer to Resource object
    sal_uInt32      nId;        ///< ResId used for error message
    ResMgr*         pResMgr;    ///< ResMgr for Resource pResObj

    void            Clear();
    void            Init( ResMgr * pMgr, const Resource * pObj, sal_uInt32 nId );
};

class TOOLS_DLLPUBLIC ResMgr
{
private:
    InternalResMgr* pImpRes;
    std::vector< ImpRCStack >  aStack; ///< resource context stack
    int             nCurStack;
    ResMgr*         pFallbackResMgr;   ///< fallback ResMgr in case the Resource
                                       ///< was not contained in this ResMgr
    ResMgr*         pOriginalResMgr;   ///< the res mgr that fell back to this
                                       ///< stack level

    TOOLS_DLLPRIVATE void incStack();
    TOOLS_DLLPRIVATE void decStack();

    TOOLS_DLLPRIVATE const ImpRCStack * StackTop( sal_uInt32 nOff = 0 ) const
    {
        return (((int)nOff >= nCurStack) ? NULL : &aStack[nCurStack-nOff]);
    }
    TOOLS_DLLPRIVATE void  Init( const OUString& rFileName );

    TOOLS_DLLPRIVATE ResMgr( InternalResMgr * pImp );

    #ifdef DBG_UTIL
    TOOLS_DLLPRIVATE static void RscError_Impl( const sal_Char* pMessage,
                                                ResMgr* pResMgr,
                                                RESOURCE_TYPE nRT,
                                                sal_uInt32 nId,
                                                std::vector< ImpRCStack >& rResStack,
                                                int nDepth );
    #endif

    // called from within GetResource() if a resource could not be found
    TOOLS_DLLPRIVATE ResMgr* CreateFallbackResMgr( const ResId& rId,
                                                   const Resource* pResource );
    // creates a 1k sized buffer set to zero for unfound resources
    // used in case RC_NOTFOUND
    static void* pEmptyBuffer;
    TOOLS_DLLPRIVATE static void* getEmptyBuffer();

    // the next two methods are needed to prevent the string hook called
    // with the res mgr mutex locked
    // like GetString, but doesn't call the string hook
    TOOLS_DLLPRIVATE static sal_uInt32 GetStringWithoutHook( OUString& rStr,
                                                             const sal_uInt8* pStr );
    // like ReadString but doesn't call the string hook
    TOOLS_DLLPRIVATE OUString ReadStringWithoutHook();

    ResMgr(const ResMgr&) = delete;
    ResMgr& operator=(const ResMgr&) = delete;

public:
    static void         DestroyAllResMgr();  ///< Called upon app shutdown

    ~ResMgr();

    /// Language-dependent resource library
    static ResMgr*      SearchCreateResMgr( const sal_Char* pPrefixName,
                                            LanguageTag& rLocale );
    static ResMgr*      CreateResMgr( const sal_Char* pPrefixName,
                                      const LanguageTag& aLocale = LanguageTag( LANGUAGE_SYSTEM) );

    #ifdef DBG_UTIL
    /// Test whether resource still exists
    void                TestStack();
    #endif

    /// Check whether resource is available
    bool                IsAvailable( const ResId& rId,
                                     const Resource* = NULL) const;

    /// Search and load resource, given its ID
    bool                GetResource( const ResId& rId, const Resource * = NULL );
    static void *       GetResourceSkipHeader( const ResId& rResId, ResMgr ** ppResMgr );
    /// Free resource context
    void                PopContext( const Resource* = NULL );

    /// Increment resource pointer
    void*               Increment( sal_uInt32 nSize );

    /// Size of an object within the resource
    static sal_uInt32   GetObjSize( RSHEADER_TYPE* pHT )
                            { return( pHT->GetGlobOff() ); }

    /// Return a string and its length out of the resource
    static sal_uInt32   GetString( OUString& rStr, const sal_uInt8* pStr );
    /// Return a byte string and its length out of the resource
    static sal_uInt32   GetByteString( OString& rStr, const sal_uInt8* pStr );

    /// Return the size of a string in the resource
    static sal_uInt32   GetStringSize( sal_uInt32 nLen )
                            { nLen++; return (nLen + nLen%2); }
    static sal_uInt32   GetStringSize( const sal_uInt8* pStr, sal_uInt32& nLen );

    /// Return a int64
    static sal_uInt64   GetUInt64( void* pDatum );
    /// Return a long
    static sal_Int32    GetLong( void * pLong );
    /// Return a short
    static sal_Int16    GetShort( void * pShort );

    /// Return a pointer to the resource
    void *              GetClass();

    RSHEADER_TYPE *     CreateBlock( const ResId & rId );

    sal_uInt32          GetRemainSize();

    const OUString& GetFileName() const;

    sal_Int16           ReadShort();
    sal_Int32           ReadLong();
    OUString            ReadString();
    OString        ReadByteString();

    /// Generate auto help ID for current resource stack
    OString        GetAutoHelpId();

    static void         SetReadStringHook( ResHookProc pProc );
    static ResHookProc  GetReadStringHook();
    static void         SetDefaultLocale( const LanguageTag& rLocale );
};

inline sal_uInt32 RSHEADER_TYPE::GetId()
{
    return (sal_uInt32)ResMgr::GetLong( &nId );
}

inline RESOURCE_TYPE RSHEADER_TYPE::GetRT()
{
    return (RESOURCE_TYPE)ResMgr::GetLong( &nRT );
}

inline sal_uInt32 RSHEADER_TYPE::GetGlobOff()
{
    return (sal_uInt32)ResMgr::GetLong( &nGlobOff );
}

inline sal_uInt32 RSHEADER_TYPE::GetLocalOff()
{
    return (sal_uInt32)ResMgr::GetLong( &nLocalOff );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
