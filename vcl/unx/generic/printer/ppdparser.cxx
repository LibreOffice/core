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

#include <comphelper/string.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/ppdparser.hxx>
#include <vcl/strhelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <unx/helper.hxx>
#include <unx/cupsmgr.hxx>
#include <unx/cpdmgr.hxx>

#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <tools/zcodec.hxx>
#include <osl/mutex.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/instance.hxx>
#include <sal/macros.h>
#include <sal/log.hxx>
#include <salhelper/linkhelper.hxx>

#include <com/sun/star/lang/Locale.hpp>

#include <unordered_map>

#ifdef ENABLE_CUPS
#include <cups/cups.h>
#endif

#include <config_dbus.h>
#include <config_gio.h>

namespace psp
{
    class PPDTranslator
    {
        struct LocaleEqual
        {
            bool operator()(const css::lang::Locale& i_rLeft,
                            const css::lang::Locale& i_rRight) const
            {
                return i_rLeft.Language == i_rRight.Language &&
                    i_rLeft.Country == i_rRight.Country &&
                    i_rLeft.Variant == i_rRight.Variant;
            }
        };

        struct LocaleHash
        {
            size_t operator()(const css::lang::Locale& rLocale) const
            { return
                  static_cast<size_t>(rLocale.Language.hashCode())
                ^ static_cast<size_t>(rLocale.Country.hashCode())
                ^ static_cast<size_t>(rLocale.Variant.hashCode())
                ;
            }
        };

        typedef std::unordered_map< css::lang::Locale, OUString, LocaleHash, LocaleEqual > translation_map;
        typedef std::unordered_map< OUString, translation_map > key_translation_map;

        key_translation_map     m_aTranslations;
        public:
        PPDTranslator() {}

        void insertValue(
            const OUString& i_rKey,
            const OUString& i_rOption,
            const OUString& i_rValue,
            const OUString& i_rTranslation,
            const css::lang::Locale& i_rLocale
            );

        void insertOption( const OUString& i_rKey,
                           const OUString& i_rOption,
                           const OUString& i_rTranslation,
                           const css::lang::Locale& i_rLocale )
        {
            insertValue( i_rKey, i_rOption, OUString(), i_rTranslation, i_rLocale );
        }

        void insertKey( const OUString& i_rKey,
                        const OUString& i_rTranslation,
                        const css::lang::Locale& i_rLocale = css::lang::Locale() )
        {
            insertValue( i_rKey, OUString(), OUString(), i_rTranslation, i_rLocale );
        }

        OUString translateValue(
            const OUString& i_rKey,
            const OUString& i_rOption
            ) const;

        OUString translateOption( const OUString& i_rKey,
                                       const OUString& i_rOption ) const
        {
            return translateValue( i_rKey, i_rOption  );
        }

        OUString translateKey( const OUString& i_rKey ) const
        {
            return translateValue( i_rKey, OUString() );
        }
    };

    static css::lang::Locale normalizeInputLocale(
        const css::lang::Locale& i_rLocale
        )
    {
        css::lang::Locale aLoc( i_rLocale );
        if( aLoc.Language.isEmpty() )
        {
            // empty locale requested, fill in application UI locale
            aLoc = Application::GetSettings().GetUILanguageTag().getLocale();

            #if OSL_DEBUG_LEVEL > 1
            static const char* pEnvLocale = getenv( "SAL_PPDPARSER_LOCALE" );
            if( pEnvLocale && *pEnvLocale )
            {
                OString aStr( pEnvLocale );
                sal_Int32 nLen = aStr.getLength();
                aLoc.Language = OStringToOUString( aStr.copy( 0, std::min(nLen, 2) ), RTL_TEXTENCODING_MS_1252 );
                if( nLen >=5 && aStr[2] == '_' )
                    aLoc.Country = OStringToOUString( aStr.copy( 3, 2 ), RTL_TEXTENCODING_MS_1252 );
                else
                    aLoc.Country.clear();
                aLoc.Variant.clear();
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
        const css::lang::Locale& i_rLocale
        )
    {
        OUStringBuffer aKey( i_rKey.getLength() + i_rOption.getLength() + i_rValue.getLength() + 2 );
        aKey.append( i_rKey );
        if( !i_rOption.isEmpty() || !i_rValue.isEmpty() )
        {
            aKey.append( ':' );
            aKey.append( i_rOption );
        }
        if( !i_rValue.isEmpty() )
        {
            aKey.append( ':' );
            aKey.append( i_rValue );
        }
        if( !aKey.isEmpty() && !i_rTranslation.isEmpty() )
        {
            OUString aK( aKey.makeStringAndClear() );
            css::lang::Locale aLoc;
            /* FIXME-BCP47: using Variant, uppercase? */
            aLoc.Language = i_rLocale.Language.toAsciiLowerCase();
            aLoc.Country  = i_rLocale.Country.toAsciiUpperCase();
            aLoc.Variant  = i_rLocale.Variant.toAsciiUpperCase();
            m_aTranslations[ aK ][ aLoc ] = i_rTranslation;
        }
    }

    OUString PPDTranslator::translateValue(
        const OUString& i_rKey,
        const OUString& i_rOption
        ) const
    {
        OUString aResult;

        OUStringBuffer aKey( i_rKey.getLength() + i_rOption.getLength() + 2 );
        aKey.append( i_rKey );
        if( !i_rOption.isEmpty() )
        {
            aKey.append( ':' );
            aKey.append( i_rOption );
        }
        if( !aKey.isEmpty() )
        {
            OUString aK( aKey.makeStringAndClear() );
            key_translation_map::const_iterator it = m_aTranslations.find( aK );
            if( it != m_aTranslations.end() )
            {
                const translation_map& rMap( it->second );

                css::lang::Locale aLoc( normalizeInputLocale( css::lang::Locale() ) );
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
                    case 0: aLoc.Variant.clear();break;
                    case 1: aLoc.Country.clear();break;
                    case 2: aLoc.Language.clear();break;
                    }
                }
            }
        }
        return aResult;
    }

    class PPDCache
    {
    public:
        std::vector< std::unique_ptr<PPDParser> > aAllParsers;
        std::unique_ptr<std::unordered_map< OUString, OUString >> pAllPPDFiles;
    };
}

