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

#ifndef INCLUDED_DTRANS_SOURCE_WIN32_FTRANSL_FTRANSL_HXX
#define INCLUDED_DTRANS_SOURCE_WIN32_FTRANSL_FTRANSL_HXX

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/datatransfer/XDataFormatTranslator.hpp>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "../misc/WinClip.hxx"

#include <vector>

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

// CDataFormatTranslator

class CDataFormatTranslator : public
    cppu::WeakImplHelper< css::datatransfer::XDataFormatTranslator,
                          css::lang::XServiceInfo >
{

public:
    CDataFormatTranslator( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    // XDataFormatTranslator

    virtual css::uno::Any SAL_CALL getSystemDataTypeFromDataFlavor( const css::datatransfer::DataFlavor& aDataFlavor )
        throw(css::uno::RuntimeException);

    virtual css::datatransfer::DataFlavor SAL_CALL getDataFlavorFromSystemDataType( const css::uno::Any& aSysDataType )
        throw(css::uno::RuntimeException);

    // XServiceInfo

    virtual OUString SAL_CALL getImplementationName(  )
        throw(css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw(css::uno::RuntimeException);

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw(css::uno::RuntimeException);

private:
    void SAL_CALL findDataFlavorForStandardFormatId( sal_Int32 aStandardFormatId, css::datatransfer::DataFlavor& aDataFlavor ) const;
    void SAL_CALL findDataFlavorForNativeFormatName( const OUString& aNativeFormatName, css::datatransfer::DataFlavor& aDataFlavor ) const;
    void SAL_CALL findStandardFormatIdForCharset( const OUString& aCharset, css::uno::Any& aAny ) const;
    void SAL_CALL setStandardFormatIdForNativeFormatName( const OUString& aNativeFormatName, css::uno::Any& aAny ) const;
    void SAL_CALL findStdFormatIdOrNativeFormatNameForFullMediaType(
        const css::uno::Reference< css::datatransfer::XMimeContentTypeFactory >& aRefXMimeFactory,
        const OUString& aFullMediaType, css::uno::Any& aAny ) const;

    sal_Bool isTextPlainMediaType( const OUString& fullMediaType ) const;

    css::datatransfer::DataFlavor SAL_CALL mkDataFlv( const OUString& cnttype, const OUString& hpname, css::uno::Type dtype );

private:
    const css::uno::Reference< css::uno::XComponentContext >  m_xContext;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
