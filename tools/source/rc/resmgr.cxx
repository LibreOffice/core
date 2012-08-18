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
#include <rtl/oustringostreaminserter.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <rtl/instance.hxx>
#include <rtl/bootstrap.hxx>
#include <i18npool/mslangid.hxx>
#include <tools/simplerm.hxx>

#include <functional>
#include <algorithm>
#include <boost/unordered_map.hpp>
#include <list>
#include <set>

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OUStringBuffer;
using ::rtl::OUStringHash;

using namespace osl;

// for thread safety
static osl::Mutex* pResMgrMutex = NULL;

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
    sal_uInt8                       pStringBlock;
    SvStream *                      pStm;
    sal_Bool                        bEqual2Content;
    sal_uInt32                      nEntries;
    OUString                        aFileName;
    OUString                        aPrefix;
    OUString                        aResName;
    bool                            bSingular;
    com::sun::star::lang::Locale    aLocale;
    boost::unordered_map<sal_uInt64, int>* pResUseDump;

                            InternalResMgr( const OUString& rFileURL,
                                            const OUString& aPrefix,
                                            const OUString& aResName,
                                            const com::sun::star::lang::Locale& rLocale );
                            ~InternalResMgr();
    sal_Bool                Create();

    sal_Bool                IsGlobalAvailable( RESOURCE_TYPE nRT, sal_uInt32 nId ) const;
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
            pResMgr( NULL ),
            nRefCount( 0 ),
            nLoadCount( 0 )
            {}
    };

    boost::unordered_map< OUString, ContainerElement, OUStringHash> m_aResFiles;
    com::sun::star::lang::Locale                             m_aDefLocale;

    ResMgrContainer() { init(); }
    ~ResMgrContainer();

    void init();

public:
    static ResMgrContainer& get();
    static void release();

    InternalResMgr* getResMgr( const OUString& rPrefix,
                               com::sun::star::lang::Locale& rLocale,
                               bool bForceNewInstance = false
                               );
    InternalResMgr* getNextFallback( InternalResMgr* pResMgr );

    void freeResMgr( InternalResMgr* pResMgr );

    void setDefLocale( const com::sun::star::lang::Locale& rLocale )
    { m_aDefLocale = rLocale; }
    const com::sun::star::lang::Locale& getDefLocale() const
    { return m_aDefLocale; }
};

ResMgrContainer* ResMgrContainer::pOneInstance = NULL;

ResMgrContainer& ResMgrContainer::get()
{
    if( ! pOneInstance )
        pOneInstance = new ResMgrContainer();
    return *pOneInstance;
}

