/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>
#include <o3tl/string_view.hxx>

#include <memory>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string_view>

#include <export.hxx>
#include <common.hxx>
#include <propmerge.hxx>
#include <utility>

namespace
{
    //Find ascii escaped unicode
    sal_Int32 lcl_IndexOfUnicode(
        std::string_view rSource, const sal_Int32 nFrom = 0 )
    {
        const OString sHexDigits = "0123456789abcdefABCDEF";
        size_t nIndex = rSource.find( "\\u", nFrom );
        if( nIndex == std::string_view::npos )
        {
            return -1;
        }
        bool bIsUnicode = true;
        for( short nDist = 2; nDist <= 5; ++nDist )
        {
            if( sHexDigits.indexOf( rSource[nIndex + nDist] ) == -1 )
            {
                bIsUnicode = false;
            }
        }
        return bIsUnicode ? nIndex : -1;
    }

    //Convert ascii escaped unicode to utf-8
    OString lcl_ConvertToUTF8( const OString& rText )
    {
        OString sResult = rText;
        sal_Int32 nIndex = lcl_IndexOfUnicode( sResult );
        while( nIndex != -1 && nIndex < rText.getLength() )
        {
            const OString sHex = sResult.copy( nIndex + 2, 4 );
            const sal_Unicode cDec =
                static_cast<sal_Unicode>( strtol( sHex.getStr(), nullptr, 16 ) );
            const OString sNewChar( &cDec, 1, RTL_TEXTENCODING_UTF8 );
            sResult = sResult.replaceAll( "\\u" + sHex, sNewChar );
            nIndex = lcl_IndexOfUnicode( sResult, nIndex );
        }
        return sResult;
    }

    //Escape unicode characters
    void lcl_PrintJavaStyle( std::string_view rText, std::ofstream &rOfstream )
    {
        const OUString sTemp =
            OStringToOUString( rText, RTL_TEXTENCODING_UTF8 );
        for ( sal_Int32 nIndex = 0; nIndex < sTemp.getLength(); ++nIndex )
        {
            sal_Unicode cUniCode = sTemp[nIndex];
            if( cUniCode < 128 )
            {
                rOfstream << static_cast<char>( cUniCode );
            }
            else
            {
                rOfstream
                    << "\\u"
                    << std::setfill('0') << std::setw(2) << std::uppercase
                    << std::hex << (cUniCode >> 8)
                    << std::setfill('0') << std::setw(2) << std::uppercase
                    << std::hex << (cUniCode & 0xFF);
            }
        }
    }
}

//Open source file and store its lines
PropParser::PropParser(
    OString _sInputFile, OString _sLang,
    const bool bMergeMode )
    : m_sSource(std::move( _sInputFile ))
    , m_sLang(std::move( _sLang ))
    , m_bIsInitialized( false )
{
    std::ifstream aIfstream( m_sSource.getStr() );
    if( aIfstream.is_open() )
    {
        std::string s;
        std::getline( aIfstream, s );
        while( !aIfstream.eof() )
        {
            OString sLine( s.data(), s.length() );
            if( bMergeMode ||
                ( !sLine.startsWith(" *") && !sLine.startsWith("/*") ) )
            {
                m_vLines.push_back( sLine );
            }
            std::getline( aIfstream, s );
        }
    }
    else
    {
        std::cerr
            << "Propex error: Cannot open source file: "
            << m_sSource << std::endl;
        return;
    }
    m_bIsInitialized = true;
}

PropParser::~PropParser()
{
}

//Extract strings form source file
void PropParser::Extract( const OString& rPOFile )
{
    assert( m_bIsInitialized );
    PoOfstream aPOStream( rPOFile, PoOfstream::APP );
    if( !aPOStream.isOpen() )
    {
        std::cerr
            << "Propex error: Cannot open pofile for extract: "
            << rPOFile << std::endl;
        return;
    }

    for( size_t nIndex = 0; nIndex < m_vLines.size(); ++nIndex )
    {
        const OString sLine = m_vLines[nIndex];
        const sal_Int32 nEqualSign = sLine.indexOf('=');
        if( nEqualSign != -1 )
        {
            std::string_view sID = o3tl::trim(sLine.subView( 0, nEqualSign ));
            OString sText = lcl_ConvertToUTF8( OString(o3tl::trim(sLine.subView( nEqualSign + 1 ))) );

            common::writePoEntry(
                "Propex", aPOStream, m_sSource, "property",
                OString(sID), OString(), OString(), sText);
        }
    }

    aPOStream.close();
}

//Merge strings to source file
void PropParser::Merge( const OString &rMergeSrc, const OString &rDestinationFile )
{
    assert( m_bIsInitialized );
    std::ofstream aDestination(
        rDestinationFile.getStr(), std::ios_base::out | std::ios_base::trunc );
    if( !aDestination.is_open() ) {
        std::cerr
            << "Propex error: Cannot open source file for merge: "
            << rDestinationFile << std::endl;
        return;
    }

    std::unique_ptr<MergeDataFile> pMergeDataFile;
    if( m_sLang != "qtz" )
    {
        pMergeDataFile.reset( new MergeDataFile( rMergeSrc, m_sSource, false, false ) );

        const std::vector<OString> vLanguages = pMergeDataFile->GetLanguages();
        if( !vLanguages.empty() && vLanguages[0] != m_sLang )
        {
            std::cerr
                << ("Propex error: given language conflicts with language of"
                    " Mergedata file: ")
                << m_sLang << " - "
                << vLanguages[0] << std::endl;
            return;
        }
    }

    for( size_t nIndex = 0; nIndex < m_vLines.size(); ++nIndex )
    {
        const OString sLine = m_vLines[nIndex];
        const sal_Int32 nEqualSign = sLine.indexOf('=');
        if( !sLine.startsWith(" *") && !sLine.startsWith("/*") &&
            nEqualSign != -1 )
        {
            const OString sID( o3tl::trim(sLine.subView( 0, sLine.indexOf('=') )) );
            ResData  aResData( sID, m_sSource );
            aResData.sResTyp = "property";
            OString sNewText;
            if( m_sLang == "qtz" )
            {
                const OString sOriginText = lcl_ConvertToUTF8(OString(o3tl::trim(sLine.subView( nEqualSign + 1 ))));
                sNewText = MergeEntrys::GetQTZText(aResData, sOriginText);
            }
            else if( pMergeDataFile )
            {
                MergeEntrys* pEntrys = pMergeDataFile->GetMergeEntrys( &aResData );
                if( pEntrys )
                {
                    pEntrys->GetText( sNewText, m_sLang );
                }
            }
            if( !sNewText.isEmpty() )
            {
                aDestination << OString(sID + "=");
                lcl_PrintJavaStyle( sNewText, aDestination );
                aDestination << std::endl;
            }
            else
            {
                aDestination << sLine << std::endl;
            }
        }
        else
        {
            aDestination << sLine << std::endl;
        }
    }
    aDestination.close();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
