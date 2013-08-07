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


#include <stdlib.h>
#include <stdio.h>

#include <boost/unordered_map.hpp>

#include <comphelper/string.hxx>
#include "vcl/ppdparser.hxx"
#include "vcl/strhelper.hxx"
#include "vcl/helper.hxx"
#include "vcl/svapp.hxx"
#include "cupsmgr.hxx"
#include "tools/urlobj.hxx"
#include "tools/stream.hxx"
#include "tools/zcodec.hxx"
#include "osl/mutex.hxx"
#include "osl/file.hxx"
#include "osl/process.h"
#include "osl/thread.h"
#include "rtl/strbuf.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/instance.hxx"
#include <sal/macros.h>
#include <salhelper/linkhelper.hxx>

#include "com/sun/star/lang/Locale.hpp"

namespace psp
{
    class PPDTranslator
    {
        struct LocaleEqual
        {
            bool operator()(const com::sun::star::lang::Locale& i_rLeft,
                            const com::sun::star::lang::Locale& i_rRight) const
            {
                return i_rLeft.Language.equals( i_rRight.Language ) &&
                i_rLeft.Country.equals( i_rRight.Country ) &&
                i_rLeft.Variant.equals( i_rRight.Variant );
            }
        };

        struct LocaleHash
        {
            size_t operator()(const com::sun::star::lang::Locale& rLocale) const
            { return
                  (size_t)rLocale.Language.hashCode()
                ^ (size_t)rLocale.Country.hashCode()
                ^ (size_t)rLocale.Variant.hashCode()
                ;
            }
        };

        typedef boost::unordered_map< com::sun::star::lang::Locale, OUString, LocaleHash, LocaleEqual > translation_map;
        typedef boost::unordered_map< OUString, translation_map, OUStringHash > key_translation_map;

        key_translation_map     m_aTranslations;
        public:
        PPDTranslator() {}
        ~PPDTranslator() {}


        void insertValue(
            const OUString& i_rKey,
            const OUString& i_rOption,
            const OUString& i_rValue,
            const OUString& i_rTranslation,
            const com::sun::star::lang::Locale& i_rLocale = com::sun::star::lang::Locale()
            );

        void insertOption( const OUString& i_rKey,
                           const OUString& i_rOption,
                           const OUString& i_rTranslation,
                           const com::sun::star::lang::Locale& i_rLocale = com::sun::star::lang::Locale() )
        {
            insertValue( i_rKey, i_rOption, OUString(), i_rTranslation, i_rLocale );
        }

        void insertKey( const OUString& i_rKey,
                        const OUString& i_rTranslation,
                        const com::sun::star::lang::Locale& i_rLocale = com::sun::star::lang::Locale() )
        {
            insertValue( i_rKey, OUString(), OUString(), i_rTranslation, i_rLocale );
        }

        OUString translateValue(
            const OUString& i_rKey,
            const OUString& i_rOption,
            const OUString& i_rValue,
            const com::sun::star::lang::Locale& i_rLocale = com::sun::star::lang::Locale()
            ) const;

        OUString translateOption( const OUString& i_rKey,
                                       const OUString& i_rOption,
                                       const com::sun::star::lang::Locale& i_rLocale = com::sun::star::lang::Locale() ) const
        {
            return translateValue( i_rKey, i_rOption, OUString(), i_rLocale );
        }

        OUString translateKey( const OUString& i_rKey,
                                    const com::sun::star::lang::Locale& i_rLocale = com::sun::star::lang::Locale() ) const
        {
            return translateValue( i_rKey, OUString(), OUString(), i_rLocale );
        }
    };

    static com::sun::star::lang::Locale normalizeInputLocale(
        const com::sun::star::lang::Locale& i_rLocale,
        bool bInsertDefault = false
        )
    {
        com::sun::star::lang::Locale aLoc( i_rLocale );
        if( bInsertDefault && aLoc.Language.isEmpty() )
        {
            // empty locale requested, fill in application UI locale
            aLoc = Application::GetSettings().GetUILanguageTag().getLocale();

            #if OSL_DEBUG_LEVEL > 1
            static const char* pEnvLocale = getenv( "SAL_PPDPARSER_LOCALE" );
            if( pEnvLocale && *pEnvLocale )
            {
                OString aStr( pEnvLocale );
                sal_Int32 nLen = aStr.getLength();
                aLoc.Language = OStringToOUString( aStr.copy( 0, nLen > 2 ? 2 : nLen ), RTL_TEXTENCODING_MS_1252 );
                if( nLen >=5 && aStr.getStr()[2] == '_' )
                    aLoc.Country = OStringToOUString( aStr.copy( 3, 2 ), RTL_TEXTENCODING_MS_1252 );
                else
                    aLoc.Country = OUString();
                aLoc.Variant = OUString();
            }
            #endif
        }
        /* FIXME-BCP47: using Variant, uppercase? */
        aLoc.Language = aLoc.Language.toAsciiLowerCase();
        aLoc.Country  = aLoc.Country.toAsciiUpperCase();
        aLoc.Variant  = aLoc.Variant.toAsciiUpperCase();

        return aLoc;
    }

    void PPDTranslator::insertValue(
        const OUString& i_rKey,
        const OUString& i_rOption,
        const OUString& i_rValue,
        const OUString& i_rTranslation,
        const com::sun::star::lang::Locale& i_rLocale
        )
    {
        OUStringBuffer aKey( i_rKey.getLength() + i_rOption.getLength() + i_rValue.getLength() + 2 );
        aKey.append( i_rKey );
        if( !i_rOption.isEmpty() || !i_rValue.isEmpty() )
        {
            aKey.append( sal_Unicode( ':' ) );
            aKey.append( i_rOption );
        }
        if( !i_rValue.isEmpty() )
        {
            aKey.append( sal_Unicode( ':' ) );
            aKey.append( i_rValue );
        }
        if( !aKey.isEmpty() && !i_rTranslation.isEmpty() )
        {
            OUString aK( aKey.makeStringAndClear() );
            com::sun::star::lang::Locale aLoc;
            /* FIXME-BCP47: using Variant, uppercase? */
            aLoc.Language = i_rLocale.Language.toAsciiLowerCase();
            aLoc.Country  = i_rLocale.Country.toAsciiUpperCase();
            aLoc.Variant  = i_rLocale.Variant.toAsciiUpperCase();
            m_aTranslations[ aK ][ aLoc ] = i_rTranslation;
        }
    }

    OUString PPDTranslator::translateValue(
        const OUString& i_rKey,
        const OUString& i_rOption,
        const OUString& i_rValue,
        const com::sun::star::lang::Locale& i_rLocale
        ) const
    {
        OUString aResult;

        OUStringBuffer aKey( i_rKey.getLength() + i_rOption.getLength() + i_rValue.getLength() + 2 );
        aKey.append( i_rKey );
        if( !i_rOption.isEmpty() || !i_rValue.isEmpty() )
        {
            aKey.append( sal_Unicode( ':' ) );
            aKey.append( i_rOption );
        }
        if( !i_rValue.isEmpty() )
        {
            aKey.append( sal_Unicode( ':' ) );
            aKey.append( i_rValue );
        }
        if( !aKey.isEmpty() )
        {
            OUString aK( aKey.makeStringAndClear() );
            key_translation_map::const_iterator it = m_aTranslations.find( aK );
            if( it != m_aTranslations.end() )
            {
                const translation_map& rMap( it->second );

                com::sun::star::lang::Locale aLoc( normalizeInputLocale( i_rLocale, true ) );
                /* FIXME-BCP47: use LanguageTag::getFallbackStrings()? */
                for( int nTry = 0; nTry < 4; nTry++ )
                {
                    translation_map::const_iterator tr = rMap.find( aLoc );
                    if( tr != rMap.end() )
                    {
                        aResult = tr->second;
                        break;
                    }
                    switch( nTry )
                    {
                    case 0: aLoc.Variant  = OUString();break;
                    case 1: aLoc.Country  = OUString();break;
                    case 2: aLoc.Language = OUString();break;
                    }
                }
            }
        }
        return aResult;
    }

    class PPDCache
    {
    public:
        std::list< PPDParser* > aAllParsers;
        boost::unordered_map< OUString, OUString, OUStringHash >* pAllPPDFiles;
        PPDCache()
            : pAllPPDFiles(NULL)
        {}
        ~PPDCache()
        {
            while( aAllParsers.begin() != aAllParsers.end() )
            {
                delete aAllParsers.front();
                aAllParsers.pop_front();
            }
            delete pAllPPDFiles;
            pAllPPDFiles = NULL;
        }
    };
}

using namespace psp;


#if OSL_DEBUG_LEVEL > 2
#define BSTRING(x) OUStringToOString( x, osl_getThreadTextEncoding() )
#endif

namespace
{
    struct thePPDCache : public rtl::Static<PPDCache, thePPDCache> {};
}

class PPDDecompressStream
{
    SvFileStream*       mpFileStream;
    SvMemoryStream*     mpMemStream;
    OUString       maFileName;

    // forbid copying
    PPDDecompressStream( const PPDDecompressStream& );
    PPDDecompressStream& operator=(const PPDDecompressStream& );

