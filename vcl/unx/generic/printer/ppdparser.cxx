/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <stdlib.h>
#include <stdio.h>

#include <boost/unordered_map.hpp>

#include "vcl/ppdparser.hxx"
#include "vcl/strhelper.hxx"
#include "vcl/helper.hxx"
#include "vcl/svapp.hxx"
#include "cupsmgr.hxx"
#include "tools/debug.hxx"
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

        typedef boost::unordered_map< com::sun::star::lang::Locale, rtl::OUString, LocaleHash, LocaleEqual > translation_map;
        typedef boost::unordered_map< rtl::OUString, translation_map, rtl::OUStringHash > key_translation_map;

        key_translation_map     m_aTranslations;
        public:
        PPDTranslator() {}
        ~PPDTranslator() {}


        void insertValue(
            const rtl::OUString& i_rKey,
            const rtl::OUString& i_rOption,
            const rtl::OUString& i_rValue,
            const rtl::OUString& i_rTranslation,
            const com::sun::star::lang::Locale& i_rLocale = com::sun::star::lang::Locale()
            );

        void insertOption( const rtl::OUString& i_rKey,
                           const rtl::OUString& i_rOption,
                           const rtl::OUString& i_rTranslation,
                           const com::sun::star::lang::Locale& i_rLocale = com::sun::star::lang::Locale() )
        {
            insertValue( i_rKey, i_rOption, rtl::OUString(), i_rTranslation, i_rLocale );
        }

        void insertKey( const rtl::OUString& i_rKey,
                        const rtl::OUString& i_rTranslation,
                        const com::sun::star::lang::Locale& i_rLocale = com::sun::star::lang::Locale() )
        {
            insertValue( i_rKey, rtl::OUString(), rtl::OUString(), i_rTranslation, i_rLocale );
        }

        rtl::OUString translateValue(
            const rtl::OUString& i_rKey,
            const rtl::OUString& i_rOption,
            const rtl::OUString& i_rValue,
            const com::sun::star::lang::Locale& i_rLocale = com::sun::star::lang::Locale()
            ) const;

        rtl::OUString translateOption( const rtl::OUString& i_rKey,
                                       const rtl::OUString& i_rOption,
                                       const com::sun::star::lang::Locale& i_rLocale = com::sun::star::lang::Locale() ) const
        {
            return translateValue( i_rKey, i_rOption, rtl::OUString(), i_rLocale );
        }

        rtl::OUString translateKey( const rtl::OUString& i_rKey,
                                    const com::sun::star::lang::Locale& i_rLocale = com::sun::star::lang::Locale() ) const
        {
            return translateValue( i_rKey, rtl::OUString(), rtl::OUString(), i_rLocale );
        }
    };

    static com::sun::star::lang::Locale normalizeInputLocale(
        const com::sun::star::lang::Locale& i_rLocale,
        bool bInsertDefault = false
        )
    {
        com::sun::star::lang::Locale aLoc( i_rLocale );
        if( bInsertDefault && aLoc.Language.getLength() == 0 )
        {
            // empty locale requested, fill in application UI locale
            aLoc = Application::GetSettings().GetUILocale();

            #if OSL_DEBUG_LEVEL > 1
            static const char* pEnvLocale = getenv( "SAL_PPDPARSER_LOCALE" );
            if( pEnvLocale && *pEnvLocale )
            {
                rtl::OString aStr( pEnvLocale );
                sal_Int32 nLen = aStr.getLength();
                aLoc.Language = rtl::OStringToOUString( aStr.copy( 0, nLen > 2 ? 2 : nLen ), RTL_TEXTENCODING_MS_1252 );
                if( nLen >=5 && aStr.getStr()[2] == '_' )
                    aLoc.Country = rtl::OStringToOUString( aStr.copy( 3, 2 ), RTL_TEXTENCODING_MS_1252 );
                else
                    aLoc.Country = rtl::OUString();
                aLoc.Variant = rtl::OUString();
            }
            #endif
        }
        aLoc.Language = aLoc.Language.toAsciiLowerCase();
        aLoc.Country  = aLoc.Country.toAsciiUpperCase();
        aLoc.Variant  = aLoc.Variant.toAsciiUpperCase();

        return aLoc;
    }

    void PPDTranslator::insertValue(
        const rtl::OUString& i_rKey,
        const rtl::OUString& i_rOption,
        const rtl::OUString& i_rValue,
        const rtl::OUString& i_rTranslation,
        const com::sun::star::lang::Locale& i_rLocale
        )
    {
        rtl::OUStringBuffer aKey( i_rKey.getLength() + i_rOption.getLength() + i_rValue.getLength() + 2 );
        aKey.append( i_rKey );
        if( i_rOption.getLength() || i_rValue.getLength() )
        {
            aKey.append( sal_Unicode( ':' ) );
            aKey.append( i_rOption );
        }
        if( i_rValue.getLength() )
        {
            aKey.append( sal_Unicode( ':' ) );
            aKey.append( i_rValue );
        }
        if( aKey.getLength() && i_rTranslation.getLength() )
        {
            rtl::OUString aK( aKey.makeStringAndClear() );
            com::sun::star::lang::Locale aLoc;
            aLoc.Language = i_rLocale.Language.toAsciiLowerCase();
            aLoc.Country  = i_rLocale.Country.toAsciiUpperCase();
            aLoc.Variant  = i_rLocale.Variant.toAsciiUpperCase();
            m_aTranslations[ aK ][ aLoc ] = i_rTranslation;
        }
    }

    rtl::OUString PPDTranslator::translateValue(
        const rtl::OUString& i_rKey,
        const rtl::OUString& i_rOption,
        const rtl::OUString& i_rValue,
        const com::sun::star::lang::Locale& i_rLocale
        ) const
    {
        rtl::OUString aResult;

        rtl::OUStringBuffer aKey( i_rKey.getLength() + i_rOption.getLength() + i_rValue.getLength() + 2 );
        aKey.append( i_rKey );
        if( i_rOption.getLength() || i_rValue.getLength() )
        {
            aKey.append( sal_Unicode( ':' ) );
            aKey.append( i_rOption );
        }
        if( i_rValue.getLength() )
        {
            aKey.append( sal_Unicode( ':' ) );
            aKey.append( i_rValue );
        }
        if( aKey.getLength() )
        {
            rtl::OUString aK( aKey.makeStringAndClear() );
            key_translation_map::const_iterator it = m_aTranslations.find( aK );
            if( it != m_aTranslations.end() )
            {
                const translation_map& rMap( it->second );

                com::sun::star::lang::Locale aLoc( normalizeInputLocale( i_rLocale, true ) );
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
                    case 0: aLoc.Variant  = rtl::OUString();break;
                    case 1: aLoc.Country  = rtl::OUString();break;
                    case 2: aLoc.Language = rtl::OUString();break;
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
        boost::unordered_map< rtl::OUString, rtl::OUString, rtl::OUStringHash >* pAllPPDFiles;
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

using ::rtl::OUString;
using ::rtl::OStringBuffer;
using ::rtl::OUStringHash;


#undef DBG_ASSERT
#if defined DBG_UTIL || (OSL_DEBUG_LEVEL > 1)
#define BSTRING(x) ByteString( x, osl_getThreadTextEncoding() )
#define DBG_ASSERT( x, y ) { if( ! (x) ) fprintf( stderr, (y) ); }
#else
#define DBG_ASSERT( x, y )
#endif

namespace
{
    struct thePPDCache : public rtl::Static<PPDCache, thePPDCache> {};
}

class PPDDecompressStream
{
    SvFileStream*       mpFileStream;
    SvMemoryStream*     mpMemStream;
    rtl::OUString       maFileName;

    // forbid copying
    PPDDecompressStream( const PPDDecompressStream& );
    PPDDecompressStream& operator=(const PPDDecompressStream& );

    public:
    PPDDecompressStream( const rtl::OUString& rFile );
    ~PPDDecompressStream();

    bool IsOpen() const;
    bool IsEof() const;
    void ReadLine( ByteString& o_rLine);
    void Open( const rtl::OUString& i_rFile );
    void Close();
    const rtl::OUString& GetFileName() const { return maFileName; }
};

PPDDecompressStream::PPDDecompressStream( const rtl::OUString& i_rFile ) :
    mpFileStream( NULL ),
    mpMemStream( NULL )
{
    Open( i_rFile );
}

PPDDecompressStream::~PPDDecompressStream()
{
    Close();
}

void PPDDecompressStream::Open( const rtl::OUString& i_rFile )
{
    Close();

    mpFileStream = new SvFileStream( i_rFile, STREAM_READ );
    maFileName = mpFileStream->GetFileName();

    if( ! mpFileStream->IsOpen() )
    {
        Close();
        return;
    }

    ByteString aLine;
    mpFileStream->ReadLine( aLine );
    mpFileStream->Seek( 0 );

    // check for compress'ed or gzip'ed file
    sal_uLong nCompressMethod = 0;
    if( aLine.Len() > 1 && static_cast<unsigned char>(aLine.GetChar( 0 )) == 0x1f )
    {
        if( static_cast<unsigned char>(aLine.GetChar( 1 )) == 0x8b ) // check for gzip
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
            // compression successfull, can get rid of file stream
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

void PPDDecompressStream::ReadLine( ByteString& o_rLine )
{
    if( mpMemStream )
        mpMemStream->ReadLine( o_rLine );
    else if( mpFileStream )
        mpFileStream->ReadLine( o_rLine );
}

static osl::FileBase::RC resolveLink( const rtl::OUString& i_rURL, rtl::OUString& o_rResolvedURL, rtl::OUString& o_rBaseName, osl::FileStatus::Type& o_rType, int nLinkLevel = 10 )
{
    osl::DirectoryItem aLinkItem;
    osl::FileBase::RC aRet = osl::FileBase::E_None;

    if( ( aRet = osl::DirectoryItem::get( i_rURL, aLinkItem ) ) == osl::FileBase::E_None )
    {
        osl::FileStatus aStatus( FileStatusMask_FileName | FileStatusMask_Type | FileStatusMask_LinkTargetURL );
        if( ( aRet = aLinkItem.getFileStatus( aStatus ) ) == osl::FileBase::E_None )
        {
            if( aStatus.getFileType() == osl::FileStatus::Link )
            {
                if( nLinkLevel > 0 )
                    aRet = resolveLink( aStatus.getLinkTargetURL(), o_rResolvedURL, o_rBaseName, o_rType, nLinkLevel-1 );
                else
                    aRet = osl::FileBase::E_MULTIHOP;
            }
            else
            {
                o_rResolvedURL = i_rURL;
                o_rBaseName = aStatus.getFileName();
                o_rType = aStatus.getFileType();
            }
        }
    }
    return aRet;
}

void PPDParser::scanPPDDir( const String& rDir )
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
            osl::FileStatus aStatus( FileStatusMask_FileName );
            if( aItem.getFileStatus( aStatus ) == osl::FileBase::E_None )
            {
                rtl::OUStringBuffer aURLBuf( rDir.Len() + 64 );
                aURLBuf.append( rDir );
                aURLBuf.append( sal_Unicode( '/' ) );
                aURLBuf.append( aStatus.getFileName() );

                rtl::OUString aFileURL, aFileName;
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
    if( rPPDCache.pAllPPDFiles->find( OUString( RTL_CONSTASCII_USTRINGPARAM( "SGENPRT" ) ) ) == rPPDCache.pAllPPDFiles->end() )
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
            fprintf( stderr, "SGENPRT %s\n", rPPDCache.pAllPPDFiles->find( OUString( RTL_CONSTASCII_USTRINGPARAM( "SGENPRT" ) ) ) == rPPDCache.pAllPPDFiles->end() ? "not found" : "found" );
#endif
        }
    }
}

void PPDParser::getKnownPPDDrivers( std::list< rtl::OUString >& o_rDrivers, bool bRefresh )
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

String PPDParser::getPPDFile( const String& rFile )
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
            rtl::OUString aBase( rFile );
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

    String aRet;
    if( aStream.IsOpen() )
    {
        ByteString aLine;
        aStream.ReadLine( aLine );
        if( aLine.Search( "*PPD-Adobe" ) == 0 )
            aRet = aStream.GetFileName();
        else
        {
            // our *Include hack does usually not begin
            // with *PPD-Adobe, so try some lines for *Include
            int nLines = 10;
            while( aLine.Search( "*Include" ) != 0 && --nLines )
                aStream.ReadLine( aLine );
            if( nLines )
                aRet = aStream.GetFileName();
        }
    }

    return aRet;
}

String PPDParser::getPPDPrinterName( const String& rFile )
{
    String aPath = getPPDFile( rFile );
    String aName;

    // read in the file
    PPDDecompressStream aStream( aPath );
    if( aStream.IsOpen() )
    {
        String aCurLine;
        while( ! aStream.IsEof() && aStream.IsOpen() )
        {
            ByteString aByteLine;
            aStream.ReadLine( aByteLine );
            aCurLine = String( aByteLine, RTL_TEXTENCODING_MS_1252 );
            if( aCurLine.CompareIgnoreCaseToAscii( "*include:", 9 ) == COMPARE_EQUAL )
            {
                aCurLine.Erase( 0, 9 );
                aCurLine.EraseLeadingChars( ' ' );
                aCurLine.EraseTrailingChars( ' ' );
                aCurLine.EraseLeadingChars( '\t' );
                aCurLine.EraseTrailingChars( '\t' );
                aCurLine.EraseTrailingChars( '\r' );
                aCurLine.EraseTrailingChars( '\n' );
                aCurLine.EraseLeadingChars( '"' );
                aCurLine.EraseTrailingChars( '"' );
                aStream.Close();
                aStream.Open( getPPDFile( aCurLine ) );
                continue;
            }
            if( aCurLine.CompareToAscii( "*ModelName:", 11 ) == COMPARE_EQUAL )
            {
                aName = aCurLine.GetToken( 1, '"' );
                break;
            }
            else if( aCurLine.CompareToAscii( "*NickName:", 10 ) == COMPARE_EQUAL )
                aName = aCurLine.GetToken( 1, '"' );
        }
    }
    return aName;
}

const PPDParser* PPDParser::getParser( const String& rFile )
{
    static ::osl::Mutex aMutex;
    ::osl::Guard< ::osl::Mutex > aGuard( aMutex );

    String aFile = rFile;
    if( rFile.CompareToAscii( "CUPS:", 5 ) != COMPARE_EQUAL )
        aFile = getPPDFile( rFile );
    if( ! aFile.Len() )
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
    if( aFile.CompareToAscii( "CUPS:", 5 ) != COMPARE_EQUAL )
        pNewParser = new PPDParser( aFile );
    else
    {
        PrinterInfoManager& rMgr = PrinterInfoManager::get();
        if( rMgr.getType() == PrinterInfoManager::CUPS )
        {
            pNewParser = const_cast<PPDParser*>(static_cast<CUPSManager&>(rMgr).createCUPSParser( aFile ));
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

PPDParser::PPDParser( const String& rFile ) :
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
    std::list< ByteString > aLines;
    PPDDecompressStream aStream( m_aFile );
    bool bLanguageEncoding = false;
    if( aStream.IsOpen() )
    {
        ByteString aCurLine;
        while( ! aStream.IsEof() )
        {
            aStream.ReadLine( aCurLine );
            if( aCurLine.GetChar( 0 ) == '*' )
            {
                if( aCurLine.CompareIgnoreCaseToAscii( "*include:", 9 ) == COMPARE_EQUAL )
                {
                    aCurLine.Erase( 0, 9 );
                    aCurLine.EraseLeadingChars( ' ' );
                    aCurLine.EraseTrailingChars( ' ' );
                    aCurLine.EraseLeadingChars( '\t' );
                    aCurLine.EraseTrailingChars( '\t' );
                    aCurLine.EraseTrailingChars( '\r' );
                    aCurLine.EraseTrailingChars( '\n' );
                    aCurLine.EraseLeadingChars( '"' );
                    aCurLine.EraseTrailingChars( '"' );
                    aStream.Close();
                    aStream.Open( getPPDFile( String( aCurLine, m_aFileEncoding ) ) );
                    continue;
                }
                else if( ! bLanguageEncoding &&
                         aCurLine.CompareIgnoreCaseToAscii( "*languageencoding", 17 ) == COMPARE_EQUAL )
                {
                    bLanguageEncoding = true; // generally only the first one counts
                    ByteString aLower = aCurLine;
                    aLower.ToLowerAscii();
                    if( aLower.Search( "isolatin1", 17 ) != STRING_NOTFOUND ||
                        aLower.Search( "windowsansi", 17 ) != STRING_NOTFOUND )
                        m_aFileEncoding = RTL_TEXTENCODING_MS_1252;
                    else if( aLower.Search( "isolatin2", 17 ) != STRING_NOTFOUND )
                        m_aFileEncoding = RTL_TEXTENCODING_ISO_8859_2;
                    else if( aLower.Search( "isolatin5", 17 ) != STRING_NOTFOUND )
                        m_aFileEncoding = RTL_TEXTENCODING_ISO_8859_5;
                    else if( aLower.Search( "jis83-rksj", 17 ) != STRING_NOTFOUND )
                        m_aFileEncoding = RTL_TEXTENCODING_SHIFT_JIS;
                    else if( aLower.Search( "macstandard", 17 ) != STRING_NOTFOUND )
                        m_aFileEncoding = RTL_TEXTENCODING_APPLE_ROMAN;
                    else if( aLower.Search( "utf-8", 17 ) != STRING_NOTFOUND )
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
    fprintf( stderr, "acquired %d Keys from PPD %s:\n", m_aKeys.size(), BSTRING( m_aFile ).GetBuffer() );
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
                 BSTRING( pKey->getKey() ).GetBuffer(),
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
                     BSTRING( pValue->m_aOption ).GetBuffer(),
                     pVType,
                     BSTRING( pValue->m_aValue ).GetBuffer() );
        }
    }
    fprintf( stderr, "constraints: (%d found)\n", m_aConstraints.size() );
    for( std::list< PPDConstraint >::const_iterator cit = m_aConstraints.begin(); cit != m_aConstraints.end(); ++cit )
    {
        fprintf( stderr, "*\"%s\" \"%s\" *\"%s\" \"%s\"\n",
                 BSTRING( cit->m_pKey1->getKey() ).GetBuffer(),
                 cit->m_pOption1 ? BSTRING( cit->m_pOption1->m_aOption ).GetBuffer() : "<nil>",
                 BSTRING( cit->m_pKey2->getKey() ).GetBuffer(),
                 cit->m_pOption2 ? BSTRING( cit->m_pOption2->m_aOption ).GetBuffer() : "<nil>"
                 );
    }
#endif

    // fill in shortcuts
    const PPDKey* pKey;

    m_pImageableAreas = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "ImageableArea" ) ) );
    if( m_pImageableAreas )
        m_pDefaultImageableArea = m_pImageableAreas->getDefaultValue();
    DBG_ASSERT( m_pImageableAreas, "Warning: no ImageableArea in PPD\n" );
    DBG_ASSERT( m_pDefaultImageableArea, "Warning: no DefaultImageableArea in PPD\n" );

    m_pPaperDimensions = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PaperDimension" ) ) );
    if( m_pPaperDimensions )
        m_pDefaultPaperDimension = m_pPaperDimensions->getDefaultValue();
    DBG_ASSERT( m_pPaperDimensions, "Warning: no PaperDimension in PPD\n" );
    DBG_ASSERT( m_pDefaultPaperDimension, "Warning: no DefaultPaperDimension in PPD\n" );

    m_pResolutions = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Resolution" ) ) );
    if( m_pResolutions )
        m_pDefaultResolution = m_pResolutions->getDefaultValue();
    DBG_ASSERT( m_pResolutions, "Warning: no Resolution in PPD\n" );
    DBG_ASSERT( m_pDefaultResolution, "Warning: no DefaultResolution in PPD\n" );

    m_pInputSlots = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "InputSlot" ) ) );
    if( m_pInputSlots )
        m_pDefaultInputSlot = m_pInputSlots->getDefaultValue();
    DBG_ASSERT( m_pPaperDimensions, "Warning: no InputSlot in PPD\n" );
    DBG_ASSERT( m_pDefaultPaperDimension, "Warning: no DefaultInputSlot in PPD\n" );

    m_pDuplexTypes = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Duplex" ) ) );
    if( m_pDuplexTypes )
        m_pDefaultDuplexType = m_pDuplexTypes->getDefaultValue();

    m_pFontList = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Font" ) ) );
    DBG_ASSERT( m_pFontList, "Warning: no Font in PPD\n" );

    // fill in direct values
    if( (pKey = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "ModelName" ) ) )) )
        m_aPrinterName = pKey->getValue( 0 )->m_aValue;
    if( (pKey = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "NickName" ) ) )) )
        m_aNickName = pKey->getValue( 0 )->m_aValue;
    if( (pKey = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "ColorDevice" ) ) )) )
        m_bColorDevice = pKey->getValue( 0 )->m_aValue.CompareIgnoreCaseToAscii( "true", 4 ) == COMPARE_EQUAL ? true : false;

    if( (pKey = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "LanguageLevel" ) ) )) )
        m_nLanguageLevel = pKey->getValue( 0 )->m_aValue.ToInt32();
    if( (pKey = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "TTRasterizer" ) ) )) )
        m_bType42Capable = pKey->getValue( 0 )->m_aValue.EqualsIgnoreCaseAscii( "Type42" ) ? true : false;
}

