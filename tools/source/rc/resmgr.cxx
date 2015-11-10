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

#include <config_folders.h>

#include <sal/config.h>

#include <cassert>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/resmgr.hxx>
#include <tools/rc.hxx>
#include <tools/rcid.h>
#include <osl/endian.h>
#include <osl/process.h>
#include <osl/thread.h>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <osl/signal.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <rtl/instance.hxx>
#include <rtl/bootstrap.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <tools/simplerm.hxx>

#include <algorithm>
#include <functional>
#include <list>
#include <set>
#include <unordered_map>

using namespace osl;

// for thread safety
static osl::Mutex* pResMgrMutex = nullptr;

static osl::Mutex& getResMgrMutex()
{
    if( !pResMgrMutex )
    {
        osl::Guard<osl::Mutex> aGuard( *osl::Mutex::getGlobalMutex() );
        if( ! pResMgrMutex )
            pResMgrMutex = new osl::Mutex();
    }
    return *pResMgrMutex;
}

struct ImpContent;

class InternalResMgr
{
    friend class ResMgr;
    friend class SimpleResMgr;
    friend class ResMgrContainer;

    ImpContent *                    pContent;
    sal_uInt32                      nOffCorrection;
    sal_uInt8 *                     pStringBlock;
    SvStream *                      pStm;
    bool                            bEqual2Content;
    sal_uInt32                      nEntries;
    OUString                        aFileName;
    OUString                        aPrefix;
    OUString                        aResName;
    bool                            bSingular;
    LanguageTag                     aLocale;
    std::unordered_map<sal_uInt64, int>* pResUseDump;

                            InternalResMgr( const OUString& rFileURL,
                                            const OUString& aPrefix,
                                            const OUString& aResName,
                                            const LanguageTag& rLocale );
                            ~InternalResMgr();
    bool                    Create();

    bool                    IsGlobalAvailable( RESOURCE_TYPE nRT, sal_uInt32 nId ) const;
    void *                  LoadGlobalRes( RESOURCE_TYPE nRT, sal_uInt32 nId,
                                           void **pResHandle );
public:
    static void             FreeGlobalRes( void *, void * );
};

class ResMgrContainer
{
    static ResMgrContainer*     pOneInstance;

    struct ContainerElement
    {
        InternalResMgr* pResMgr;
        OUString        aFileURL;
        int             nRefCount;
        int             nLoadCount;

        ContainerElement() :
            pResMgr( nullptr ),
            nRefCount( 0 ),
            nLoadCount( 0 )
            {}
    };

    std::unordered_map< OUString, ContainerElement, OUStringHash> m_aResFiles;
    LanguageTag     m_aDefLocale;

    ResMgrContainer() : m_aDefLocale( LANGUAGE_SYSTEM) { init(); }
    ~ResMgrContainer();

    void init();

public:
    static ResMgrContainer& get();
    static void release();

    InternalResMgr* getResMgr( const OUString& rPrefix,
                               LanguageTag& rLocale,
                               bool bForceNewInstance = false
                               );
    InternalResMgr* getNextFallback( InternalResMgr* pResMgr );

    void freeResMgr( InternalResMgr* pResMgr );

    void setDefLocale( const LanguageTag& rLocale )
    { m_aDefLocale = rLocale; }
    const LanguageTag& getDefLocale() const
    { return m_aDefLocale; }
};

ResMgrContainer* ResMgrContainer::pOneInstance = nullptr;

ResMgrContainer& ResMgrContainer::get()
{
    if( ! pOneInstance )
        pOneInstance = new ResMgrContainer();
    return *pOneInstance;
}

ResMgrContainer::~ResMgrContainer()
{
    for( std::unordered_map< OUString, ContainerElement, OUStringHash >::iterator it =
            m_aResFiles.begin(); it != m_aResFiles.end(); ++it )
    {
        OSL_TRACE( "Resource file %s loaded %d times",
                         OUStringToOString( it->second.aFileURL, osl_getThreadTextEncoding() ).getStr(),
                         it->second.nLoadCount );
        delete it->second.pResMgr;
    }
}

void ResMgrContainer::release()
{
    delete pOneInstance;
    pOneInstance = nullptr;
}

void ResMgrContainer::init()
{
    assert( m_aResFiles.empty() );

    // get resource path
    OUString uri("$BRAND_BASE_DIR/" LIBO_SHARE_RESOURCE_FOLDER "/");
    rtl::Bootstrap::expandMacros(uri); //TODO: detect failure

    // collect all possible resource files
    Directory aDir( uri );
    if( aDir.open() == FileBase::E_None )
    {
        DirectoryItem aItem;
        while( aDir.getNextItem( aItem ) == FileBase::E_None )
        {
            FileStatus aStatus(osl_FileStatus_Mask_FileName);
            if( aItem.getFileStatus( aStatus ) == FileBase::E_None )
            {
                OUString aFileName = aStatus.getFileName();
                if( ! aFileName.endsWithIgnoreAsciiCase( ".res" ) )
                    continue;
                OUString aResName = aFileName.copy( 0, aFileName.getLength() - strlen(".res") );
                if( aResName.isEmpty() )
                    continue;
                assert( m_aResFiles.find( aResName ) == m_aResFiles.end() );
                m_aResFiles[ aResName ].aFileURL = uri + aFileName;
                SAL_INFO(
                    "tools.rc",
                    "ResMgrContainer: " << aResName << " -> "
                        << m_aResFiles[ aResName ].aFileURL );
            }
        }
    }
    else
        SAL_WARN( "tools.rc", "opening dir " << uri << " failed" );

    // set default language
    LanguageType nLang = MsLangId::getSystemUILanguage();
    m_aDefLocale.reset( nLang);
}

namespace
{
    bool isAlreadyPureenUS(const LanguageTag &rLocale)
    {
        return ( rLocale.getLanguageType() == LANGUAGE_ENGLISH_US );
    }
}

