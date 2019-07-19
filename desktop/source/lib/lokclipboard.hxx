/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DESKTOP_SOURCE_LIB_LOKCLIPBOARD_HXX
#define INCLUDED_DESKTOP_SOURCE_LIB_LOKCLIPBOARD_HXX

#include <vector>

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>

/// A clipboard implementation for LibreOfficeKit.
class LOKClipboard : public cppu::WeakImplHelper<css::datatransfer::clipboard::XClipboard>
{
    css::uno::Reference<css::datatransfer::XTransferable> m_xTransferable;

public:
    css::uno::Reference<css::datatransfer::XTransferable> SAL_CALL getContents() override;

    void SAL_CALL setContents(
        const css::uno::Reference<css::datatransfer::XTransferable>& xTransferable,
        const css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner>& xClipboardOwner)
        override;

    OUString SAL_CALL getName() override;
};

/// Represents the contents of LOKClipboard.
class LOKTransferable : public cppu::WeakImplHelper<css::datatransfer::XTransferable>
{
    css::uno::Sequence<css::datatransfer::DataFlavor> m_aFlavors;
    std::vector<css::uno::Any> m_aContent;

public:
    LOKTransferable(const size_t   nInCount,
                    const char   **pInMimeTypes,
                    const size_t  *pInSizes,
                    const char   **pInStreams);

    css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;

    css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors() override;

    sal_Bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