PPDParser::~PPDParser()
{
    for( PPDParser::hash_type::iterator it = m_aKeys.begin(); it != m_aKeys.end(); ++it )
        delete it->second;
    delete m_pTranslator;
}

void PPDParser::insertKey( const String& rKey, PPDKey* pKey )
{
    m_aKeys[ rKey ] = pKey;
    m_aOrderedKeys.push_back( pKey );
}

const PPDKey* PPDParser::getKey( int n ) const
{
    return ((unsigned int)n < m_aOrderedKeys.size() && n >= 0) ? m_aOrderedKeys[n] : NULL;
}

const PPDKey* PPDParser::getKey( const String& rKey ) const
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

String PPDParser::handleTranslation( const ByteString& i_rString, bool bIsGlobalized )
{
    int nOrigLen = i_rString.Len();
    OStringBuffer aTrans( nOrigLen );
    const sal_Char* pStr = i_rString.GetBuffer();
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

void PPDParser::parse( ::std::list< ByteString >& rLines )
{
    std::list< ByteString >::iterator line = rLines.begin();
    PPDParser::hash_type::const_iterator keyit;
    while( line != rLines.end() )
    {
        ByteString aCurrentLine( *line );
        ++line;
        if( aCurrentLine.GetChar(0) != '*' )
            continue;
        if( aCurrentLine.GetChar(1) == '%' )
            continue;

        ByteString aKey = GetCommandLineToken( 0, aCurrentLine.GetToken( 0, ':' ) );
        int nPos = aKey.Search( '/' );
        if( nPos != STRING_NOTFOUND )
            aKey.Erase( nPos );
        aKey.Erase( 0, 1 ); // remove the '*'

        if( aKey.Equals( "CloseUI" ) || aKey.Equals( "OpenGroup" ) || aKey.Equals( "CloseGroup" ) || aKey.Equals( "End" ) || aKey.Equals( "OpenSubGroup" ) || aKey.Equals( "CloseSubGroup" ) )
            continue;

        if( aKey.Equals( "OpenUI" ) )
        {
            parseOpenUI( aCurrentLine );
            continue;
        }
        else if( aKey.Equals( "OrderDependency" ) )
        {
            parseOrderDependency( aCurrentLine );
            continue;
        }
        else if( aKey.Equals( "UIConstraints" ) || aKey.Equals( "NonUIConstraints" ) )
            continue; // parsed in pass 2
        else if( aKey.Equals( "CustomPageSize" ) ) // currently not handled
            continue;

        // default values are parsed in pass 2
        if( aKey.CompareTo( "Default", 7 ) == COMPARE_EQUAL )
            continue;

        bool bQuery     = false;
        if( aKey.GetChar( 0 ) == '?' )
        {
            aKey.Erase( 0, 1 );
            bQuery = true;
        }

        String aUniKey( aKey, RTL_TEXTENCODING_MS_1252 );
        // handle CUPS extension for globalized PPDs
        bool bIsGlobalizedLine = false;
        com::sun::star::lang::Locale aTransLocale;
        if( ( aUniKey.Len() > 3 && aUniKey.GetChar( 2 ) == '.' ) ||
            ( aUniKey.Len() > 5 && aUniKey.GetChar( 2 ) == '_' && aUniKey.GetChar( 5 ) == '.' ) )
        {
            if( aUniKey.GetChar( 2 ) == '.' )
            {
                aTransLocale.Language = aUniKey.Copy( 0, 2 );
                aUniKey = aUniKey.Copy( 3 );
            }
            else
            {
                aTransLocale.Language = aUniKey.Copy( 0, 2 );
                aTransLocale.Country = aUniKey.Copy( 3, 2 );
                aUniKey = aUniKey.Copy( 6 );
            }
            bIsGlobalizedLine = true;
        }

        String aOption;
        nPos = aCurrentLine.Search( ':' );
        if( nPos != STRING_NOTFOUND )
        {
            aOption = String( aCurrentLine.Copy( 1, nPos-1 ), RTL_TEXTENCODING_MS_1252 );
            aOption = GetCommandLineToken( 1, aOption );
            int nTransPos = aOption.Search( '/' );
            if( nTransPos != STRING_NOTFOUND )
                aOption.Erase( nTransPos );
        }

        PPDValueType eType = eNo;
        String aValue;
        rtl::OUString aOptionTranslation;
        rtl::OUString aValueTranslation;
        if( nPos != STRING_NOTFOUND )
        {
            // found a colon, there may be an option
            ByteString aLine = aCurrentLine.Copy( 1, nPos-1 );
            aLine = WhitespaceToSpace( aLine );
            int nTransPos = aLine.Search( '/' );
            if( nTransPos != STRING_NOTFOUND )
                aOptionTranslation = handleTranslation( aLine.Copy( nTransPos+1 ), bIsGlobalizedLine );

            // read in more lines if necessary for multiline values
            aLine = aCurrentLine.Copy( nPos+1 );
            if( aLine.Len() )
            {
                while( ! ( aLine.GetTokenCount( '"' ) & 1 ) &&
                       line != rLines.end() )
                    // while there is an even number of tokens; that means
                    // an odd number of doubleqoutes
                {
                    // copy the newlines also
                    aLine += '\n';
                    aLine += *line;
                    ++line;
                }
            }
            aLine = WhitespaceToSpace( aLine );

            // #i100644# handle a missing value (actually a broken PPD)
            if( ! aLine.Len() )
            {
                if( aOption.Len() &&
                    aUniKey.CompareToAscii( "JCL", 3 ) != COMPARE_EQUAL )
                    eType = eInvocation;
                else
                    eType = eQuoted;
            }
            // check for invocation or quoted value
            else if( aLine.GetChar(0) == '"' )
            {
                aLine.Erase( 0, 1 );
                nTransPos = aLine.Search( '"' );
                aValue = String( aLine.Copy( 0, nTransPos ), RTL_TEXTENCODING_MS_1252 );
                // after the second doublequote can follow a / and a translation
                aValueTranslation = handleTranslation( aLine.Copy( nTransPos+2 ), bIsGlobalizedLine );
                // check for quoted value
                if( aOption.Len() &&
                    aUniKey.CompareToAscii( "JCL", 3 ) != COMPARE_EQUAL )
                    eType = eInvocation;
                else
                    eType = eQuoted;
            }
            // check for symbol value
            else if( aLine.GetChar(0) == '^' )
            {
                aLine.Erase( 0, 1 );
                aValue = String( aLine, RTL_TEXTENCODING_MS_1252 );
                eType = eSymbol;
            }
            else
            {
                // must be a string value then
                // strictly this is false because string values
                // can contain any whitespace which is reduced
                // to one space by now
                // who cares ...
                nTransPos = aLine.Search( '/' );
                if( nTransPos == STRING_NOTFOUND )
                    nTransPos = aLine.Len();
                aValue = String( aLine.Copy( 0, nTransPos ), RTL_TEXTENCODING_MS_1252 );
                aValueTranslation = handleTranslation( aLine.Copy( nTransPos+1 ), bIsGlobalizedLine );
                eType = eString;
            }
        }

        // handle globalized PPD entries
        if( bIsGlobalizedLine )
        {
            // handle main key translations of form:
            // *ll_CC.Translation MainKeyword/translated text: ""
            if( aUniKey.EqualsAscii( "Translation" ) )
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

        if( aOptionTranslation.getLength() )
            m_pTranslator->insertOption( aUniKey, aOption, aOptionTranslation, aTransLocale );
        if( aValueTranslation.getLength() )
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
        ByteString aLine( *line );
        if( aLine.CompareTo( "*Default", 8 ) == COMPARE_EQUAL )
        {
            String aKey( aLine.Copy( 8 ), RTL_TEXTENCODING_MS_1252 );
            sal_uInt16 nPos = aKey.Search( ':' );
            if( nPos != STRING_NOTFOUND )
            {
                aKey.Erase( nPos );
                String aOption( WhitespaceToSpace( aLine.Copy( nPos+9 ) ), RTL_TEXTENCODING_MS_1252 );
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
        else if( aLine.CompareTo( "*UIConstraints", 14 ) == COMPARE_EQUAL  ||
                 aLine.CompareTo( "*NonUIConstraints", 17 ) == COMPARE_EQUAL )
            parseConstraint( aLine );

    }
}

void PPDParser::parseOpenUI( const ByteString& rLine )
{
    String aTranslation;
    ByteString aKey = rLine;

    int nPos = aKey.Search( ':' );
    if( nPos != STRING_NOTFOUND )
        aKey.Erase( nPos );
    nPos = aKey.Search( '/' );
    if( nPos != STRING_NOTFOUND )
    {
        aTranslation = handleTranslation( aKey.Copy( nPos + 1 ), false );
        aKey.Erase( nPos );
    }
    aKey = GetCommandLineToken( 1, aKey );
    aKey.Erase( 0, 1 );

    String aUniKey( aKey, RTL_TEXTENCODING_MS_1252 );
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

    ByteString aValue = WhitespaceToSpace( rLine.GetToken( 1, ':' ) );
    if( aValue.CompareIgnoreCaseToAscii( "boolean" ) == COMPARE_EQUAL )
        pKey->m_eUIType = PPDKey::Boolean;
    else if( aValue.CompareIgnoreCaseToAscii( "pickmany" ) == COMPARE_EQUAL )
        pKey->m_eUIType = PPDKey::PickMany;
    else
        pKey->m_eUIType = PPDKey::PickOne;
}

void PPDParser::parseOrderDependency( const ByteString& rLine )
{
    ByteString aLine( rLine );
    int nPos = aLine.Search( ':' );
    if( nPos != STRING_NOTFOUND )
        aLine.Erase( 0, nPos+1 );

    int nOrder = GetCommandLineToken( 0, aLine ).ToInt32();
    ByteString aSetup = GetCommandLineToken( 1, aLine );
    String aKey( GetCommandLineToken( 2, aLine ), RTL_TEXTENCODING_MS_1252 );
    if( aKey.GetChar( 0 ) != '*' )
        return; // invalid order depency
    aKey.Erase( 0, 1 );

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
    if( aSetup.Equals( "ExitServer" ) )
        pKey->m_eSetupType = PPDKey::ExitServer;
    else if( aSetup.Equals( "Prolog" ) )
        pKey->m_eSetupType = PPDKey::Prolog;
    else if( aSetup.Equals( "DocumentSetup" ) )
        pKey->m_eSetupType = PPDKey::DocumentSetup;
    else if( aSetup.Equals( "PageSetup" ) )
        pKey->m_eSetupType = PPDKey::PageSetup;
    else if( aSetup.Equals( "JCLSetup" ) )
        pKey->m_eSetupType = PPDKey::JCLSetup;
    else
        pKey->m_eSetupType = PPDKey::AnySetup;
}

void PPDParser::parseConstraint( const ByteString& rLine )
{
    bool bFailed = false;

    String aLine( rLine, RTL_TEXTENCODING_MS_1252 );
    aLine.Erase( 0, rLine.Search( ':' )+1 );
    PPDConstraint aConstraint;
    int nTokens = GetCommandLineTokenCount( aLine );
    for( int i = 0; i < nTokens; i++ )
    {
        String aToken = GetCommandLineToken( i, aLine );
        if( aToken.GetChar( 0 ) == '*' )
        {
            aToken.Erase( 0, 1 );
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
        fprintf( stderr, "Warning: constraint \"%s\" is invalid\n", rLine.GetStr() );
#endif
    }
    else
        m_aConstraints.push_back( aConstraint );
}

String PPDParser::getDefaultPaperDimension() const
{
    if( m_pDefaultPaperDimension )
        return m_pDefaultPaperDimension->m_aOption;

    return String();
}

bool PPDParser::getMargins(
                           const String& rPaperName,
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
    String aArea = m_pImageableAreas->getValue( nImArea )->m_aValue;
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
                                  const String& rPaperName,
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
    String aArea = m_pPaperDimensions->getValue( nPDim )->m_aValue;
    PDWidth     = StringToDouble( GetCommandLineToken( 0, aArea ) );
    PDHeight    = StringToDouble( GetCommandLineToken( 1, aArea ) );
    rHeight = (int)(PDHeight + 0.5);
    rWidth  = (int)(PDWidth + 0.5);

    return true;
}

String PPDParser::matchPaper( int nWidth, int nHeight ) const
{
    if( ! m_pPaperDimensions )
        return String();

    int nPDim = -1;
    double PDWidth, PDHeight;
    double fSort = 2e36, fNewSort;

    for( int i = 0; i < m_pPaperDimensions->countValues(); i++ )
    {
        String aArea =  m_pPaperDimensions->getValue( i )->m_aValue;
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
        String rRet = matchPaper( nHeight, nWidth );
        bDontSwap = false;
        return rRet;
    }

    return nPDim != -1 ? m_pPaperDimensions->getValue( nPDim )->m_aOption : String();
}

String PPDParser::getDefaultInputSlot() const
{
    if( m_pDefaultInputSlot )
        return m_pDefaultInputSlot->m_aValue;
    return String();
}

String PPDParser::getSlot( int nSlot ) const
{
    if( ! m_pInputSlots )
        return String();

    if( nSlot > 0 && nSlot < m_pInputSlots->countValues() )
        return m_pInputSlots->getValue( nSlot )->m_aOption;
    else if( m_pInputSlots->countValues() > 0 )
        return m_pInputSlots->getValue( (sal_uLong)0 )->m_aOption;

    return String();
}

String PPDParser::getSlotCommand( int nSlot ) const
{
    if( ! m_pInputSlots )
        return String();

    if( nSlot > 0 && nSlot < m_pInputSlots->countValues() )
        return m_pInputSlots->getValue( nSlot )->m_aValue;
    else if( m_pInputSlots->countValues() > 0 )
        return m_pInputSlots->getValue( (sal_uLong)0 )->m_aValue;

    return String();
}

String PPDParser::getSlotCommand( const String& rSlot ) const
{
    if( ! m_pInputSlots )
        return String();

    for( int i=0; i < m_pInputSlots->countValues(); i++ )
    {
        const PPDValue* pValue = m_pInputSlots->getValue( i );
        if( pValue->m_aOption == rSlot )
            return pValue->m_aValue;
    }
    return String();
}

String PPDParser::getPaperDimension( int nPaperDimension ) const
{
    if( ! m_pPaperDimensions )
        return String();

    if( nPaperDimension > 0 && nPaperDimension < m_pPaperDimensions->countValues() )
        return m_pPaperDimensions->getValue( nPaperDimension )->m_aOption;
    else if( m_pPaperDimensions->countValues() > 0 )
        return m_pPaperDimensions->getValue( (sal_uLong)0 )->m_aOption;

    return String();
}

String PPDParser::getPaperDimensionCommand( int nPaperDimension ) const
{
    if( ! m_pPaperDimensions )
        return String();

    if( nPaperDimension > 0 && nPaperDimension < m_pPaperDimensions->countValues() )
        return m_pPaperDimensions->getValue( nPaperDimension )->m_aValue;
    else if( m_pPaperDimensions->countValues() > 0 )
        return m_pPaperDimensions->getValue( (sal_uLong)0 )->m_aValue;

    return String();
}

String PPDParser::getPaperDimensionCommand( const String& rPaperDimension ) const
{
    if( ! m_pPaperDimensions )
        return String();

    for( int i=0; i < m_pPaperDimensions->countValues(); i++ )
    {
        const PPDValue* pValue = m_pPaperDimensions->getValue( i );
        if( pValue->m_aOption == rPaperDimension )
            return pValue->m_aValue;
    }
    return String();
}

void PPDParser::getResolutionFromString(
                                        const String& rString,
                                        int& rXRes, int& rYRes ) const
{
    int nDPIPos;

    rXRes = rYRes = 300;

    nDPIPos = rString.SearchAscii( "dpi" );
    if( nDPIPos != STRING_NOTFOUND )
    {
        int nPos = 0;
        if( ( nPos = rString.Search( 'x' ) ) != STRING_NOTFOUND )
        {
            rXRes = rString.Copy( 0, nPos ).ToInt32();
            rYRes = rString.GetToken( 1, 'x' ).Erase( nDPIPos - nPos - 1 ).ToInt32();
        }
        else
            rXRes = rYRes = rString.Copy( 0, nDPIPos ).ToInt32();
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

int PPDParser::getResolutions() const
{
    if( ( ! m_pResolutions || m_pResolutions->countValues() == 0 ) &&
        m_pDefaultResolution )
        return 1;
    return m_pResolutions ? m_pResolutions->countValues() : 0;
}

void PPDParser::getResolution( int nNr, int& rXRes, int& rYRes ) const
{
    if( ( ! m_pResolutions || m_pResolutions->countValues() == 0 ) && m_pDefaultResolution && nNr == 0 )
    {
        getDefaultResolution( rXRes, rYRes );
        return;
    }
    if( ! m_pResolutions )
        return;

    getResolutionFromString( m_pResolutions->getValue( nNr )->m_aOption,
                             rXRes, rYRes );
}

String PPDParser::getResolutionCommand( int nXRes, int nYRes ) const
{
    if( ( ! m_pResolutions || m_pResolutions->countValues() == 0 ) && m_pDefaultResolution )
        return m_pDefaultResolution->m_aValue;

    if( ! m_pResolutions )
        return String();

    int nX, nY;
    for( int i = 0; i < m_pResolutions->countValues(); i++ )
    {
        getResolutionFromString( m_pResolutions->getValue( i )->m_aOption,
                                 nX, nY );
        if( nX == nXRes && nY == nYRes )
            return m_pResolutions->getValue( i )->m_aValue;
    }
    return String();
}

String PPDParser::getDefaultDuplexType() const
{
    if( m_pDefaultDuplexType )
        return m_pDefaultDuplexType->m_aValue;
    return String();
}

String PPDParser::getDuplex( int nDuplex ) const
{
    if( ! m_pDuplexTypes )
        return String();

    if( nDuplex > 0 && nDuplex < m_pDuplexTypes->countValues() )
        return m_pDuplexTypes->getValue( nDuplex )->m_aOption;
    else if( m_pDuplexTypes->countValues() > 0 )
        return m_pDuplexTypes->getValue( (sal_uLong)0 )->m_aOption;

    return String();
}

String PPDParser::getDuplexCommand( int nDuplex ) const
{
    if( ! m_pDuplexTypes )
        return String();

    if( nDuplex > 0 && nDuplex < m_pDuplexTypes->countValues() )
        return m_pDuplexTypes->getValue( nDuplex )->m_aValue;
    else if( m_pDuplexTypes->countValues() > 0 )
        return m_pDuplexTypes->getValue( (sal_uLong)0 )->m_aValue;

    return String();
}

String PPDParser::getDuplexCommand( const String& rDuplex ) const
{
    if( ! m_pDuplexTypes )
        return String();

    for( int i=0; i < m_pDuplexTypes->countValues(); i++ )
    {
        const PPDValue* pValue = m_pDuplexTypes->getValue( i );
        if( pValue->m_aOption == rDuplex )
            return pValue->m_aValue;
    }
    return String();
}

void PPDParser::getFontAttributes(
                                  int nFont,
                                  String& rEncoding,
                                  String& rCharset ) const
{
    if( m_pFontList && nFont >= 0 && nFont < m_pFontList->countValues() )
    {
        String aAttribs =
            WhitespaceToSpace( m_pFontList->getValue( nFont )->m_aValue );
        rEncoding   = GetCommandLineToken( 0, aAttribs );
        rCharset    = GetCommandLineToken( 2, aAttribs );
    }
}

void PPDParser::getFontAttributes(
                                  const String& rFont,
                                  String& rEncoding,
                                  String& rCharset ) const
{
    if( m_pFontList )
    {
        for( int i = 0; i < m_pFontList->countValues(); i++ )
            if( m_pFontList->getValue( i )->m_aOption == rFont )
                getFontAttributes( i, rEncoding, rCharset );
    }
}

String PPDParser::getFont( int nFont ) const
{
    if( ! m_pFontList )
        return String();

    if( nFont >=0 && nFont < m_pFontList->countValues() )
        return m_pFontList->getValue( nFont )->m_aOption;
    return String();
}

rtl::OUString PPDParser::translateKey( const rtl::OUString& i_rKey,
                                       const com::sun::star::lang::Locale& i_rLocale ) const
{
    rtl::OUString aResult( m_pTranslator->translateKey( i_rKey, i_rLocale ) );
    if( aResult.getLength() == 0 )
        aResult = i_rKey;
    return aResult;
}

rtl::OUString PPDParser::translateOption( const rtl::OUString& i_rKey,
                                          const rtl::OUString& i_rOption,
                                          const com::sun::star::lang::Locale& i_rLocale ) const
{
    rtl::OUString aResult( m_pTranslator->translateOption( i_rKey, i_rOption, i_rLocale ) );
    if( aResult.getLength() == 0 )
        aResult = i_rOption;
    return aResult;
}

rtl::OUString PPDParser::translateValue( const rtl::OUString& i_rKey,
                                         const rtl::OUString& i_rOption,
                                         const rtl::OUString& i_rValue,
                                         const com::sun::star::lang::Locale& i_rLocale ) const
{
    rtl::OUString aResult( m_pTranslator->translateValue( i_rKey, i_rOption, i_rValue, i_rLocale ) );
    if( aResult.getLength() == 0 )
        aResult = i_rValue;
    return aResult;
}

/*
 *  PPDKey
 */

PPDKey::PPDKey( const String& rKey ) :
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

const PPDValue* PPDKey::getValue( const String& rOption ) const
{
    PPDKey::hash_type::const_iterator it = m_aValues.find( rOption );
    return it != m_aValues.end() ? &it->second : NULL;
}

// -------------------------------------------------------------------

const PPDValue* PPDKey::getValueCaseInsensitive( const String& rOption ) const
{
    const PPDValue* pValue = getValue( rOption );
    if( ! pValue )
    {
        for( size_t n = 0; n < m_aOrderedValues.size() && ! pValue; n++ )
            if( m_aOrderedValues[n]->m_aOption.EqualsIgnoreCaseAscii( rOption ) )
                pValue = m_aOrderedValues[n];
    }

    return pValue;
}

// -------------------------------------------------------------------

void PPDKey::eraseValue( const String& rOption )
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

PPDValue* PPDKey::insertValue( const String& rOption )
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

    const PPDValue* pResetValue = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "None" ) ) );
    if( ! pResetValue )
        pResetValue = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "False" ) ) );
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
    if( pNewValue->m_aOption.EqualsAscii( "None" ) || pNewValue->m_aOption.EqualsAscii( "False" ) ||
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
                    ! pOtherKeyOption->m_aOption.EqualsAscii( "None" ) &&
                    ! pOtherKeyOption->m_aOption.EqualsAscii( "False" ) )
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
                    ! pNewValue->m_aOption.EqualsAscii( "None" ) &&
                    ! pNewValue->m_aOption.EqualsAscii( "False" ) )
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
            if( ! pOtherValue->m_aOption.EqualsAscii( "None" )  &&
                ! pOtherValue->m_aOption.EqualsAscii( "False" )     &&
                ! pNewValue->m_aOption.EqualsAscii( "None" )        &&
                ! pNewValue->m_aOption.EqualsAscii( "False" ) )
                return false;
        }
    }
    return true;
}

