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

#include <QtCore/QMimeData>
#include <QtGui/QClipboard>

/**
 * Qt5Transferable classes are used to read QMimeData via the XTransferable
 * interface. All the general functionality is implemented in the abstract
 * Qt5Transferable.
 *
 * There are two users, which provide QMimeData: the QClipboard and the D'n'D
 * functionality.
 *
 * It's the "mirror" interface of the Qt5MimeData.
 **/
class Qt5Transferable : public cppu::WeakImplHelper<css::datatransfer::XTransferable>
{
    Qt5Transferable(const Qt5Transferable&) = delete;

    const QMimeData* m_pMimeData;
    std::vector<css::datatransfer::DataFlavor> m_aMimeTypes;

protected:
    const QMimeData* mimeData() const { return m_pMimeData; }
    std::vector<css::datatransfer::DataFlavor>& getTransferDataFlavorsAsVector();

public:
    Qt5Transferable(const QMimeData* pMimeData);

    css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors() override;
    sal_Bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor) override;
    css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;
};

/**
 * The QClipboard's QMimeData is volatile. As written in the QClipboard::mimeData
 * documentation, "the pointer returned might become invalidated when the contents
 * of the clipboard changes". Therefore it can just be accessed relyable inside
 * the QClipboard's object thread, which is the QApplication's thread, so all of
 * the access runs through RunInMainThread().
 **/
class Qt5ClipboardTransferable final : public Qt5Transferable
{
    // stored to detect in-flight access collisions
    const QClipboard::Mode m_aMode;

public:
    explicit Qt5ClipboardTransferable(QClipboard::Mode aMode);

    // these are the same then Qt5Transferable, except they run throught RunInMainThread
    css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors() override;
    sal_Bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor) override;
    css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;
};

/**
 * This just uses the QMimeData provided by the QWidgets D'n'D events.
 **/
typedef Qt5Transferable Qt5DnDTransferable;

/**
 * A lazy loading QMimeData for XTransferable reads
 *
 * This is an interface class to make a XTransferable read accessibale as a
 * QMimeData. The mime data is just stored inside the XTransferable, never
 * in the QMimeData itself! It's objects are just used for QClipboard to read
 * the XTransferable data.
 *
 * Like XTransferable itself, this class should be considered an immutable
 * container for mime data. There is no need to ever set any of its data.
 *
 * It's the "mirror" interface of the Qt5Transferable.
 **/
class Qt5MimeData final : public QMimeData
{
    friend class Qt5ClipboardTransferable;

    const css::uno::Reference<css::datatransfer::XTransferable> m_aContents;

    QVariant retrieveData(const QString& mimeType, QVariant::Type type) const override;

public:
    explicit Qt5MimeData(const css::uno::Reference<css::datatransfer::XTransferable>& aContents);

    bool hasFormat(const QString& mimeType) const override;
    QStringList formats() const override;

    bool deepCopy(QMimeData** const) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