using namespace psp;

namespace
{
    struct thePPDCache : public rtl::Static<PPDCache, thePPDCache> {};
}

class PPDDecompressStream
{
private:
    PPDDecompressStream(const PPDDecompressStream&) = delete;
    PPDDecompressStream& operator=(const PPDDecompressStream&) = delete;

    std::unique_ptr<SvFileStream>   mpFileStream;
    std::unique_ptr<SvMemoryStream> mpMemStream;
    OUString       maFileName;

public:
    explicit PPDDecompressStream( const OUString& rFile );
    ~PPDDecompressStream();

    bool IsOpen() const;
    bool eof() const;
    OString ReadLine();
    void Open( const OUString& i_rFile );
    void Close();
    const OUString& GetFileName() const { return maFileName; }
};

PPDDecompressStream::PPDDecompressStream( const OUString& i_rFile )
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

    mpFileStream.reset( new SvFileStream( i_rFile, StreamMode::READ ) );
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
    if( aLine.getLength() > 1 && static_cast<unsigned char>(aLine[0]) == 0x1f
        && static_cast<unsigned char>(aLine[1]) == 0x8b /* check for gzip */ )
    {
        // so let's try to decompress the stream
        mpMemStream.reset( new SvMemoryStream( 4096, 4096 ) );
        ZCodec aCodec;
        aCodec.BeginCompression( ZCODEC_DEFAULT_COMPRESSION, false, true );
        long nComp = aCodec.Decompress( *mpFileStream, *mpMemStream );
        aCodec.EndCompression();
        if( nComp < 0 )
        {
            // decompression failed, must be an uncompressed stream after all
            mpMemStream.reset();
            mpFileStream->Seek( 0 );
        }
        else
        {
            // compression successful, can get rid of file stream
            mpFileStream.reset();
            mpMemStream->Seek( 0 );
        }
    }
}

void PPDDecompressStream::Close()
{
    mpMemStream.reset();
    mpFileStream.reset();
}

bool PPDDecompressStream::IsOpen() const
{
    return (mpMemStream || (mpFileStream && mpFileStream->IsOpen()));
}

