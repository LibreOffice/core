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

#include "DataFmtTransl.hxx"
#include <rtl/string.hxx>
#include <osl/diagnose.h>
#include <rtl/tencinfo.h>
#include "../misc/ImplHelper.hxx"
#include "../misc/WinClip.hxx"
#include "MimeAttrib.hxx"
#include "DTransHelper.hxx"
#include <rtl/string.h>
#include "Fetc.hxx"

#if defined _MSC_VER
#pragma warning(push,1)
#pragma warning(disable:4917)
#endif
#include <windows.h>
#if (_MSC_VER < 1300) && !defined(__MINGW32__)
#include <olestd.h>
#endif
#include <shlobj.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif


//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace std;
using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::lang;

using ::rtl::OUString;

//------------------------------------------------------------------------
// const
//------------------------------------------------------------------------

const Type       CPPUTYPE_SALINT32   = getCppuType((sal_Int32*)0);
const Type       CPPUTYPE_SALINT8    = getCppuType((sal_Int8*)0);
const Type       CPPUTYPE_OUSTRING   = getCppuType((OUString*)0);
const Type       CPPUTYPE_SEQSALINT8 = getCppuType((Sequence< sal_Int8>*)0);
const sal_Int32  MAX_CLIPFORMAT_NAME = 256;

const OUString TEXT_PLAIN_CHARSET   ("text/plain;charset=");
const OUString HPNAME_OEM_ANSI_TEXT ("OEM/ANSI Text");

