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
 * Abstract XTransferable used for clipboard and D'n'D transfers
 **/
class Qt5Transferable : public cppu::WeakImplHelper<css::datatransfer::XTransferable>
{
    Qt5Transferable() = delete;
    Qt5Transferable(const Qt5Transferable&) = delete;

protected:
    /**
     * for D'n'D this is just a reference, but for the Clipboard it's a deep copy,
     * which must be deleted, so just keep it accessible for sub classes.
     **/
    const QMimeData* m_pMimeData;

    Qt5Transferable(const QMimeData* pMimeData);
    std::vector<css::datatransfer::DataFlavor> getTransferDataFlavorsAsVector();

public:
    ~Qt5Transferable() override;

    css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors() override;
    sal_Bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor) override;
};

class Qt5ClipboardTransferable final : public Qt5Transferable
{
public:
    explicit Qt5ClipboardTransferable(QClipboard::Mode aMode);
    ~Qt5ClipboardTransferable() override;

    css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;
};

class Qt5DnDTransferable final : public Qt5Transferable
{
public:
    Qt5DnDTransferable(const QMimeData* pMimeData);

    css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