    public:
    PPDDecompressStream( const OUString& rFile );
    ~PPDDecompressStream();

    bool IsOpen() const;
    bool IsEof() const;
    OString ReadLine();
    void Open( const OUString& i_rFile );
    void Close();
    const OUString& GetFileName() const { return maFileName; }
};

PPDDecompressStream::PPDDecompressStream( const OUString& i_rFile ) :
    mpFileStream( NULL ),
    mpMemStream( NULL )
{
    Open( i_rFile );
}

PPDDecompressStream::~PPDDecompressStream()
{
    Close();
}

void PPDDecompressStream::Open( const OUString& i_rFile )
{
    Close();

    mpFileStream = new SvFileStream( i_rFile, STREAM_READ );
    maFileName = mpFileStream->GetFileName();

    if( ! mpFileStream->IsOpen() )
    {
        Close();
        return;
    }

    OString aLine;
    mpFileStream->ReadLine( aLine );
    mpFileStream->Seek( 0 );

    // check for compress'ed or gzip'ed file
    sal_uLong nCompressMethod = 0;
    if( aLine.getLength() > 1 && static_cast<unsigned char>(aLine[0]) == 0x1f )
    {
        if( static_cast<unsigned char>(aLine[1]) == 0x8b ) // check for gzip
            nCompressMethod = ZCODEC_DEFAULT | ZCODEC_GZ_LIB;
    }

    if( nCompressMethod != 0 )
    {
        // so let's try to decompress the stream
        mpMemStream = new SvMemoryStream( 4096, 4096 );
        ZCodec aCodec;
        aCodec.BeginCompression( nCompressMethod );
        long nComp = aCodec.Decompress( *mpFileStream, *mpMemStream );
        aCodec.EndCompression();
        if( nComp < 0 )
        {
            // decompression failed, must be an uncompressed stream after all
            delete mpMemStream, mpMemStream = NULL;
            mpFileStream->Seek( 0 );
        }
        else
        {
            // compression successful, can get rid of file stream
            delete mpFileStream, mpFileStream = NULL;
            mpMemStream->Seek( 0 );
        }
    }
}

void PPDDecompressStream::Close()
{
    delete mpMemStream, mpMemStream = NULL;
    delete mpFileStream, mpFileStream = NULL;
}

bool PPDDecompressStream::IsOpen() const
{
    return (mpMemStream || (mpFileStream && mpFileStream->IsOpen()));
}

bool PPDDecompressStream::IsEof() const
{
    return ( mpMemStream ? mpMemStream->IsEof() : ( mpFileStream ? mpFileStream->IsEof() : true ) );
}

OString PPDDecompressStream::ReadLine()
{
    OString o_rLine;
    if( mpMemStream )
        mpMemStream->ReadLine( o_rLine );
    else if( mpFileStream )
        mpFileStream->ReadLine( o_rLine );
    return o_rLine;
}

static osl::FileBase::RC resolveLink( const OUString& i_rURL, OUString& o_rResolvedURL, OUString& o_rBaseName, osl::FileStatus::Type& o_rType, int nLinkLevel = 10 )
{
    salhelper::LinkResolver aResolver(osl_FileStatus_Mask_FileName |
                                      osl_FileStatus_Mask_Type |
                                      osl_FileStatus_Mask_FileURL);

    osl::FileBase::RC aRet = aResolver.fetchFileStatus(i_rURL, nLinkLevel);

    if (aRet  == osl::FileBase::E_None)
    {
        o_rResolvedURL = aResolver.m_aStatus.getFileURL();
        o_rBaseName = aResolver.m_aStatus.getFileName();
        o_rType = aResolver.m_aStatus.getFileType();
    }

    return aRet;
}

void PPDParser::scanPPDDir( const OUString& rDir )
{
    static struct suffix_t
    {
        const sal_Char* pSuffix;
        const sal_Int32 nSuffixLen;
    } const pSuffixes[] =
    { { ".PS", 3 },  { ".PPD", 4 }, { ".PS.GZ", 6 }, { ".PPD.GZ", 7 } };

    const int nSuffixes = SAL_N_ELEMENTS(pSuffixes);

    PPDCache &rPPDCache = thePPDCache::get();

    osl::Directory aDir( rDir );
    if ( aDir.open() == osl::FileBase::E_None )
    {
        osl::DirectoryItem aItem;

        INetURLObject aPPDDir(rDir);
        while( aDir.getNextItem( aItem ) == osl::FileBase::E_None )
        {
            osl::FileStatus aStatus( osl_FileStatus_Mask_FileName );
            if( aItem.getFileStatus( aStatus ) == osl::FileBase::E_None )
            {
                OUStringBuffer aURLBuf( rDir.getLength() + 64 );
                aURLBuf.append( rDir );
                aURLBuf.append( sal_Unicode( '/' ) );
                aURLBuf.append( aStatus.getFileName() );

                OUString aFileURL, aFileName;
                osl::FileStatus::Type eType = osl::FileStatus::Unknown;

                if( resolveLink( aURLBuf.makeStringAndClear(), aFileURL, aFileName, eType ) == osl::FileBase::E_None )
                {
                    if( eType == osl::FileStatus::Regular )
                    {
                        INetURLObject aPPDFile = aPPDDir;
                        aPPDFile.Append( aFileName );

                        // match extension
                        for( int nSuffix = 0; nSuffix < nSuffixes; nSuffix++ )
                        {
                            if( aFileName.getLength() > pSuffixes[nSuffix].nSuffixLen )
                            {
                                if( aFileName.endsWithIgnoreAsciiCaseAsciiL( pSuffixes[nSuffix].pSuffix, pSuffixes[nSuffix].nSuffixLen ) )
                                {
                                (*rPPDCache.pAllPPDFiles)[ aFileName.copy( 0, aFileName.getLength() - pSuffixes[nSuffix].nSuffixLen ) ] = aPPDFile.PathToFileName();
                                    break;
                                }
                            }
                        }
                    }
                    else if( eType == osl::FileStatus::Directory )
                    {
                        scanPPDDir( aFileURL );
                    }
                }
            }
        }
        aDir.close();
    }
}

void PPDParser::initPPDFiles()
{
    PPDCache &rPPDCache = thePPDCache::get();
    if( rPPDCache.pAllPPDFiles )
        return;

    rPPDCache.pAllPPDFiles = new boost::unordered_map< OUString, OUString, OUStringHash >();

    // check installation directories
    std::list< OUString > aPathList;
    psp::getPrinterPathList( aPathList, PRINTER_PPDDIR );
    for( std::list< OUString >::const_iterator ppd_it = aPathList.begin(); ppd_it != aPathList.end(); ++ppd_it )
    {
        INetURLObject aPPDDir( *ppd_it, INET_PROT_FILE, INetURLObject::ENCODE_ALL );
        scanPPDDir( aPPDDir.GetMainURL( INetURLObject::NO_DECODE ) );
    }
    if( rPPDCache.pAllPPDFiles->find( OUString( "SGENPRT" ) ) == rPPDCache.pAllPPDFiles->end() )
    {
        // last try: search in directory of executable (mainly for setup)
        OUString aExe;
        if( osl_getExecutableFile( &aExe.pData ) == osl_Process_E_None )
        {
            INetURLObject aDir( aExe );
            aDir.removeSegment();
#ifdef DEBUG
            fprintf( stderr, "scanning last chance dir: %s\n", OUStringToOString( aDir.GetMainURL( INetURLObject::NO_DECODE ), osl_getThreadTextEncoding() ).getStr() );
#endif
            scanPPDDir( aDir.GetMainURL( INetURLObject::NO_DECODE ) );
#ifdef DEBUG
            fprintf( stderr, "SGENPRT %s\n", rPPDCache.pAllPPDFiles->find( OUString( "SGENPRT" ) ) == rPPDCache.pAllPPDFiles->end() ? "not found" : "found" );
#endif
        }
    }
}

void PPDParser::getKnownPPDDrivers( std::list< OUString >& o_rDrivers, bool bRefresh )
{
    PPDCache &rPPDCache = thePPDCache::get();

    if( bRefresh )
    {
        delete rPPDCache.pAllPPDFiles;
        rPPDCache.pAllPPDFiles = NULL;
    }

    initPPDFiles();
    o_rDrivers.clear();

    boost::unordered_map< OUString, OUString, OUStringHash >::const_iterator it;
    for( it = rPPDCache.pAllPPDFiles->begin(); it != rPPDCache.pAllPPDFiles->end(); ++it )
        o_rDrivers.push_back( it->first );
}

