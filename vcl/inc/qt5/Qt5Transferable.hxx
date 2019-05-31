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
    const QMimeData* m_pMimeData;

    Qt5Transferable(const QMimeData* pMimeData);
    std::vector<css::datatransfer::DataFlavor> getTransferDataFlavorsAsVector();

    bool getXTransferableData(const css::datatransfer::DataFlavor& rFlavor, css::uno::Any& rAny);

public:
    css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors() override;
    sal_Bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor) override;
};

class Qt5ClipboardTransferable final : public Qt5Transferable
{
public:
    explicit Qt5ClipboardTransferable(QClipboard::Mode aMode);

    css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;
};

class Qt5DnDTransferable final : public Qt5Transferable
{
public:
    Qt5DnDTransferable(const QMimeData* pMimeData);

    css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
