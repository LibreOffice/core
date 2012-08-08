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


#ifndef _DATAFMTTRANSL_HXX_
#define _DATAFMTTRANSL_HXX_

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