bool PPDDecompressStream::eof() const
{
    return ( mpMemStream ? mpMemStream->eof() : ( mpFileStream == nullptr || mpFileStream->eof() ) );
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

static osl::FileBase::RC resolveLink( const OUString& i_rURL, OUString& o_rResolvedURL, OUString& o_rBaseName, osl::FileStatus::Type& o_rType)
{
    salhelper::LinkResolver aResolver(osl_FileStatus_Mask_FileName |
                                      osl_FileStatus_Mask_Type |
                                      osl_FileStatus_Mask_FileURL);

    osl::FileBase::RC aRet = aResolver.fetchFileStatus(i_rURL, 10/*nLinkLevel*/);

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
                OUString aFileURL, aFileName;
                osl::FileStatus::Type eType = osl::FileStatus::Unknown;
                OUString aURL = rDir + "/" + aStatus.getFileName();

                if(resolveLink( aURL, aFileURL, aFileName, eType ) == osl::FileBase::E_None)
                {
                    if( eType == osl::FileStatus::Regular )
                    {
                        INetURLObject aPPDFile = aPPDDir;
                        aPPDFile.Append( aFileName );

                        // match extension
                        for(const suffix_t & rSuffix : pSuffixes)
                        {
                            if( aFileName.getLength() > rSuffix.nSuffixLen )
                            {
                                if( aFileName.endsWithIgnoreAsciiCaseAsciiL( rSuffix.pSuffix, rSuffix.nSuffixLen ) )
                                {
                                    (*rPPDCache.pAllPPDFiles)[ aFileName.copy( 0, aFileName.getLength() - rSuffix.nSuffixLen ) ] = aPPDFile.PathToFileName();
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

void PPDParser::initPPDFiles(PPDCache &rPPDCache)
{
    if( rPPDCache.pAllPPDFiles )
        return;

    rPPDCache.pAllPPDFiles.reset(new std::unordered_map< OUString, OUString >);

    // check installation directories
    std::vector< OUString > aPathList;
    psp::getPrinterPathList( aPathList, PRINTER_PPDDIR );
    for (auto const& path : aPathList)
    {
        INetURLObject aPPDDir( path, INetProtocol::File, INetURLObject::EncodeMechanism::All );
        scanPPDDir( aPPDDir.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    }
    if( rPPDCache.pAllPPDFiles->find( OUString( "SGENPRT" ) ) == rPPDCache.pAllPPDFiles->end() )
    {
        // last try: search in directory of executable (mainly for setup)
        OUString aExe;
        if( osl_getExecutableFile( &aExe.pData ) == osl_Process_E_None )
        {
            INetURLObject aDir( aExe );
            aDir.removeSegment();
            SAL_INFO("vcl.unx.print", "scanning last chance dir: "
                    << aDir.GetMainURL(INetURLObject::DecodeMechanism::NONE));
            scanPPDDir( aDir.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
            SAL_INFO("vcl.unx.print", "SGENPRT "
                    << (rPPDCache.pAllPPDFiles->find("SGENPRT") ==
                        rPPDCache.pAllPPDFiles->end() ? "not found" : "found"));
        }
    }
}

OUString PPDParser::getPPDFile( const OUString& rFile )
{
    INetURLObject aPPD( rFile, INetProtocol::File, INetURLObject::EncodeMechanism::All );
    // someone might enter a full qualified name here
    PPDDecompressStream aStream( aPPD.PathToFileName() );
    if( ! aStream.IsOpen() )
    {
        std::unordered_map< OUString, OUString >::const_iterator it;
        PPDCache &rPPDCache = thePPDCache::get();

        bool bRetry = true;
        do
        {
            initPPDFiles(rPPDCache);
            // some PPD files contain dots beside the extension, so try name first
            // and cut of points after that
            OUString aBase( rFile );
            sal_Int32 nLastIndex = aBase.lastIndexOf( '/' );
            if( nLastIndex >= 0 )
                aBase = aBase.copy( nLastIndex+1 );
            do
            {
                it = rPPDCache.pAllPPDFiles->find( aBase );
                nLastIndex = aBase.lastIndexOf( '.' );
                if( nLastIndex > 0 )
                    aBase = aBase.copy( 0, nLastIndex );
            } while( it == rPPDCache.pAllPPDFiles->end() && nLastIndex > 0 );

            if( it == rPPDCache.pAllPPDFiles->end() && bRetry )
            {
                // a new file ? rehash
                rPPDCache.pAllPPDFiles.reset();
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
        if (aLine.startsWith("*PPD-Adobe"))
            aRet = aStream.GetFileName();
        else
        {
            // our *Include hack does usually not begin
            // with *PPD-Adobe, so try some lines for *Include
            int nLines = 10;
            while (aLine.indexOf("*Include") != 0 && --nLines)
                aLine = aStream.ReadLine();
            if( nLines )
                aRet = aStream.GetFileName();
        }
    }

    return aRet;
}

const PPDParser* PPDParser::getParser( const OUString& rFile )
{
    static ::osl::Mutex aMutex;
    ::osl::Guard< ::osl::Mutex > aGuard( aMutex );

    OUString aFile = rFile;
    if( !rFile.startsWith( "CUPS:" ) && !rFile.startsWith( "CPD:" ) )
        aFile = getPPDFile( rFile );
    if( aFile.isEmpty() )
    {
        SAL_INFO("vcl.unx.print", "Could not get printer PPD file \""
                << rFile << "\" !");
        return nullptr;
    }
    else
        SAL_INFO("vcl.unx.print", "Parsing printer info from \""
                 << rFile << "\" !");


    PPDCache &rPPDCache = thePPDCache::get();
    for( auto const & i : rPPDCache.aAllParsers )
        if( i->m_aFile == aFile )
            return i.get();

    PPDParser* pNewParser = nullptr;
    if( !aFile.startsWith( "CUPS:" ) && !aFile.startsWith( "CPD:" ) )
        pNewParser = new PPDParser( aFile );
    else
    {
        PrinterInfoManager& rMgr = PrinterInfoManager::get();
        if( rMgr.getType() == PrinterInfoManager::Type::CUPS )
        {
#ifdef ENABLE_CUPS
            pNewParser = const_cast<PPDParser*>(static_cast<CUPSManager&>(rMgr).createCUPSParser( aFile ));
#endif
        } else if ( rMgr.getType() == PrinterInfoManager::Type::CPD )
        {
#if ENABLE_DBUS && ENABLE_GIO
            pNewParser = const_cast<PPDParser*>(static_cast<CPDManager&>(rMgr).createCPDParser( aFile ));
#endif
        }
    }
    if( pNewParser )
    {
        // this may actually be the SGENPRT parser,
        // so ensure uniqueness here (but don't remove last we delete us!)
        if (std::none_of(
                    rPPDCache.aAllParsers.begin(),
                    rPPDCache.aAllParsers.end(),
                    [pNewParser] (std::unique_ptr<PPDParser> const & x) { return x.get() == pNewParser; } ))
        {
            // insert new parser to vector
            rPPDCache.aAllParsers.emplace_back(pNewParser);
        }
    }
    return pNewParser;
}

PPDParser::PPDParser(const OUString& rFile, const std::vector<PPDKey*>& keys)
    : m_aFile(rFile)
    , m_bColorDevice(false)
    , m_bType42Capable(false)
    , m_nLanguageLevel(0)
    , m_aFileEncoding(RTL_TEXTENCODING_MS_1252)
    , m_pImageableAreas(nullptr)
    , m_pDefaultPaperDimension(nullptr)
    , m_pPaperDimensions(nullptr)
    , m_pDefaultInputSlot(nullptr)
    , m_pDefaultResolution(nullptr)
    , m_pTranslator(new PPDTranslator())
{
    for (auto & key: keys)
    {
        insertKey( std::unique_ptr<PPDKey>(key) );
    }

    // fill in shortcuts
    const PPDKey* pKey;

    pKey = getKey( OUString( "PageSize" ) );

    if ( pKey ) {
        std::unique_ptr<PPDKey> pImageableAreas(new PPDKey("ImageableArea"));
        std::unique_ptr<PPDKey> pPaperDimensions(new PPDKey("PaperDimension"));
#if defined(CUPS_VERSION_MAJOR)
#if (CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 7) || CUPS_VERSION_MAJOR > 1
        for (int i = 0; i < pKey->countValues(); i++) {
            const PPDValue* pValue = pKey -> getValue(i);
            OUString aValueName = pValue -> m_aOption;
            PPDValue* pImageableAreaValue = pImageableAreas -> insertValue( aValueName, eQuoted );
            PPDValue* pPaperDimensionValue = pPaperDimensions -> insertValue( aValueName, eQuoted );
            rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
            OString o = OUStringToOString( aValueName, aEncoding );
            pwg_media_t *pPWGMedia = pwgMediaForPWG(o.pData->buffer);
            if (pPWGMedia != nullptr) {
                OUStringBuffer aBuf( 256 );
                aBuf.append( "0 0 " );
                aBuf.append( PWG_TO_POINTS(pPWGMedia -> width) );
                aBuf.append( " " );
                aBuf.append( PWG_TO_POINTS(pPWGMedia -> length) );
                if ( pImageableAreaValue )
                    pImageableAreaValue->m_aValue = aBuf.makeStringAndClear();
                aBuf.append( PWG_TO_POINTS(pPWGMedia -> width) );
                aBuf.append( " " );
                aBuf.append( PWG_TO_POINTS(pPWGMedia -> length) );
                if ( pPaperDimensionValue )
                    pPaperDimensionValue->m_aValue = aBuf.makeStringAndClear();
                if (aValueName.equals(pKey -> getDefaultValue() -> m_aOption)) {
                    pImageableAreas -> m_pDefaultValue = pImageableAreaValue;
                    pPaperDimensions -> m_pDefaultValue = pPaperDimensionValue;
                }
            }
        }
#endif // HAVE_CUPS_API_1_7
#endif
        insertKey(std::move(pImageableAreas));
        insertKey(std::move(pPaperDimensions));
    }

    m_pImageableAreas = getKey(  OUString( "ImageableArea" ) );
    const PPDValue* pDefaultImageableArea = nullptr;
    if( m_pImageableAreas )
        pDefaultImageableArea = m_pImageableAreas->getDefaultValue();
    if (m_pImageableAreas == nullptr) {
        SAL_WARN( "vcl.unx.print", "no ImageableArea in " << m_aFile);
    }
    if (pDefaultImageableArea == nullptr) {
        SAL_WARN( "vcl.unx.print", "no DefaultImageableArea in " << m_aFile);
    }

    m_pPaperDimensions = getKey( OUString( "PaperDimension" ) );
    if( m_pPaperDimensions )
        m_pDefaultPaperDimension = m_pPaperDimensions->getDefaultValue();
    if (m_pPaperDimensions == nullptr) {
        SAL_WARN( "vcl.unx.print", "no PaperDimensions in " << m_aFile);
    }
    if (m_pDefaultPaperDimension == nullptr) {
        SAL_WARN( "vcl.unx.print", "no DefaultPaperDimensions in " << m_aFile);
    }

    auto pResolutions = getKey( OUString( "Resolution" ) );
    if( pResolutions )
        m_pDefaultResolution = pResolutions->getDefaultValue();
    if (pResolutions == nullptr) {
        SAL_WARN( "vcl.unx.print", "no Resolution in " << m_aFile);
    }
    SAL_INFO_IF(!m_pDefaultResolution, "vcl.unx.print", "no DefaultResolution in " + m_aFile);

    auto pInputSlots = getKey( OUString( "InputSlot" ) );
    if( pInputSlots )
        m_pDefaultInputSlot = pInputSlots->getDefaultValue();
    SAL_INFO_IF(!pInputSlots, "vcl.unx.print", "no InputSlot in " << m_aFile);
    SAL_INFO_IF(!m_pDefaultInputSlot, "vcl.unx.print", "no DefaultInputSlot in " << m_aFile);

    auto pFontList = getKey( OUString( "Font" ) );
    if (pFontList == nullptr) {
        SAL_WARN( "vcl.unx.print", "no Font in " << m_aFile);
    }

    // fill in direct values
    if( (pKey = getKey( OUString( "print-color-mode" ) )) )
        m_bColorDevice = pKey->countValues() > 1;
}

PPDParser::PPDParser( const OUString& rFile ) :
        m_aFile( rFile ),
        m_bColorDevice( false ),
        m_bType42Capable( false ),
        m_nLanguageLevel( 0 ),
        m_aFileEncoding( RTL_TEXTENCODING_MS_1252 ),
        m_pImageableAreas( nullptr ),
        m_pDefaultPaperDimension( nullptr ),
        m_pPaperDimensions( nullptr ),
        m_pDefaultInputSlot( nullptr ),
        m_pDefaultResolution( nullptr ),
        m_pTranslator( new PPDTranslator() )
{
    // read in the file
    std::vector< OString > aLines;
    PPDDecompressStream aStream( m_aFile );
    if( aStream.IsOpen() )
    {
        bool bLanguageEncoding = false;
        while( ! aStream.eof() )
        {
            OString aCurLine = aStream.ReadLine();
            if( aCurLine.startsWith("*") )
            {
                if (aCurLine.matchIgnoreAsciiCase("*include:"))
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
                         aCurLine.matchIgnoreAsciiCase("*languageencoding") )
                {
                    bLanguageEncoding = true; // generally only the first one counts
                    OString aLower = aCurLine.toAsciiLowerCase();
                    if( aLower.indexOf("isolatin1", 17 ) != -1 ||
                        aLower.indexOf("windowsansi", 17 ) != -1 )
                        m_aFileEncoding = RTL_TEXTENCODING_MS_1252;
                    else if( aLower.indexOf("isolatin2", 17 ) != -1 )
                        m_aFileEncoding = RTL_TEXTENCODING_ISO_8859_2;
                    else if( aLower.indexOf("isolatin5", 17 ) != -1 )
                        m_aFileEncoding = RTL_TEXTENCODING_ISO_8859_5;
                    else if( aLower.indexOf("jis83-rksj", 17 ) != -1 )
                        m_aFileEncoding = RTL_TEXTENCODING_SHIFT_JIS;
                    else if( aLower.indexOf("macstandard", 17 ) != -1 )
                        m_aFileEncoding = RTL_TEXTENCODING_APPLE_ROMAN;
                    else if( aLower.indexOf("utf-8", 17 ) != -1 )
                        m_aFileEncoding = RTL_TEXTENCODING_UTF8;
                }
            }
            aLines.push_back( aCurLine );
        }
    }
    aStream.Close();

    // now get the Values
    parse( aLines );
#if OSL_DEBUG_LEVEL > 1
    SAL_INFO("vcl.unx.print", "acquired " << m_aKeys.size()
            << " Keys from PPD " << m_aFile << ":");
    for (auto const& key : m_aKeys)
    {
        const PPDKey* pKey = key.second;
        char const* pSetupType = "<unknown>";
        switch( pKey->m_eSetupType )
        {
            case PPDKey::SetupType::ExitServer:        pSetupType = "ExitServer";break;
            case PPDKey::SetupType::Prolog:            pSetupType = "Prolog";break;
            case PPDKey::SetupType::DocumentSetup:     pSetupType = "DocumentSetup";break;
            case PPDKey::SetupType::PageSetup:         pSetupType = "PageSetup";break;
            case PPDKey::SetupType::JCLSetup:          pSetupType = "JCLSetup";break;
            case PPDKey::SetupType::AnySetup:          pSetupType = "AnySetup";break;
            default: break;
        };
        SAL_INFO("vcl.unx.print", "\t\"" << pKey->getKey() << "\" ("
                << pKey->countValues() << "values) OrderDependency: "
                << pKey->m_nOrderDependency << pSetupType );
        for( int j = 0; j < pKey->countValues(); j++ )
        {
            const PPDValue* pValue = pKey->getValue( j );
            char const* pVType = "<unknown>";
            switch( pValue->m_eType )
            {
                case eInvocation:       pVType = "invocation";break;
                case eQuoted:           pVType = "quoted";break;
                case eString:           pVType = "string";break;
                case eSymbol:           pVType = "symbol";break;
                case eNo:               pVType = "no";break;
                default: break;
            };
            SAL_INFO("vcl.unx.print", "\t\t"
                << (pValue == pKey->m_pDefaultValue ? "(Default:) " : "")
                << "option: \"" << pValue->m_aOption
                << "\", value: type " << pVType << " \""
                << pValue->m_aValue << "\"");
        }
    }
    SAL_INFO("vcl.unx.print",
            "constraints: (" << m_aConstraints.size() << " found)");
    for (auto const& constraint : m_aConstraints)
    {
        SAL_INFO("vcl.unx.print", "*\"" << constraint.m_pKey1->getKey() << "\" \""
                << (constraint.m_pOption1 ? constraint.m_pOption1->m_aOption : "<nil>")
                << "\" *\"" << constraint.m_pKey2->getKey() << "\" \""
                << (constraint.m_pOption2 ? constraint.m_pOption2->m_aOption : "<nil>")
                << "\"");
    }
#endif

    // fill in shortcuts
    const PPDKey* pKey;

    m_pImageableAreas = getKey(  OUString( "ImageableArea" ) );
    const PPDValue * pDefaultImageableArea = nullptr;
    if( m_pImageableAreas )
        pDefaultImageableArea = m_pImageableAreas->getDefaultValue();
    if (m_pImageableAreas == nullptr) {
        SAL_WARN( "vcl.unx.print", "no ImageableArea in " << m_aFile);
    }
    if (pDefaultImageableArea == nullptr) {
        SAL_WARN( "vcl.unx.print", "no DefaultImageableArea in " << m_aFile);
    }

    m_pPaperDimensions = getKey( OUString( "PaperDimension" ) );
    if( m_pPaperDimensions )
        m_pDefaultPaperDimension = m_pPaperDimensions->getDefaultValue();
    if (m_pPaperDimensions == nullptr) {
        SAL_WARN( "vcl.unx.print", "no PaperDimensions in " << m_aFile);
    }
    if (m_pDefaultPaperDimension == nullptr) {
        SAL_WARN( "vcl.unx.print", "no DefaultPaperDimensions in " << m_aFile);
    }

    auto pResolutions = getKey( OUString( "Resolution" ) );
    if( pResolutions )
        m_pDefaultResolution = pResolutions->getDefaultValue();
    if (pResolutions == nullptr) {
        SAL_WARN( "vcl.unx.print", "no Resolution in " << m_aFile);
    }
    SAL_INFO_IF(!m_pDefaultResolution, "vcl.unx.print", "no DefaultResolution in " + m_aFile);

    auto pInputSlots = getKey( OUString( "InputSlot" ) );
    if( pInputSlots )
        m_pDefaultInputSlot = pInputSlots->getDefaultValue();
    SAL_INFO_IF(!pInputSlots, "vcl.unx.print", "no InputSlot in " << m_aFile);
    SAL_INFO_IF(!m_pDefaultInputSlot, "vcl.unx.print", "no DefaultInputSlot in " << m_aFile);

    auto pFontList = getKey( OUString( "Font" ) );
    if (pFontList == nullptr) {
        SAL_WARN( "vcl.unx.print", "no Font in " << m_aFile);
    }

    // fill in direct values
    if ((pKey = getKey(OUString("ColorDevice"))))
    {
        if (const PPDValue* pValue = pKey->getValue(0))
            m_bColorDevice = pValue->m_aValue.startsWithIgnoreAsciiCase("true");
    }

    if ((pKey = getKey(OUString("LanguageLevel"))))
    {
        if (const PPDValue* pValue = pKey->getValue(0))
            m_nLanguageLevel = pValue->m_aValue.toInt32();
    }
    if ((pKey = getKey(OUString("TTRasterizer"))))
    {
        if (const PPDValue* pValue = pKey->getValue(0))
            m_bType42Capable = pValue->m_aValue.equalsIgnoreAsciiCase( "Type42" );
    }
}

PPDParser::~PPDParser()
{
    m_pTranslator.reset();
}

void PPDParser::insertKey( std::unique_ptr<PPDKey> pKey )
{
    m_aOrderedKeys.push_back( pKey.get() );
    m_aKeys[ pKey->getKey() ] = std::move(pKey);
}

const PPDKey* PPDParser::getKey( int n ) const
{
    return (static_cast<unsigned int>(n) < m_aOrderedKeys.size() && n >= 0) ? m_aOrderedKeys[n] : nullptr;
}

const PPDKey* PPDParser::getKey( const OUString& rKey ) const
{
    PPDParser::hash_type::const_iterator it = m_aKeys.find( rKey );
    return it != m_aKeys.end() ? it->second.get() : nullptr;
}

bool PPDParser::hasKey( const PPDKey* pKey ) const
{
    return pKey && ( m_aKeys.find( pKey->getKey() ) != m_aKeys.end() );
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

void PPDParser::parse( ::std::vector< OString >& rLines )
{
    // Name for PPD group into which all options are put for which the PPD
    // does not explicitly define a group.
    // This is similar to how CUPS handles it,
    // s. Sweet, Michael R. (2001): Common UNIX Printing System, p. 251:
    // "Each option in turn is associated with a group stored in the
    // ppd_group_t structure. Groups can be specified in the PPD file; if an
    // option is not associated with a group, it is put in a "General" or
    // "Extra" group depending on the option.
    static const OString aDefaultPPDGroupName("General");

    std::vector< OString >::iterator line = rLines.begin();
    PPDParser::hash_type::const_iterator keyit;

    // name of the PPD group that is currently being processed
    OString aCurrentGroup = aDefaultPPDGroupName;

    while( line != rLines.end() )
    {
        OString aCurrentLine( *line );
        ++line;

        SAL_INFO("vcl.unx.print", "Parse line '" << aCurrentLine << "'");

        if (aCurrentLine.getLength() < 2 || aCurrentLine[0] != '*')
            continue;
        if( aCurrentLine[1] == '%' )
            continue;

        OString aKey = GetCommandLineToken( 0, aCurrentLine.getToken(0, ':') );
        sal_Int32 nPos = aKey.indexOf('/');
        if (nPos != -1)
            aKey = aKey.copy(0, nPos);
        if(!aKey.isEmpty())
        {
            aKey = aKey.copy(1); // remove the '*'
        }
        if(aKey.isEmpty())
        {
            continue;
        }

        if (aKey == "CloseGroup")
        {
            aCurrentGroup = aDefaultPPDGroupName;
            continue;
        }
        if (aKey == "OpenGroup")
        {
            OString aGroupName = aCurrentLine;
            sal_Int32 nPosition = aGroupName.indexOf('/');
            if (nPosition != -1)
            {
                aGroupName = aGroupName.copy(0, nPosition);
            }

            aCurrentGroup = GetCommandLineToken(1, aGroupName);
            continue;
        }
        if ((aKey == "CloseUI") ||
            (aKey == "JCLCloseUI") ||
            (aKey == "End") ||
            (aKey == "JCLEnd") ||
            (aKey == "OpenSubGroup") ||
            (aKey == "CloseSubGroup"))
        {
            continue;
        }

        if ((aKey == "OpenUI") || (aKey == "JCLOpenUI"))
        {
            parseOpenUI( aCurrentLine, aCurrentGroup);
            continue;
        }
        else if (aKey == "OrderDependency")
        {
            parseOrderDependency( aCurrentLine );
            continue;
        }
        else if (aKey == "UIConstraints" ||
                 aKey == "NonUIConstraints")
        {
            continue; // parsed in pass 2
        }
        else if( aKey == "CustomPageSize" ) // currently not handled
            continue;
        else if (aKey.startsWith("Custom", &aKey) )
        {
            //fdo#43049 very basic support for Custom entries, we ignore the
            //validation params and types
            OUString aUniKey(OStringToOUString(aKey, RTL_TEXTENCODING_MS_1252));
            keyit = m_aKeys.find( aUniKey );
            if(keyit != m_aKeys.end())
            {
                PPDKey* pKey = keyit->second.get();
                pKey->insertValue("Custom", eInvocation, true);
            }
            continue;
        }

        // default values are parsed in pass 2
        if (aKey.startsWith("Default"))
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
        css::lang::Locale aTransLocale;
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
                aOption = aOption.copy(0,  nTransPos);
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
            if( aUniKey == "Translation" )
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

        PPDKey* pKey = nullptr;
        keyit = m_aKeys.find( aUniKey );
        if( keyit == m_aKeys.end() )
        {
            pKey = new PPDKey( aUniKey );
            insertKey( std::unique_ptr<PPDKey>(pKey) );
        }
        else
            pKey = keyit->second.get();

        if( eType == eNo && bQuery )
            continue;

        PPDValue* pValue = pKey->insertValue( aOption, eType );
        if( ! pValue )
            continue;
        pValue->m_aValue = aValue;

        if( !aOptionTranslation.isEmpty() )
            m_pTranslator->insertOption( aUniKey, aOption, aOptionTranslation, aTransLocale );
        if( !aValueTranslation.isEmpty() )
            m_pTranslator->insertValue( aUniKey, aOption, aValue, aValueTranslation, aTransLocale );

        // eventually update query and remove from option list
        if( bQuery && !pKey->m_bQueryValue )
        {
            pKey->m_bQueryValue = true;
            pKey->eraseValue( pValue->m_aOption );
        }
    }

    // second pass: fill in defaults
    for( const auto& aLine : rLines )
    {
        if (aLine.startsWith("*Default"))
        {
            SAL_INFO("vcl.unx.print", "Found a default: '" << aLine << "'");
            OUString aKey(OStringToOUString(aLine.copy(8), RTL_TEXTENCODING_MS_1252));
            sal_Int32 nPos = aKey.indexOf( ':' );
            if( nPos != -1 )
            {
                aKey = aKey.copy(0, nPos);
                OUString aOption(OStringToOUString(
                    WhitespaceToSpace(aLine.copy(nPos+9)),
                    RTL_TEXTENCODING_MS_1252));
                keyit = m_aKeys.find( aKey );
                if( keyit != m_aKeys.end() )
                {
                    PPDKey* pKey = keyit->second.get();
                    const PPDValue* pDefValue = pKey->getValue( aOption );
                    if( pKey->m_pDefaultValue == nullptr )
                        pKey->m_pDefaultValue = pDefValue;
                }
                else
                {
                    // some PPDs contain defaults for keys that
                    // do not exist otherwise
                    // (example: DefaultResolution)
                    // so invent that key here and have a default value
                    std::unique_ptr<PPDKey> pKey(new PPDKey( aKey ));
                    pKey->insertValue( aOption, eInvocation /*or what ?*/ );
                    pKey->m_pDefaultValue = pKey->getValue( aOption );
                    insertKey( std::move(pKey) );
                }
            }
        }
        else if (aLine.startsWith("*UIConstraints") ||
                 aLine.startsWith("*NonUIConstraints"))
        {
            parseConstraint( aLine );
        }
    }
}

void PPDParser::parseOpenUI(const OString& rLine, const OString& rPPDGroup)
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
        insertKey( std::unique_ptr<PPDKey>(pKey) );
    }
    else
        pKey = keyit->second.get();

    pKey->m_bUIOption = true;
    m_pTranslator->insertKey( pKey->getKey(), aTranslation );

    pKey->m_aGroup = OStringToOUString(rPPDGroup, RTL_TEXTENCODING_MS_1252);
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
        return; // invalid order dependency
    aKey = aKey.replaceAt( 0, 1, "" );

    PPDKey* pKey;
    PPDParser::hash_type::const_iterator keyit = m_aKeys.find( aKey );
    if( keyit == m_aKeys.end() )
    {
        pKey = new PPDKey( aKey );
        insertKey( std::unique_ptr<PPDKey>(pKey) );
    }
    else
        pKey = keyit->second.get();

    pKey->m_nOrderDependency = nOrder;
    if( aSetup == "ExitServer" )
        pKey->m_eSetupType = PPDKey::SetupType::ExitServer;
    else if( aSetup == "Prolog" )
        pKey->m_eSetupType = PPDKey::SetupType::Prolog;
    else if( aSetup == "DocumentSetup" )
        pKey->m_eSetupType = PPDKey::SetupType::DocumentSetup;
    else if( aSetup == "PageSetup" )
        pKey->m_eSetupType = PPDKey::SetupType::PageSetup;
    else if( aSetup == "JCLSetup" )
        pKey->m_eSetupType = PPDKey::SetupType::JCLSetup;
    else
        pKey->m_eSetupType = PPDKey::SetupType::AnySetup;
}

void PPDParser::parseConstraint( const OString& rLine )
{
    bool bFailed = false;

    OUString aLine(OStringToOUString(rLine, RTL_TEXTENCODING_MS_1252));
    sal_Int32 nIdx = rLine.indexOf(':');
    if (nIdx != -1)
        aLine = aLine.replaceAt(0, nIdx + 1, "");
    PPDConstraint aConstraint;
    int nTokens = GetCommandLineTokenCount( aLine );
    for( int i = 0; i < nTokens; i++ )
    {
        OUString aToken = GetCommandLineToken( i, aLine );
        if( !aToken.isEmpty() && aToken[ 0 ] == '*' )
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
        SAL_INFO("vcl.unx.print",
                "Warning: constraint \"" << rLine << "\" is invalid");
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
    rLeft  = static_cast<int>(ImLLx + 0.5);
    rLower = static_cast<int>(ImLLy + 0.5);
    rUpper = static_cast<int>(PDHeight - ImURy + 0.5);
    rRight = static_cast<int>(PDWidth - ImURx + 0.5);

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
    rHeight = static_cast<int>(PDHeight + 0.5);
    rWidth  = static_cast<int>(PDWidth + 0.5);

    return true;
}

OUString PPDParser::matchPaper( int nWidth, int nHeight ) const
{
    if( ! m_pPaperDimensions )
        return OUString();

    int nPDim = -1;
    double fSort = 2e36, fNewSort;

    for( int i = 0; i < m_pPaperDimensions->countValues(); i++ )
    {
        OUString aArea =  m_pPaperDimensions->getValue( i )->m_aValue;
        double PDWidth     = StringToDouble( GetCommandLineToken( 0, aArea ) );
        double PDHeight    = StringToDouble( GetCommandLineToken( 1, aArea ) );
        PDWidth     /= static_cast<double>(nWidth);
        PDHeight    /= static_cast<double>(nHeight);
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
                                        int& rXRes, int& rYRes )
{
    rXRes = rYRes = 300;

    const sal_Int32 nDPIPos {rString.indexOf( "dpi" )};
    if( nDPIPos != -1 )
    {
        const sal_Int32 nPos {rString.indexOf( 'x' )};
        if( nPos >=0 )
        {
            rXRes = rString.copy( 0, nPos ).toInt32();
            rYRes = rString.copy(nPos+1, nDPIPos - nPos - 1).toInt32();
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

OUString PPDParser::translateKey( const OUString& i_rKey ) const
{
    OUString aResult( m_pTranslator->translateKey( i_rKey ) );
    if( aResult.isEmpty() )
        aResult = i_rKey;
    return aResult;
}

OUString PPDParser::translateOption( const OUString& i_rKey,
                                          const OUString& i_rOption ) const
{
    OUString aResult( m_pTranslator->translateOption( i_rKey, i_rOption ) );
    if( aResult.isEmpty() )
        aResult = i_rOption;
    return aResult;
}

/*
 *  PPDKey
 */

PPDKey::PPDKey( const OUString& rKey ) :
        m_aKey( rKey ),
        m_pDefaultValue( nullptr ),
        m_bQueryValue( false ),
        m_bUIOption( false ),
        m_nOrderDependency( 100 ),
        m_eSetupType( SetupType::AnySetup )
{
}

PPDKey::~PPDKey()
{
}

const PPDValue* PPDKey::getValue( int n ) const
{
    return (static_cast<unsigned int>(n) < m_aOrderedValues.size() && n >= 0) ? m_aOrderedValues[n] : nullptr;
}

const PPDValue* PPDKey::getValue( const OUString& rOption ) const
{
    PPDKey::hash_type::const_iterator it = m_aValues.find( rOption );
    return it != m_aValues.end() ? &it->second : nullptr;
}

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

void PPDKey::eraseValue( const OUString& rOption )
{
    PPDKey::hash_type::iterator it = m_aValues.find( rOption );
    if( it == m_aValues.end() )
        return;

    auto vit = std::find(m_aOrderedValues.begin(), m_aOrderedValues.end(), &(it->second ));
    if( vit != m_aOrderedValues.end() )
        m_aOrderedValues.erase( vit );

    m_aValues.erase( it );
}

PPDValue* PPDKey::insertValue(const OUString& rOption, PPDValueType eType, bool bCustomOption)
{
    if( m_aValues.find( rOption ) != m_aValues.end() )
        return nullptr;

    PPDValue aValue;
    aValue.m_aOption = rOption;
    aValue.m_bCustomOption = bCustomOption;
    aValue.m_eType = eType;
    m_aValues[ rOption ] = aValue;
    PPDValue* pValue = &m_aValues[rOption];
    m_aOrderedValues.push_back( pValue );
    return pValue;
}

/*
 * PPDContext
 */

PPDContext::PPDContext() :
        m_pParser( nullptr )
{
}

PPDContext& PPDContext::operator=( const PPDContext& rCopy )
{
    m_pParser           = rCopy.m_pParser;
    m_aCurrentValues    = rCopy.m_aCurrentValues;
    return *this;
}

PPDContext& PPDContext::operator=( PPDContext&& rCopy )
{
    std::swap(m_pParser, rCopy.m_pParser);
    std::swap(m_aCurrentValues, rCopy.m_aCurrentValues);
    return *this;
}

const PPDKey* PPDContext::getModifiedKey( int n ) const
{
    if( m_aCurrentValues.size() < static_cast<hash_type::size_type>(n) )
        return nullptr;

    hash_type::const_iterator it = m_aCurrentValues.begin();
    std::advance(it, n);
    return it->first;
}

void PPDContext::setParser( const PPDParser* pParser )
{
    if( pParser != m_pParser )
    {
        m_aCurrentValues.clear();
        m_pParser = pParser;
    }
}

const PPDValue* PPDContext::getValue( const PPDKey* pKey ) const
{
    if( ! m_pParser )
        return nullptr;

    hash_type::const_iterator it;
    it = m_aCurrentValues.find( pKey );
    if( it != m_aCurrentValues.end() )
        return it->second;

    if( ! m_pParser->hasKey( pKey ) )
        return nullptr;

    const PPDValue* pValue = pKey->getDefaultValue();
    if( ! pValue )
        pValue = pKey->getValue( 0 );

    return pValue;
}

const PPDValue* PPDContext::setValue( const PPDKey* pKey, const PPDValue* pValue, bool bDontCareForConstraints )
{
    if( ! m_pParser || ! pKey )
        return nullptr;

    // pValue can be NULL - it means ignore this option

    if( ! m_pParser->hasKey( pKey ) )
        return nullptr;

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
                    SAL_INFO("vcl.unx.print", "PPDContext::setValue: option "
                         << it->first->getKey()
                         << " (" << it->second->m_aOption
                         << ") is constrained after setting "
                         << pKey->getKey()
                         << " to " << pValue->m_aOption);
                    resetValue( it->first, true );
                    it = m_aCurrentValues.begin();
                }
                else
                    ++it;
            }
        }
    }
    else
        m_aCurrentValues[ pKey ] = nullptr;

    return pValue;
}

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

bool PPDContext::resetValue( const PPDKey* pKey, bool bDefaultable )
{
    if( ! pKey || ! m_pParser || ! m_pParser->hasKey( pKey ) )
        return false;

    const PPDValue* pResetValue = pKey->getValue( OUString( "None" ) );
    if( ! pResetValue )
        pResetValue = pKey->getValue( OUString( "False" ) );
    if( ! pResetValue && bDefaultable )
        pResetValue = pKey->getDefaultValue();

    bool bRet = pResetValue && ( setValue( pKey, pResetValue ) == pResetValue );

    return bRet;
}

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
    if( pNewValue->m_aOption == "None" || pNewValue->m_aOption == "False" ||
        pNewValue == pKey->getDefaultValue() )
        return true;

    const ::std::vector< PPDParser::PPDConstraint >& rConstraints( m_pParser->getConstraints() );
    for (auto const& constraint : rConstraints)
    {
        const PPDKey* pLeft     = constraint.m_pKey1;
        const PPDKey* pRight    = constraint.m_pKey2;
        if( ! pLeft || ! pRight || ( pKey != pLeft && pKey != pRight ) )
            continue;

        const PPDKey* pOtherKey = pKey == pLeft ? pRight : pLeft;
        const PPDValue* pOtherKeyOption = pKey == pLeft ? constraint.m_pOption2 : constraint.m_pOption1;
        const PPDValue* pKeyOption = pKey == pLeft ? constraint.m_pOption1 : constraint.m_pOption2;

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
                    pOtherKeyOption->m_aOption != "None" &&
                    pOtherKeyOption->m_aOption != "False" )
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
                    pNewValue->m_aOption != "None" &&
                    pNewValue->m_aOption != "False" )
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
            if( pOtherValue->m_aOption != "None"  &&
                pOtherValue->m_aOption != "False" &&
                pNewValue->m_aOption != "None"    &&
                pNewValue->m_aOption != "False" )
                return false;
        }
    }
    return true;
}

char* PPDContext::getStreamableBuffer( sal_uLong& rBytes ) const
{
    rBytes = 0;
    if( m_aCurrentValues.empty() )
        return nullptr;
    for (auto const& elem : m_aCurrentValues)
    {
        OString aCopy(OUStringToOString(elem.first->getKey(), RTL_TEXTENCODING_MS_1252));
        rBytes += aCopy.getLength();
        rBytes += 1; // for ':'
        if( elem.second )
        {
            aCopy = OUStringToOString(elem.second->m_aOption, RTL_TEXTENCODING_MS_1252);
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
    for (auto const& elem : m_aCurrentValues)
    {
        OString aCopy(OUStringToOString(elem.first->getKey(), RTL_TEXTENCODING_MS_1252));
        int nBytes = aCopy.getLength();
        memcpy( pRun, aCopy.getStr(), nBytes );
        pRun += nBytes;
        *pRun++ = ':';
        if( elem.second )
            aCopy = OUStringToOString(elem.second->m_aOption, RTL_TEXTENCODING_MS_1252);
        else
            aCopy = "*nil";
        nBytes = aCopy.getLength();
        memcpy( pRun, aCopy.getStr(), nBytes );
        pRun += nBytes;

        *pRun++ = 0;
    }
    return pBuffer;
}

void PPDContext::rebuildFromStreamBuffer(const std::vector<char> &rBuffer)
{
    if( ! m_pParser )
        return;

    m_aCurrentValues.clear();

    const size_t nBytes = rBuffer.size() - 1;
    size_t nRun = 0;
    while (nRun < nBytes && rBuffer[nRun])
    {
        OString aLine(rBuffer.data() + nRun);
        sal_Int32 nPos = aLine.indexOf(':');
        if( nPos != -1 )
        {
            const PPDKey* pKey = m_pParser->getKey( OStringToOUString( aLine.copy( 0, nPos ), RTL_TEXTENCODING_MS_1252 ) );
            if( pKey )
            {
                const PPDValue* pValue = nullptr;
                OUString aOption(OStringToOUString(aLine.copy(nPos+1), RTL_TEXTENCODING_MS_1252));
                if (aOption != "*nil")
                    pValue = pKey->getValue( aOption );
                m_aCurrentValues[ pKey ] = pValue;
                SAL_INFO("vcl.unx.print",
                    "PPDContext::rebuildFromStreamBuffer: read PPDKeyValue { "
                    << pKey->getKey() << " , "
                    << (pValue ? aOption : "<nil>")
                    << " }");
            }
        }
        nRun += aLine.getLength()+1;
    }
}

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
                PPDParser::getResolutionFromString( pValue->m_aOption, nDPIx, nDPIy );
            else
                m_pParser->getDefaultResolution( nDPIx, nDPIy );
        }
        else
            m_pParser->getDefaultResolution( nDPIx, nDPIy );

        nDPI = std::max(nDPIx, nDPIy);
    }
    return  nDPI;
}

void PPDContext::getPageSize( OUString& rPaper, int& rWidth, int& rHeight ) const
{
    // initialize to reasonable default, if parser is not set
    rPaper  = "A4";
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
