/*************************************************************************
 *
 *  $RCSfile: FetcList.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-19 09:11:24 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _FETCLIST_HXX_
#include "FetcList.hxx"
#endif

#ifndef _FETC_HXX_
#include "Fetc.hxx"
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPEFACTORY_HPP_
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPE_HPP_
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#endif

#ifndef _DATAFORMATTRANSLATOR_HXX_
#include "DataFmtTransl.hxx"
#endif

#ifndef _IMPLHELPER_HXX_
#include "..\misc\ImplHelper.hxx"
#endif

#include <algorithm>

#include "MimeAttrib.hxx"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace rtl;
using namespace std;

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

LCID       CFormatRegistrar::m_TxtLocale           = 0;
sal_uInt32 CFormatRegistrar::m_TxtCodePage         = GetACP( );

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
    return ( m_FormatMap.size( ) > 0 );
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
    const Sequence< DataFlavor >& aFlavorList, CFormatEtcContainer& aFormatEtcContainer )
{
    sal_Int32  nFlavors = aFlavorList.getLength( );
    sal_Bool   bSuccess = sal_False;
    DataFlavor aFlavor;

    for( sal_Int32 i = 0; i < nFlavors; i++ )
    {
        aFlavor = aFlavorList[i];
        CFormatEtc fetc = dataFlavorToFormatEtc( aFlavor );

        if ( needsToSynthesizeAccompanyFormats( fetc ) )
        {
            // we don't validate if the operation succeded
            // because an accompany format might have be
            // registered so that it's no problem if the
            // operation fails because of a duplicate format
            // e.g. text was already registered we have also
            // registered unicode text, if we later try to
            // register unicode text it will fail
            aFormatEtcContainer.addFormatEtc( fetc );

            synthesizeAndRegisterAccompanyFormats(
                fetc, aFlavor, aFormatEtcContainer );
        }
        else
            aFormatEtcContainer.addFormatEtc( fetc );
    }
}

/*
    For all Flavors from Transferable
        if not is TextFlavor
            add Format to FormatList
        else if not UnicodeTextRegistred
            if isUnicodeText or Transferable Has Unicodetext
                add Unicode Format to FormatList
                add Text to FormatList
                UnicodeTextRegistered = true
            else
                if findLocaleForTextCharset successful
                    add Unicode Format to FormatList
                    add Text or OemText to FormatList
                    add Locale to FormatList
                    HasSynthesizedLocale = true
                    save the TextFlavor so that access is easier in GetData
                end if
            end if
        end if
*/

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

inline
sal_Bool SAL_CALL CFormatRegistrar::isOemOrAnsiTextFormat( CLIPFORMAT cf ) const
{
    return ( (cf == CF_TEXT) || (cf == CF_OEMTEXT) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
sal_Bool SAL_CALL CFormatRegistrar::isUnicodeTextFormat( CLIPFORMAT cf ) const
{
    return ( cf == CF_UNICODETEXT );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
sal_Bool SAL_CALL CFormatRegistrar::isTextFormat( CLIPFORMAT cf ) const
{
    return ( isOemOrAnsiTextFormat( cf ) ||
             isUnicodeTextFormat( cf ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc SAL_CALL CFormatRegistrar::dataFlavorToFormatEtc( const DataFlavor& aFlavor ) const
{
    return m_DataFormatTranslator.getFormatEtcFromDataFlavor( aFlavor );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
sal_Bool SAL_CALL CFormatRegistrar::needsToSynthesizeAccompanyFormats( const CFormatEtc& aFormatEtc ) const
{
    return ( isOemOrAnsiTextFormat( aFormatEtc.getClipformat() ) ||
             isUnicodeTextFormat( aFormatEtc.getClipformat() ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFormatRegistrar::synthesizeAndRegisterAccompanyFormats(
    CFormatEtc& aFormatEtc, const DataFlavor& aFlavor, CFormatEtcContainer& aFormatEtcContainer )
{
    CLIPFORMAT cf = aFormatEtc.getClipformat();
    OSL_ASSERT( isOemOrAnsiTextFormat(cf) || isUnicodeTextFormat(cf) );

    CFormatEtc fetc;

    if ( isOemOrAnsiTextFormat( cf ) )
    {
        OUString charset = getCharsetFromDataFlavor( aFlavor );
        if ( charset.getLength( ) )
            m_TxtCodePage = getWinCPFromMimeCharset( charset );
        else
            m_TxtCodePage = GetACP( );

        OSL_ASSERT( IsValidCodePage( m_TxtCodePage ) );

        if ( !isEqualCurrentSystemCodePage( m_TxtCodePage ) )
        {
            fetc = getFormatEtcForClipformat( CF_LOCALE );

            if ( findLocaleForTextCodePage( ) )
            {
                aFormatEtcContainer.addFormatEtc( fetc );
                m_bHasSynthesizedLocale = sal_True;
            }
            else // could not find a locale for this charset
            {
                // remove the locale if there is one
                // already registered
                aFormatEtcContainer.removeFormatEtc( fetc );
                m_bHasSynthesizedLocale = sal_False;
            }
        }

        // register may fail if we have already
        // registered CF_UNICODETEXT but
        // then it doesn't matter because in this
        // case CF_TEXT is already registered
        fetc = getFormatEtcForClipformat( CF_UNICODETEXT );
        aFormatEtcContainer.addFormatEtc( fetc );
    }
    else // CF_UNICODETEXT
    {
        // register may fail if we have already
        // registered CF_TEXT or CF_OEMTEXT but
        // then it doesn't matter because in this
        // case CF_UNICODETEXT is already registered
        fetc = getFormatEtcForClipformat( CF_TEXT );
        aFormatEtcContainer.addFormatEtc( fetc );
    }
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
            m_SrvMgr->createInstance( OUString::createFromAscii( \
                "com.sun.star.datatransfer.MimeContentTypeFactory" ) ), UNO_QUERY );

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
        OSL_ENSURE( sal_False, "Unexpected" );
    }
    catch(...)
    {
        OSL_ENSURE( sal_False, "Invalid data flavor" );
    }

    return charset;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
CFormatEtc SAL_CALL CFormatRegistrar::getFormatEtcForClipformat( CLIPFORMAT aClipformat ) const
{
    return m_DataFormatTranslator.getFormatEtcForClipformat( aClipformat );
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
    EnumSystemLocalesA( CFormatRegistrar::EnumLocalesProc, LCID_SUPPORTED );
    return ( IsValidLocale( m_TxtLocale, LCID_SUPPORTED ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CFormatRegistrar::isLocaleCodePage( LCID lcid, LCTYPE lctype, sal_uInt32 codepage )
{
    char  buff[6];
    sal_uInt32 localeCodePage;

    // get the ansi codepage of the current locale
    GetLocaleInfoA( lcid, lctype, buff, sizeof( buff ) );
    localeCodePage = atoi( buff );

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
    LCID lcid = atoi( lpLocaleStr );

    if ( isLocaleAnsiCodePage( lcid, CFormatRegistrar::m_TxtCodePage ) ||
         isLocaleOemCodePage( lcid, CFormatRegistrar::m_TxtCodePage ) )
    {
        CFormatRegistrar::m_TxtLocale = lcid;
        return sal_False; // stop enumerating
    }

    return sal_True;
}

