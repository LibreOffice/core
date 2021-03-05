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

#pragma once

#include <com/sun/star/datatransfer/XTransferable.hpp>

#include <cppuhelper/implbase.hxx>
#include "DataFmtTransl.hxx"
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#include <com/sun/star/datatransfer/XSystemTransferable.hpp>
#include <cppuhelper/weakref.hxx>

#include <systools/win32/comtools.hxx>

#include <vector>

// forward
class CFormatEtc;

class CDOTransferable : public ::cppu::WeakImplHelper<
                            css::datatransfer::XTransferable,
                            css::datatransfer::XSystemTransferable>
{
public:
    typedef css::uno::Sequence< sal_Int8 > ByteSequence_t;

    // XTransferable

    virtual css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& aFlavor ) override;

    virtual css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  ) override;

    virtual sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& aFlavor ) override;

    // XSystemTransferable

    virtual css::uno::Any SAL_CALL getData( const css::uno::Sequence<sal_Int8>& aProcessId  ) override;

    explicit CDOTransferable(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Reference<css::datatransfer::clipboard::XClipboard>& xClipboard,
        const std::vector<sal_uInt32>& rFormats);

    explicit CDOTransferable(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        IDataObjectPtr rDataObject );

private:
    // some helper functions

    void initFlavorList( );
    void initFlavorListFromFormatList(const std::vector<sal_uInt32>& rFormats);

    void addSupportedFlavor( const css::datatransfer::DataFlavor& aFlavor );
    css::datatransfer::DataFlavor formatEtcToDataFlavor(sal_uInt32 cfFormat);

    void tryToGetIDataObjectIfAbsent();
    ByteSequence_t getClipboardData( CFormatEtc& aFormatEtc );
    OUString synthesizeUnicodeText( );

    LCID getLocaleFromClipboard( );

    bool compareDataFlavors( const css::datatransfer::DataFlavor& lhs,
                                          const css::datatransfer::DataFlavor& rhs );

private:
    css::uno::WeakReference<css::datatransfer::clipboard::XClipboard> m_xClipboard;
    IDataObjectPtr                                                                          m_rDataObject;
    css::uno::Sequence< css::datatransfer::DataFlavor >               m_FlavorList;
    const css::uno::Reference< css::uno::XComponentContext >          m_xContext;
    CDataFormatTranslator                                             m_DataFormatTranslator;
    css::uno::Reference< css::datatransfer::XMimeContentTypeFactory > m_rXMimeCntFactory;
    ::osl::Mutex                                                      m_aMutex;
    bool                                                              m_bUnicodeRegistered;
    CLIPFORMAT                                                        m_TxtFormatOnClipboard;

// non supported operations
private:
    CDOTransferable( const CDOTransferable& );
    CDOTransferable& operator=( const CDOTransferable& );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