InternalResMgr* ResMgrContainer::getResMgr( const OUString& rPrefix,
                                            LanguageTag& rLocale,
                                            bool bForceNewInstance
                                            )
{
    LanguageTag aLocale( rLocale );
    std::unordered_map< OUString, ContainerElement, OUStringHash >::iterator it = m_aResFiles.end();

    ::std::vector< OUString > aFallbacks( aLocale.getFallbackStrings( true));
    if (!isAlreadyPureenUS( aLocale))
        aFallbacks.push_back( "en-US");     // last resort if all fallbacks fail

    for (::std::vector< OUString >::const_iterator fb( aFallbacks.begin()); fb != aFallbacks.end(); ++fb)
    {
        OUString aSearch( rPrefix + *fb );
        it = m_aResFiles.find( aSearch );
        if( it != m_aResFiles.end() )
        {
            // ensure InternalResMgr existence
            if( ! it->second.pResMgr )
            {
                InternalResMgr* pImp =
                    new InternalResMgr( it->second.aFileURL, rPrefix, it->first, aLocale );
                if( ! pImp->Create() )
                {
                    delete pImp;
                    continue;
                }
                it->second.pResMgr = pImp;
            }
            break;
        }
    }
    // try if there is anything with this prefix at all
    if( it == m_aResFiles.end() )
    {
        aLocale.reset( LANGUAGE_SYSTEM);
        it = m_aResFiles.find( rPrefix );
        if( it == m_aResFiles.end() )
        {
            for( it = m_aResFiles.begin(); it != m_aResFiles.end(); ++it )
            {
                if( it->first.matchIgnoreAsciiCase( rPrefix ) )
                {
                    // ensure InternalResMgr existence
                    if( ! it->second.pResMgr )
                    {
                        InternalResMgr* pImp =
                            new InternalResMgr( it->second.aFileURL,
                                                rPrefix,
                                                it->first,
                                                aLocale );
                        if( ! pImp->Create() )
                        {
                            delete pImp;
                            continue;
                        }
                        it->second.pResMgr = pImp;
                    }
                    // try to guess locale
                    sal_Int32 nIndex = rPrefix.getLength();
                    if (nIndex < it->first.getLength())
                        aLocale.reset( it->first.copy( nIndex));
                    else
                    {
                        SAL_WARN( "tools.rc", "ResMgrContainer::getResMgr: it->first " <<
                                it->first << " shorter than prefix " << rPrefix);
                    }
                    break;
                }
            }
        }
    }
    // give up
    if( it == m_aResFiles.end() )
    {
        OUString sURL = rPrefix + rLocale.getBcp47() + ".res";
        if ( m_aResFiles.find(sURL) == m_aResFiles.end() )
        {
            m_aResFiles[ sURL ].aFileURL = sURL;
            return getResMgr(rPrefix,rLocale,bForceNewInstance);
        } // if ( m_aResFiles.find(sURL) == m_aResFiles.end() )
        return nullptr;
    }

    rLocale = aLocale;
    // at this point it->second.pResMgr must be filled either by creating a new one
    // (then the refcount is still 0) or because we already had one
    InternalResMgr* pImp = it->second.pResMgr;

    if( it->second.nRefCount == 0 )
        it->second.nLoadCount++;

    // for SimpleResMgr
    if( bForceNewInstance )
    {
        if( it->second.nRefCount == 0 )
        {
            // shortcut: the match algorithm already created the InternalResMgr
            // take it instead of creating yet another one
            it->second.pResMgr = nullptr;
            pImp->bSingular = true;
        }
        else
        {
            pImp = new InternalResMgr( it->second.aFileURL, rPrefix, it->first, aLocale );
            pImp->bSingular = true;
            if( !pImp->Create() )
            {
                delete pImp;
                pImp = nullptr;
            }
            else
                it->second.nLoadCount++;
        }
    }
    else
        it->second.nRefCount++;

    return pImp;
}

InternalResMgr* ResMgrContainer::getNextFallback( InternalResMgr* pMgr )
{
    /* TODO-BCP47: this is nasty, but the previous code simply stripped a
     * locale's variant and country in subsequent calls to end up with language
     * only and then fallback to en-US if all failed, so this is at least
     * equivalent if not better. Maybe this method could be changed to get
     * passed / remember a fallback list and an index within to pick the next.
     * */

    ::std::vector< OUString > aFallbacks( pMgr->aLocale.getFallbackStrings( true));
    // The first is the locale itself, use next fallback or en-US.
    /* TODO: what happens if the chain is "en-US", "en" -> "en-US", ...
     * This was already an issue with the previous code. */
    LanguageTag aLocale( ((aFallbacks.size() > 1) ? aFallbacks[1] : OUString( "en-US")));
    InternalResMgr* pNext = getResMgr( pMgr->aPrefix, aLocale, pMgr->bSingular );
    // prevent recursion
    if( pNext == pMgr || ( pNext && pNext->aResName.equals( pMgr->aResName ) ) )
    {
        if( pNext->bSingular )
            delete pNext;
        pNext = nullptr;
    }
    return pNext;
}

void ResMgrContainer::freeResMgr( InternalResMgr* pResMgr )
{
    if( pResMgr->bSingular )
        delete pResMgr;
    else
    {
        std::unordered_map< OUString, ContainerElement, OUStringHash >::iterator it =
        m_aResFiles.find( pResMgr->aResName );
        if( it != m_aResFiles.end() )
        {
            DBG_ASSERT( it->second.nRefCount > 0, "InternalResMgr freed too often" );
            if( it->second.nRefCount > 0 )
                it->second.nRefCount--;
            if( it->second.nRefCount == 0 )
            {
                delete it->second.pResMgr;
                it->second.pResMgr = nullptr;
            }
        }
    }
}

#ifdef DBG_UTIL
void Resource::TestRes()
{
    if( m_pResMgr )
        m_pResMgr->TestStack();
}
#endif

struct ImpContent
{
    sal_uInt64   nTypeAndId;
    sal_uInt32   nOffset;
};

struct ImpContentLessCompare : public ::std::binary_function< ImpContent, ImpContent, bool>
{
    inline bool operator() (const ImpContent& lhs, const ImpContent& rhs) const
    {
        return lhs.nTypeAndId < rhs.nTypeAndId;
    }
};

static ResHookProc pImplResHookProc = nullptr;

InternalResMgr::InternalResMgr( const OUString& rFileURL,
                                const OUString& rPrefix,
                                const OUString& rResName,
                                const LanguageTag& rLocale )
    : pContent( nullptr )
    , nOffCorrection( 0 )
    , pStringBlock( nullptr )
    , pStm( nullptr )
    , bEqual2Content( true )
    , nEntries( 0 )
    , aFileName( rFileURL )
    , aPrefix( rPrefix )
    , aResName( rResName )
    , bSingular( false )
    , aLocale( rLocale )
    , pResUseDump( nullptr )
{
}