OUString PPDParser::getPPDFile( const OUString& rFile )
{
    INetURLObject aPPD( rFile, INET_PROT_FILE, INetURLObject::ENCODE_ALL );
    // someone might enter a full qualified name here
    PPDDecompressStream aStream( aPPD.PathToFileName() );
    if( ! aStream.IsOpen() )
    {
        boost::unordered_map< OUString, OUString, OUStringHash >::const_iterator it;
        PPDCache &rPPDCache = thePPDCache::get();

        bool bRetry = true;
        do
        {
            initPPDFiles();
            // some PPD files contain dots beside the extension, so try name first
            // and cut of points after that
            OUString aBase( rFile );
            sal_Int32 nLastIndex = aBase.lastIndexOf( sal_Unicode( '/' ) );
            if( nLastIndex >= 0 )
                aBase = aBase.copy( nLastIndex+1 );
            do
            {
                it = rPPDCache.pAllPPDFiles->find( aBase );
                nLastIndex = aBase.lastIndexOf( sal_Unicode( '.' ) );
                if( nLastIndex > 0 )
                    aBase = aBase.copy( 0, nLastIndex );
            } while( it == rPPDCache.pAllPPDFiles->end() && nLastIndex > 0 );

            if( it == rPPDCache.pAllPPDFiles->end() && bRetry )
            {
                // a new file ? rehash
                delete rPPDCache.pAllPPDFiles; rPPDCache.pAllPPDFiles = NULL;
                bRetry = false;
                // note this is optimized for office start where
                // no new files occur and initPPDFiles is called only once
            }
        } while( ! rPPDCache.pAllPPDFiles );

        if( it != rPPDCache.pAllPPDFiles->end() )
            aStream.Open( it->second );
    }

    OUString aRet;
    if( aStream.IsOpen() )
    {
        OString aLine = aStream.ReadLine();
        if (aLine.indexOfL(RTL_CONSTASCII_STRINGPARAM("*PPD-Adobe")) == 0)
            aRet = aStream.GetFileName();
        else
        {
            // our *Include hack does usually not begin
            // with *PPD-Adobe, so try some lines for *Include
            int nLines = 10;
            while (aLine.indexOfL(RTL_CONSTASCII_STRINGPARAM("*Include")) != 0 && --nLines)
                aLine = aStream.ReadLine();
            if( nLines )
                aRet = aStream.GetFileName();
        }
    }

    return aRet;
}

OUString PPDParser::getPPDPrinterName( const OUString& rFile )
{
    OUString aPath = getPPDFile( rFile );
    OUString aName;

    // read in the file
    PPDDecompressStream aStream( aPath );
    if( aStream.IsOpen() )
    {
        OUString aCurLine;
        while( ! aStream.IsEof() && aStream.IsOpen() )
        {
            OString aByteLine = aStream.ReadLine();
            aCurLine = OStringToOUString(aByteLine, RTL_TEXTENCODING_MS_1252);
            if( aCurLine.startsWithIgnoreAsciiCase( "*include:" ) )
            {
                aCurLine.replaceAt( 0, 9, "" );
                aCurLine = comphelper::string::stripStart(aCurLine, ' ');
                aCurLine = comphelper::string::stripEnd(aCurLine, ' ');
                aCurLine = comphelper::string::stripStart(aCurLine, '\t');
                aCurLine = comphelper::string::stripEnd(aCurLine, '\t');
                aCurLine = comphelper::string::stripEnd(aCurLine, '\r');
                aCurLine = comphelper::string::stripEnd(aCurLine, '\n');
                aCurLine = comphelper::string::stripStart(aCurLine, '"');
                aCurLine = comphelper::string::stripEnd(aCurLine, '"');
                aStream.Close();
                aStream.Open( getPPDFile( aCurLine ) );
                continue;
            }
            if( aCurLine.startsWith( "*ModelName:" ) )
            {
                aName = aCurLine.getToken( 1, '"' );
                break;
            }
            else if( aCurLine.startsWith( "*NickName:" ) )
                aName = aCurLine.getToken( 1, '"' );
        }
    }
    return aName;
}

const PPDParser* PPDParser::getParser( const OUString& rFile )
{
    static ::osl::Mutex aMutex;
    ::osl::Guard< ::osl::Mutex > aGuard( aMutex );

    OUString aFile = rFile;
    if( !rFile.startsWith( "CUPS:" ) )
        aFile = getPPDFile( rFile );
    if( aFile.isEmpty() )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "Could not get printer PPD file \"%s\" !\n", OUStringToOString( rFile, osl_getThreadTextEncoding() ).getStr() );
#endif
        return NULL;
    }

    PPDCache &rPPDCache = thePPDCache::get();
    for( ::std::list< PPDParser* >::const_iterator it = rPPDCache.aAllParsers.begin(); it != rPPDCache.aAllParsers.end(); ++it )
        if( (*it)->m_aFile == aFile )
            return *it;

    PPDParser* pNewParser = NULL;
    if( !aFile.startsWith( "CUPS:" ) )
        pNewParser = new PPDParser( aFile );
    else
    {
        PrinterInfoManager& rMgr = PrinterInfoManager::get();
        if( rMgr.getType() == PrinterInfoManager::CUPS )
        {
#ifdef ENABLE_CUPS
            pNewParser = const_cast<PPDParser*>(static_cast<CUPSManager&>(rMgr).createCUPSParser( aFile ));
#endif
        }
    }
    if( pNewParser )
    {
        // this may actually be the SGENPRT parser,
        // so ensure uniquness here
        rPPDCache.aAllParsers.remove( pNewParser );
        // insert new parser to list
        rPPDCache.aAllParsers.push_front( pNewParser );
    }
    return pNewParser;
}