// -------------------------------------------------------------------

void PPDContext::getUnconstrainedValues( const PPDKey* pKey, ::std::list< const PPDValue* >& rValues )
{
    rValues.clear();

    if( ! m_pParser || ! pKey || ! m_pParser->hasKey( pKey ) )
        return;

    int nValues = pKey->countValues();
    for( int i = 0; i < nValues; i++ )
    {
        const PPDValue* pValue = pKey->getValue( i );
        if( checkConstraints( pKey, pValue ) )
            rValues.push_back( pValue );
    }
}


// -------------------------------------------------------------------

void* PPDContext::getStreamableBuffer( sal_uLong& rBytes ) const
{
    rBytes = 0;
    if( ! m_aCurrentValues.size() )
        return NULL;
    hash_type::const_iterator it;
    for( it = m_aCurrentValues.begin(); it != m_aCurrentValues.end(); ++it )
    {
        ByteString aCopy( it->first->getKey(), RTL_TEXTENCODING_MS_1252 );
        rBytes += aCopy.Len();
        rBytes += 1; // for ':'
        if( it->second )
        {
            aCopy = ByteString( it->second->m_aOption, RTL_TEXTENCODING_MS_1252 );
            rBytes += aCopy.Len();
        }
        else
            rBytes += 4;
        rBytes += 1; // for '\0'
    }
    rBytes += 1;
    void* pBuffer = new char[ rBytes ];
    memset( pBuffer, 0, rBytes );
    char* pRun = (char*)pBuffer;
    for( it = m_aCurrentValues.begin(); it != m_aCurrentValues.end(); ++it )
    {
        ByteString aCopy( it->first->getKey(), RTL_TEXTENCODING_MS_1252 );
        int nBytes = aCopy.Len();
        memcpy( pRun, aCopy.GetBuffer(), nBytes );
        pRun += nBytes;
        *pRun++ = ':';
        if( it->second )
            aCopy = ByteString( it->second->m_aOption, RTL_TEXTENCODING_MS_1252 );
        else
            aCopy = "*nil";
        nBytes = aCopy.Len();
        memcpy( pRun, aCopy.GetBuffer(), nBytes );
        pRun += nBytes;

        *pRun++ = 0;
    }
    return pBuffer;
}

