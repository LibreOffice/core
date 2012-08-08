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

#include <osl/diagnose.h>
#include "FetcList.hxx"
#include "Fetc.hxx"
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>

#include "DataFmtTransl.hxx"
#include "../misc/ImplHelper.hxx"
#include "../misc/WinClip.hxx"

#include <algorithm>

#include "MimeAttrib.hxx"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace std;

using ::rtl::OUString;

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

LCID       CFormatRegistrar::m_TxtLocale   = 0;
sal_uInt32 CFormatRegistrar::m_TxtCodePage = GetACP( );

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtcContainer::CFormatEtcContainer( )
{
    m_EnumIterator = m_FormatMap.begin( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void CFormatEtcContainer::addFormatEtc( const CFormatEtc& fetc )
{
    m_FormatMap.push_back( CFormatEtc( fetc ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFormatEtcContainer::removeFormatEtc( const CFormatEtc& fetc )
{
    FormatEtcMap_t::iterator iter =
        find( m_FormatMap.begin(), m_FormatMap.end(), fetc );

    if ( iter != m_FormatMap.end( ) )
        m_FormatMap.erase( iter );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFormatEtcContainer::removeAllFormatEtc( )
{
    m_FormatMap.clear( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool CFormatEtcContainer::hasFormatEtc( const CFormatEtc& fetc ) const
{
    FormatEtcMap_t::const_iterator iter =
        find( m_FormatMap.begin(), m_FormatMap.end(), fetc );

    return ( iter != m_FormatMap.end( ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool CFormatEtcContainer::hasElements( ) const
{
    return !m_FormatMap.empty();
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void CFormatEtcContainer::beginEnumFormatEtc( )
{
    m_EnumIterator = m_FormatMap.begin( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_uInt32 SAL_CALL CFormatEtcContainer::nextFormatEtc( LPFORMATETC lpFetc,
                                                        sal_uInt32 aNum )
{
    OSL_ASSERT( lpFetc );
    OSL_ASSERT( !IsBadWritePtr( lpFetc, sizeof( FORMATETC ) * aNum ) );

    sal_uInt32 nFetched = 0;

    if ( m_EnumIterator != m_FormatMap.end( ) )
    {
        for ( sal_uInt32 i = 0; i < aNum; i++, nFetched++, lpFetc++, ++m_EnumIterator )
            CopyFormatEtc( lpFetc, *m_EnumIterator );
    }

    return nFetched;
}


//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CFormatEtcContainer::skipFormatEtc( sal_uInt32 aNum )
{
    FormatEtcMap_t::const_iterator iter_end = m_FormatMap.end( );
    for ( sal_uInt32 i = 0;
          (i < aNum) && (m_EnumIterator != iter_end);
          i++, ++m_EnumIterator )
        ;/* intentionally left empty */

    return ( m_EnumIterator != m_FormatMap.end( ) );
}


//#########################################################################


//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatRegistrar::CFormatRegistrar( const Reference< XMultiServiceFactory >& ServiceManager,
                                    const CDataFormatTranslator& aDataFormatTranslator ) :
    m_DataFormatTranslator( aDataFormatTranslator ),
    m_bHasSynthesizedLocale( sal_False ),
    m_SrvMgr( ServiceManager )
{
}

// ----------------------------------------------------------------------------------------
// this function converts all DataFlavors of the given FlavorList into
// an appropriate FORMATETC structure, for some formats like unicodetext,
// text and text/html we will offer an accompany format e.g.:
//
// DataFlavor               | Registered Clipformat     |   Registered accompany clipformat
// -------------------------|---------------------------|-----------------------------------
// text/plain;charset=ansi  | CF_TEXT                   |   CF_UNICODETEXT
//                          |                           |   CF_LOCALE (if charset != GetACP()
//                          |                           |
// text/plain;charset=oem   | CF_OEMTEXT                |   CF_UNICODETEXT
//                          |                           |   CF_LOCALE (if charset != GetOEMCP()
//                          |                           |
// text/plain;charset=utf-16| CF_UNICODETEXT            |   CF_TEXT
//                          |                           |
// text/html                | HTML (Hypertext ...)      |   HTML Format
//                          |                           |
//
// if some tries to register different text formats with different charsets the last
// registered wins and the others are ignored
// ----------------------------------------------------------------------------------------

void SAL_CALL CFormatRegistrar::RegisterFormats(
    const Reference< XTransferable >& aXTransferable, CFormatEtcContainer& aFormatEtcContainer )
{
    Sequence< DataFlavor > aFlavorList = aXTransferable->getTransferDataFlavors( );
    sal_Int32  nFlavors                = aFlavorList.getLength( );
    sal_Bool   bUnicodeRegistered      = sal_False;
    DataFlavor aFlavor;

    for( sal_Int32 i = 0; i < nFlavors; i++ )
    {
        aFlavor = aFlavorList[i];
        CFormatEtc fetc = m_DataFormatTranslator.getFormatEtcFromDataFlavor( aFlavor );

        // maybe an internal format so we ignore it
        if ( CF_INVALID == fetc.getClipformat( ) )
            continue;

        if ( !needsToSynthesizeAccompanyFormats( fetc ) )
            aFormatEtcContainer.addFormatEtc( fetc );
        else
        {
            // if we haven't registered any text format up to now
            if ( m_DataFormatTranslator.isTextFormat( fetc.getClipformat() ) && !bUnicodeRegistered )
            {
                // if the transferable supports unicode text we ignore
                // any further text format the transferable offers
                // because we can create it from Unicode text in addition
                // we register CF_TEXT for non unicode clients
                if ( m_DataFormatTranslator.isUnicodeTextFormat( fetc.getClipformat() ) )
                {
                    aFormatEtcContainer.addFormatEtc( fetc ); // add CF_UNICODE
                    aFormatEtcContainer.addFormatEtc(
                        m_DataFormatTranslator.getFormatEtcForClipformat( CF_TEXT ) ); // add CF_TEXT
                    bUnicodeRegistered = sal_True;
                }
                else if ( !hasUnicodeFlavor( aXTransferable ) )
                {
                    // we try to investigate the charset and make a valid
                    // windows codepage from this charset the default
                    // return value is the result of GetACP( )
                    OUString charset = getCharsetFromDataFlavor( aFlavor );
                    sal_uInt32 txtCP = getWinCPFromMimeCharset( charset );

                    // we try to get a Locale appropriate for this codepage
                    if ( findLocaleForTextCodePage( ) )
                    {
                        m_TxtCodePage = txtCP;

                        aFormatEtcContainer.addFormatEtc(
                            m_DataFormatTranslator.getFormatEtcForClipformat( CF_UNICODETEXT ) );

                        if ( !IsOEMCP( m_TxtCodePage ) )
                            aFormatEtcContainer.addFormatEtc(
                                m_DataFormatTranslator.getFormatEtcForClipformat( CF_TEXT ) );
                        else
                            aFormatEtcContainer.addFormatEtc(
                                m_DataFormatTranslator.getFormatEtcForClipformat( CF_OEMTEXT ) );

                        aFormatEtcContainer.addFormatEtc(
                            m_DataFormatTranslator.getFormatEtcForClipformat( CF_LOCALE ) );

                        // we save the flavor so it's easier when
                        // queried for it in XTDataObject::GetData(...)
                        m_RegisteredTextFlavor  = aFlavor;
                        m_bHasSynthesizedLocale = sal_True;
                    }
                }
            }
            else if ( m_DataFormatTranslator.isTextHtmlFormat( fetc.getClipformat( ) ) ) // Html (Hyper Text...)
            {
                // we add text/html ( HTML (HyperText Markup Language) )
                aFormatEtcContainer.addFormatEtc( fetc );

                // and HTML Format
                OUString htmlFormat( "HTML Format" );
                aFormatEtcContainer.addFormatEtc(
                    m_DataFormatTranslator.getFormatEtcForClipformatName( htmlFormat ) );
            }
        }
    }
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CFormatRegistrar::hasSynthesizedLocale( ) const
{
    return m_bHasSynthesizedLocale;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

LCID SAL_CALL CFormatRegistrar::getSynthesizedLocale( ) const
{
    return m_TxtLocale;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_uInt32 SAL_CALL CFormatRegistrar::getRegisteredTextCodePage( ) const
{
    return m_TxtCodePage;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

DataFlavor SAL_CALL CFormatRegistrar::getRegisteredTextFlavor( ) const
{
    return m_RegisteredTextFlavor;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CFormatRegistrar::isSynthesizeableFormat( const CFormatEtc& aFormatEtc ) const
{
    return ( m_DataFormatTranslator.isOemOrAnsiTextFormat( aFormatEtc.getClipformat() ) ||
             m_DataFormatTranslator.isUnicodeTextFormat( aFormatEtc.getClipformat() ) ||
             m_DataFormatTranslator.isHTMLFormat( aFormatEtc.getClipformat() ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
sal_Bool SAL_CALL CFormatRegistrar::needsToSynthesizeAccompanyFormats( const CFormatEtc& aFormatEtc ) const
{
    return ( m_DataFormatTranslator.isOemOrAnsiTextFormat( aFormatEtc.getClipformat() ) ||
             m_DataFormatTranslator.isUnicodeTextFormat( aFormatEtc.getClipformat() ) ||
             m_DataFormatTranslator.isTextHtmlFormat( aFormatEtc.getClipformat( ) ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CFormatRegistrar::getCharsetFromDataFlavor( const DataFlavor& aFlavor )
{
    OUString charset;

    try
    {
        Reference< XMimeContentTypeFactory > xMimeFac(
            m_SrvMgr->createInstance( OUString( "com.sun.star.datatransfer.MimeContentTypeFactory" ) ), UNO_QUERY );

        if( xMimeFac.is( ) )
        {
            Reference< XMimeContentType > xMimeType( xMimeFac->createMimeContentType( aFlavor.MimeType ) );
            if ( xMimeType->hasParameter( TEXTPLAIN_PARAM_CHARSET ) )
                charset = xMimeType->getParameterValue( TEXTPLAIN_PARAM_CHARSET );
            else
                charset = getMimeCharsetFromWinCP( GetACP( ), PRE_WINDOWS_CODEPAGE );
        }
    }
    catch(NoSuchElementException&)
    {
        OSL_FAIL( "Unexpected" );
    }
    catch(...)
    {
        OSL_FAIL( "Invalid data flavor" );
    }

    return charset;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CFormatRegistrar::hasUnicodeFlavor( const Reference< XTransferable >& aXTransferable ) const
{
    CFormatEtc fetc( CF_UNICODETEXT );

    DataFlavor aFlavor =
        m_DataFormatTranslator.getDataFlavorFromFormatEtc( fetc );

    return aXTransferable->isDataFlavorSupported( aFlavor );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
sal_Bool CFormatRegistrar::isEqualCurrentSystemCodePage( sal_uInt32 aCodePage ) const
{
    return ( (aCodePage == GetOEMCP()) || (aCodePage == GetACP()) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CFormatRegistrar::findLocaleForTextCodePage( )
{
    m_TxtLocale = 0;
    EnumSystemLocalesA( CFormatRegistrar::EnumLocalesProc, LCID_INSTALLED );
    return ( IsValidLocale( m_TxtLocale, LCID_INSTALLED ) ) ? sal_True : sal_False;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CFormatRegistrar::isLocaleCodePage( LCID lcid, LCTYPE lctype, sal_uInt32 codepage )
{
    char  buff[6];
    sal_uInt32 localeCodePage;

    OSL_ASSERT( IsValidLocale( lcid, LCID_INSTALLED ) );

    // get the ansi codepage of the current locale
    GetLocaleInfoA( lcid, lctype, buff, sizeof( buff ) );
    localeCodePage = atol( buff );

    return ( localeCodePage == codepage );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
sal_Bool SAL_CALL CFormatRegistrar::isLocaleOemCodePage( LCID lcid, sal_uInt32 codepage )
{
    return isLocaleCodePage( lcid, LOCALE_IDEFAULTCODEPAGE, codepage );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
sal_Bool SAL_CALL CFormatRegistrar::isLocaleAnsiCodePage( LCID lcid, sal_uInt32 codepage )
{
    return isLocaleCodePage( lcid, LOCALE_IDEFAULTANSICODEPAGE, codepage );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

BOOL CALLBACK CFormatRegistrar::EnumLocalesProc( LPSTR lpLocaleStr )
{
    // the lpLocaleStr parametere is hexadecimal
    LCID lcid = strtol( lpLocaleStr, NULL, 16 );

    if ( isLocaleAnsiCodePage( lcid, CFormatRegistrar::m_TxtCodePage ) ||
         isLocaleOemCodePage( lcid, CFormatRegistrar::m_TxtCodePage ) )
    {
        CFormatRegistrar::m_TxtLocale = lcid;
        return sal_False; // stop enumerating
    }

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