ResMgrContainer::~ResMgrContainer()
{
    for( boost::unordered_map< OUString, ContainerElement, OUStringHash >::iterator it =
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
    pOneInstance = NULL;
}

void ResMgrContainer::init()
{
    // get resource path
    std::list< OUString > aDirs;
    sal_Int32 nIndex = 0;

    // 1. fixed locations
    rtl::OUString uri(
        RTL_CONSTASCII_USTRINGPARAM("$BRAND_BASE_DIR/program/resource"));
    rtl::Bootstrap::expandMacros(uri);
    aDirs.push_back(uri);

    // 2. in STAR_RESOURCEPATH
    const sal_Char* pEnv = getenv( "STAR_RESOURCEPATH" );
    if( pEnv )
    {
        OUString aEnvPath( OStringToOUString( OString( pEnv ), osl_getThreadTextEncoding() ) );
        nIndex = 0;
        while( nIndex >= 0 )
        {
            OUString aPathElement( aEnvPath.getToken( 0, SAL_PATHSEPARATOR, nIndex ) );
            if( !aPathElement.isEmpty() )
            {
                OUString aFileURL;
                File::getFileURLFromSystemPath( aPathElement, aFileURL );
                aDirs.push_back( aFileURL);
            }
        }
    }

    // collect all possible resource files
    for( std::list< OUString >::const_iterator dir_it = aDirs.begin(); dir_it != aDirs.end(); ++dir_it )
    {
        Directory aDir( *dir_it );
        if( aDir.open() == FileBase::E_None )
        {
            DirectoryItem aItem;
            while( aDir.getNextItem( aItem ) == FileBase::E_None )
            {
                FileStatus aStatus(osl_FileStatus_Mask_FileName);
                if( aItem.getFileStatus( aStatus ) == FileBase::E_None )
                {
                    OUString aFileName = aStatus.getFileName();
                    if( aFileName.getLength() < 5 )
                        continue;
                    if( ! aFileName.endsWithIgnoreAsciiCaseAsciiL( ".res", 4 ) )
                        continue;
                    OUString aResName = aFileName.copy( 0, aFileName.getLength()-4 );
                    if( m_aResFiles.find( aResName ) != m_aResFiles.end() )
                        continue;
                    OUStringBuffer aURL( dir_it->getLength() + aFileName.getLength() + 1 );
                    aURL.append( *dir_it );
                    if( !dir_it->endsWithIgnoreAsciiCaseAsciiL( "/", 1 ) )
                        aURL.append( sal_Unicode('/') );
                    aURL.append( aFileName );
                    m_aResFiles[ aResName ].aFileURL = aURL.makeStringAndClear();
                }
            }
        }
        #if OSL_DEBUG_LEVEL > 1
        else
            OSL_TRACE( "opening dir %s failed", OUStringToOString( *dir_it, osl_getThreadTextEncoding() ).getStr() );
        #endif
    }
    #if OSL_DEBUG_LEVEL > 1
    for( boost::unordered_map< OUString, ContainerElement, OUStringHash >::const_iterator it =
            m_aResFiles.begin(); it != m_aResFiles.end(); ++it )
    {
        OSL_TRACE( "ResMgrContainer: %s -> %s",
                 OUStringToOString( it->first, osl_getThreadTextEncoding() ).getStr(),
                 OUStringToOString( it->second.aFileURL, osl_getThreadTextEncoding() ).getStr() );
    }
    #endif

    // set default language
    LanguageType nLang = MsLangId::getSystemUILanguage();
    MsLangId::convertLanguageToLocale(nLang, m_aDefLocale);
}

namespace
{
    bool isAlreadyPureenUS(const com::sun::star::lang::Locale &rLocale)
    {
        return ( rLocale.Language == "en" && rLocale.Country == "US" && rLocale.Variant.isEmpty() );
    }
}

InternalResMgr* ResMgrContainer::getResMgr( const OUString& rPrefix,
                                            com::sun::star::lang::Locale& rLocale,
                                            bool bForceNewInstance
                                            )
{
    com::sun::star::lang::Locale aLocale( rLocale );
    OUStringBuffer aSearch( rPrefix.getLength() + 16 );
    boost::unordered_map< OUString, ContainerElement, OUStringHash >::iterator it = m_aResFiles.end();

    int nTries = 0;
    if( !aLocale.Language.isEmpty() )
        nTries = 1;
    if( !aLocale.Country.isEmpty() )
        nTries = 2;
    if( !aLocale.Variant.isEmpty() )
        nTries = 3;
    while( nTries-- )
    {
        aSearch.append( rPrefix );
        if( nTries > -1 )
        {
            aSearch.append( aLocale.Language );
        }
        if( nTries > 0 )
        {
            aSearch.append( sal_Unicode('-') );
            aSearch.append( aLocale.Country );
        }
        if( nTries > 1 )
        {
            aSearch.append( sal_Unicode('-') );
            aSearch.append( aLocale.Variant );
        }
        it = m_aResFiles.find( aSearch.makeStringAndClear() );
        if( it != m_aResFiles.end() )
        {
            // ensure InternalResMgr existance
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
        if( nTries == 0 && !isAlreadyPureenUS(aLocale) )
        {
            // locale fallback failed
            // fallback to en-US locale
            nTries = 2;
            aLocale.Language = OUString( RTL_CONSTASCII_USTRINGPARAM( "en" ) );
            aLocale.Country  = OUString( RTL_CONSTASCII_USTRINGPARAM( "US" ) );
            aLocale.Variant = OUString();
        }
    }
    // try if there is anything with this prefix at all
    if( it == m_aResFiles.end() )
    {
        aLocale = com::sun::star::lang::Locale();
        it = m_aResFiles.find( rPrefix );
        if( it == m_aResFiles.end() )
        {
            for( it = m_aResFiles.begin(); it != m_aResFiles.end(); ++it )
            {
                if( it->first.matchIgnoreAsciiCase( rPrefix ) )
                {
                    // ensure InternalResMgr existance
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
                    aLocale.Language = it->first.getToken( 0, '-', nIndex );
                    if( nIndex > 0 )
                        aLocale.Country = it->first.getToken( 0, '-', nIndex );
                    if( nIndex > 0 )
                        aLocale.Variant = it->first.getToken( 0, '-', nIndex );
                    break;
                }
            }
        }
    }
    // give up
    if( it == m_aResFiles.end() )
    {
        OUStringBuffer sKey = rPrefix;
        sKey.append( rLocale.Language );
        if( !rLocale.Country.isEmpty() )
        {
            sKey.append( sal_Unicode('-') );
            sKey.append( rLocale.Country );
        }
        if( !rLocale.Variant.isEmpty() )
        {
            sKey.append( sal_Unicode('-') );
            sKey.append( rLocale.Variant );
        } // if( !aLocale.Variant.isEmpty() )
        ::rtl::OUString sURL = sKey.makeStringAndClear();
        sURL += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".res"));
        if ( m_aResFiles.find(sURL) == m_aResFiles.end() )
        {
            m_aResFiles[ sURL ].aFileURL = sURL;
            return getResMgr(rPrefix,rLocale,bForceNewInstance);
        } // if ( m_aResFiles.find(sURL) == m_aResFiles.end() )
        return NULL;
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
            it->second.pResMgr = NULL;
            pImp->bSingular = true;
        }
        else
        {
            pImp = new InternalResMgr( it->second.aFileURL, rPrefix, it->first, aLocale );
            pImp->bSingular = true;
            if( !pImp->Create() )
            {
                delete pImp;
                pImp = NULL;
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
    com::sun::star::lang::Locale aLocale = pMgr->aLocale;
    if( !aLocale.Variant.isEmpty() )
        aLocale.Variant = OUString();
    else if( !aLocale.Country.isEmpty() )
        aLocale.Country = OUString();
    else if( !isAlreadyPureenUS(aLocale) )
    {
        aLocale.Language = OUString( RTL_CONSTASCII_USTRINGPARAM( "en" ) );
        aLocale.Country = OUString( RTL_CONSTASCII_USTRINGPARAM( "US" ) );
    }
    InternalResMgr* pNext = getResMgr( pMgr->aPrefix, aLocale, pMgr->bSingular );
    // prevent recursion
    if( pNext == pMgr || ( pNext && pNext->aResName.equals( pMgr->aResName ) ) )
    {
        if( pNext->bSingular )
            delete pNext;
        pNext = NULL;
    }
    return pNext;
}

void ResMgrContainer::freeResMgr( InternalResMgr* pResMgr )
{
    if( pResMgr->bSingular )
        delete pResMgr;
    else
    {
        boost::unordered_map< OUString, ContainerElement, OUStringHash >::iterator it =
        m_aResFiles.find( pResMgr->aResName );
        if( it != m_aResFiles.end() )
        {
            DBG_ASSERT( it->second.nRefCount > 0, "InternalResMgr freed too often" );
            if( it->second.nRefCount > 0 )
                it->second.nRefCount--;
            if( it->second.nRefCount == 0 )
            {
                delete it->second.pResMgr;
                it->second.pResMgr = NULL;
            }
        }
    }
}

void Resource::TestRes()
{
    if( m_pResMgr )
        m_pResMgr->TestStack( this );
}

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

static ResHookProc pImplResHookProc = 0;

InternalResMgr::InternalResMgr( const OUString& rFileURL,
                                const OUString& rPrefix,
                                const OUString& rResName,
                                const com::sun::star::lang::Locale& rLocale )
    : pContent( NULL )
    , pStringBlock( NULL )
    , pStm( NULL )
    , bEqual2Content( sal_True )
    , nEntries( 0 )
    , aFileName( rFileURL )
    , aPrefix( rPrefix )
    , aResName( rResName )
    , bSingular( false )
    , aLocale( rLocale )
    , pResUseDump( 0 )
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
            SvFileStream aStm( UniString( pLogFile, RTL_TEXTENCODING_ASCII_US ), STREAM_WRITE );
            aStm.Seek( STREAM_SEEK_TO_END );
            rtl::OStringBuffer aLine(RTL_CONSTASCII_STRINGPARAM("FileName: "));
            aLine.append(rtl::OUStringToOString(aFileName,
                RTL_TEXTENCODING_UTF8));
            aStm.WriteLine(aLine.makeStringAndClear());

            for( boost::unordered_map<sal_uInt64, int>::const_iterator it = pResUseDump->begin();
                 it != pResUseDump->end(); ++it )
            {
                sal_uInt64 nKeyId = it->first;
                aLine.append(RTL_CONSTASCII_STRINGPARAM("Type/Id: "));
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

sal_Bool InternalResMgr::Create()
{
    ResMgrContainer::get();
    sal_Bool bDone = sal_False;

    pStm = new SvFileStream( aFileName, (STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE) );
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
        sal_uInt8* pContentBuf = (sal_uInt8*)rtl_allocateMemory( lContLen );
        pStm->Read( pContentBuf, lContLen );
        // allocate ImpContent space (sizeof(ImpContent) per unit, not necessarily 12)
        pContent = (ImpContent *)rtl_allocateMemory( sizeof(ImpContent)*lContLen/12 );
        // Auf die Anzahl der ImpContent k�rzen
        nEntries = (sal_uInt32)lContLen / 12;
        bEqual2Content = sal_True;  // Die Daten der Resourcen liegen
                                // genauso wie das Inhaltsverzeichnis
        sal_Bool bSorted = sal_True;
        if( nEntries )
        {
#ifdef DBG_UTIL
            const sal_Char* pLogFile = getenv( "STAR_RESOURCE_LOGGING" );
            if ( pLogFile )
            {
                pResUseDump = new boost::unordered_map<sal_uInt64, int>;
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
                    bSorted = sal_False;
                if( (pContent[i].nTypeAndId & 0xFFFFFFFF00000000LL) == (pContent[j].nTypeAndId & 0xFFFFFFFF00000000LL)
                    && pContent[i].nOffset >= pContent[j].nOffset )
                    bEqual2Content = sal_False;
            }
        }
        rtl_freeMemory( pContentBuf );
        OSL_ENSURE( bSorted, "content not sorted" );
        OSL_ENSURE( bEqual2Content, "resource structure wrong" );
        if( !bSorted )
            ::std::sort(pContent,pContent+nEntries,ImpContentLessCompare());
            //  qsort( pContent, nEntries, sizeof( ImpContent ), Compare );

        bDone = sal_True;
    }

    return bDone;
}


sal_Bool InternalResMgr::IsGlobalAvailable( RESOURCE_TYPE nRT, sal_uInt32 nId ) const
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
    // Anfang der Strings suchen
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
            // String Optimierung
            if( !pStringBlock )
            {
                // Anfang der Strings suchen
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
                pStringBlock = (sal_uInt8*)rtl_allocateMemory( nSize );
                pStm->Seek( pFirst->nOffset );
                pStm->Read( pStringBlock, nSize );
            }
            *pResHandle = pStringBlock;
            return (sal_uInt8*)pStringBlock + pFind->nOffset - nOffCorrection;
        } // if( nRT == RSC_STRING && bEqual2Content )
        else
        {
            *pResHandle = 0;
            RSHEADER_TYPE aHeader;
            pStm->Seek( pFind->nOffset );
            pStm->Read( &aHeader, sizeof( RSHEADER_TYPE ) );
            void * pRes = rtl_allocateMemory( aHeader.GetGlobOff() );
            memcpy( pRes, &aHeader, sizeof( RSHEADER_TYPE ) );
            pStm->Read( (sal_uInt8*)pRes + sizeof( RSHEADER_TYPE ),
                        aHeader.GetGlobOff() - sizeof( RSHEADER_TYPE ) );
            return pRes;
        }
    } // if( pFind && (pFind != pEnd) && (pFind->nTypeAndId == nValue) )
    *pResHandle = 0;
    //Resource holen
    return NULL;
}