const OUString HTML_FORMAT_NAME_WINDOWS ("HTML Format");
const OUString HTML_FORMAT_NAME_SOFFICE ("HTML (HyperText Markup Language)");

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CDataFormatTranslator::CDataFormatTranslator( const Reference< XMultiServiceFactory >& aServiceManager ) :
    m_SrvMgr( aServiceManager )
{
    m_XDataFormatTranslator = Reference< XDataFormatTranslator >(
        m_SrvMgr->createInstance( OUString("com.sun.star.datatransfer.DataFormatTranslator") ), UNO_QUERY );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc CDataFormatTranslator::getFormatEtcFromDataFlavor( const DataFlavor& aDataFlavor ) const
{
    sal_Int32 cf = CF_INVALID;

    try
    {
        if( m_XDataFormatTranslator.is( ) )
        {
            Any aFormat = m_XDataFormatTranslator->getSystemDataTypeFromDataFlavor( aDataFlavor );

            if ( aFormat.hasValue( ) )
            {
                if ( aFormat.getValueType( ) == CPPUTYPE_SALINT32 )
                {
                    aFormat >>= cf;
                    OSL_ENSURE( CF_INVALID != cf, "Invalid Clipboard format delivered" );
                }
                else if ( aFormat.getValueType( ) == CPPUTYPE_OUSTRING )
                {
                    OUString aClipFmtName;
                    aFormat >>= aClipFmtName;

                    OSL_ASSERT( aClipFmtName.getLength( ) );
                    cf = RegisterClipboardFormatW( reinterpret_cast<LPCWSTR>(aClipFmtName.getStr( )) );

                    OSL_ENSURE( CF_INVALID != cf, "RegisterClipboardFormat failed" );
                }
                else
                    OSL_FAIL( "Wrong Any-Type detected" );
            }
        }
    }
    catch( ... )
    {
        OSL_FAIL( "Unexpected error" );
    }

    return sal::static_int_cast<CFormatEtc>(getFormatEtcForClipformat( sal::static_int_cast<CLIPFORMAT>(cf) ));
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

DataFlavor CDataFormatTranslator::getDataFlavorFromFormatEtc( const FORMATETC& aFormatEtc, LCID lcid ) const
{
    DataFlavor aFlavor;

    try
    {
        CLIPFORMAT aClipformat = aFormatEtc.cfFormat;

        Any aAny;
        aAny <<= static_cast< sal_Int32 >( aClipformat );

        if ( isOemOrAnsiTextFormat( aClipformat ) )
        {
            aFlavor.MimeType             = TEXT_PLAIN_CHARSET;
            aFlavor.MimeType            += getTextCharsetFromLCID( lcid, aClipformat );

            aFlavor.HumanPresentableName = HPNAME_OEM_ANSI_TEXT;
            aFlavor.DataType             = CPPUTYPE_SEQSALINT8;
        }
        else if ( CF_INVALID != aClipformat )
        {
            if ( m_XDataFormatTranslator.is( ) )
            {
                aFlavor = m_XDataFormatTranslator->getDataFlavorFromSystemDataType( aAny );

                if ( !aFlavor.MimeType.getLength( ) )
                {
                    // lookup of DataFlavor from clipboard format id
                    // failed, so we try to resolve via clipboard
                    // format name
                    OUString clipFormatName = getClipboardFormatName( aClipformat );

                    // if we could not get a clipboard format name an
                    // error must have occurred or it is a standard
                    // clipboard format that we don't translate, e.g.
                    // CF_BITMAP (the office only uses CF_DIB)
                    if ( clipFormatName.getLength( ) )
                    {
                        aAny <<= clipFormatName;
                        aFlavor = m_XDataFormatTranslator->getDataFlavorFromSystemDataType( aAny );
                    }
                }
            }
        }
    }
    catch( ... )
    {
        OSL_FAIL( "Unexpected error" );
    }

    return aFlavor;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc SAL_CALL CDataFormatTranslator::getFormatEtcForClipformatName( const OUString& aClipFmtName ) const
{
    // check parameter
    if ( !aClipFmtName.getLength( ) )
        return CFormatEtc( CF_INVALID );

    CLIPFORMAT cf = sal::static_int_cast<CLIPFORMAT>(RegisterClipboardFormatW( reinterpret_cast<LPCWSTR>(aClipFmtName.getStr( )) ));
    return getFormatEtcForClipformat( cf );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString CDataFormatTranslator::getClipboardFormatName( CLIPFORMAT aClipformat ) const
{
    OSL_PRECOND( CF_INVALID != aClipformat, "Invalid clipboard format" );

    sal_Unicode wBuff[ MAX_CLIPFORMAT_NAME ];
    sal_Int32   nLen = GetClipboardFormatNameW( aClipformat, reinterpret_cast<LPWSTR>(wBuff), MAX_CLIPFORMAT_NAME );

    return OUString( wBuff, nLen );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc SAL_CALL CDataFormatTranslator::getFormatEtcForClipformat( CLIPFORMAT cf ) const
{
    CFormatEtc fetc( cf, TYMED_NULL, NULL, DVASPECT_CONTENT );

    switch( cf )
    {
    case CF_METAFILEPICT:
        fetc.setTymed( TYMED_MFPICT );
        break;

    case CF_ENHMETAFILE:
        fetc.setTymed( TYMED_ENHMF );
        break;

    default:
        fetc.setTymed( TYMED_HGLOBAL /*| TYMED_ISTREAM*/ );
    }

    /*
        hack: in order to paste urls copied by Internet Explorer
        with "copy link" we set the lindex member to 0
        but if we really want to support CFSTR_FILECONTENT and
        the accompany format CFSTR_FILEDESCRIPTOR (FileGroupDescriptor)
        the client of the clipboard service has to provide a id
        of which FileContents it wants to paste
        see MSDN: "Handling Shell Data Transfer Scenarios"
    */
    if ( cf == RegisterClipboardFormatA( CFSTR_FILECONTENTS ) )
         fetc.setLindex( 0 );

    return fetc;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CDataFormatTranslator::isOemOrAnsiTextFormat( CLIPFORMAT cf ) const
{
    return ( (cf == CF_TEXT) || (cf == CF_OEMTEXT) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CDataFormatTranslator::isUnicodeTextFormat( CLIPFORMAT cf ) const
{
    return ( cf == CF_UNICODETEXT );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CDataFormatTranslator::isTextFormat( CLIPFORMAT cf ) const
{
    return ( isOemOrAnsiTextFormat( cf ) || isUnicodeTextFormat( cf ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CDataFormatTranslator::isHTMLFormat( CLIPFORMAT cf ) const
{
    OUString clipFormatName = getClipboardFormatName( cf );
    return ( clipFormatName == HTML_FORMAT_NAME_WINDOWS );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CDataFormatTranslator::isTextHtmlFormat( CLIPFORMAT cf ) const
{
    OUString clipFormatName = getClipboardFormatName( cf );
    return ( clipFormatName.equalsIgnoreAsciiCase( HTML_FORMAT_NAME_SOFFICE ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CDataFormatTranslator::getTextCharsetFromLCID( LCID lcid, CLIPFORMAT aClipformat ) const
{
    OSL_ASSERT( isOemOrAnsiTextFormat( aClipformat ) );

    OUString charset;
    if ( CF_TEXT == aClipformat )
    {
        charset = getMimeCharsetFromLocaleId(
                    lcid,
                    LOCALE_IDEFAULTANSICODEPAGE,
                    PRE_WINDOWS_CODEPAGE );
    }
    else if ( CF_OEMTEXT == aClipformat )
    {
        charset = getMimeCharsetFromLocaleId(
                    lcid,
                    LOCALE_IDEFAULTCODEPAGE,
                    PRE_OEM_CODEPAGE );
    }
    else // CF_UNICODE
        OSL_ASSERT( sal_False );

    return charset;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