PPDParser::PPDParser( const OUString& rFile ) :
        m_aFile( rFile ),
        m_bType42Capable( false ),
        m_aFileEncoding( RTL_TEXTENCODING_MS_1252 ),
        m_pDefaultImageableArea( NULL ),
        m_pImageableAreas( NULL ),
        m_pDefaultPaperDimension( NULL ),
        m_pPaperDimensions( NULL ),
        m_pDefaultInputSlot( NULL ),
        m_pInputSlots( NULL ),
        m_pDefaultResolution( NULL ),
        m_pResolutions( NULL ),
        m_pDefaultDuplexType( NULL ),
        m_pDuplexTypes( NULL ),
        m_pFontList( NULL ),
        m_pTranslator( new PPDTranslator() )
{
    // read in the file
    std::list< OString > aLines;
    PPDDecompressStream aStream( m_aFile );
    if( aStream.IsOpen() )
    {
        bool bLanguageEncoding = false;
        while( ! aStream.IsEof() )
        {
            OString aCurLine = aStream.ReadLine();
            if( aCurLine[0] == '*' )
            {
                if (aCurLine.matchIgnoreAsciiCase(OString("*include:")))
                {
                    aCurLine = aCurLine.copy(9);
                    aCurLine = comphelper::string::stripStart(aCurLine, ' ');
                    aCurLine = comphelper::string::stripEnd(aCurLine, ' ');
                    aCurLine = comphelper::string::stripStart(aCurLine, '\t');
                    aCurLine = comphelper::string::stripEnd(aCurLine, '\t');
                    aCurLine = comphelper::string::stripEnd(aCurLine, '\r');
                    aCurLine = comphelper::string::stripEnd(aCurLine, '\n');
                    aCurLine = comphelper::string::stripStart(aCurLine, '"');
                    aCurLine = comphelper::string::stripEnd(aCurLine, '"');
                    aStream.Close();
                    aStream.Open(getPPDFile(OStringToOUString(aCurLine, m_aFileEncoding)));
                    continue;
                }
                else if( ! bLanguageEncoding &&
                         aCurLine.matchIgnoreAsciiCase(OString("*languageencoding")) )
                {
                    bLanguageEncoding = true; // generally only the first one counts
                    OString aLower = aCurLine.toAsciiLowerCase();
                    if( aLower.indexOfL(RTL_CONSTASCII_STRINGPARAM("isolatin1"), 17 ) != -1 ||
                        aLower.indexOfL(RTL_CONSTASCII_STRINGPARAM("windowsansi"), 17 ) != -1 )
                        m_aFileEncoding = RTL_TEXTENCODING_MS_1252;
                    else if( aLower.indexOfL(RTL_CONSTASCII_STRINGPARAM("isolatin2"), 17 ) != -1 )
                        m_aFileEncoding = RTL_TEXTENCODING_ISO_8859_2;
                    else if( aLower.indexOfL(RTL_CONSTASCII_STRINGPARAM("isolatin5"), 17 ) != -1 )
                        m_aFileEncoding = RTL_TEXTENCODING_ISO_8859_5;
                    else if( aLower.indexOfL(RTL_CONSTASCII_STRINGPARAM("jis83-rksj"), 17 ) != -1 )
                        m_aFileEncoding = RTL_TEXTENCODING_SHIFT_JIS;
                    else if( aLower.indexOfL(RTL_CONSTASCII_STRINGPARAM("macstandard"), 17 ) != -1 )
                        m_aFileEncoding = RTL_TEXTENCODING_APPLE_ROMAN;
                    else if( aLower.indexOfL(RTL_CONSTASCII_STRINGPARAM("utf-8"), 17 ) != -1 )
                        m_aFileEncoding = RTL_TEXTENCODING_UTF8;
                }
            }
            aLines.push_back( aCurLine );
        }
    }
    aStream.Close();

    // now get the Values
    parse( aLines );
#if OSL_DEBUG_LEVEL > 2
    fprintf( stderr, "acquired %d Keys from PPD %s:\n", m_aKeys.size(), BSTRING( m_aFile ).getStr() );
    for( PPDParser::hash_type::const_iterator it = m_aKeys.begin(); it != m_aKeys.end(); ++it )
    {
        const PPDKey* pKey = it->second;
        char* pSetupType = "<unknown>";
        switch( pKey->m_eSetupType )
        {
            case PPDKey::ExitServer:        pSetupType = "ExitServer";break;
            case PPDKey::Prolog:            pSetupType = "Prolog";break;
            case PPDKey::DocumentSetup: pSetupType = "DocumentSetup";break;
            case PPDKey::PageSetup:     pSetupType = "PageSetup";break;
            case PPDKey::JCLSetup:          pSetupType = "JCLSetup";break;
            case PPDKey::AnySetup:          pSetupType = "AnySetup";break;
            default: break;
        };
        fprintf( stderr, "\t\"%s\" (%d values) OrderDependency: %d %s\n",
                 BSTRING( pKey->getKey() ).getStr(),
                 pKey->countValues(),
                 pKey->m_nOrderDependency,
                 pSetupType );
        for( int j = 0; j < pKey->countValues(); j++ )
        {
            fprintf( stderr, "\t\t" );
            const PPDValue* pValue = pKey->getValue( j );
            if( pValue == pKey->m_pDefaultValue )
                fprintf( stderr, "(Default:) " );
            char* pVType = "<unknown>";
            switch( pValue->m_eType )
            {
                case eInvocation:       pVType = "invocation";break;
                case eQuoted:           pVType = "quoted";break;
                case eString:           pVType = "string";break;
                case eSymbol:           pVType = "symbol";break;
                case eNo:               pVType = "no";break;
                default: break;
            };
            fprintf( stderr, "option: \"%s\", value: type %s \"%s\"\n",
                     BSTRING( pValue->m_aOption ).getStr(),
                     pVType,
                     BSTRING( pValue->m_aValue ).getStr() );
        }
    }
    fprintf( stderr, "constraints: (%d found)\n", m_aConstraints.size() );
    for( std::list< PPDConstraint >::const_iterator cit = m_aConstraints.begin(); cit != m_aConstraints.end(); ++cit )
    {
        fprintf( stderr, "*\"%s\" \"%s\" *\"%s\" \"%s\"\n",
                 BSTRING( cit->m_pKey1->getKey() ).getStr(),
                 cit->m_pOption1 ? BSTRING( cit->m_pOption1->m_aOption ).getStr() : "<nil>",
                 BSTRING( cit->m_pKey2->getKey() ).getStr(),
                 cit->m_pOption2 ? BSTRING( cit->m_pOption2->m_aOption ).getStr() : "<nil>"
                 );
    }
#endif

    // fill in shortcuts
    const PPDKey* pKey;

    m_pImageableAreas = getKey(  OUString( "ImageableArea" ) );
    if( m_pImageableAreas )
        m_pDefaultImageableArea = m_pImageableAreas->getDefaultValue();
    if (m_pImageableAreas == 0) {
        OSL_TRACE(
            OSL_LOG_PREFIX "Warning: no ImageableArea in %s\n",
            OUStringToOString(m_aFile, RTL_TEXTENCODING_UTF8).getStr());
    }
    if (m_pDefaultImageableArea == 0) {
        OSL_TRACE(
            OSL_LOG_PREFIX "Warning: no DefaultImageableArea in %s\n",
            OUStringToOString(m_aFile, RTL_TEXTENCODING_UTF8).getStr());
    }

    m_pPaperDimensions = getKey( OUString( "PaperDimension" ) );
    if( m_pPaperDimensions )
        m_pDefaultPaperDimension = m_pPaperDimensions->getDefaultValue();
    if (m_pPaperDimensions == 0) {
        OSL_TRACE(
            OSL_LOG_PREFIX "Warning: no PaperDimensions in %s\n",
            OUStringToOString(m_aFile, RTL_TEXTENCODING_UTF8).getStr());
    }
    if (m_pDefaultPaperDimension == 0) {
        OSL_TRACE(
            OSL_LOG_PREFIX "Warning: no DefaultPaperDimensions in %s\n",
            OUStringToOString(m_aFile, RTL_TEXTENCODING_UTF8).getStr());
    }

    m_pResolutions = getKey( OUString( "Resolution" ) );
    if( m_pResolutions )
        m_pDefaultResolution = m_pResolutions->getDefaultValue();
    if (m_pResolutions == 0) {
        OSL_TRACE(
            OSL_LOG_PREFIX "Warning: no Resolution in %s\n",
            OUStringToOString(m_aFile, RTL_TEXTENCODING_UTF8).getStr());
    }
    if (m_pDefaultResolution == 0) {
        OSL_TRACE(
            OSL_LOG_PREFIX "Warning: no DefaultResolution in %s\n",
            OUStringToOString(m_aFile, RTL_TEXTENCODING_UTF8).getStr());
    }

    m_pInputSlots = getKey( OUString( "InputSlot" ) );
    if( m_pInputSlots )
        m_pDefaultInputSlot = m_pInputSlots->getDefaultValue();
    if (m_pInputSlots == 0) {
        OSL_TRACE(
            OSL_LOG_PREFIX "Warning: no InputSlot in %s\n",
            OUStringToOString(m_aFile, RTL_TEXTENCODING_UTF8).getStr());
    }
    if (m_pDefaultInputSlot == 0) {
        OSL_TRACE(
            OSL_LOG_PREFIX "Warning: no DefaultInputSlot in %s\n",
            OUStringToOString(m_aFile, RTL_TEXTENCODING_UTF8).getStr());
    }

    m_pDuplexTypes = getKey( OUString( "Duplex" ) );
    if( m_pDuplexTypes )
        m_pDefaultDuplexType = m_pDuplexTypes->getDefaultValue();

    m_pFontList = getKey( OUString( "Font" ) );
    if (m_pFontList == 0) {
        OSL_TRACE(
            OSL_LOG_PREFIX "Warning: no Font in %s\n",
            OUStringToOString(m_aFile, RTL_TEXTENCODING_UTF8).getStr());
    }

    // fill in direct values
    if( (pKey = getKey( OUString( "ModelName" ) )) )
        m_aPrinterName = pKey->getValue( 0 )->m_aValue;
    if( (pKey = getKey( OUString( "NickName" ) )) )
        m_aNickName = pKey->getValue( 0 )->m_aValue;
    if( (pKey = getKey( OUString( "ColorDevice" ) )) )
        m_bColorDevice = pKey->getValue( 0 )->m_aValue.startsWithIgnoreAsciiCase( "true" );

    if( (pKey = getKey( OUString( "LanguageLevel" ) )) )
        m_nLanguageLevel = pKey->getValue( 0 )->m_aValue.toInt32();
    if( (pKey = getKey( OUString( "TTRasterizer" ) )) )
        m_bType42Capable = pKey->getValue( 0 )->m_aValue.equalsIgnoreAsciiCase( "Type42" );
}

PPDParser::~PPDParser()
{
    for( PPDParser::hash_type::iterator it = m_aKeys.begin(); it != m_aKeys.end(); ++it )
        delete it->second;
    delete m_pTranslator;
}

void PPDParser::insertKey( const OUString& rKey, PPDKey* pKey )
{
    m_aKeys[ rKey ] = pKey;
    m_aOrderedKeys.push_back( pKey );
}

const PPDKey* PPDParser::getKey( int n ) const
{
    return ((unsigned int)n < m_aOrderedKeys.size() && n >= 0) ? m_aOrderedKeys[n] : NULL;
}

const PPDKey* PPDParser::getKey( const OUString& rKey ) const
{
    PPDParser::hash_type::const_iterator it = m_aKeys.find( rKey );
    return it != m_aKeys.end() ? it->second : NULL;
}

bool PPDParser::hasKey( const PPDKey* pKey ) const
{
    return
        pKey ?
        ( m_aKeys.find( pKey->getKey() ) != m_aKeys.end() ? true : false ) :
        false;
}

static sal_uInt8 getNibble( sal_Char cChar )
{
    sal_uInt8 nRet = 0;
    if( cChar >= '0' && cChar <= '9' )
        nRet = sal_uInt8( cChar - '0' );
    else if( cChar >= 'A' && cChar <= 'F' )
        nRet = 10 + sal_uInt8( cChar - 'A' );
    else if( cChar >= 'a' && cChar <= 'f' )
        nRet = 10 + sal_uInt8( cChar - 'a' );
    return nRet;
}

OUString PPDParser::handleTranslation(const OString& i_rString, bool bIsGlobalized)
{
    sal_Int32 nOrigLen = i_rString.getLength();
    OStringBuffer aTrans( nOrigLen );
    const sal_Char* pStr = i_rString.getStr();
    const sal_Char* pEnd = pStr + nOrigLen;
    while( pStr < pEnd )
    {
        if( *pStr == '<' )
        {
            pStr++;
            sal_Char cChar;
            while( *pStr != '>' && pStr < pEnd-1 )
            {
                cChar = getNibble( *pStr++ ) << 4;
                cChar |= getNibble( *pStr++ );
                aTrans.append( cChar );
            }
            pStr++;
        }
        else
            aTrans.append( *pStr++ );
    }
    return OStringToOUString( aTrans.makeStringAndClear(), bIsGlobalized ? RTL_TEXTENCODING_UTF8 : m_aFileEncoding );
}