void InternalResMgr::FreeGlobalRes( void * pResHandle, void * pResource )
{
    if ( !pResHandle )
        // REsource wurde extra allokiert
        rtl_freeMemory(pResource);
}

#ifdef DBG_UTIL

UniString GetTypeRes_Impl( const ResId& rTypeId )
{
    // Funktion verlassen, falls Resourcefehler in dieser Funktion
    static int bInUse = sal_False;
    rtl::OUString aTypStr(OUString::valueOf(static_cast<sal_Int32>(rTypeId.GetId())));

    if ( !bInUse )
    {
        bInUse = sal_True;

        ResId aResId( sal_uInt32(RSCVERSION_ID), *rTypeId.GetResMgr() );
        aResId.SetRT( RSC_VERSIONCONTROL );

        if ( rTypeId.GetResMgr()->GetResource( aResId ) )
        {
            rTypeId.SetRT( RSC_STRING );
            if ( rTypeId.GetResMgr()->IsAvailable( rTypeId ) )
            {
                aTypStr = rTypeId.toString();
                // Versions Resource Klassenzeiger ans Ende setzen
                rTypeId.GetResMgr()->Increment( sizeof( RSHEADER_TYPE ) );
            }
        }
        bInUse = sal_False;
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

    rtl::OStringBuffer aStr(rtl::OUStringToOString(pResMgr->GetFileName(),
        RTL_TEXTENCODING_UTF8));

    if (aStr.getLength())
        aStr.append('\n');

    aStr.append(RTL_CONSTASCII_STRINGPARAM("Class: "));
    aStr.append(rtl::OUStringToOString(GetTypeRes_Impl(ResId(nRT, *pNewResMgr)),
        RTL_TEXTENCODING_UTF8));
    aStr.append(RTL_CONSTASCII_STRINGPARAM(", Id: "));
    aStr.append(static_cast<sal_Int32>(nId));
    aStr.append(RTL_CONSTASCII_STRINGPARAM(". "));
    aStr.append(pMessage);

    aStr.append(RTL_CONSTASCII_STRINGPARAM("\nResource Stack\n"));
    while( nDepth > 0 )
    {
        aStr.append(RTL_CONSTASCII_STRINGPARAM("Class: "));
        aStr.append(rtl::OUStringToOString(GetTypeRes_Impl(
            ResId(rResStack[nDepth].pResource->GetRT(), *pNewResMgr)),
            RTL_TEXTENCODING_UTF8));
        aStr.append(RTL_CONSTASCII_STRINGPARAM(", Id: "));
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
    switch ( osl_raiseSignal( OSL_SIGNAL_USER_RESOURCEFAILURE, (void*)"" ) )
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
    pResource       = NULL;
    pClassRes       = NULL;
    Flags           = RC_NOTYPE;
    aResHandle      = NULL;
    pResObj         = pObj;
    nId             = Id & ~RSC_DONTRELEASE; //TLX: Besser Init aendern
    pResMgr         = pMgr;
    if ( !(Id & RSC_DONTRELEASE) )
        Flags      |= RC_AUTORELEASE;
}

void ImpRCStack::Clear()
{
    pResource       = NULL;
    pClassRes       = NULL;
    Flags           = RC_NOTYPE;
    aResHandle      = NULL;
    pResObj         = NULL;
    nId             = 0;
    pResMgr         = NULL;
}

static RSHEADER_TYPE* LocalResource( const ImpRCStack* pStack,
                                     RESOURCE_TYPE nRTType,
                                     sal_uInt32 nId )
{
    // Gibt die Position der Resource zurueck, wenn sie gefunden wurde.
    // Ansonsten gibt die Funktion Null zurueck.
    RSHEADER_TYPE*  pTmp;   // Zeiger auf Kind-Resourceobjekte
    RSHEADER_TYPE*  pEnd;   // Zeiger auf das Ende der Resource

    if ( pStack->pResource && pStack->pClassRes )
    {
        pTmp = (RSHEADER_TYPE*)
               ((sal_uInt8*)pStack->pResource + pStack->pResource->GetLocalOff());
        pEnd = (RSHEADER_TYPE*)
               ((sal_uInt8*)pStack->pResource + pStack->pResource->GetGlobOff());
        while ( pTmp != pEnd )
        {
            if ( pTmp->GetRT() == nRTType && pTmp->GetId() == nId )
                return pTmp;
            pTmp = (RSHEADER_TYPE*)((sal_uInt8*)pTmp + pTmp->GetGlobOff());
        }
    }

    return NULL;
}

void* ResMgr::pEmptyBuffer = NULL;

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
            pEmptyBuffer = NULL;
        }
        ResMgrContainer::release();
    }
    delete pResMgrMutex;
    pResMgrMutex = NULL;
}

