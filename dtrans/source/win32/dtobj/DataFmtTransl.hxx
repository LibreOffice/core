/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataFmtTransl.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:04:48 $
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


#ifndef _DATAFMTTRANSL_HXX_
#define _DATAFMTTRANSL_HXX_

//-----------------------------------------------
// includes
//-----------------------------------------------

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XDATAFORMATTRANSLATOR_HPP_
#include <com/sun/star/datatransfer/XDataFormatTranslator.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//-----------------------------------------------
// declaration
//-----------------------------------------------

class CFormatEtc;

class CDataFormatTranslator
{
public:
    CDataFormatTranslator( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&  aServiceManager );

    CFormatEtc getFormatEtcFromDataFlavor( const com::sun::star::datatransfer::DataFlavor& aDataFlavor ) const;
    com::sun::star::datatransfer::DataFlavor getDataFlavorFromFormatEtc(
        const FORMATETC& aFormatEtc, LCID lcid = GetThreadLocale( ) ) const;

    CFormatEtc    SAL_CALL getFormatEtcForClipformat( CLIPFORMAT cf ) const;
    CFormatEtc    SAL_CALL getFormatEtcForClipformatName( const rtl::OUString& aClipFmtName ) const;
    rtl::OUString SAL_CALL getClipboardFormatName( CLIPFORMAT aClipformat ) const;

    sal_Bool SAL_CALL isHTMLFormat( CLIPFORMAT cf ) const;
    sal_Bool SAL_CALL isTextHtmlFormat( CLIPFORMAT cf ) const;
    sal_Bool SAL_CALL isOemOrAnsiTextFormat( CLIPFORMAT cf ) const;
    sal_Bool SAL_CALL isUnicodeTextFormat( CLIPFORMAT cf ) const;
    sal_Bool SAL_CALL isTextFormat( CLIPFORMAT cf ) const;

private:
    rtl::OUString SAL_CALL getTextCharsetFromLCID( LCID lcid, CLIPFORMAT aClipformat ) const;

private:
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  m_SrvMgr;
    com::sun::star::uno::Reference< com::sun::star::datatransfer::XDataFormatTranslator >   m_XDataFormatTranslator;
};

#endif