namespace
{
    bool oddDoubleQuoteCount(OStringBuffer &rBuffer)
    {
        bool bHasOddCount = false;
        for (sal_Int32 i = 0; i < rBuffer.getLength(); ++i)
        {
            if (rBuffer[i] == '"')
                bHasOddCount = !bHasOddCount;
        }
        return bHasOddCount;
    }
}

void PPDParser::parse( ::std::list< OString >& rLines )
{
    std::list< OString >::iterator line = rLines.begin();
    PPDParser::hash_type::const_iterator keyit;
    while( line != rLines.end() )
    {
        OString aCurrentLine( *line );
        ++line;
        if( aCurrentLine[0] != '*' )
            continue;
        if( aCurrentLine[1] == '%' )
            continue;

        OString aKey = GetCommandLineToken( 0, aCurrentLine.getToken(0, ':') );
        sal_Int32 nPos = aKey.indexOf('/');
        if (nPos != -1)
            aKey = aKey.copy(0, nPos);
        aKey = aKey.copy(1); // remove the '*'

        if ((aKey == "CloseUI") ||
            (aKey == "JCLCloseUI") ||
            (aKey == "OpenGroup") ||
            (aKey == "CloseGroup") ||
            (aKey == "End") ||
            (aKey == "JCLEnd") ||
            (aKey == "OpenSubGroup") ||
            (aKey == "CloseSubGroup"))
        {
            continue;
        }

        if ((aKey == "OpenUI") || (aKey == "JCLOpenUI"))
        {
            parseOpenUI( aCurrentLine );
            continue;
        }
        else if (aKey.equalsL(RTL_CONSTASCII_STRINGPARAM("OrderDependency")))
        {
            parseOrderDependency( aCurrentLine );
            continue;
        }
        else if (aKey.equalsL(RTL_CONSTASCII_STRINGPARAM("UIConstraints")) ||
                 aKey.equalsL(RTL_CONSTASCII_STRINGPARAM("NonUIConstraints")))
        {
            continue; // parsed in pass 2
        }
        else if( aKey.equalsL(RTL_CONSTASCII_STRINGPARAM("CustomPageSize")) ) // currently not handled
            continue;

        // default values are parsed in pass 2
        if (aKey.matchL(RTL_CONSTASCII_STRINGPARAM("Default")))
            continue;

        bool bQuery     = false;
        if (aKey[0] == '?')
        {
            aKey = aKey.copy(1);
            bQuery = true;
        }

        OUString aUniKey(OStringToOUString(aKey, RTL_TEXTENCODING_MS_1252));
        // handle CUPS extension for globalized PPDs
        /* FIXME-BCP47: really only ISO 639-1 two character language codes?
         * goodnight.. */
        bool bIsGlobalizedLine = false;
        com::sun::star::lang::Locale aTransLocale;
        if( ( aUniKey.getLength() > 3 && aUniKey[ 2 ] == '.' ) ||
            ( aUniKey.getLength() > 5 && aUniKey[ 2 ] == '_' && aUniKey[ 5 ] == '.' ) )
        {
            if( aUniKey[ 2 ] == '.' )
            {
                aTransLocale.Language = aUniKey.copy( 0, 2 );
                aUniKey = aUniKey.copy( 3 );
            }
            else
            {
                aTransLocale.Language = aUniKey.copy( 0, 2 );
                aTransLocale.Country = aUniKey.copy( 3, 2 );
                aUniKey = aUniKey.copy( 6 );
            }
            bIsGlobalizedLine = true;
        }

        OUString aOption;
        nPos = aCurrentLine.indexOf(':');
        if( nPos != -1 )
        {
            aOption = OStringToOUString( aCurrentLine.copy( 1, nPos-1 ), RTL_TEXTENCODING_MS_1252 );
            aOption = GetCommandLineToken( 1, aOption );
            sal_Int32 nTransPos = aOption.indexOf( '/' );
            if( nTransPos != -1 )
                aOption = aOption.replaceAt( nTransPos, 1, "" );
        }

        PPDValueType eType = eNo;
        OUString aValue;
        OUString aOptionTranslation;
        OUString aValueTranslation;
        if( nPos != -1 )
        {
            // found a colon, there may be an option
            OString aLine = aCurrentLine.copy( 1, nPos-1 );
            aLine = WhitespaceToSpace( aLine );
            sal_Int32 nTransPos = aLine.indexOf('/');
            if (nTransPos != -1)
                aOptionTranslation = handleTranslation( aLine.copy(nTransPos+1), bIsGlobalizedLine );

            // read in more lines if necessary for multiline values
            aLine = aCurrentLine.copy( nPos+1 );
            if (!aLine.isEmpty())
            {
                OStringBuffer aBuffer(aLine);
                while (line != rLines.end() && oddDoubleQuoteCount(aBuffer))
                {
                    // copy the newlines also
                    aBuffer.append('\n');
                    aBuffer.append(*line);
                    ++line;
                }
                aLine = aBuffer.makeStringAndClear();
            }
            aLine = WhitespaceToSpace( aLine );

            // #i100644# handle a missing value (actually a broken PPD)
            if( aLine.isEmpty() )
            {
                if( !aOption.isEmpty() &&
                    !aUniKey.startsWith( "JCL" ) )
                    eType = eInvocation;
                else
                    eType = eQuoted;
            }
            // check for invocation or quoted value
            else if(aLine[0] == '"')
            {
                aLine = aLine.copy(1);
                nTransPos = aLine.indexOf('"');
                if (nTransPos == -1)
                    nTransPos = aLine.getLength();
                aValue = OStringToOUString(aLine.copy(0, nTransPos), RTL_TEXTENCODING_MS_1252);
                // after the second doublequote can follow a / and a translation
                if (nTransPos < aLine.getLength() - 2)
                {
                    aValueTranslation = handleTranslation( aLine.copy( nTransPos+2 ), bIsGlobalizedLine );
                }
                // check for quoted value
                if( !aOption.isEmpty() &&
                    !aUniKey.startsWith( "JCL" ) )
                    eType = eInvocation;
                else
                    eType = eQuoted;
            }
            // check for symbol value
            else if(aLine[0] == '^')
            {
                aLine = aLine.copy(1);
                aValue = OStringToOUString(aLine, RTL_TEXTENCODING_MS_1252);
                eType = eSymbol;
            }
            else
            {
                // must be a string value then
                // strictly this is false because string values
                // can contain any whitespace which is reduced
                // to one space by now
                // who cares ...
                nTransPos = aLine.indexOf('/');
                if (nTransPos == -1)
                    nTransPos = aLine.getLength();
                aValue = OStringToOUString(aLine.copy(0, nTransPos), RTL_TEXTENCODING_MS_1252);
                if (nTransPos+1 < aLine.getLength())
                    aValueTranslation = handleTranslation( aLine.copy( nTransPos+1 ), bIsGlobalizedLine );
                eType = eString;
            }
        }

        // handle globalized PPD entries
        if( bIsGlobalizedLine )
        {
            // handle main key translations of form:
            // *ll_CC.Translation MainKeyword/translated text: ""
            if( aUniKey.equalsAscii( "Translation" ) )
            {
                m_pTranslator->insertKey( aOption, aOptionTranslation, aTransLocale );
            }
            // handle options translations of for:
            // *ll_CC.MainKeyword OptionKeyword/translated text: ""
            else
            {
                m_pTranslator->insertOption( aUniKey, aOption, aOptionTranslation, aTransLocale );
            }
            continue;
        }

        PPDKey* pKey = NULL;
        keyit = m_aKeys.find( aUniKey );
        if( keyit == m_aKeys.end() )
        {
            pKey = new PPDKey( aUniKey );
            insertKey( aUniKey, pKey );
        }
        else
            pKey = keyit->second;

        if( eType == eNo && bQuery )
            continue;

        PPDValue* pValue = pKey->insertValue( aOption );
        if( ! pValue )
            continue;
        pValue->m_eType = eType;
        pValue->m_aValue = aValue;

        if( !aOptionTranslation.isEmpty() )
            m_pTranslator->insertOption( aUniKey, aOption, aOptionTranslation, aTransLocale );
        if( !aValueTranslation.isEmpty() )
            m_pTranslator->insertValue( aUniKey, aOption, aValue, aValueTranslation, aTransLocale );

        // eventually update query and remove from option list
        if( bQuery && pKey->m_bQueryValue == sal_False )
        {
            pKey->m_aQueryValue = *pValue;
            pKey->m_bQueryValue = true;
            pKey->eraseValue( pValue->m_aOption );
        }
    }

    // second pass: fill in defaults
    for( line = rLines.begin(); line != rLines.end(); ++line )
    {
        OString aLine(*line);
        if (aLine.matchL(RTL_CONSTASCII_STRINGPARAM("*Default")))
        {
            OUString aKey(OStringToOUString(aLine.copy(8), RTL_TEXTENCODING_MS_1252));
            sal_Int32 nPos = aKey.indexOf( ':' );
            if( nPos != -1 )
            {
                aKey = aKey.replaceAt( nPos, 1, "" );
                OUString aOption(OStringToOUString(
                    WhitespaceToSpace(aLine.copy(nPos+9)),
                    RTL_TEXTENCODING_MS_1252));
                keyit = m_aKeys.find( aKey );
                if( keyit != m_aKeys.end() )
                {
                    PPDKey* pKey = keyit->second;
                    const PPDValue* pDefValue = pKey->getValue( aOption );
                    if( pKey->m_pDefaultValue == NULL )
                        pKey->m_pDefaultValue = pDefValue;
                }
                else
                {
                    // some PPDs contain defaults for keys that
                    // do not exist otherwise
                    // (example: DefaultResolution)
                    // so invent that key here and have a default value
                    PPDKey* pKey = new PPDKey( aKey );
                    PPDValue* pNewValue = pKey->insertValue( aOption );
                    pNewValue->m_eType = eInvocation; // or what ?
                    insertKey( aKey, pKey );
                }
            }
        }
        else if (aLine.matchL(RTL_CONSTASCII_STRINGPARAM("*UIConstraints")) ||
                 aLine.matchL(RTL_CONSTASCII_STRINGPARAM("*NonUIConstraints")))
        {
            parseConstraint( aLine );
        }
    }
}

