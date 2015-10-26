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
    virtual css::uno::Reference<css::datatransfer::XTransferable> SAL_CALL getContents()
    throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setContents(const css::uno::Reference<css::datatransfer::XTransferable>& xTransferable,
                                      const css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner>& xClipboardOwner)
    throw(css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
};

/// Represents the contents of LOKClipboard.
class LOKTransferable : public cppu::WeakImplHelper<css::datatransfer::XTransferable>
{
    OString m_aMimeType;
    css::uno::Sequence<sal_Int8> m_aSequence;

    /// Provides a list of flavors, used by getTransferDataFlavors() and isDataFlavorSupported().
    std::vector<css::datatransfer::DataFlavor> getTransferDataFlavorsAsVector();

public:
    LOKTransferable(const char* pMimeType, const char* pData, size_t nSize);

    virtual css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor)
    throw(css::datatransfer::UnsupportedFlavorException, css::io::IOException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors()
    throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor)
    throw(css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