InternalResMgr::~InternalResMgr()
{
    rtl_freeMemory(pContent);
    rtl_freeMemory(pStringBlock);
    delete pStm;

#ifdef DBG_UTIL
    if( pResUseDump )
    {
        const sal_Char* pLogFile = getenv( "STAR_RESOURCE_LOGGING" );
        if ( pLogFile )
        {
            SvFileStream aStm( OUString::createFromAscii( pLogFile ), StreamMode::WRITE );
            aStm.Seek( STREAM_SEEK_TO_END );
            OStringBuffer aLine("FileName: ");
            aLine.append(OUStringToOString(aFileName,
                RTL_TEXTENCODING_UTF8));
            aStm.WriteLine(aLine.makeStringAndClear());

            for( std::unordered_map<sal_uInt64, int>::const_iterator it = pResUseDump->begin();
                 it != pResUseDump->end(); ++it )
            {
                sal_uInt64 nKeyId = it->first;
                aLine.append("Type/Id: ");
                aLine.append(sal::static_int_cast< sal_Int32 >((nKeyId >> 32) & 0xFFFFFFFF));
                aLine.append('/');
                aLine.append(sal::static_int_cast< sal_Int32 >(nKeyId & 0xFFFFFFFF));
                aStm.WriteLine(aLine.makeStringAndClear());
            }
        }
    }
#endif

    delete pResUseDump;
}

bool InternalResMgr::Create()
{
    ResMgrContainer::get();
    bool bDone = false;

    pStm = new SvFileStream( aFileName, StreamMode::READ | StreamMode::SHARE_DENYWRITE | StreamMode::NOCREATE );
    if( pStm->GetError() == 0 )
    {
        sal_Int32   lContLen = 0;

        pStm->Seek( STREAM_SEEK_TO_END );
        /*
        if( ( pInternalResMgr->pHead = (RSHEADER_TYPE *)mmap( 0, nResourceFileSize,
                                                        PROT_READ, MAP_PRIVATE,
                                                        fRes, 0 ) ) != (RSHEADER_TYPE *)-1)
                                                        */
        pStm->SeekRel( - (int)sizeof( lContLen ) );
        pStm->Read( &lContLen, sizeof( lContLen ) );
        // is bigendian, swab to the right endian
        lContLen = ResMgr::GetLong( &lContLen );
        pStm->SeekRel( -lContLen );
        // allocate stored ImpContent data (12 bytes per unit)
        sal_uInt8* pContentBuf = static_cast<sal_uInt8*>(rtl_allocateMemory( lContLen ));
        pStm->Read( pContentBuf, lContLen );
        // allocate ImpContent space (sizeof(ImpContent) per unit, not necessarily 12)
        pContent = static_cast<ImpContent *>(rtl_allocateMemory( sizeof(ImpContent)*lContLen/12 ));
        // Shorten to number of ImpContent
        nEntries = (sal_uInt32)lContLen / 12;
        bEqual2Content = true;
        bool bSorted = true;
        if( nEntries )
        {
#ifdef DBG_UTIL
            const sal_Char* pLogFile = getenv( "STAR_RESOURCE_LOGGING" );
            if ( pLogFile )
            {
                pResUseDump = new std::unordered_map<sal_uInt64, int>;
                for( sal_uInt32 i = 0; i < nEntries; ++i )
                    (*pResUseDump)[pContent[i].nTypeAndId] = 1;
            }
#endif
            // swap the content to the right endian
            pContent[0].nTypeAndId = ResMgr::GetUInt64( pContentBuf );
            pContent[0].nOffset = ResMgr::GetLong( pContentBuf+8 );
            sal_uInt32 nCount = nEntries - 1;
            for( sal_uInt32 i = 0,j=1; i < nCount; ++i,++j )
            {
                // swap the content to the right endian
                pContent[j].nTypeAndId = ResMgr::GetUInt64( pContentBuf + (12*j) );
                pContent[j].nOffset = ResMgr::GetLong( pContentBuf + (12*j+8) );
                if( pContent[i].nTypeAndId >= pContent[j].nTypeAndId )
                    bSorted = false;
                if( (pContent[i].nTypeAndId & 0xFFFFFFFF00000000LL) == (pContent[j].nTypeAndId & 0xFFFFFFFF00000000LL)
                    && pContent[i].nOffset >= pContent[j].nOffset )
                    bEqual2Content = false;
            }
        }
        rtl_freeMemory( pContentBuf );
        OSL_ENSURE( bSorted, "content not sorted" );
        OSL_ENSURE( bEqual2Content, "resource structure wrong" );
        if( !bSorted )
            ::std::sort(pContent,pContent+nEntries,ImpContentLessCompare());
            //  qsort( pContent, nEntries, sizeof( ImpContent ), Compare );

        bDone = true;
    }

    return bDone;
}


bool InternalResMgr::IsGlobalAvailable( RESOURCE_TYPE nRT, sal_uInt32 nId ) const
{
    // Anfang der Strings suchen
    ImpContent aValue;
    aValue.nTypeAndId = ((sal_uInt64(nRT) << 32) | nId);
    ImpContent * pFind = ::std::lower_bound(pContent,
                                            pContent + nEntries,
                                            aValue,
                                            ImpContentLessCompare());
    return (pFind != (pContent + nEntries)) && (pFind->nTypeAndId == aValue.nTypeAndId);
}


void* InternalResMgr::LoadGlobalRes( RESOURCE_TYPE nRT, sal_uInt32 nId,
                                     void **pResHandle )
{
#ifdef DBG_UTIL
    if( pResUseDump )
        pResUseDump->erase( (sal_uInt64(nRT) << 32) | nId );
#endif
    // search beginning of string
    ImpContent aValue;
    aValue.nTypeAndId = ((sal_uInt64(nRT) << 32) | nId);
    ImpContent* pEnd = (pContent + nEntries);
    ImpContent* pFind = ::std::lower_bound( pContent,
                                            pEnd,
                                            aValue,
                                            ImpContentLessCompare());
    if( pFind && (pFind != pEnd) && (pFind->nTypeAndId == aValue.nTypeAndId) )
    {
        if( nRT == RSC_STRING && bEqual2Content )
        {
            // string optimization
            if( !pStringBlock )
            {
                // search beginning of string
                ImpContent * pFirst = pFind;
                ImpContent * pLast = pFirst;
                while( pFirst > pContent && ((pFirst -1)->nTypeAndId >> 32) == RSC_STRING )
                    --pFirst;
                while( pLast < pEnd && (pLast->nTypeAndId >> 32) == RSC_STRING )
                    ++pLast;
                nOffCorrection = pFirst->nOffset;
                sal_uInt32 nSize;
                --pLast;
                pStm->Seek( pLast->nOffset );
                RSHEADER_TYPE aHdr;
                pStm->Read( &aHdr, sizeof( aHdr ) );
                nSize = pLast->nOffset + aHdr.GetGlobOff() - nOffCorrection;
                pStringBlock = static_cast<sal_uInt8*>(rtl_allocateMemory( nSize ));
                pStm->Seek( pFirst->nOffset );
                pStm->Read( pStringBlock, nSize );
            }
            *pResHandle = pStringBlock;
            return pStringBlock + pFind->nOffset - nOffCorrection;
        } // if( nRT == RSC_STRING && bEqual2Content )
        else
        {
            *pResHandle = nullptr;
            RSHEADER_TYPE aHeader;
            pStm->Seek( pFind->nOffset );
            pStm->Read( &aHeader, sizeof( RSHEADER_TYPE ) );
            void * pRes = rtl_allocateMemory( aHeader.GetGlobOff() );
            memcpy( pRes, &aHeader, sizeof( RSHEADER_TYPE ) );
            pStm->Read( static_cast<sal_uInt8*>(pRes) + sizeof( RSHEADER_TYPE ),
                        aHeader.GetGlobOff() - sizeof( RSHEADER_TYPE ) );
            return pRes;
        }
    } // if( pFind && (pFind != pEnd) && (pFind->nTypeAndId == nValue) )
    *pResHandle = nullptr;
    return nullptr;
}