void PPDParser::parseOpenUI(const OString& rLine)
{
    OUString aTranslation;
    OString aKey = rLine;

    sal_Int32 nPos = aKey.indexOf(':');
    if( nPos != -1 )
        aKey = aKey.copy(0, nPos);
    nPos = aKey.indexOf('/');
    if( nPos != -1 )
    {
        aTranslation = handleTranslation( aKey.copy( nPos + 1 ), false );
        aKey = aKey.copy(0, nPos);
    }
    aKey = GetCommandLineToken( 1, aKey );
    aKey = aKey.copy(1);

    OUString aUniKey(OStringToOUString(aKey, RTL_TEXTENCODING_MS_1252));
    PPDParser::hash_type::const_iterator keyit = m_aKeys.find( aUniKey );
    PPDKey* pKey;
    if( keyit == m_aKeys.end() )
    {
        pKey = new PPDKey( aUniKey );
        insertKey( aUniKey, pKey );
    }
    else
        pKey = keyit->second;

    pKey->m_bUIOption = true;
    m_pTranslator->insertKey( pKey->getKey(), aTranslation );

    sal_Int32 nIndex = 0;
    OString aValue = WhitespaceToSpace( rLine.getToken( 1, ':', nIndex ) );
    if( aValue.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("boolean")))
        pKey->m_eUIType = PPDKey::Boolean;
    else if (aValue.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("pickmany")))
        pKey->m_eUIType = PPDKey::PickMany;
    else
        pKey->m_eUIType = PPDKey::PickOne;
}

void PPDParser::parseOrderDependency(const OString& rLine)
{
    OString aLine(rLine);
    sal_Int32 nPos = aLine.indexOf(':');
    if( nPos != -1 )
        aLine = aLine.copy( nPos+1 );

    sal_Int32 nOrder = GetCommandLineToken( 0, aLine ).toInt32();
    OString aSetup = GetCommandLineToken( 1, aLine );
    OUString aKey(OStringToOUString(GetCommandLineToken(2, aLine), RTL_TEXTENCODING_MS_1252));
    if( aKey[ 0 ] != '*' )
        return; // invalid order depency
    aKey = aKey.replaceAt( 0, 1, "" );

    PPDKey* pKey;
    PPDParser::hash_type::const_iterator keyit = m_aKeys.find( aKey );
    if( keyit == m_aKeys.end() )
    {
        pKey = new PPDKey( aKey );
        insertKey( aKey, pKey );
    }
    else
        pKey = keyit->second;

    pKey->m_nOrderDependency = nOrder;
    if( aSetup.equalsL(RTL_CONSTASCII_STRINGPARAM("ExitServer")) )
        pKey->m_eSetupType = PPDKey::ExitServer;
    else if( aSetup.equalsL(RTL_CONSTASCII_STRINGPARAM("Prolog")) )
        pKey->m_eSetupType = PPDKey::Prolog;
    else if( aSetup.equalsL(RTL_CONSTASCII_STRINGPARAM("DocumentSetup")) )
        pKey->m_eSetupType = PPDKey::DocumentSetup;
    else if( aSetup.equalsL(RTL_CONSTASCII_STRINGPARAM("PageSetup")) )
        pKey->m_eSetupType = PPDKey::PageSetup;
    else if( aSetup.equalsL(RTL_CONSTASCII_STRINGPARAM("JCLSetup")) )
        pKey->m_eSetupType = PPDKey::JCLSetup;
    else
        pKey->m_eSetupType = PPDKey::AnySetup;
}

void PPDParser::parseConstraint( const OString& rLine )
{
    bool bFailed = false;

    OUString aLine(OStringToOUString(rLine, RTL_TEXTENCODING_MS_1252));
    aLine = aLine.replaceAt(0, rLine.indexOf(':') + 1, "");
    PPDConstraint aConstraint;
    int nTokens = GetCommandLineTokenCount( aLine );
    for( int i = 0; i < nTokens; i++ )
    {
        OUString aToken = GetCommandLineToken( i, aLine );
        if( aToken[ 0 ] == '*' )
        {
            aToken = aToken.replaceAt( 0, 1, "" );
            if( aConstraint.m_pKey1 )
                aConstraint.m_pKey2 = getKey( aToken );
            else
                aConstraint.m_pKey1 = getKey( aToken );
        }
        else
        {
            if( aConstraint.m_pKey2 )
            {
                if( ! ( aConstraint.m_pOption2 = aConstraint.m_pKey2->getValue( aToken ) ) )
                    bFailed = true;
            }
            else if( aConstraint.m_pKey1 )
            {
                if( ! ( aConstraint.m_pOption1 = aConstraint.m_pKey1->getValue( aToken ) ) )
                    bFailed = true;
            }
            else
                // constraint for nonexistent keys; this happens
                // e.g. in HP4PLUS3
                bFailed = true;
        }
    }
    // there must be two keywords
    if( ! aConstraint.m_pKey1 || ! aConstraint.m_pKey2 || bFailed )
    {
#ifdef __DEBUG
        fprintf( stderr, "Warning: constraint \"%s\" is invalid\n", rLine.getStr() );
#endif
    }
    else
        m_aConstraints.push_back( aConstraint );
}

OUString PPDParser::getDefaultPaperDimension() const
{
    if( m_pDefaultPaperDimension )
        return m_pDefaultPaperDimension->m_aOption;

    return OUString();
}

bool PPDParser::getMargins(
                           const OUString& rPaperName,
                           int& rLeft, int& rRight,
                           int& rUpper, int& rLower ) const
{
    if( ! m_pImageableAreas || ! m_pPaperDimensions )
        return false;

    int nPDim=-1, nImArea=-1, i;
    for( i = 0; i < m_pImageableAreas->countValues(); i++ )
        if( rPaperName == m_pImageableAreas->getValue( i )->m_aOption )
            nImArea = i;
    for( i = 0; i < m_pPaperDimensions->countValues(); i++ )
        if( rPaperName == m_pPaperDimensions->getValue( i )->m_aOption )
            nPDim = i;
    if( nPDim == -1 || nImArea == -1 )
        return false;

    double ImLLx, ImLLy, ImURx, ImURy;
    double PDWidth, PDHeight;
    OUString aArea = m_pImageableAreas->getValue( nImArea )->m_aValue;
    ImLLx = StringToDouble( GetCommandLineToken( 0, aArea ) );
    ImLLy = StringToDouble( GetCommandLineToken( 1, aArea ) );
    ImURx = StringToDouble( GetCommandLineToken( 2, aArea ) );
    ImURy = StringToDouble( GetCommandLineToken( 3, aArea ) );
    aArea = m_pPaperDimensions->getValue( nPDim )->m_aValue;
    PDWidth     = StringToDouble( GetCommandLineToken( 0, aArea ) );
    PDHeight    = StringToDouble( GetCommandLineToken( 1, aArea ) );
    rLeft  = (int)(ImLLx + 0.5);
    rLower = (int)(ImLLy + 0.5);
    rUpper = (int)(PDHeight - ImURy + 0.5);
    rRight = (int)(PDWidth - ImURx + 0.5);

    return true;
}

bool PPDParser::getPaperDimension(
                                  const OUString& rPaperName,
                                  int& rWidth, int& rHeight ) const
{
    if( ! m_pPaperDimensions )
        return false;

    int nPDim=-1;
    for( int i = 0; i < m_pPaperDimensions->countValues(); i++ )
        if( rPaperName == m_pPaperDimensions->getValue( i )->m_aOption )
            nPDim = i;
    if( nPDim == -1 )
        return false;

    double PDWidth, PDHeight;
    OUString aArea = m_pPaperDimensions->getValue( nPDim )->m_aValue;
    PDWidth     = StringToDouble( GetCommandLineToken( 0, aArea ) );
    PDHeight    = StringToDouble( GetCommandLineToken( 1, aArea ) );
    rHeight = (int)(PDHeight + 0.5);
    rWidth  = (int)(PDWidth + 0.5);

    return true;
}

