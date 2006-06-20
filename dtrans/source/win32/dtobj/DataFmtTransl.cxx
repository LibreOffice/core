/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataFmtTransl.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:04:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _FETC_HXX_
#include "Fetc.hxx"
#endif

#if defined _MSC_VER
#pragma warning(push,1)
#pragma warning(disable:4917)
#endif
#include <windows.h>
#if (_MSC_VER < 1300)
#include <olestd.h>
#endif
#include <shlobj.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif


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

const OUString TEXT_PLAIN_CHARSET   = OUString::createFromAscii( "text/plain;charset=" );
const OUString HPNAME_OEM_ANSI_TEXT = OUString::createFromAscii( "OEM/ANSI Text" );

const OUString HTML_FORMAT_NAME_WINDOWS = OUString::createFromAscii( "HTML Format" );
const OUString HTML_FORMAT_NAME_SOFFICE = OUString::createFromAscii( "HTML (HyperText Markup Language)" );

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CDataFormatTranslator::CDataFormatTranslator( const Reference< XMultiServiceFactory >& aServiceManager ) :
    m_SrvMgr( aServiceManager )
{
    m_XDataFormatTranslator = Reference< XDataFormatTranslator >(
        m_SrvMgr->createInstance( OUString::createFromAscii( "com.sun.star.datatransfer.DataFormatTranslator" ) ), UNO_QUERY );
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
                    cf = RegisterClipboardFormatW( aClipFmtName.getStr( ) );

                    OSL_ENSURE( CF_INVALID != cf, "RegisterClipboardFormat failed" );
                }
                else
                    OSL_ENSURE( sal_False, "Wrong Any-Type detected" );
            }
        }
    }
    catch( ... )
    {
        OSL_ENSURE( sal_False, "Unexpected error" );
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
                    // error must have occured or it is a standard
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
        OSL_ENSURE( sal_False, "Unexpected error" );
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

    CLIPFORMAT cf = sal::static_int_cast<CLIPFORMAT>(RegisterClipboardFormatW( aClipFmtName.getStr( ) ));
    return getFormatEtcForClipformat( cf );
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