void InternalResMgr::FreeGlobalRes( void * pResHandle, void * pResource )
{
    if ( !pResHandle )
        // Free allocated resource
        rtl_freeMemory(pResource);
}

#ifdef DBG_UTIL

OUString GetTypeRes_Impl( const ResId& rTypeId )
{
    // Return on resource errors
    static bool bInUse = false;
    OUString aTypStr(OUString::number(rTypeId.GetId()));

    if ( !bInUse )
    {
        bInUse = true;

        ResId aResId( sal_uInt32(RSCVERSION_ID), *rTypeId.GetResMgr() );
        aResId.SetRT( RSC_VERSIONCONTROL );

        if ( rTypeId.GetResMgr()->GetResource( aResId ) )
        {
            rTypeId.SetRT( RSC_STRING );
            if ( rTypeId.GetResMgr()->IsAvailable( rTypeId ) )
            {
                aTypStr = rTypeId.toString();
                // Set class pointer to the end
                rTypeId.GetResMgr()->Increment( sizeof( RSHEADER_TYPE ) );
            }
        }
        bInUse = false;
    }

    return aTypStr;
}

void ResMgr::RscError_Impl( const sal_Char* pMessage, ResMgr* pResMgr,
                            RESOURCE_TYPE nRT, sal_uInt32 nId,
                            std::vector< ImpRCStack >& rResStack, int nDepth )
{
    // create a separate ResMgr with its own stack
    // first get a second reference of the InternalResMgr
    InternalResMgr* pImp =
        ResMgrContainer::get().getResMgr( pResMgr->pImpRes->aPrefix,
                                          pResMgr->pImpRes->aLocale,
                                          true );

    ResMgr* pNewResMgr = new ResMgr( pImp );

    OStringBuffer aStr(OUStringToOString(pResMgr->GetFileName(),
        RTL_TEXTENCODING_UTF8));

    if (aStr.getLength())
        aStr.append('\n');

    aStr.append("Class: ");
    aStr.append(OUStringToOString(GetTypeRes_Impl(ResId(nRT, *pNewResMgr)),
        RTL_TEXTENCODING_UTF8));
    aStr.append(", Id: ");
    aStr.append(static_cast<sal_Int32>(nId));
    aStr.append(". ");
    aStr.append(pMessage);

    aStr.append("\nResource Stack\n");
    while( nDepth > 0 )
    {
        aStr.append("Class: ");
        aStr.append(OUStringToOString(GetTypeRes_Impl(
            ResId(rResStack[nDepth].pResource->GetRT(), *pNewResMgr)),
            RTL_TEXTENCODING_UTF8));
        aStr.append(", Id: ");
        aStr.append(static_cast<sal_Int32>(
            rResStack[nDepth].pResource->GetId()));
        nDepth--;
    }

    // clean up
    delete pNewResMgr;

    OSL_FAIL(aStr.getStr());
}

#endif

static void RscException_Impl()
{
    switch ( osl_raiseSignal( OSL_SIGNAL_USER_RESOURCEFAILURE, const_cast<char *>("") ) )
    {
    case osl_Signal_ActCallNextHdl:
        abort();

    case osl_Signal_ActIgnore:
        return;

    case osl_Signal_ActAbortApp:
        abort();

    default:
    case osl_Signal_ActKillApp:
        exit(-1);
    }
}

void ImpRCStack::Init( ResMgr* pMgr, const Resource* pObj, sal_uInt32 Id )
{
    pResource       = nullptr;
    pClassRes       = nullptr;
    Flags           = RCFlags::NONE;
    aResHandle      = nullptr;
    pResObj         = pObj;
    nId             = Id & ~RSC_DONTRELEASE; //TLX: Besser Init aendern
    pResMgr         = pMgr;
    if ( !(Id & RSC_DONTRELEASE) )
        Flags      |= RCFlags::AUTORELEASE;
}

void ImpRCStack::Clear()
{
    pResource       = nullptr;
    pClassRes       = nullptr;
    Flags           = RCFlags::NONE;
    aResHandle      = nullptr;
    pResObj         = nullptr;
    nId             = 0;
    pResMgr         = nullptr;
}

static RSHEADER_TYPE* LocalResource( const ImpRCStack* pStack,
                                     RESOURCE_TYPE nRTType,
                                     sal_uInt32 nId )
{
    // Returns position of the resource if found or NULL otherwise

    if ( pStack->pResource && pStack->pClassRes )
    {
        RSHEADER_TYPE*  pTmp;   // Pointer to child resource
        RSHEADER_TYPE*  pEnd;   // Pointer to the end of this resource
        pTmp = reinterpret_cast<RSHEADER_TYPE*>
               (reinterpret_cast<sal_uInt8*>(pStack->pResource) + pStack->pResource->GetLocalOff());
        pEnd = reinterpret_cast<RSHEADER_TYPE*>
               (reinterpret_cast<sal_uInt8*>(pStack->pResource) + pStack->pResource->GetGlobOff());
        while ( pTmp != pEnd )
        {
            if ( pTmp->GetRT() == nRTType && pTmp->GetId() == nId )
                return pTmp;
            pTmp = reinterpret_cast<RSHEADER_TYPE*>(reinterpret_cast<sal_uInt8*>(pTmp) + pTmp->GetGlobOff());
        }
    }

    return nullptr;
}

void* ResMgr::pEmptyBuffer = nullptr;

void* ResMgr::getEmptyBuffer()
{
    if( ! pEmptyBuffer )
        pEmptyBuffer = rtl_allocateZeroMemory( 1024 );
    return pEmptyBuffer;
}