OUString PPDParser::matchPaper( int nWidth, int nHeight ) const
{
    if( ! m_pPaperDimensions )
        return OUString();

    int nPDim = -1;
    double PDWidth, PDHeight;
    double fSort = 2e36, fNewSort;

    for( int i = 0; i < m_pPaperDimensions->countValues(); i++ )
    {
        OUString aArea =  m_pPaperDimensions->getValue( i )->m_aValue;
        PDWidth     = StringToDouble( GetCommandLineToken( 0, aArea ) );
        PDHeight    = StringToDouble( GetCommandLineToken( 1, aArea ) );
        PDWidth     /= (double)nWidth;
        PDHeight    /= (double)nHeight;
        if( PDWidth >= 0.9      &&  PDWidth <= 1.1      &&
            PDHeight >= 0.9     &&  PDHeight <= 1.1         )
        {
            fNewSort =
                (1.0-PDWidth)*(1.0-PDWidth) + (1.0-PDHeight)*(1.0-PDHeight);
            if( fNewSort == 0.0 ) // perfect match
                return m_pPaperDimensions->getValue( i )->m_aOption;

            if( fNewSort < fSort )
            {
                fSort = fNewSort;
                nPDim = i;
            }
        }
    }

    static bool bDontSwap = false;
    if( nPDim == -1 && ! bDontSwap )
    {
        // swap portrait/landscape and try again
        bDontSwap = true;
        OUString rRet = matchPaper( nHeight, nWidth );
        bDontSwap = false;
        return rRet;
    }

    return nPDim != -1 ? m_pPaperDimensions->getValue( nPDim )->m_aOption : OUString();
}

OUString PPDParser::getDefaultInputSlot() const
{
    if( m_pDefaultInputSlot )
        return m_pDefaultInputSlot->m_aValue;
    return OUString();
}

void PPDParser::getResolutionFromString(
                                        const OUString& rString,
                                        int& rXRes, int& rYRes ) const
{
    sal_Int32 nDPIPos;

    rXRes = rYRes = 300;

    nDPIPos = rString.indexOf( "dpi" );
    if( nDPIPos != -1 )
    {
        sal_Int32 nPos = 0;
        if( ( nPos = rString.indexOf( 'x' ) ) != -1 )
        {
            rXRes = rString.copy( 0, nPos ).toInt32();
            rYRes = rString.getToken( 1, 'x' ).replaceAt( nDPIPos - nPos - 1, 1, "" ).toInt32();
        }
        else
            rXRes = rYRes = rString.copy( 0, nDPIPos ).toInt32();
    }
}

void PPDParser::getDefaultResolution( int& rXRes, int& rYRes ) const
{
    if( m_pDefaultResolution )
    {
        getResolutionFromString( m_pDefaultResolution->m_aValue, rXRes, rYRes );
        return;
    }

    rXRes = 300;
    rYRes = 300;
}

OUString PPDParser::getFont( int nFont ) const
{
    if( ! m_pFontList )
        return OUString();

    if( nFont >=0 && nFont < m_pFontList->countValues() )
        return m_pFontList->getValue( nFont )->m_aOption;
    return OUString();
}

OUString PPDParser::translateKey( const OUString& i_rKey,
                                       const com::sun::star::lang::Locale& i_rLocale ) const
{
    OUString aResult( m_pTranslator->translateKey( i_rKey, i_rLocale ) );
    if( aResult.isEmpty() )
        aResult = i_rKey;
    return aResult;
}

OUString PPDParser::translateOption( const OUString& i_rKey,
                                          const OUString& i_rOption,
                                          const com::sun::star::lang::Locale& i_rLocale ) const
{
    OUString aResult( m_pTranslator->translateOption( i_rKey, i_rOption, i_rLocale ) );
    if( aResult.isEmpty() )
        aResult = i_rOption;
    return aResult;
}

/*
 *  PPDKey
 */

PPDKey::PPDKey( const OUString& rKey ) :
        m_aKey( rKey ),
        m_pDefaultValue( NULL ),
        m_bQueryValue( false ),
        m_bUIOption( false ),
        m_eUIType( PickOne ),
        m_nOrderDependency( 100 ),
        m_eSetupType( AnySetup )
{
}

// -------------------------------------------------------------------

PPDKey::~PPDKey()
{
}

// -------------------------------------------------------------------

const PPDValue* PPDKey::getValue( int n ) const
{
    return ((unsigned int)n < m_aOrderedValues.size() && n >= 0) ? m_aOrderedValues[n] : NULL;
}

// -------------------------------------------------------------------

const PPDValue* PPDKey::getValue( const OUString& rOption ) const
{
    PPDKey::hash_type::const_iterator it = m_aValues.find( rOption );
    return it != m_aValues.end() ? &it->second : NULL;
}

// -------------------------------------------------------------------

const PPDValue* PPDKey::getValueCaseInsensitive( const OUString& rOption ) const
{
    const PPDValue* pValue = getValue( rOption );
    if( ! pValue )
    {
        for( size_t n = 0; n < m_aOrderedValues.size() && ! pValue; n++ )
            if( m_aOrderedValues[n]->m_aOption.equalsIgnoreAsciiCase( rOption ) )
                pValue = m_aOrderedValues[n];
    }

    return pValue;
}

// -------------------------------------------------------------------

void PPDKey::eraseValue( const OUString& rOption )
{
    PPDKey::hash_type::iterator it = m_aValues.find( rOption );
    if( it == m_aValues.end() )
        return;

    for( PPDKey::value_type::iterator vit = m_aOrderedValues.begin(); vit != m_aOrderedValues.end(); ++vit )
    {
        if( *vit == &(it->second ) )
        {
            m_aOrderedValues.erase( vit );
            break;
        }
    }
    m_aValues.erase( it );
}

// -------------------------------------------------------------------

PPDValue* PPDKey::insertValue( const OUString& rOption )
{
    if( m_aValues.find( rOption ) != m_aValues.end() )
        return NULL;

    PPDValue aValue;
    aValue.m_aOption = rOption;
    m_aValues[ rOption ] = aValue;
    PPDValue* pValue = &m_aValues[rOption];
    m_aOrderedValues.push_back( pValue );
    return pValue;
}

// -------------------------------------------------------------------

/*
 * PPDContext
 */

PPDContext::PPDContext( const PPDParser* pParser ) :
        m_pParser( pParser )
{
}

// -------------------------------------------------------------------

PPDContext& PPDContext::operator=( const PPDContext& rCopy )
{
    m_pParser           = rCopy.m_pParser;
    m_aCurrentValues    = rCopy.m_aCurrentValues;
    return *this;
}

// -------------------------------------------------------------------

PPDContext::~PPDContext()
{
}

// -------------------------------------------------------------------

const PPDKey* PPDContext::getModifiedKey( int n ) const
{
    hash_type::const_iterator it;
    for( it = m_aCurrentValues.begin(); it != m_aCurrentValues.end() && n--; ++it )
        ;
    return it != m_aCurrentValues.end() ? it->first : NULL;
}

// -------------------------------------------------------------------

void PPDContext::setParser( const PPDParser* pParser )
{
    if( pParser != m_pParser )
    {
        m_aCurrentValues.clear();
        m_pParser = pParser;
    }
}

// -------------------------------------------------------------------

const PPDValue* PPDContext::getValue( const PPDKey* pKey ) const
{
    if( ! m_pParser )
        return NULL;

    hash_type::const_iterator it;
    it = m_aCurrentValues.find( pKey );
    if( it != m_aCurrentValues.end() )
        return it->second;

    if( ! m_pParser->hasKey( pKey ) )
        return NULL;

    const PPDValue* pValue = pKey->getDefaultValue();
    if( ! pValue )
        pValue = pKey->getValue( 0 );

    return pValue;
}

// -------------------------------------------------------------------

const PPDValue* PPDContext::setValue( const PPDKey* pKey, const PPDValue* pValue, bool bDontCareForConstraints )
{
    if( ! m_pParser || ! pKey )
        return NULL;

    // pValue can be NULL - it means ignore this option

    if( ! m_pParser->hasKey( pKey ) )
        return NULL;

    // check constraints
    if( pValue )
    {
        if( bDontCareForConstraints )
        {
            m_aCurrentValues[ pKey ] = pValue;
        }
        else if( checkConstraints( pKey, pValue, true ) )
        {
            m_aCurrentValues[ pKey ] = pValue;

            // after setting this value, check all constraints !
            hash_type::iterator it = m_aCurrentValues.begin();
            while(  it != m_aCurrentValues.end() )
            {
                if( it->first != pKey &&
                    ! checkConstraints( it->first, it->second, false ) )
                {
#ifdef __DEBUG
                    fprintf( stderr, "PPDContext::setValue: option %s (%s) is constrained after setting %s to %s\n",
                             it->first->getKey().GetStr(),
                             it->second->m_aOption.GetStr(),
                             pKey->getKey().GetStr(),
                             pValue->m_aOption.GetStr() );
#endif
                    resetValue( it->first, true );
                    it = m_aCurrentValues.begin();
                }
                else
                    ++it;
            }
        }
    }
    else
        m_aCurrentValues[ pKey ] = NULL;

    return pValue;
}

// -------------------------------------------------------------------