void ResMgr::Init( const OUString& rFileName )
{
    (void) rFileName; // avoid warning about unused parameter
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if ( !pImpRes )
    {
#ifdef DBG_UTIL
        rtl::OStringBuffer aStr(
            RTL_CONSTASCII_STRINGPARAM("Resourcefile not found:\n"));
        aStr.append(OUStringToOString(rFileName, RTL_TEXTENCODING_UTF8));
        OSL_FAIL(aStr.getStr());
#endif
        RscException_Impl();
    }
#ifdef DBG_UTIL
    else
    {
        void* aResHandle = 0;     // Hilfvariable fuer Resource
        void* pVoid;              // Zeiger auf die Resource

        pVoid = pImpRes->LoadGlobalRes( RSC_VERSIONCONTROL, RSCVERSION_ID,
                                        &aResHandle );
        if ( pVoid )
            InternalResMgr::FreeGlobalRes( aResHandle, pVoid );
        else
        {
            SAL_WARN("tools", "Wrong version: " << pImpRes->aFileName);
        }
    }
#endif
    nCurStack = -1;
    aStack.clear();
    pFallbackResMgr = pOriginalResMgr = NULL;
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
        if( ( aStack[nCurStack].Flags & (RC_GLOBAL | RC_NOTFOUND) ) == RC_GLOBAL )
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
    if( (aStack[nCurStack].Flags & RC_FALLBACK_UP) )
    {
        nCurStack--;
        // warning: this will delete *this, see below
        pOriginalResMgr->decStack();
    }
    else
    {
        ImpRCStack& rTop = aStack[nCurStack];
        if( (rTop.Flags & RC_FALLBACK_DOWN) )
        {
            #if OSL_DEBUG_LEVEL > 1
            OSL_TRACE( "returning from fallback %s",
                     OUStringToOString(pFallbackResMgr->GetFileName(), osl_getThreadTextEncoding() ).getStr() );
            #endif
            delete pFallbackResMgr;
            pFallbackResMgr = NULL;
        }
        nCurStack--;
    }
}

#ifdef DBG_UTIL

void ResMgr::TestStack( const Resource* pResObj )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    int upperLimit = nCurStack;

    if ( upperLimit < 0 )
    {
        OSL_FAIL( "resource stack underrun!" );
        upperLimit = aStack.size() - 1;
    }
    else if ( upperLimit >=  static_cast<int>(aStack.size()) )
    {
        OSL_FAIL( "stack occupation index > allocated stack size" );
        upperLimit = aStack.size() - 1;
    }

    if ( DbgIsResource() )
    {
        for( int i = 1; i <= upperLimit; ++i )
        {
            if ( aStack[i].pResObj == pResObj )
            {
                RscError_Impl( "Resource not freed! ", this,
                               aStack[i].pResource->GetRT(),
                               aStack[i].pResource->GetId(),
                               aStack, i-1 );
            }
        }
    }
}

#else

void ResMgr::TestStack( const Resource* )
{
}

#endif

sal_Bool ResMgr::IsAvailable( const ResId& rId, const Resource* pResObj ) const
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    sal_Bool            bAvailable = sal_False;
    RSHEADER_TYPE*  pClassRes = rId.GetpResource();
    RESOURCE_TYPE   nRT = rId.GetRT2();
    sal_uInt32      nId = rId.GetId();
    const ResMgr*   pMgr = rId.GetResMgr();

    if ( !pMgr )
        pMgr = this;

    if( pMgr->pFallbackResMgr )
    {
        ResId aId( rId );
        aId.SetResMgr( NULL );
        return pMgr->pFallbackResMgr->IsAvailable( aId, pResObj );
    }

    if ( !pResObj || pResObj == pMgr->aStack[pMgr->nCurStack].pResObj )
    {
        if ( !pClassRes )
            pClassRes = LocalResource( &pMgr->aStack[pMgr->nCurStack], nRT, nId );
        if ( pClassRes )
        {
            if ( pClassRes->GetRT() == nRT )
                bAvailable = sal_True;
        }
    }

    // vieleicht globale Resource
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