void ResMgr::DestroyAllResMgr()
{
    {
        osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );
        if( pEmptyBuffer )
        {
            rtl_freeMemory( pEmptyBuffer );
            pEmptyBuffer = nullptr;
        }
        ResMgrContainer::release();
    }
    delete pResMgrMutex;
    pResMgrMutex = nullptr;
}

void ResMgr::Init( const OUString& rFileName )
{
    (void) rFileName; // avoid warning about unused parameter
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if ( !pImpRes )
    {
#ifdef DBG_UTIL
        OStringBuffer aStr("Resourcefile not found:\n");
        aStr.append(OUStringToOString(rFileName, RTL_TEXTENCODING_UTF8));
        OSL_FAIL(aStr.getStr());
#endif
        RscException_Impl();
    }
#ifdef DBG_UTIL
    else
    {
        void* aResHandle = nullptr;     // Helper variable for resource handles
        void* pVoid;              // Pointer on the resource

        pVoid = pImpRes->LoadGlobalRes( RSC_VERSIONCONTROL, RSCVERSION_ID,
                                        &aResHandle );
        if ( pVoid )
            InternalResMgr::FreeGlobalRes( aResHandle, pVoid );
        else
        {
            SAL_WARN("tools.rc", "Wrong version: " << pImpRes->aFileName);
        }
    }
#endif
    nCurStack = -1;
    aStack.clear();
    pFallbackResMgr = pOriginalResMgr = nullptr;
    incStack();
}

ResMgr::ResMgr( InternalResMgr * pImpMgr )
{
    pImpRes = pImpMgr;
    Init( pImpMgr->aFileName );
}

ResMgr::~ResMgr()
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    ResMgrContainer::get().freeResMgr( pImpRes );

    // clean up possible left rc stack frames
    while( nCurStack > 0 )
    {
        if( ( aStack[nCurStack].Flags & (RCFlags::GLOBAL | RCFlags::NOTFOUND) ) == RCFlags::GLOBAL )
            InternalResMgr::FreeGlobalRes( aStack[nCurStack].aResHandle,
                                    aStack[nCurStack].pResource );
        nCurStack--;
    }
}

void ResMgr::incStack()
{
    nCurStack++;
    if( nCurStack >= int(aStack.size()) )
        aStack.push_back( ImpRCStack() );
    aStack[nCurStack].Clear();

    DBG_ASSERT( nCurStack < 32, "Resource stack unreasonably large" );
}

void ResMgr::decStack()
{
    DBG_ASSERT( nCurStack > 0, "resource stack underrun  !" );
    if( (aStack[nCurStack].Flags & RCFlags::FALLBACK_UP) )
    {
        nCurStack--;
        // warning: this will delete *this, see below
        pOriginalResMgr->decStack();
    }
    else
    {
        ImpRCStack& rTop = aStack[nCurStack];
        if( (rTop.Flags & RCFlags::FALLBACK_DOWN) )
        {
            #if OSL_DEBUG_LEVEL > 1
            OSL_TRACE( "returning from fallback %s",
                     OUStringToOString(pFallbackResMgr->GetFileName(), osl_getThreadTextEncoding() ).getStr() );
            #endif
            delete pFallbackResMgr;
            pFallbackResMgr = nullptr;
        }
        nCurStack--;
    }
}

#ifdef DBG_UTIL

void ResMgr::TestStack()
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    int upperLimit = nCurStack;

    if ( upperLimit < 0 )
    {
        OSL_FAIL( "resource stack underrun!" );
    }
    else if ( upperLimit >=  static_cast<int>(aStack.size()) )
    {
        OSL_FAIL( "stack occupation index > allocated stack size" );
    }
}

#endif

bool ResMgr::IsAvailable( const ResId& rId, const Resource* pResObj ) const
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    bool            bAvailable = false;
    RSHEADER_TYPE*  pClassRes = rId.GetpResource();
    RESOURCE_TYPE   nRT = rId.GetRT2();
    sal_uInt32      nId = rId.GetId();
    const ResMgr*   pMgr = rId.GetResMgr();

    if ( !pMgr )
        pMgr = this;

    if( pMgr->pFallbackResMgr )
    {
        ResId aId( rId );
        aId.SetResMgr( nullptr );
        return pMgr->pFallbackResMgr->IsAvailable( aId, pResObj );
    }

    if ( !pResObj || pResObj == pMgr->aStack[pMgr->nCurStack].pResObj )
    {
        if ( !pClassRes )
            pClassRes = LocalResource( &pMgr->aStack[pMgr->nCurStack], nRT, nId );
        if ( pClassRes )
        {
            if ( pClassRes->GetRT() == nRT )
                bAvailable = true;
        }
    }

    if ( !pClassRes )
        bAvailable = pMgr->pImpRes->IsGlobalAvailable( nRT, nId );

    return bAvailable;
}

void* ResMgr::GetClass()
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
        return pFallbackResMgr->GetClass();

    return aStack[nCurStack].pClassRes;
}