bool PPDContext::checkConstraints( const PPDKey* pKey, const PPDValue* pValue )
{
    if( ! m_pParser || ! pKey || ! pValue )
        return false;

    // ensure that this key is already in the list if it exists at all
    if( m_aCurrentValues.find( pKey ) != m_aCurrentValues.end() )
        return checkConstraints( pKey, pValue, false );

    // it is not in the list, insert it temporarily
    bool bRet = false;
    if( m_pParser->hasKey( pKey ) )
    {
        const PPDValue* pDefValue = pKey->getDefaultValue();
        m_aCurrentValues[ pKey ] = pDefValue;
        bRet = checkConstraints( pKey, pValue, false );
        m_aCurrentValues.erase( pKey );
    }

    return bRet;
}

// -------------------------------------------------------------------

bool PPDContext::resetValue( const PPDKey* pKey, bool bDefaultable )
{
    if( ! pKey || ! m_pParser || ! m_pParser->hasKey( pKey ) )
        return false;

    const PPDValue* pResetValue = pKey->getValue( OUString( "None" ) );
    if( ! pResetValue )
        pResetValue = pKey->getValue( OUString( "False" ) );
    if( ! pResetValue && bDefaultable )
        pResetValue = pKey->getDefaultValue();

    bool bRet = pResetValue ? ( setValue( pKey, pResetValue ) == pResetValue ? true : false ) : false;

    return bRet;
}

// -------------------------------------------------------------------

bool PPDContext::checkConstraints( const PPDKey* pKey, const PPDValue* pNewValue, bool bDoReset )
{
    if( ! pNewValue )
        return true;

    // sanity checks
    if( ! m_pParser )
        return false;

    if( pKey->getValue( pNewValue->m_aOption ) != pNewValue )
        return false;

    // None / False and the default can always be set, but be careful !
    // setting them might influence constrained values
    if( pNewValue->m_aOption.equalsAscii( "None" ) || pNewValue->m_aOption.equalsAscii( "False" ) ||
        pNewValue == pKey->getDefaultValue() )
        return true;

    const ::std::list< PPDParser::PPDConstraint >& rConstraints( m_pParser->getConstraints() );
    for( ::std::list< PPDParser::PPDConstraint >::const_iterator it = rConstraints.begin(); it != rConstraints.end(); ++it )
    {
        const PPDKey* pLeft     = it->m_pKey1;
        const PPDKey* pRight    = it->m_pKey2;
        if( ! pLeft || ! pRight || ( pKey != pLeft && pKey != pRight ) )
            continue;

        const PPDKey* pOtherKey = pKey == pLeft ? pRight : pLeft;
        const PPDValue* pOtherKeyOption = pKey == pLeft ? it->m_pOption2 : it->m_pOption1;
        const PPDValue* pKeyOption = pKey == pLeft ? it->m_pOption1 : it->m_pOption2;

        // syntax *Key1 option1 *Key2 option2
        if( pKeyOption && pOtherKeyOption )
        {
            if( pNewValue != pKeyOption )
                continue;
            if( pOtherKeyOption == getValue( pOtherKey ) )
            {
                return false;
            }
        }
        // syntax *Key1 option *Key2  or  *Key1 *Key2 option
        else if( pOtherKeyOption || pKeyOption )
        {
            if( pKeyOption )
            {
                if( ! ( pOtherKeyOption = getValue( pOtherKey ) ) )
                    continue; // this should not happen, PPD broken

                if( pKeyOption == pNewValue &&
                    ! pOtherKeyOption->m_aOption.equalsAscii( "None" ) &&
                    ! pOtherKeyOption->m_aOption.equalsAscii( "False" ) )
                {
                    // check if the other value can be reset and
                    // do so if possible
                    if( bDoReset && resetValue( pOtherKey ) )
                        continue;

                    return false;
                }
            }
            else if( pOtherKeyOption )
            {
                if( getValue( pOtherKey ) == pOtherKeyOption &&
                    ! pNewValue->m_aOption.equalsAscii( "None" ) &&
                    ! pNewValue->m_aOption.equalsAscii( "False" ) )
                    return false;
            }
            else
            {
                // this should not happen, PPD is broken
            }
        }
        // syntax *Key1 *Key2
        else
        {
            const PPDValue* pOtherValue = getValue( pOtherKey );
            if( ! pOtherValue->m_aOption.equalsAscii( "None" )  &&
                ! pOtherValue->m_aOption.equalsAscii( "False" )     &&
                ! pNewValue->m_aOption.equalsAscii( "None" )        &&
                ! pNewValue->m_aOption.equalsAscii( "False" ) )
                return false;
        }
    }
    return true;
}

// -------------------------------------------------------------------

char* PPDContext::getStreamableBuffer( sal_uLong& rBytes ) const
{
    rBytes = 0;
    if( ! m_aCurrentValues.size() )
        return NULL;
    hash_type::const_iterator it;
    for( it = m_aCurrentValues.begin(); it != m_aCurrentValues.end(); ++it )
    {
        OString aCopy(OUStringToOString(it->first->getKey(), RTL_TEXTENCODING_MS_1252));
        rBytes += aCopy.getLength();
        rBytes += 1; // for ':'
        if( it->second )
        {
            aCopy = OUStringToOString(it->second->m_aOption, RTL_TEXTENCODING_MS_1252);
            rBytes += aCopy.getLength();
        }
        else
            rBytes += 4;
        rBytes += 1; // for '\0'
    }
    rBytes += 1;
    char* pBuffer = new char[ rBytes ];
    memset( pBuffer, 0, rBytes );
    char* pRun = pBuffer;
    for( it = m_aCurrentValues.begin(); it != m_aCurrentValues.end(); ++it )
    {
        OString aCopy(OUStringToOString(it->first->getKey(), RTL_TEXTENCODING_MS_1252));
        int nBytes = aCopy.getLength();
        memcpy( pRun, aCopy.getStr(), nBytes );
        pRun += nBytes;
        *pRun++ = ':';
        if( it->second )
            aCopy = OUStringToOString(it->second->m_aOption, RTL_TEXTENCODING_MS_1252);
        else
            aCopy = "*nil";
        nBytes = aCopy.getLength();
        memcpy( pRun, aCopy.getStr(), nBytes );
        pRun += nBytes;

        *pRun++ = 0;
    }
    return pBuffer;
}

// -------------------------------------------------------------------

void PPDContext::rebuildFromStreamBuffer( char* pBuffer, sal_uLong nBytes )
{
    if( ! m_pParser )
        return;

    m_aCurrentValues.clear();

    char* pRun = pBuffer;
    while( nBytes && *pRun )
    {
        OString aLine( pRun );
        sal_Int32 nPos = aLine.indexOf(':');
        if( nPos != -1 )
        {
            const PPDKey* pKey = m_pParser->getKey( OStringToOUString( aLine.copy( 0, nPos ), RTL_TEXTENCODING_MS_1252 ) );
            if( pKey )
            {
                const PPDValue* pValue = NULL;
                OUString aOption(OStringToOUString(aLine.copy(nPos+1), RTL_TEXTENCODING_MS_1252));
                if (aOption != "*nil")
                    pValue = pKey->getValue( aOption );
                m_aCurrentValues[ pKey ] = pValue;
#ifdef __DEBUG
                fprintf( stderr, "PPDContext::rebuildFromStreamBuffer: read PPDKeyValue { %s, %s }\n", pKV->m_pKey->getKey().GetStr(), pKV->m_pCurrentValue ? pKV->m_pCurrentValue->m_aOption.GetStr() : "<nil>" );
#endif
            }
        }
        nBytes -= aLine.getLength()+1;
        pRun += aLine.getLength()+1;
    }
}

// -------------------------------------------------------------------

int PPDContext::getRenderResolution() const
{
    // initialize to reasonable default, if parser is not set
    int nDPI = 300;
    if( m_pParser )
    {
        int nDPIx = 300, nDPIy = 300;
        const PPDKey* pKey = m_pParser->getKey( OUString( "Resolution" ) );
        if( pKey )
        {
            const PPDValue* pValue = getValue( pKey );
            if( pValue )
                m_pParser->getResolutionFromString( pValue->m_aOption, nDPIx, nDPIy );
            else
                m_pParser->getDefaultResolution( nDPIx, nDPIy );
        }
        else
            m_pParser->getDefaultResolution( nDPIx, nDPIy );

        nDPI = (nDPIx > nDPIy) ? nDPIx : nDPIy;
    }
    return  nDPI;
}

// -------------------------------------------------------------------

void PPDContext::getPageSize( OUString& rPaper, int& rWidth, int& rHeight ) const
{
    // initialize to reasonable default, if parser is not set
    rPaper  = OUString( "A4" );
    rWidth  = 595;
    rHeight = 842;
    if( m_pParser )
    {
        const PPDKey* pKey = m_pParser->getKey( OUString( "PageSize" ) );
        if( pKey )
        {
            const PPDValue* pValue = getValue( pKey );
            if( pValue )
            {
                rPaper = pValue->m_aOption;
                m_pParser->getPaperDimension( rPaper, rWidth, rHeight );
            }
            else
            {
                rPaper = m_pParser->getDefaultPaperDimension();
                m_pParser->getDefaultPaperDimension( rWidth, rHeight );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
