/*************************************************************************
 *
 *  $RCSfile: DataFmtTransl.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2001-02-27 07:52:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _DATAFMTTRANSL_HXX_
#include "DataFmtTransl.hxx"
#endif

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#ifndef _IMPLHELPER_HXX_
#include "..\misc\ImplHelper.hxx"
#endif

#ifndef _WINCLIP_HXX_
#include "..\misc\WinClip.hxx"
#endif

#ifndef _MIMEATTRIB_HXX_
#include "MimeAttrib.hxx"
#endif

#ifndef _DTRANSHELPER_HXX_
#include "DTransHelper.hxx"
#endif

#ifndef _RTL_STRING_H_
#include <rtl/string.h>
#endif

#include <windows.h>
#include <olestd.h>

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace rtl;
using namespace std;
using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::lang;

//------------------------------------------------------------------------
// const
//------------------------------------------------------------------------

const Type       CPPUTYPE_SALINT32   = getCppuType((sal_Int32*)0);
const Type       CPPUTYPE_SALINT8    = getCppuType((sal_Int8*)0);
const Type       CPPUTYPE_OUSTRING   = getCppuType((OUString*)0);
const Type       CPPUTYPE_SEQSALINT8 = getCppuType((Sequence< sal_Int8>*)0);
const sal_Int32  MAX_CLIPFORMAT_NAME = 256;

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CDataFormatTranslator::CDataFormatTranslator( const Reference< XMultiServiceFactory >& aServiceManager ) :
    m_SrvMgr( aServiceManager )
{
    m_XDataFormatTranslator = Reference< XDataFormatTranslator >(
        m_SrvMgr->createInstance( OUString::createFromAscii( "com.sun.star.datatransfer.DataFormatTranslator" ) ), UNO_QUERY );

    OSL_ASSERT( !m_XDataFormatTranslator.is( ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

FORMATETC CDataFormatTranslator::getFormatEtcFromDataFlavor( const DataFlavor& aDataFlavor ) const
{
    CLIPFORMAT cf = CF_INVALID;

    try
    {
        Any aFormat = m_XDataFormatTranslator->getSystemDataTypeFromDataFlavor( aDataFlavor );

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
            cf = RegisterClipboardFormatW( aClipFmtName.getStr( ) );

            OSL_ENSURE( CF_INVALID != cf, "RegisterClipboardFormat failed" );
        }
        else
            OSL_ENSURE( sal_False, "Wrong Any-Type detected" );
    }
    catch( ... )
    {
        OSL_ENSURE( sal_False, "Unexpected error" );
    }

    return getFormatEtcForClipformat( cf );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

DataFlavor CDataFormatTranslator::getDataFlavorFromFormatEtc( const Reference< XTransferable >& refXTransferable, const FORMATETC& aFormatEtc ) const
{
    DataFlavor aFlavor;

    try
    {
        CLIPFORMAT aClipformat = aFormatEtc.cfFormat;

        Any aAny;
        aAny <<= static_cast< sal_Int32 >( aClipformat );

        if ( isOemOrAnsiTextClipformat( aClipformat ) )
        {
            aFlavor.MimeType             = OUString::createFromAscii( "text/plain;charset=" );
            aFlavor.MimeType            += getTextCharsetFromClipboard( refXTransferable, aClipformat );

            aFlavor.HumanPresentableName = OUString::createFromAscii( "OEM/ANSI Text" );
            aFlavor.DataType             = CPPUTYPE_SALINT8;
        }
        else
        {
            aFlavor = m_XDataFormatTranslator->getDataFlavorFromSystemDataType( aAny );

            if ( !aFlavor.MimeType.getLength( ) )
            {
                aAny <<= getClipboardFormatName( aClipformat );
                aFlavor = m_XDataFormatTranslator->getDataFlavorFromSystemDataType( aAny );
            }
        }
    }
    catch( ... )
    {
        OSL_ENSURE( sal_False, "Unexpected error" );
    }

    return aFlavor;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString CDataFormatTranslator::getClipboardFormatName( CLIPFORMAT aClipformat ) const
{
    OSL_PRECOND( CF_INVALID != aClipformat, "Invalid clipboard format" );

    sal_Unicode wBuff[ MAX_CLIPFORMAT_NAME ];
    sal_Int32   nLen = GetClipboardFormatNameW( aClipformat, wBuff, MAX_CLIPFORMAT_NAME );

    return OUString( wBuff, nLen );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

FORMATETC SAL_CALL CDataFormatTranslator::getFormatEtcForClipformat( CLIPFORMAT cf ) const
{
    FORMATETC fetc;

    fetc.cfFormat = cf;
    fetc.ptd      = NULL;
    fetc.dwAspect = DVASPECT_CONTENT;
    fetc.lindex   = -1;

    switch( cf )
    {
    case CF_METAFILEPICT:
        fetc.tymed = TYMED_MFPICT;
        break;

    case CF_ENHMETAFILE:
        fetc.tymed = TYMED_ENHMF;
        break;

    default:
        fetc.tymed = TYMED_HGLOBAL | TYMED_ISTREAM;
    }

    return fetc;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
sal_Bool SAL_CALL CDataFormatTranslator::isOemOrAnsiTextClipformat( CLIPFORMAT aClipformat ) const
{
    return ( (aClipformat == CF_TEXT) ||
             (aClipformat == CF_OEMTEXT) );
}

//------------------------------------------------------------------------
// helper function
//------------------------------------------------------------------------

OUString SAL_CALL CDataFormatTranslator::getCodePageFromLocaleId( LCID locale, LCTYPE lcType ) const
{
    OSL_ASSERT( IsValidLocale( locale, LCID_SUPPORTED ) );

    // we use the GetLocaleInfoA because don't want to provide
    // a unicode wrapper function for Win9x in sal/systools
    char buff[6];
    GetLocaleInfoA( locale, lcType, buff, sizeof( buff ) );
    rtl_TextEncoding tenc = rtl_getTextEncodingFromPCCodePage( CP_ACP );

    return OUString( buff, rtl_str_getLength( buff ), tenc );
}

//------------------------------------------------------------------------
// should be called only if there is realy text on the clipboard
//------------------------------------------------------------------------

LCID SAL_CALL CDataFormatTranslator::getCurrentLocaleFromClipboard(
    const Reference< XTransferable >& refXTransferable ) const
{
    Any       aAny;
    FORMATETC fetc;

    fetc.cfFormat = CF_LOCALE;
    DataFlavor aFlavor = getDataFlavorFromFormatEtc( refXTransferable, fetc );

    OSL_ASSERT( aFlavor.MimeType.getLength( ) );

    LCID lcid;

    try
    {
        if ( refXTransferable->isDataFlavorSupported( aFlavor ) )
        {
            aAny = refXTransferable->getTransferData( aFlavor );
            if ( aAny.hasValue( ) )
            {
                OSL_ASSERT( aAny.getValueType( ) == CPPUTYPE_SEQSALINT8 );
                Sequence< sal_Int8 > seqSalInt8;
                aAny >>= seqSalInt8;

                lcid = *reinterpret_cast< sal_Int32* >( seqSalInt8.getArray( ) );
            }
        }
    }
    catch( ... )
    {
        OSL_ENSURE( sal_False, "Unexpected error" );
    }

    if ( !IsValidLocale( lcid, LCID_SUPPORTED ) )
            lcid = GetThreadLocale( );

    return lcid;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
OUString SAL_CALL CDataFormatTranslator::makeMimeCharsetFromLocaleId(
    LCID lcid, LCTYPE lcType, const OUString& aCharsetValuePrefix ) const
{
    OUString charset = getCodePageFromLocaleId( lcid, lcType );
    return OUString::createFromAscii( "charset=" ) + charset;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CDataFormatTranslator::getTextCharsetFromClipboard(
    const Reference< XTransferable >& refXTransferable, CLIPFORMAT aClipformat ) const
{
    OSL_ASSERT( isOemOrAnsiTextClipformat( aClipformat ) );

    OUString charset;
    if ( CF_TEXT == aClipformat )
    {
        LCID lcid = getCurrentLocaleFromClipboard( refXTransferable );
        charset = makeMimeCharsetFromLocaleId(
                    lcid,
                    LOCALE_IDEFAULTANSICODEPAGE,
                    PRE_WINDOWS_CODEPAGE );
    }
    else if ( CF_OEMTEXT == aClipformat )
    {
        LCID lcid  = getCurrentLocaleFromClipboard( refXTransferable );
        charset = makeMimeCharsetFromLocaleId(
                    lcid,
                    LOCALE_IDEFAULTCODEPAGE,
                    PRE_OEM_CODEPAGE );
    }
    else // CF_UNICODE
        OSL_ASSERT( sal_False );

    return charset;
}