bool ResMgr::GetResource( const ResId& rId, const Resource* pResObj )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
    {
        ResId aId( rId );
        aId.SetResMgr( nullptr );
        return pFallbackResMgr->GetResource( aId, pResObj );
    }

    ResMgr* pMgr = rId.GetResMgr();
    if ( pMgr && (this != pMgr) )
        return pMgr->GetResource( rId, pResObj );

    // normally Increment will pop the context; this is
    // not possible in RCFlags::NOTFOUND case, so pop a frame here
    ImpRCStack* pTop = &aStack[nCurStack];
    if( (pTop->Flags & RCFlags::NOTFOUND) )
    {
        decStack();
    }

    RSHEADER_TYPE*  pClassRes = rId.GetpResource();
    RESOURCE_TYPE   nRT = rId.GetRT2();
    sal_uInt32      nId = rId.GetId();

    incStack();
    pTop = &aStack[nCurStack];
    pTop->Init( pMgr, pResObj, nId |
                (rId.IsAutoRelease() ? 0 : RSC_DONTRELEASE) );

    if ( pClassRes )
    {
        if ( pClassRes->GetRT() == nRT )
            pTop->pClassRes = pClassRes;
        else
        {
#ifdef DBG_UTIL
            RscError_Impl( "Different class and resource type!",
                           this, nRT, nId, aStack, nCurStack-1 );
#endif
            pTop->Flags |= RCFlags::NOTFOUND;
            pTop->pClassRes = getEmptyBuffer();
            pTop->pResource = static_cast<RSHEADER_TYPE*>(pTop->pClassRes);
            return false;
        }
    }
    else
    {
        OSL_ENSURE( nCurStack > 0, "stack of 1 to shallow" );
        pTop->pClassRes = LocalResource( &aStack[nCurStack-1], nRT, nId );
    }

    if ( pTop->pClassRes )
        // local Resource, not a system Resource
        pTop->pResource = static_cast<RSHEADER_TYPE *>(pTop->pClassRes);
    else
    {
        pTop->pClassRes = pImpRes->LoadGlobalRes( nRT, nId, &pTop->aResHandle );
        if ( pTop->pClassRes )
        {
            pTop->Flags |= RCFlags::GLOBAL;
            pTop->pResource = static_cast<RSHEADER_TYPE *>(pTop->pClassRes);
        }
        else
        {
            // try to get a fallback resource
            pFallbackResMgr = CreateFallbackResMgr( rId, pResObj );
            if( pFallbackResMgr )
            {
                pTop->Flags |= RCFlags::FALLBACK_DOWN;
#ifdef DBG_UTIL
                OStringBuffer aMess("found resource ");
                aMess.append(static_cast<sal_Int32>(nId));
                aMess.append(" in fallback ");
                aMess.append(OUStringToOString(
                    pFallbackResMgr->GetFileName(),
                    osl_getThreadTextEncoding()));
                aMess.append('\n');
                RscError_Impl(aMess.getStr(),
                              this, nRT, nId, aStack, nCurStack-1);
#endif
            }
            else
            {
                #ifdef DBG_UTIL
                RscError_Impl( "Cannot load resource! ",
                              this, nRT, nId, aStack, nCurStack-1 );
                #endif
                pTop->Flags |= RCFlags::NOTFOUND;
                pTop->pClassRes = getEmptyBuffer();
                pTop->pResource = static_cast<RSHEADER_TYPE*>(pTop->pClassRes);
                return false;
            }
        }
    }

    return true;
}

void * ResMgr::GetResourceSkipHeader( const ResId& rResId, ResMgr ** ppResMgr )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    *ppResMgr = rResId.GetResMgr();
    assert(*ppResMgr != nullptr);
    (*ppResMgr)->GetResource( rResId );
    (*ppResMgr)->Increment( sizeof( RSHEADER_TYPE ) );
    return (*ppResMgr)->GetClass();
}

void ResMgr::PopContext( const Resource* pResObj )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
    {
        pFallbackResMgr->PopContext( pResObj );
        return;
    }

    if ( nCurStack > 0 )
    {
        ImpRCStack* pTop = &aStack[nCurStack];
        // free resource
        if( (pTop->Flags & (RCFlags::GLOBAL | RCFlags::NOTFOUND)) == RCFlags::GLOBAL )
            // free global resource if resource is foreign
            InternalResMgr::FreeGlobalRes( pTop->aResHandle, pTop->pResource );
        decStack();
    }
}

RSHEADER_TYPE* ResMgr::CreateBlock( const ResId& rId )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
    {
        ResId aId( rId );
        aId.SetResMgr( nullptr );
        return pFallbackResMgr->CreateBlock( aId );
    }

    RSHEADER_TYPE* pHeader = nullptr;
    if ( GetResource( rId ) )
    {
        // Pointer is at the beginning of the resource, thus
        // class pointer points to the header, and the remaining size
        // equals to total size of allocated memory
        pHeader = static_cast<RSHEADER_TYPE*>(rtl_allocateMemory( GetRemainSize() ));
        memcpy( pHeader, GetClass(), GetRemainSize() );
        Increment( pHeader->GetLocalOff() ); //ans Ende setzen
        if ( pHeader->GetLocalOff() != pHeader->GetGlobOff() )
            // Has sub-resources, thus release them as well
            PopContext();
    }

    return pHeader;
}

sal_Int16 ResMgr::GetShort( void * pShort )
{
    return ((*(static_cast<sal_uInt8*>(pShort) + 0) << 8) |
            (*(static_cast<sal_uInt8*>(pShort) + 1) << 0)   );
}

sal_Int32 ResMgr::GetLong( void * pLong )
{
    return ((*(static_cast<sal_uInt8*>(pLong) + 0) << 24) |
            (*(static_cast<sal_uInt8*>(pLong) + 1) << 16) |
            (*(static_cast<sal_uInt8*>(pLong) + 2) <<  8) |
            (*(static_cast<sal_uInt8*>(pLong) + 3) <<  0)   );
}

sal_uInt64 ResMgr::GetUInt64( void* pDatum )
{
    return ((sal_uInt64(*(static_cast<sal_uInt8*>(pDatum) + 0)) << 56) |
            (sal_uInt64(*(static_cast<sal_uInt8*>(pDatum) + 1)) << 48) |
            (sal_uInt64(*(static_cast<sal_uInt8*>(pDatum) + 2)) << 40) |
            (sal_uInt64(*(static_cast<sal_uInt8*>(pDatum) + 3)) << 32) |
            (sal_uInt64(*(static_cast<sal_uInt8*>(pDatum) + 4)) << 24) |
            (sal_uInt64(*(static_cast<sal_uInt8*>(pDatum) + 5)) << 16) |
            (sal_uInt64(*(static_cast<sal_uInt8*>(pDatum) + 6)) <<  8) |
            (sal_uInt64(*(static_cast<sal_uInt8*>(pDatum) + 7)) <<  0)   );
}

sal_uInt32 ResMgr::GetStringWithoutHook( OUString& rStr, const sal_uInt8* pStr )
{
    sal_uInt32 nLen=0;
    sal_uInt32 nRet = GetStringSize( pStr, nLen );
    const sal_Char* str = reinterpret_cast< const sal_Char* >( pStr );
    OUString aString( str, strlen( str ), RTL_TEXTENCODING_UTF8,
                       RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE |
                       RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                       RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT );
    rStr = aString;
    return nRet;
}

sal_uInt32 ResMgr::GetString( OUString& rStr, const sal_uInt8* pStr )
{
    OUString aString;
    sal_uInt32 nRet =  GetStringWithoutHook( aString, pStr );
    if ( pImplResHookProc )
        aString = pImplResHookProc( aString );
    rStr = aString;
    return nRet;
}

sal_uInt32 ResMgr::GetByteString( OString& rStr, const sal_uInt8* pStr )
{
    sal_uInt32 nLen=0;
    sal_uInt32 nRet = GetStringSize( pStr, nLen );
    rStr = OString( reinterpret_cast<const char*>(pStr), nLen );
    return nRet;
}

