/*************************************************************************
 *
 *  $RCSfile: FetcList.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-01 15:39:15 $
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

HANDLE     CFormatRegistrar::m_hEvtEnumLocaleReady = NULL;
LCID       CFormatRegistrar::m_TxtLocale           = 0;
sal_uInt32 CFormatRegistrar::m_TxtCodePage         = GetACP( );

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc::CFormatEtc( const FORMATETC& aFormatEtc )
{
    CopyFormatEtc( &m_FormatEtc, &const_cast< FORMATETC& >( aFormatEtc ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc::CFormatEtc( CLIPFORMAT cf, DWORD tymed, DVTARGETDEVICE* ptd, DWORD dwAspect, LONG lindex )
{
    m_FormatEtc.cfFormat = cf;
    m_FormatEtc.ptd      = CopyTargetDevice( ptd );
    m_FormatEtc.dwAspect = dwAspect;
    m_FormatEtc.lindex   = lindex;
    m_FormatEtc.tymed    = tymed;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc::CFormatEtc( const CFormatEtc& theOther )
{
    m_FormatEtc.cfFormat = theOther.m_FormatEtc.cfFormat;
    m_FormatEtc.ptd      = CopyTargetDevice( theOther.m_FormatEtc.ptd );
    m_FormatEtc.dwAspect = theOther.m_FormatEtc.dwAspect;
    m_FormatEtc.lindex   = theOther.m_FormatEtc.lindex;
    m_FormatEtc.tymed    = theOther.m_FormatEtc.tymed;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc::~CFormatEtc( )
{
    DeleteTargetDevice( m_FormatEtc.ptd );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc& CFormatEtc::operator=( const CFormatEtc& theOther )
{
    if ( this != &theOther )
    {
        DeleteTargetDevice( m_FormatEtc.ptd );

        m_FormatEtc.cfFormat = theOther.m_FormatEtc.cfFormat;
        m_FormatEtc.ptd      = CopyTargetDevice( theOther.m_FormatEtc.ptd );
        m_FormatEtc.dwAspect = theOther.m_FormatEtc.dwAspect;
        m_FormatEtc.lindex   = theOther.m_FormatEtc.lindex;
        m_FormatEtc.tymed    = theOther.m_FormatEtc.tymed;
    }

    return *this;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc::operator FORMATETC*( )
{
    return &m_FormatEtc;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void CFormatEtc::getFORMATETC( LPFORMATETC lpFormatEtc )
{
    OSL_ASSERT( lpFormatEtc );
    OSL_ASSERT( !IsBadWritePtr( lpFormatEtc, sizeof( FORMATETC ) ) );

    CopyFormatEtc( lpFormatEtc, &m_FormatEtc );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CLIPFORMAT CFormatEtc::getClipformat( ) const
{
    return m_FormatEtc.cfFormat;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

DWORD CFormatEtc::getTymed( ) const
{
    return m_FormatEtc.tymed;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void CFormatEtc::getTargetDevice( DVTARGETDEVICE** lpDvTargetDevice ) const
{
    OSL_ASSERT( lpDvTargetDevice );
    OSL_ASSERT( !IsBadWritePtr( lpDvTargetDevice, sizeof( DVTARGETDEVICE ) ) );

    *lpDvTargetDevice = NULL;

    if ( m_FormatEtc.ptd )
        *lpDvTargetDevice = CopyTargetDevice( m_FormatEtc.ptd );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

DWORD CFormatEtc::getAspect( ) const
{
    return m_FormatEtc.dwAspect;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

LONG CFormatEtc::getLindex( ) const
{
    return m_FormatEtc.lindex;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Int32 operator==( CFormatEtc& lhs, CFormatEtc& rhs )
{
    return CompareFormatEtc( &lhs.m_FormatEtc, &rhs.m_FormatEtc );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Int32 operator!=( CFormatEtc& lhs, CFormatEtc& rhs )
{
    return ( ( lhs == rhs ) != 1 );
}


//#########################################################################


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

sal_Bool CFormatEtcContainer::addFormatEtc( const FORMATETC& fetc )
{
    return m_FormatMap.insert(
        make_pair( fetc.cfFormat, CFormatEtc( fetc ) ) ).second;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFormatEtcContainer::removeFormatEtc( const FORMATETC& fetc )
{
    FormatEtcMap_t::iterator iter = m_FormatMap.find( fetc.cfFormat );

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

sal_Bool CFormatEtcContainer::hasFormatEtc( const FORMATETC& fetc ) const
{
    FormatEtcMap_t::const_iterator iter = m_FormatMap.find( fetc.cfFormat );

    if ( iter != m_FormatMap.end( ) )
    {
        return CFormatEtc( fetc ) == CFormatEtc( iter->second );
    }

    return sal_False;
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
        {
            CopyFormatEtc( lpFetc, m_EnumIterator->second );
        }
    }

    return nFetched;
}


//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CFormatEtcContainer::skipFormatEtc( sal_uInt32 aNum )
{
    for ( sal_uInt32 i = 0; i < aNum; i++, ++m_EnumIterator )
        /* intentionally left empty */;

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
    CFormatRegistrar::m_hEvtEnumLocaleReady = CreateEvent( NULL,
                                                             sal_False,
                                                           sal_False,
                                                           NULL );

    OSL_ASSERT( CFormatRegistrar::m_hEvtEnumLocaleReady );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatRegistrar::~CFormatRegistrar( )
{
    if ( CFormatRegistrar::m_hEvtEnumLocaleReady )
        CloseHandle( CFormatRegistrar::m_hEvtEnumLocaleReady );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFormatRegistrar::RegisterFormats( const Sequence< DataFlavor >& aFlavorList,
                                                 CFormatEtcContainer& aFormatEtcContainer )
{
    sal_Int32  nFlavors = aFlavorList.getLength( );
    sal_Bool   bSuccess = sal_False;
    DataFlavor aFlavor;

    for( sal_Int32 i = 0; i < nFlavors; i++ )
    {
        aFlavor = aFlavorList[i];
        FORMATETC fetc = dataFlavorToFormatEtc( aFlavor );

        if ( needsToSynthesizeAccompanyFormats( fetc ) )
        {

#ifdef _DEBUG
            FORMATETC fetcdbg;

            if ( fetc.cfFormat == CF_TEXT )
            {
                fetcdbg = getFormatEtcForClipformat( CF_OEMTEXT );
                OSL_ASSERT( !aFormatEtcContainer.hasFormatEtc( fetcdbg ) );
            }
            else if ( fetc.cfFormat == CF_OEMTEXT )
            {
                fetcdbg = getFormatEtcForClipformat( CF_TEXT );
                OSL_ASSERT( !aFormatEtcContainer.hasFormatEtc( fetcdbg ) );
            }
#endif

            // we don't validate if the operation succeded
            // because an accompany format might have be
            // registered so that it's no problem if the
            // operation fails because of a duplicate format
            // e.g. text was already registered we have also
            // registered unicode text, if we later try to
            // register unicode text it will fail
            aFormatEtcContainer.addFormatEtc( fetc );

            synthesizeAndRegisterAccompanyFormats( fetc, aFlavor, aFormatEtcContainer );
        }
        else
        {
            bSuccess = aFormatEtcContainer.addFormatEtc( fetc );
            OSL_ASSERT( bSuccess );
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

FORMATETC SAL_CALL CFormatRegistrar::dataFlavorToFormatEtc( const DataFlavor& aFlavor ) const
{
    return m_DataFormatTranslator.getFormatEtcFromDataFlavor( aFlavor );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
sal_Bool SAL_CALL CFormatRegistrar::needsToSynthesizeAccompanyFormats( const FORMATETC& aFormatEtc ) const
{
    return ( isOemOrAnsiTextFormat( aFormatEtc.cfFormat ) ||
             isUnicodeTextFormat( aFormatEtc.cfFormat ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CFormatRegistrar::synthesizeAndRegisterAccompanyFormats(
    FORMATETC& aFormatEtc, const DataFlavor& aFlavor, CFormatEtcContainer& aFormatEtcContainer )
{
    CLIPFORMAT cf = aFormatEtc.cfFormat;
    OSL_ASSERT( isOemOrAnsiTextFormat(cf) || isUnicodeTextFormat(cf) );

    FORMATETC fetc;

    if ( isOemOrAnsiTextFormat( cf ) )
    {
        OUString charset = getCharsetFromDataFlavor( aFlavor );
        if ( charset.getLength( ) )
            m_TxtCodePage = getWinCodePageFromMimeCharset( charset );
        else
            m_TxtCodePage = GetACP( );

        OSL_ASSERT( IsValidCodePage( m_TxtCodePage ) );

        if ( !isEqualCurrentSystemCodePage( m_TxtCodePage ) )
        {
            FindLocaleForTextCodePage( );
            fetc = getFormatEtcForClipformat( CF_LOCALE );
            aFormatEtcContainer.addFormatEtc( fetc );
            m_bHasSynthesizedLocale = sal_True;
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
            charset = xMimeType->getParameterValue( TEXTPLAIN_PARAM_CHARSET );
        }
    }
    catch(NoSuchElementException&)
    {
        OSL_ENSURE( sal_False, "Text data flavor without charset not allowed" );
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
FORMATETC SAL_CALL CFormatRegistrar::getFormatEtcForClipformat( CLIPFORMAT aClipformat ) const
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

void SAL_CALL CFormatRegistrar::FindLocaleForTextCodePage( )
{
    EnumSystemLocalesA( CFormatRegistrar::EnumLocalesProc, LCID_SUPPORTED );
    WaitForSingleObject( CFormatRegistrar::m_hEvtEnumLocaleReady, INFINITE );
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
        SetEvent( CFormatRegistrar::m_hEvtEnumLocaleReady );
        return sal_False; // stop enumerating
    }

    return sal_True;
}