// -------------------------------------------------------------------

void PPDContext::rebuildFromStreamBuffer( void* pBuffer, sal_uLong nBytes )
{
    if( ! m_pParser )
        return;

    m_aCurrentValues.clear();

    char* pRun = (char*)pBuffer;
    while( nBytes && *pRun )
    {
        ByteString aLine( pRun );
        int nPos = aLine.Search( ':' );
        if( nPos != STRING_NOTFOUND )
        {
            const PPDKey* pKey = m_pParser->getKey( String( aLine.Copy( 0, nPos ), RTL_TEXTENCODING_MS_1252 ) );
            if( pKey )
            {
                const PPDValue* pValue = NULL;
                String aOption( aLine.Copy( nPos+1 ), RTL_TEXTENCODING_MS_1252 );
                if( ! aOption.EqualsAscii( "*nil" ) )
                    pValue = pKey->getValue( aOption );
                m_aCurrentValues[ pKey ] = pValue;
#ifdef __DEBUG
                fprintf( stderr, "PPDContext::rebuildFromStreamBuffer: read PPDKeyValue { %s, %s }\n", pKV->m_pKey->getKey().GetStr(), pKV->m_pCurrentValue ? pKV->m_pCurrentValue->m_aOption.GetStr() : "<nil>" );
#endif
            }
        }
        nBytes -= aLine.Len()+1;
        pRun += aLine.Len()+1;
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
        const PPDKey* pKey = m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Resolution" ) ) );
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

void PPDContext::getPageSize( String& rPaper, int& rWidth, int& rHeight ) const
{
    // initialize to reasonable default, if parser is not set
    rPaper  = String( RTL_CONSTASCII_USTRINGPARAM( "A4" ) );
    rWidth  = 595;
    rHeight = 842;
    if( m_pParser )
    {
        const PPDKey* pKey = m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) );
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
