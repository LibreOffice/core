/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FetcList.hxx,v $
 * $Revision: 1.11 $
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


#ifndef _FETCLIST_HXX_
#define _FETCLIST_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <sal/types.h>
#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include "Fetc.hxx"

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <vector>

/*****************************************************************
    a simple container for FORMATECT structures
    instances of this class are not thread-safe
*****************************************************************/

class CFormatEtcContainer
{
public:
    CFormatEtcContainer( );

    // duplicates not allowed
    void SAL_CALL addFormatEtc( const CFormatEtc& fetc );

    // removes the specified formatetc
    void SAL_CALL removeFormatEtc( const CFormatEtc& fetc );

    // removes the formatetc at pos
    void SAL_CALL removeAllFormatEtc( );

    sal_Bool SAL_CALL hasFormatEtc( const CFormatEtc& fetc ) const;

    sal_Bool SAL_CALL hasElements( ) const;

    // begin enumeration
    void SAL_CALL beginEnumFormatEtc( );

    // copies the specified number of formatetc structures starting
    // at the current enum position
    // the return value is the number of copied elements; if the
    // current enum position is at the end the return value is 0
    sal_uInt32 SAL_CALL nextFormatEtc( LPFORMATETC lpFetc, sal_uInt32 aNum = 1 );

    // skips the specified number of elements in the container
    sal_Bool SAL_CALL skipFormatEtc( sal_uInt32 aNum );

protected:
    typedef std::vector< CFormatEtc > FormatEtcMap_t;

private:
    FormatEtcMap_t           m_FormatMap;
    FormatEtcMap_t::iterator m_EnumIterator;
};

/*****************************************************************
    a helper class which converts data flavors to clipformats,
    creates an appropriate formatetc structures and if possible
    synthesizes clipboard formats if necessary, e.g. if text
    is provided a locale will also be provided;
    the class registers the formatetc within a CFormatEtcContainer

    instances of this class are not thread-safe and multiple
    instances of this class would use the same static variables
    that's why this class should not be used by multiple threads,
    only one thread of a process should use it
*****************************************************************/

// forward
class CDataFormatTranslator;

class CFormatRegistrar
{
public:
    CFormatRegistrar( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& ServiceManager,
                      const CDataFormatTranslator& aDataFormatTranslator );

    void SAL_CALL RegisterFormats( const com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable >& aXTransferable,
                                   CFormatEtcContainer& aFormatEtcContainer );

    sal_Bool   SAL_CALL hasSynthesizedLocale( ) const;
    LCID       SAL_CALL getSynthesizedLocale( ) const;
    sal_uInt32 SAL_CALL getRegisteredTextCodePage( ) const;
    com::sun::star::datatransfer::DataFlavor SAL_CALL getRegisteredTextFlavor( ) const;

    sal_Bool  SAL_CALL isSynthesizeableFormat( const CFormatEtc& aFormatEtc ) const;
    sal_Bool  SAL_CALL needsToSynthesizeAccompanyFormats( const CFormatEtc& aFormatEtc ) const;

private:
    sal_Bool      SAL_CALL isEqualCurrentSystemCodePage( sal_uInt32 aCodePage ) const;
    rtl::OUString SAL_CALL getCharsetFromDataFlavor( const com::sun::star::datatransfer::DataFlavor& aFlavor );

    sal_Bool SAL_CALL hasUnicodeFlavor(
        const com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable >& aXTransferable ) const;

    sal_Bool SAL_CALL findLocaleForTextCodePage( );

    static sal_Bool SAL_CALL isLocaleOemCodePage( LCID lcid, sal_uInt32 codepage );
    static sal_Bool SAL_CALL isLocaleAnsiCodePage( LCID lcid, sal_uInt32 codepage );
    static sal_Bool SAL_CALL isLocaleCodePage( LCID lcid, LCTYPE lctype, sal_uInt32 codepage );

    static BOOL CALLBACK EnumLocalesProc( LPSTR lpLocaleStr );

private:
    const CDataFormatTranslator&             m_DataFormatTranslator;
    sal_Bool                                 m_bHasSynthesizedLocale;
    com::sun::star::datatransfer::DataFlavor m_RegisteredTextFlavor;

    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  m_SrvMgr;

    static LCID       m_TxtLocale;
    static sal_uInt32 m_TxtCodePage;

private:
    CFormatRegistrar( const CFormatRegistrar& );
    CFormatRegistrar& operator=( const CFormatRegistrar& );
};

#endif