sal_Bool ResMgr::GetResource( const ResId& rId, const Resource* pResObj )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
    {
        ResId aId( rId );
        aId.SetResMgr( NULL );
        return pFallbackResMgr->GetResource( aId, pResObj );
    }

    ResMgr* pMgr = rId.GetResMgr();
    if ( pMgr && (this != pMgr) )
        return pMgr->GetResource( rId, pResObj );

    // normally Increment will pop the context; this is
    // not possible in RC_NOTFOUND case, so pop a frame here
    ImpRCStack* pTop = &aStack[nCurStack];
    if( (pTop->Flags & RC_NOTFOUND) )
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
            pTop->Flags |= RC_NOTFOUND;
            pTop->pClassRes = getEmptyBuffer();
            pTop->pResource = (RSHEADER_TYPE*)pTop->pClassRes;
            return sal_False;
        }
    }
    else
    {
        OSL_ENSURE( nCurStack > 0, "stack of 1 to shallow" );
        pTop->pClassRes = LocalResource( &aStack[nCurStack-1], nRT, nId );
    }

    if ( pTop->pClassRes )
        // lokale Resource, nicht system Resource
        pTop->pResource = (RSHEADER_TYPE *)pTop->pClassRes;
    else
    {
        pTop->pClassRes = pImpRes->LoadGlobalRes( nRT, nId, &pTop->aResHandle );
        if ( pTop->pClassRes )
        {
            pTop->Flags |= RC_GLOBAL;
            pTop->pResource = (RSHEADER_TYPE *)pTop->pClassRes;
        }
        else
        {
            // try to get a fallback resource
            pFallbackResMgr = CreateFallbackResMgr( rId, pResObj );
            if( pFallbackResMgr )
            {
                pTop->Flags |= RC_FALLBACK_DOWN;
#ifdef DBG_UTIL
                rtl::OStringBuffer aMess(
                    RTL_CONSTASCII_STRINGPARAM("found resource "));
                aMess.append(static_cast<sal_Int32>(nId));
                aMess.append(RTL_CONSTASCII_STRINGPARAM(" in fallback "));
                aMess.append(rtl::OUStringToOString(
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
                pTop->Flags |= RC_NOTFOUND;
                pTop->pClassRes = getEmptyBuffer();
                pTop->pResource = (RSHEADER_TYPE*)pTop->pClassRes;
                return sal_False;
            }
        }
    }

    return sal_True;
}

void * ResMgr::GetResourceSkipHeader( const ResId& rResId, ResMgr ** ppResMgr )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    DBG_ASSERT( rResId.GetResMgr(), "illegal ResId without ResMgr" );
    *ppResMgr = rResId.GetResMgr();
    if( *ppResMgr )
    {
        (*ppResMgr)->GetResource( rResId );
        (*ppResMgr)->Increment( sizeof( RSHEADER_TYPE ) );
        return (*ppResMgr)->GetClass();
    }
    return getEmptyBuffer();
}

void ResMgr::PopContext( const Resource* pResObj )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
    {
        pFallbackResMgr->PopContext( pResObj );
        return;
    }

#ifdef DBG_UTIL
    if ( DbgIsResource() )
    {
        if ( (aStack[nCurStack].pResObj != pResObj) || nCurStack == 0 )
        {
            RscError_Impl( "Cannot free resource! ", this,
                           RSC_NOTYPE, 0, aStack, nCurStack );
        }
    }
#endif

    if ( nCurStack > 0 )
    {
        ImpRCStack* pTop = &aStack[nCurStack];
#ifdef DBG_UTIL
        if ( DbgIsResource() && !(pTop->Flags & RC_NOTFOUND) )
        {
            void* pRes = (sal_uInt8*)pTop->pResource +
                         pTop->pResource->GetLocalOff();

            if ( pTop->pClassRes != pRes )
            {
                RscError_Impl( "Classpointer not at the end!",
                               this, pTop->pResource->GetRT(),
                               pTop->pResource->GetId(),
                               aStack, nCurStack-1 );
            }
        }
#endif

        // Resource freigeben
        if( (pTop->Flags & (RC_GLOBAL | RC_NOTFOUND)) == RC_GLOBAL )
            // kann auch Fremd-Resource sein
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
        aId.SetResMgr( NULL );
        return pFallbackResMgr->CreateBlock( aId );
    }

    RSHEADER_TYPE* pHeader = NULL;
    if ( GetResource( rId ) )
    {
        // Der Zeiger steht am Anfang, deswegen zeigt der Klassen-Pointer
        // auf den Header und die restliche Groesse ist die Gesammte.
        pHeader = (RSHEADER_TYPE*)rtl_allocateMemory( GetRemainSize() );
        memcpy( pHeader, GetClass(), GetRemainSize() );
        Increment( pHeader->GetLocalOff() ); //ans Ende setzen
        if ( pHeader->GetLocalOff() != pHeader->GetGlobOff() )
            // Hat Sub-Resourcen, deshalb extra freigeben
            PopContext();
    }

    return pHeader;
}

sal_Int16 ResMgr::GetShort( void * pShort )
{
    return ((*((sal_uInt8*)pShort + 0) << 8) |
            (*((sal_uInt8*)pShort + 1) << 0)   );
}

sal_Int32 ResMgr::GetLong( void * pLong )
{
    return ((*((sal_uInt8*)pLong + 0) << 24) |
            (*((sal_uInt8*)pLong + 1) << 16) |
            (*((sal_uInt8*)pLong + 2) <<  8) |
            (*((sal_uInt8*)pLong + 3) <<  0)   );
}

sal_uInt64 ResMgr::GetUInt64( void* pDatum )
{
    return ((sal_uInt64(*((sal_uInt8*)pDatum + 0)) << 56) |
            (sal_uInt64(*((sal_uInt8*)pDatum + 1)) << 48) |
            (sal_uInt64(*((sal_uInt8*)pDatum + 2)) << 40) |
            (sal_uInt64(*((sal_uInt8*)pDatum + 3)) << 32) |
            (sal_uInt64(*((sal_uInt8*)pDatum + 4)) << 24) |
            (sal_uInt64(*((sal_uInt8*)pDatum + 5)) << 16) |
            (sal_uInt64(*((sal_uInt8*)pDatum + 6)) <<  8) |
            (sal_uInt64(*((sal_uInt8*)pDatum + 7)) <<  0)   );
}

sal_uInt32 ResMgr::GetStringWithoutHook( UniString& rStr, const sal_uInt8* pStr )
{
    sal_uInt32 nLen=0;
    sal_uInt32 nRet = GetStringSize( pStr, nLen );
    UniString aString( (sal_Char*)pStr, RTL_TEXTENCODING_UTF8,
                       RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE |
                       RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                       RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT );
    rStr = aString;
    return nRet;
}

