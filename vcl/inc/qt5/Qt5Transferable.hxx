/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <cppuhelper/compbase.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>

#include <QtGui/QClipboard>

/**
 * Qt5Transferable classes are used to read QMimeData via the XTransferable
 * interface. All the general functionality is implemented in the abstract
 * Qt5Transferable.
 *
 * There are two users, which provide QMimeData: the QClipboard and the D'n'D
 * functionality.
 *
 * There is some special handling of the case, where the provided QMimeData
 * is actually a Qt5MimeData, so we can directly query the XTransferable for
 * its data, completely ignoring the QMimeData itself.
 *
 * It's the "mirror" interface of the Qt5MimeData.
 **/
class Qt5Transferable : public cppu::WeakImplHelper<css::datatransfer::XTransferable>
{
    Qt5Transferable(const Qt5Transferable&) = delete;

protected:
    virtual const QMimeData* mimeData() const = 0;
    virtual const css::uno::Reference<css::datatransfer::XTransferable> xTransferable() const = 0;

    Qt5Transferable();
    std::vector<css::datatransfer::DataFlavor> getTransferDataFlavorsAsVector();

public:
    css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors() override;
    sal_Bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor) override;
};

/**
 * The QClipboard's QMimeData is volatile. As written in the QClipboard::mimeData
 * documentation, "the pointer returned might become invalidated when the contents
 * of the clipboard changes". Therefore it's futile to store or use the QMimeData
 * pointer, as there is no mechanism to get notified before it changes.
 *
 * Still QClipboard can't change out of thin air. The QClipboard object lives in
 * the QApplication thread, so all of the access should probably use RunInMain().
 *
 * So this resolves to store just the QClipboard::Mode. On access it'll query the
 * clipboard itself, or short-circuit to an embedded XAccessible.
 **/
class Qt5ClipboardTransferable final : public Qt5Transferable
{
    const QClipboard::Mode m_aMode;

public:
    explicit Qt5ClipboardTransferable(QClipboard::Mode aMode);

    const QMimeData* mimeData() const override;
    const css::uno::Reference<css::datatransfer::XTransferable> xTransferable() const override;

    css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;
};

/**
 * This just uses the QMimeData provided by the QWidgets D'n'D events.
 * It just stores the pointer, which seems to be sufficient.
 **/
class Qt5DnDTransferable final : public Qt5Transferable
{
    const QMimeData* m_pMimeData;

public:
    explicit Qt5DnDTransferable(const QMimeData* pMimeData);

    const css::uno::Reference<css::datatransfer::XTransferable> xTransferable() const override;
    const QMimeData* mimeData() const override;

    css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
