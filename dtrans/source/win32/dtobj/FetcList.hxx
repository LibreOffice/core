/*************************************************************************
 *
 *  $RCSfile: FetcList.hxx,v $
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


#ifndef _FETCLIST_HXX_
#define _FETCLIST_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DATAFLAVOR_HPP_
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#endif

#include <windows.h>
#include <map>

/**********************************************************************
 stl container elements must fulfill the following requirements:
 1. they need a copy ctor and assignement operator(?)
 2. they must be compareable
 because the FORMATETC structure has a pointer to a TARGETDEVICE
 structure we need a simple wrapper class to fulfill these needs
***********************************************************************/

class CFormatEtc
{
public:
    CFormatEtc( const FORMATETC& aFormatEtc );
    CFormatEtc( CLIPFORMAT cf, DWORD tymed, DVTARGETDEVICE* ptd = NULL, DWORD dwAspect = DVASPECT_CONTENT, LONG lindex = -1 );
    CFormatEtc( const CFormatEtc& theOther );

    ~CFormatEtc( );

    CFormatEtc& operator=( const CFormatEtc& theOther );
    operator FORMATETC*( );

    void getFORMATETC( LPFORMATETC lpFormatEtc );

    CLIPFORMAT getClipformat( ) const;
    DWORD      getTymed( ) const;
    void       getTargetDevice( DVTARGETDEVICE** ptd ) const;
    DWORD      getAspect( ) const;
    LONG       getLindex( ) const;

private:
    FORMATETC m_FormatEtc;

    friend sal_Int32 operator==( CFormatEtc& lhs, CFormatEtc& rhs );
    friend sal_Int32 operator!=( CFormatEtc& lhs, CFormatEtc& rhs );
};

sal_Int32 operator==( CFormatEtc& lhs, CFormatEtc& rhs );
sal_Int32 operator!=( CFormatEtc& lhs, CFormatEtc& rhs );

/*****************************************************************
    a simple container for FORMATECT structures
    instances of this class are not thread-safe
*****************************************************************/

class CFormatEtcContainer
{
public:
    CFormatEtcContainer( );

    // duplicates not allowed
    sal_Bool SAL_CALL addFormatEtc( const FORMATETC& fetc );

    // removes the specified formatetc
    void SAL_CALL removeFormatEtc( const FORMATETC& fetc );

    // removes the formatetc at pos
    void SAL_CALL removeAllFormatEtc( );

    sal_Bool SAL_CALL hasFormatEtc( const FORMATETC& fetc ) const;

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
    typedef std::map< CLIPFORMAT, CFormatEtc > FormatEtcMap_t;

private:
    sal_uInt32               m_nCurrentEnumPos;
    FormatEtcMap_t           m_FormatMap;
    FormatEtcMap_t::iterator m_EnumIterator;

private:
    //CFormatEtcContainer( const CFormatEtcContainer& );
    //CFormatEtcContainer& operator=( const CFormatEtcContainer& );
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
    ~CFormatRegistrar( );

    void SAL_CALL RegisterFormats( const com::sun::star::uno::Sequence< com::sun::star::datatransfer::DataFlavor >& aFlavorList,
                                   CFormatEtcContainer& aFormatEtcContainer );
    sal_Bool SAL_CALL hasSynthesizedLocale( ) const;
    LCID     SAL_CALL getSynthesizedLocale( ) const;

private:
    sal_Bool  SAL_CALL isOemOrAnsiTextFormat( CLIPFORMAT cf ) const;
    sal_Bool  SAL_CALL isUnicodeTextFormat( CLIPFORMAT cf ) const;
    sal_Bool  SAL_CALL isTextFormat( CLIPFORMAT cf ) const;
    FORMATETC SAL_CALL dataFlavorToFormatEtc( const com::sun::star::datatransfer::DataFlavor& aFlavor ) const;
    sal_Bool  SAL_CALL needsToSynthesizeAccompanyFormats( const FORMATETC& aFormatEtc ) const;
    void      SAL_CALL synthesizeAndRegisterAccompanyFormats( FORMATETC& aFormatEtc,
                                                     const com::sun::star::datatransfer::DataFlavor& aFlavor,
                                                     CFormatEtcContainer& aFormatEtcContainer );
    sal_Bool SAL_CALL isEqualCurrentSystemCodePage( sal_uInt32 aCodePage ) const;
    rtl::OUString SAL_CALL getCharsetFromDataFlavor( const com::sun::star::datatransfer::DataFlavor& aFlavor );
    FORMATETC SAL_CALL getFormatEtcForClipformat( CLIPFORMAT aClipformat ) const;

    void SAL_CALL FindLocaleForTextCodePage( );

    static sal_Bool SAL_CALL isLocaleOemCodePage( LCID lcid, sal_uInt32 codepage );
    static sal_Bool SAL_CALL isLocaleAnsiCodePage( LCID lcid, sal_uInt32 codepage );
    static sal_Bool SAL_CALL isLocaleCodePage( LCID lcid, LCTYPE lctype, sal_uInt32 codepage );

    static BOOL CALLBACK EnumLocalesProc( LPSTR lpLocaleStr );

private:
    const CDataFormatTranslator& m_DataFormatTranslator;
    sal_Bool                     m_bHasSynthesizedLocale;

    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  m_SrvMgr;

    static HANDLE     m_hEvtEnumLocaleReady;
    static LCID       m_TxtLocale;
    static sal_uInt32 m_TxtCodePage;

private:
    CFormatRegistrar( const CFormatRegistrar& );
    CFormatRegistrar& operator=( const CFormatRegistrar& );
};

#endif