sal_uInt32 ResMgr::GetString( UniString& rStr, const sal_uInt8* pStr )
{
    UniString aString;
    sal_uInt32 nRet =  GetStringWithoutHook( aString, pStr );
    if ( pImplResHookProc )
        aString = pImplResHookProc( aString );
    rStr = aString;
    return nRet;
}

sal_uInt32 ResMgr::GetByteString( rtl::OString& rStr, const sal_uInt8* pStr )
{
    sal_uInt32 nLen=0;
    sal_uInt32 nRet = GetStringSize( pStr, nLen );
    rStr = rtl::OString( (const sal_Char*)pStr, nLen );
    return nRet;
}

sal_uInt32 ResMgr::GetStringSize( const sal_uInt8* pStr, sal_uInt32& nLen )
{
    nLen = static_cast< sal_uInt32 >( strlen( (const char*)pStr ) );
    return GetStringSize( nLen );
}

sal_uInt32 ResMgr::GetRemainSize()
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
        return pFallbackResMgr->GetRemainSize();

    const ImpRCStack& rTop = aStack[nCurStack];
    return  (sal_uInt32)((long)(sal_uInt8 *)rTop.pResource +
                     rTop.pResource->GetLocalOff() -
                     (long)(sal_uInt8 *)rTop.pClassRes);
}

void* ResMgr::Increment( sal_uInt32 nSize )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
        return pFallbackResMgr->Increment( nSize );

    ImpRCStack& rStack = aStack[nCurStack];
    if( (rStack.Flags & RC_NOTFOUND) )
        return rStack.pClassRes;

    sal_uInt8* pClassRes = (sal_uInt8*)rStack.pClassRes + nSize;

    rStack.pClassRes = pClassRes;

    RSHEADER_TYPE* pRes = rStack.pResource;

    sal_uInt32 nLocalOff = pRes->GetLocalOff();
    if ( (pRes->GetGlobOff() == nLocalOff) &&
         (((char*)pRes + nLocalOff) == rStack.pClassRes) &&
         (rStack.Flags & RC_AUTORELEASE))
    {
        PopContext( rStack.pResObj );
    }

    return pClassRes;
}

ResMgr* ResMgr::CreateFallbackResMgr( const ResId& rId, const Resource* pResource )
{
    ResMgr *pFallback = NULL;
    if( nCurStack > 0 )
    {
        // get the next fallback level in resource file scope
        InternalResMgr* pRes = ResMgrContainer::get().getNextFallback( pImpRes );
        if( pRes )
        {
            // check that the fallback locale is not already in the chain of
            // fallbacks - prevent fallback loops
            ResMgr* pResMgr = this;
            while( pResMgr &&
                   ( pResMgr->pImpRes->aLocale.Language != pRes->aLocale.Language ||
                     pResMgr->pImpRes->aLocale.Country  != pRes->aLocale.Country  ||
                     pResMgr->pImpRes->aLocale.Variant  != pRes->aLocale.Variant )
                 )
            {
                pResMgr = pResMgr->pOriginalResMgr;
            }
            if( pResMgr )
            {
                // found a recursion, no fallback possible
                ResMgrContainer::get().freeResMgr( pRes );
                return NULL;
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
                    pFallback->aStack[pFallback->nCurStack].Flags |= RC_FALLBACK_UP;
            }
            if( !bHaveStack )
            {
                delete pFallback;
                pFallback = NULL;
            }
        }
    }
    return pFallback;
}

// method left here for SDK compatibility,
// used in "framework/source/services/substitutepathvars.cxx"
// phone numbers no longer in use for resource files