sal_uInt32 ResMgr::GetStringSize( const sal_uInt8* pStr, sal_uInt32& nLen )
{
    nLen = static_cast< sal_uInt32 >( strlen( reinterpret_cast<const char*>(pStr) ) );
    return GetStringSize( nLen );
}

sal_uInt32 ResMgr::GetRemainSize()
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
        return pFallbackResMgr->GetRemainSize();

    const ImpRCStack& rTop = aStack[nCurStack];
    return (sal_uInt32)(reinterpret_cast<sal_IntPtr>(rTop.pResource) +
                        rTop.pResource->GetLocalOff() -
                        reinterpret_cast<sal_IntPtr>(rTop.pClassRes));
}

void* ResMgr::Increment( sal_uInt32 nSize )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
        return pFallbackResMgr->Increment( nSize );

    ImpRCStack& rStack = aStack[nCurStack];
    if( (rStack.Flags & RCFlags::NOTFOUND) )
        return rStack.pClassRes;

    sal_uInt8* pClassRes = static_cast<sal_uInt8*>(rStack.pClassRes) + nSize;

    rStack.pClassRes = pClassRes;

    RSHEADER_TYPE* pRes = rStack.pResource;

    sal_uInt32 nLocalOff = pRes->GetLocalOff();
    if ( (pRes->GetGlobOff() == nLocalOff) &&
         ((reinterpret_cast<char*>(pRes) + nLocalOff) == rStack.pClassRes) &&
         (rStack.Flags & RCFlags::AUTORELEASE))
    {
        PopContext( rStack.pResObj );
    }

    return pClassRes;
}

ResMgr* ResMgr::CreateFallbackResMgr( const ResId& rId, const Resource* pResource )
{
    ResMgr *pFallback = nullptr;
    if( nCurStack > 0 )
    {
        // get the next fallback level in resource file scope
        InternalResMgr* pRes = ResMgrContainer::get().getNextFallback( pImpRes );
        if( pRes )
        {
            // check that the fallback locale is not already in the chain of
            // fallbacks - prevent fallback loops
            ResMgr* pResMgr = this;
            while( pResMgr && (pResMgr->pImpRes->aLocale != pRes->aLocale))
            {
                pResMgr = pResMgr->pOriginalResMgr;
            }
            if( pResMgr )
            {
                // found a recursion, no fallback possible
                ResMgrContainer::get().freeResMgr( pRes );
                return nullptr;
            }
            OSL_TRACE( "trying fallback: %s", OUStringToOString( pRes->aFileName, osl_getThreadTextEncoding() ).getStr() );
            pFallback = new ResMgr( pRes );
            pFallback->pOriginalResMgr = this;
            // try to recreate the resource stack
            bool bHaveStack = true;
            for( int i = 1; i < nCurStack; i++ )
            {
                if( !aStack[i].pResource )
                {
                    bHaveStack = false;
                    break;
                }
                ResId aId( aStack[i].pResource->GetId(), *pFallbackResMgr );
                aId.SetRT( aStack[i].pResource->GetRT() );
                if( !pFallback->GetResource( aId ) )
                {
                    bHaveStack = false;
                    break;
                }
            }
            if( bHaveStack )
            {
                ResId aId( rId.GetId(), *pFallback );
                aId.SetRT( rId.GetRT() );
                if( !pFallback->GetResource( aId, pResource ) )
                    bHaveStack = false;
                else
                    pFallback->aStack[pFallback->nCurStack].Flags |= RCFlags::FALLBACK_UP;
            }
            if( !bHaveStack )
            {
                delete pFallback;
                pFallback = nullptr;
            }
        }
    }
    return pFallback;
}

ResMgr* ResMgr::CreateResMgr( const sal_Char* pPrefixName,
                              const LanguageTag& _aLocale )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    OUString aPrefix( pPrefixName, strlen( pPrefixName ), osl_getThreadTextEncoding() );

    LanguageTag aLocale = _aLocale;
    if( aLocale.isSystemLocale() )
        aLocale = ResMgrContainer::get().getDefLocale();

    InternalResMgr* pImp = ResMgrContainer::get().getResMgr( aPrefix, aLocale );
    return pImp ? new ResMgr( pImp ) : nullptr;
}

ResMgr* ResMgr::SearchCreateResMgr(
    const sal_Char* pPrefixName,
    LanguageTag& rLocale )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    OUString aPrefix( pPrefixName, strlen( pPrefixName ), osl_getThreadTextEncoding() );

    if( rLocale.isSystemLocale() )
        rLocale = ResMgrContainer::get().getDefLocale();

    InternalResMgr* pImp = ResMgrContainer::get().getResMgr( aPrefix, rLocale );
    return pImp ? new ResMgr( pImp ) : nullptr;
}

sal_Int16 ResMgr::ReadShort()
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
        return pFallbackResMgr->ReadShort();

    sal_Int16 n = GetShort( GetClass() );
    Increment( sizeof( sal_Int16 ) );
    return n;
}

sal_Int32 ResMgr::ReadLong()
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
        return pFallbackResMgr->ReadLong();

    sal_Int32 n = GetLong( GetClass() );
    Increment( sizeof( sal_Int32 ) );
    return n;
}

OUString ResMgr::ReadStringWithoutHook()
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
        return pFallbackResMgr->ReadStringWithoutHook();

    OUString aRet;

    const ImpRCStack& rTop = aStack[nCurStack];
    if( (rTop.Flags & RCFlags::NOTFOUND) )
    {
        #if OSL_DEBUG_LEVEL > 0
        aRet = "<resource not found>";
        #endif
    }
    else
        Increment( GetStringWithoutHook( aRet, static_cast<const sal_uInt8*>(GetClass()) ) );

    return aRet;
}

OUString ResMgr::ReadString()
{
    OUString aRet = ReadStringWithoutHook();
    if ( pImplResHookProc )
        aRet = pImplResHookProc( aRet );
    return aRet;
}

OString ResMgr::ReadByteString()
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
        return pFallbackResMgr->ReadByteString();

    OString aRet;

    const ImpRCStack& rTop = aStack[nCurStack];
    if( (rTop.Flags & RCFlags::NOTFOUND) )
    {
        #if OSL_DEBUG_LEVEL > 0
        aRet = OString( "<resource not found>" );
        #endif
    }
    else
        Increment( GetByteString( aRet, static_cast<const sal_uInt8*>(GetClass()) ) );

    return aRet;
}

