/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DataFmtTransl.hxx,v $
 * $Revision: 1.10 $
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


#ifndef _DATAFMTTRANSL_HXX_
#define _DATAFMTTRANSL_HXX_

//-----------------------------------------------
// includes
//-----------------------------------------------

#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/datatransfer/XDataFormatTranslator.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <sal/types.h>
#include <rtl/ustring.hxx>

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