const char* ResMgr::GetLang( LanguageType& nType, sal_uInt16 nPrio )
{
    if ( nType == LANGUAGE_SYSTEM || nType == LANGUAGE_DONTKNOW )
        nType = MsLangId::getSystemUILanguage();

    if ( nPrio == 0 )
    {
        switch ( nType )
        {
            case LANGUAGE_DANISH:
                return "45";

            case LANGUAGE_DUTCH:
            case LANGUAGE_DUTCH_BELGIAN:
                return "31";

            case LANGUAGE_ENGLISH:
            case LANGUAGE_ENGLISH_UK:
            case LANGUAGE_ENGLISH_EIRE:
            case LANGUAGE_ENGLISH_SAFRICA:
            case LANGUAGE_ENGLISH_JAMAICA:
            case LANGUAGE_ENGLISH_BELIZE:
            case LANGUAGE_ENGLISH_TRINIDAD:
            case LANGUAGE_ENGLISH_ZIMBABWE:
            case LANGUAGE_ENGLISH_PHILIPPINES:
                return "44";

            case LANGUAGE_ENGLISH_US:
            case LANGUAGE_ENGLISH_CAN:
                return "01";

            case LANGUAGE_ENGLISH_AUS:
            case LANGUAGE_ENGLISH_NZ:
                return "61";
            case LANGUAGE_ESTONIAN:
                return "77";


            case LANGUAGE_FINNISH:
                return "35";

            case LANGUAGE_FRENCH_CANADIAN:
                return "02";

            case LANGUAGE_FRENCH:
            case LANGUAGE_FRENCH_BELGIAN:
            case LANGUAGE_FRENCH_SWISS:
            case LANGUAGE_FRENCH_LUXEMBOURG:
            case LANGUAGE_FRENCH_MONACO:
                return "33";

            case LANGUAGE_GERMAN:
            case LANGUAGE_GERMAN_SWISS:
            case LANGUAGE_GERMAN_AUSTRIAN:
            case LANGUAGE_GERMAN_LUXEMBOURG:
            case LANGUAGE_GERMAN_LIECHTENSTEIN:
                return "49";

            case LANGUAGE_ITALIAN:
            case LANGUAGE_ITALIAN_SWISS:
                return "39";

            case LANGUAGE_NORWEGIAN:
            case LANGUAGE_NORWEGIAN_BOKMAL:
                return "47";

            case LANGUAGE_PORTUGUESE:
                return "03";

            case LANGUAGE_PORTUGUESE_BRAZILIAN:
                return "55";

            case LANGUAGE_SPANISH_DATED:
            case LANGUAGE_SPANISH_MEXICAN:
            case LANGUAGE_SPANISH_MODERN:
            case LANGUAGE_SPANISH_GUATEMALA:
            case LANGUAGE_SPANISH_COSTARICA:
            case LANGUAGE_SPANISH_PANAMA:
            case LANGUAGE_SPANISH_DOMINICAN_REPUBLIC:
            case LANGUAGE_SPANISH_VENEZUELA:
            case LANGUAGE_SPANISH_COLOMBIA:
            case LANGUAGE_SPANISH_PERU:
            case LANGUAGE_SPANISH_ARGENTINA:
            case LANGUAGE_SPANISH_ECUADOR:
            case LANGUAGE_SPANISH_CHILE:
            case LANGUAGE_SPANISH_URUGUAY:
            case LANGUAGE_SPANISH_PARAGUAY:
            case LANGUAGE_SPANISH_BOLIVIA:
                return "34";

            case LANGUAGE_SWEDISH:
                return "46";

            case LANGUAGE_POLISH:
                return "48";
            case LANGUAGE_CZECH:
                return "42";
            case LANGUAGE_SLOVENIAN:
                return "50";
            case LANGUAGE_HUNGARIAN:
                return "36";
            case LANGUAGE_RUSSIAN:
                return "07";
            case LANGUAGE_SLOVAK:
                return "43";
            case LANGUAGE_GREEK:
                return "30";
            case LANGUAGE_TURKISH:
                return "90";

            case LANGUAGE_CHINESE_SIMPLIFIED:
                return "86";
            case LANGUAGE_CHINESE_TRADITIONAL:
                return "88";
            case LANGUAGE_JAPANESE:
                return "81";
            case LANGUAGE_KOREAN:
            case LANGUAGE_KOREAN_JOHAB:
                return "82";
            case LANGUAGE_THAI:
                return "66";
            case LANGUAGE_HINDI:
                return "91";

            case LANGUAGE_ARABIC_PRIMARY_ONLY:
            case LANGUAGE_ARABIC_IRAQ:
            case LANGUAGE_ARABIC_EGYPT:
            case LANGUAGE_ARABIC_LIBYA:
            case LANGUAGE_ARABIC_ALGERIA:
            case LANGUAGE_ARABIC_MOROCCO:
            case LANGUAGE_ARABIC_TUNISIA:
            case LANGUAGE_ARABIC_OMAN:
            case LANGUAGE_ARABIC_YEMEN:
            case LANGUAGE_ARABIC_SYRIA:
            case LANGUAGE_ARABIC_JORDAN:
            case LANGUAGE_ARABIC_LEBANON:
            case LANGUAGE_ARABIC_KUWAIT:
            case LANGUAGE_ARABIC_UAE:
            case LANGUAGE_ARABIC_BAHRAIN:
            case LANGUAGE_ARABIC_QATAR:
                return "96";

            case LANGUAGE_HEBREW:
                return "97";

            case LANGUAGE_CATALAN:
                return "37";

            default:
                return "99";
        }
    }
    else if ( nPrio == 1 )
    {
        switch ( nType )
        {
            case LANGUAGE_FRENCH_CANADIAN:
                return "33";

            case LANGUAGE_PORTUGUESE_BRAZILIAN:
                return "03";

            default:
                return NULL;
        }
    }
    else if ( nPrio == 2 )
        return "01";
    else if ( nPrio == 3 )
        return "44";
    else if ( nPrio == 4 )
        return "49";
    else
        return "99";
}

ResMgr* ResMgr::CreateResMgr( const sal_Char* pPrefixName,
                              com::sun::star::lang::Locale aLocale )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    OUString aPrefix( pPrefixName, strlen( pPrefixName ), osl_getThreadTextEncoding() );

    if( aLocale.Language.isEmpty() )
        aLocale = ResMgrContainer::get().getDefLocale();

    InternalResMgr* pImp = ResMgrContainer::get().getResMgr( aPrefix, aLocale );
    return pImp ? new ResMgr( pImp ) : NULL;
}

ResMgr* ResMgr::SearchCreateResMgr(
    const sal_Char* pPrefixName,
    com::sun::star::lang::Locale& rLocale )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    OUString aPrefix( pPrefixName, strlen( pPrefixName ), osl_getThreadTextEncoding() );

    if( rLocale.Language.isEmpty() )
        rLocale = ResMgrContainer::get().getDefLocale();

    InternalResMgr* pImp = ResMgrContainer::get().getResMgr( aPrefix, rLocale );
    return pImp ? new ResMgr( pImp ) : NULL;
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

UniString ResMgr::ReadStringWithoutHook()
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
        return pFallbackResMgr->ReadStringWithoutHook();

    UniString aRet;

    const ImpRCStack& rTop = aStack[nCurStack];
    if( (rTop.Flags & RC_NOTFOUND) )
    {
        #if OSL_DEBUG_LEVEL > 0
        aRet = OUString( RTL_CONSTASCII_USTRINGPARAM( "<resource not found>" ) );
        #endif
    }
    else
        Increment( GetStringWithoutHook( aRet, (const sal_uInt8*)GetClass() ) );

    return aRet;
}

UniString ResMgr::ReadString()
{
    UniString aRet = ReadStringWithoutHook();
    if ( pImplResHookProc )
        aRet = pImplResHookProc( aRet );
    return aRet;
}

rtl::OString ResMgr::ReadByteString()
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
        return pFallbackResMgr->ReadByteString();

    rtl::OString aRet;

    const ImpRCStack& rTop = aStack[nCurStack];
    if( (rTop.Flags & RC_NOTFOUND) )
    {
        #if OSL_DEBUG_LEVEL > 0
        aRet = OString( "<resource not found>" );
        #endif
    }
    else
        Increment( GetByteString( aRet, (const sal_uInt8*)GetClass() ) );

    return aRet;
}