OString ResMgr::GetAutoHelpId()
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
        return pFallbackResMgr->GetAutoHelpId();

    OSL_ENSURE( nCurStack, "resource stack empty in Auto help id generation" );
    if( nCurStack < 1 || nCurStack > 2 )
        return OString();

    // prepare HID, start with resource prefix
    OStringBuffer aHID( 32 );
    aHID.append( OUStringToOString( pImpRes->aPrefix, RTL_TEXTENCODING_UTF8 ) );
    aHID.append( '.' );

    // append type
    const ImpRCStack *pRC = StackTop();
    OSL_ENSURE( pRC, "missing resource stack level" );

    if ( nCurStack == 1 )
    {
        // auto help ids for top level windows
        switch( pRC->pResource->GetRT() ) {
            case RSC_DOCKINGWINDOW:     aHID.append( "DockingWindow" );    break;
            case RSC_WORKWIN:           aHID.append( "WorkWindow" );       break;
            default: return OString();
        }
    }
    else
    {
        // only controls with the following parents get auto help ids
        const ImpRCStack *pRC1 = StackTop(1);
        switch( pRC1->pResource->GetRT() ) {
            case RSC_DOCKINGWINDOW:
            case RSC_WORKWIN:
                // intentionally no breaks!
                // auto help ids for controls
                switch( pRC->pResource->GetRT() ) {
                    case RSC_RADIOBUTTON:       aHID.append( "RadioButton" );      break;
                    case RSC_CHECKBOX:          aHID.append( "CheckBox" );         break;
                    case RSC_EDIT:              aHID.append( "Edit" );             break;
                    case RSC_LISTBOX:           aHID.append( "ListBox" );          break;
                    case RSC_COMBOBOX:          aHID.append( "ComboBox" );         break;
                    case RSC_PUSHBUTTON:        aHID.append( "PushButton" );       break;
                    case RSC_SPINFIELD:         aHID.append( "SpinField" );        break;
                    case RSC_NUMERICFIELD:      aHID.append( "NumericField" );     break;
                    case RSC_METRICFIELD:       aHID.append( "MetricField" );      break;
                    case RSC_IMAGEBUTTON:       aHID.append( "ImageButton" );      break;
                    default:
                        // no type, no auto HID
                        return OString();
                }
                break;
            default:
                return OString();
        }
    }

    // append resource id hierarchy
    for( int nOff = nCurStack-1; nOff >= 0; nOff-- )
    {
        aHID.append( '.' );
        pRC = StackTop( nOff );

        OSL_ENSURE( pRC->pResource, "missing resource in resource stack level !" );
        if( pRC->pResource )
            aHID.append( sal_Int32( pRC->pResource->GetId() ) );
    }

    return aHID.makeStringAndClear();
}

void ResMgr::SetReadStringHook( ResHookProc pProc )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );
    pImplResHookProc = pProc;
}

ResHookProc ResMgr::GetReadStringHook()
{
    return pImplResHookProc;
}

void ResMgr::SetDefaultLocale( const LanguageTag& rLocale )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );
    ResMgrContainer::get().setDefLocale( rLocale );
}

const OUString& ResMgr::GetFileName() const
{
    return pImpRes->aFileName;
}

SimpleResMgr::SimpleResMgr( const sal_Char* pPrefixName,
                            const LanguageTag& rLocale )
{
    OUString aPrefix( pPrefixName, strlen( pPrefixName ), osl_getThreadTextEncoding() );
    LanguageTag aLocale( rLocale );

    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );
    if( aLocale.isSystemLocale() )
        aLocale = ResMgrContainer::get().getDefLocale();

    m_pResImpl = ResMgrContainer::get().getResMgr( aPrefix, aLocale, true );
    DBG_ASSERT( m_pResImpl, "SimpleResMgr::SimpleResMgr : have no impl class !" );
}

SimpleResMgr::~SimpleResMgr()
{
    delete m_pResImpl;
}

SimpleResMgr* SimpleResMgr::Create(const sal_Char* pPrefixName, const LanguageTag& rLocale)
{
    return new SimpleResMgr(pPrefixName, rLocale);
}

bool SimpleResMgr::IsAvailable( RESOURCE_TYPE _resourceType, sal_uInt32 _resourceId )
{
    osl::MutexGuard aGuard(m_aAccessSafety);

    if ( ( RSC_STRING != _resourceType ) && ( RSC_RESOURCE != _resourceType ) )
        return false;

    DBG_ASSERT( m_pResImpl, "SimpleResMgr::IsAvailable: have no impl class !" );
    return m_pResImpl->IsGlobalAvailable( _resourceType, _resourceId );
}

OUString SimpleResMgr::ReadString( sal_uInt32 nId )
{
    osl::MutexGuard aGuard(m_aAccessSafety);

    DBG_ASSERT( m_pResImpl, "SimpleResMgr::ReadString : have no impl class !" );
    // perhaps constructed with an invalid filename ?

    OUString sReturn;
    if ( !m_pResImpl )
        return sReturn;

    void* pResHandle = nullptr;
    InternalResMgr* pFallback = m_pResImpl;
    RSHEADER_TYPE* pResHeader = static_cast<RSHEADER_TYPE*>(m_pResImpl->LoadGlobalRes( RSC_STRING, nId, &pResHandle ));
    if ( !pResHeader )
    {
        osl::Guard<osl::Mutex> aGuard2( getResMgrMutex() );

        // try fallback
        while( ! pResHandle && pFallback )
        {
            InternalResMgr* pOldFallback = pFallback;
            pFallback = ResMgrContainer::get().getNextFallback( pFallback );
            if( pOldFallback != m_pResImpl )
                ResMgrContainer::get().freeResMgr( pOldFallback );
            if( pFallback )
            {
                // handle possible recursion
                if( pFallback->aLocale != m_pResImpl->aLocale )
                {
                    pResHeader = static_cast<RSHEADER_TYPE*>(pFallback->LoadGlobalRes( RSC_STRING, nId, &pResHandle ));
                }
                else
                {
                    ResMgrContainer::get().freeResMgr( pFallback );
                    pFallback = nullptr;
                }
            }
        }
        if( ! pResHandle )
            // no such resource
            return sReturn;
    }

    // sal_uIntPtr nLen = pResHeader->GetLocalOff() - sizeof(RSHEADER_TYPE);
    ResMgr::GetString( sReturn, reinterpret_cast<sal_uInt8*>(pResHeader+1) );

    // not necessary with the current implementation which holds the string table permanently, but to be sure ....
    // note: pFallback cannot be NULL here and is either the fallback or m_pResImpl
    InternalResMgr::FreeGlobalRes( pResHeader, pResHandle );
    if( m_pResImpl != pFallback )
    {
        osl::Guard<osl::Mutex> aGuard2( getResMgrMutex() );

        ResMgrContainer::get().freeResMgr( pFallback );
    }
    return sReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