rtl::OString ResMgr::GetAutoHelpId()
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );

    if( pFallbackResMgr )
        return pFallbackResMgr->GetAutoHelpId();

    OSL_ENSURE( nCurStack, "resource stack empty in Auto help id generation" );
    if( nCurStack < 1 || nCurStack > 2 )
        return rtl::OString();

    // prepare HID, start with resource prefix
    rtl::OStringBuffer aHID( 32 );
    aHID.append( rtl::OUStringToOString( pImpRes->aPrefix, RTL_TEXTENCODING_UTF8 ) );
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
            case RSC_MODELESSDIALOG:    aHID.append( "ModelessDialog" );   break;
            case RSC_FLOATINGWINDOW:    aHID.append( "FloatingWindow" );   break;
            case RSC_MODALDIALOG:       aHID.append( "ModalDialog" );      break;
            case RSC_TABPAGE:           aHID.append( "TabPage" );          break;
            default: return rtl::OString();
        }
    }
    else
    {
        // only controls with the following parents get auto help ids
        const ImpRCStack *pRC1 = StackTop(1);
        switch( pRC1->pResource->GetRT() ) {
            case RSC_DOCKINGWINDOW:
            case RSC_WORKWIN:
            case RSC_MODELESSDIALOG:
            case RSC_FLOATINGWINDOW:
            case RSC_MODALDIALOG:
            case RSC_TABPAGE:
                // intentionally no breaks!
                // auto help ids for controls
                switch( pRC->pResource->GetRT() ) {
                    case RSC_TABCONTROL:        aHID.append( "TabControl" );       break;
                    case RSC_RADIOBUTTON:       aHID.append( "RadioButton" );      break;
                    case RSC_CHECKBOX:          aHID.append( "CheckBox" );         break;
                    case RSC_TRISTATEBOX:       aHID.append( "TriStateBox" );      break;
                    case RSC_EDIT:              aHID.append( "Edit" );             break;
                    case RSC_MULTILINEEDIT:     aHID.append( "MultiLineEdit" );    break;
                    case RSC_MULTILISTBOX:      aHID.append( "MultiListBox" );     break;
                    case RSC_LISTBOX:           aHID.append( "ListBox" );          break;
                    case RSC_COMBOBOX:          aHID.append( "ComboBox" );         break;
                    case RSC_PUSHBUTTON:        aHID.append( "PushButton" );       break;
                    case RSC_SPINFIELD:         aHID.append( "SpinField" );        break;
                    case RSC_PATTERNFIELD:      aHID.append( "PatternField" );     break;
                    case RSC_NUMERICFIELD:      aHID.append( "NumericField" );     break;
                    case RSC_METRICFIELD:       aHID.append( "MetricField" );      break;
                    case RSC_CURRENCYFIELD:     aHID.append( "CurrencyField" );    break;
                    case RSC_DATEFIELD:         aHID.append( "DateField" );        break;
                    case RSC_TIMEFIELD:         aHID.append( "TimeField" );        break;
                    case RSC_IMAGERADIOBUTTON:  aHID.append( "ImageRadioButton" ); break;
                    case RSC_NUMERICBOX:        aHID.append( "NumericBox" );       break;
                    case RSC_METRICBOX:         aHID.append( "MetricBox" );        break;
                    case RSC_CURRENCYBOX:       aHID.append( "CurrencyBox" );      break;
                    case RSC_DATEBOX:           aHID.append( "DateBox" );          break;
                    case RSC_TIMEBOX:           aHID.append( "TimeBox" );          break;
                    case RSC_IMAGEBUTTON:       aHID.append( "ImageButton" );      break;
                    case RSC_MENUBUTTON:        aHID.append( "MenuButton" );       break;
                    case RSC_MOREBUTTON:        aHID.append( "MoreButton" );       break;
                    default:
                        // no type, no auto HID
                        return rtl::OString();
                }
                break;
            default:
                return rtl::OString();
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

void ResMgr::SetDefaultLocale( const com::sun::star::lang::Locale& rLocale )
{
    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );
    ResMgrContainer::get().setDefLocale( rLocale );
}

const OUString& ResMgr::GetFileName() const
{
    return pImpRes->aFileName;
}

SimpleResMgr::SimpleResMgr( const sal_Char* pPrefixName,
                            const ::com::sun::star::lang::Locale& rLocale )
{
    OUString aPrefix( pPrefixName, strlen( pPrefixName ), osl_getThreadTextEncoding() );
    com::sun::star::lang::Locale aLocale( rLocale );

    osl::Guard<osl::Mutex> aGuard( getResMgrMutex() );
    if( aLocale.Language.isEmpty() )
        aLocale = ResMgrContainer::get().getDefLocale();

    m_pResImpl = ResMgrContainer::get().getResMgr( aPrefix, aLocale, true );
    DBG_ASSERT( m_pResImpl, "SimpleResMgr::SimpleResMgr : have no impl class !" );
}

SimpleResMgr::~SimpleResMgr()
{
    delete m_pResImpl;
}

SimpleResMgr* SimpleResMgr::Create( const sal_Char* pPrefixName, com::sun::star::lang::Locale aLocale )
{
    return new SimpleResMgr( pPrefixName, aLocale );
}

bool SimpleResMgr::IsAvailable( RESOURCE_TYPE _resourceType, sal_uInt32 _resourceId )
{
    osl::MutexGuard aGuard(m_aAccessSafety);

    if ( ( RSC_STRING != _resourceType ) && ( RSC_RESOURCE != _resourceType ) )
        return false;

    DBG_ASSERT( m_pResImpl, "SimpleResMgr::IsAvailable: have no impl class !" );
    return m_pResImpl->IsGlobalAvailable( _resourceType, _resourceId );
}

rtl::OUString SimpleResMgr::ReadString( sal_uInt32 nId )
{
    osl::MutexGuard aGuard(m_aAccessSafety);

    DBG_ASSERT( m_pResImpl, "SimpleResMgr::ReadString : have no impl class !" );
    // perhaps constructed with an invalid filename ?

    UniString sReturn;
    if ( !m_pResImpl )
        return sReturn;

    void* pResHandle = NULL;
    InternalResMgr* pFallback = m_pResImpl;
    RSHEADER_TYPE* pResHeader = (RSHEADER_TYPE*)m_pResImpl->LoadGlobalRes( RSC_STRING, nId, &pResHandle );
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
                if( pFallback->aLocale.Language != m_pResImpl->aLocale.Language ||
                    pFallback->aLocale.Country  != m_pResImpl->aLocale.Country  ||
                    pFallback->aLocale.Variant  != m_pResImpl->aLocale.Variant )
                {
                    pResHeader = (RSHEADER_TYPE*)pFallback->LoadGlobalRes( RSC_STRING, nId, &pResHandle );
                }
                else
                {
                    ResMgrContainer::get().freeResMgr( pFallback );
                    pFallback = NULL;
                }
            }
        }
        if( ! pResHandle )
            // no such resource
            return sReturn;
    }

    // sal_uIntPtr nLen = pResHeader->GetLocalOff() - sizeof(RSHEADER_TYPE);
    ResMgr::GetString( sReturn, (const sal_uInt8*)(pResHeader+1) );

    // not neccessary with te current implementation which holds the string table permanently, but to be sure ....
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
