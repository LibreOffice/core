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
#include <QtCore/QStringList>
#include <QtGui/QClipboard>

/**
 * QtTransferable classes are used to read QMimeData via the XTransferable
 * interface. All the functionality is already implemented in the QtTransferable.
 *
 * The specialisations map to the two users, which provide QMimeData: the Clipboard
 * and the Drag'n'Drop functionality.
 *
 * LO itself seem to just accept "text/plain;charset=utf-16", so it relies on the
 * backend to convert to this charset, but still offers "text/plain" itself.
 *
 * It's the "mirror" interface of the QtMimeData, which is defined below.
 **/
class QtTransferable : public cppu::WeakImplHelper<css::datatransfer::XTransferable>
{
    QtTransferable(const QtTransferable&) = delete;

    const QMimeData* m_pMimeData;

protected:
    /** Sets new mime data.
     *  Since data flavors supported by this class depend on the mime data,
     *  results from previous calls to the public methods of this
     *  class are no longer valid after setting new mime data using this method.
     *
     *  Subclasses that set new mime data must ensure that no data race exists
     *  on m_pMimeData.
     *  (For the current only subclass doing so, QtClipboardTransferable, all access
     *  to m_pMimeData happens with the SolarMutex held.)
     */
    void setMimeData(const QMimeData* pMimeData) { m_pMimeData = pMimeData; }
    const QMimeData* mimeData() const { return m_pMimeData; }

public:
    QtTransferable(const QMimeData* pMimeData);

    css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors() override;
    sal_Bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor) override;
    css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;
};

/**
 * The QClipboard's QMimeData is volatile. As written in the QClipboard::mimeData
 * documentation, "the pointer returned might become invalidated when the contents
 * of the clipboard changes". Therefore it can just be accessed reliably inside
 * the QClipboard's object thread, which is the QApplication's thread, so all of
 * the access has to go through RunInMainThread().
 *
 * If we detect a QMimeData change, the mime data is updated with the new one from
 * the system clipboard. Note however that this means that results of any previous
 * calls of the XTransferable interface will be out of sync with the newly set mime
 * data, so this scenario should generally be avoided.
 **/
class QtClipboardTransferable final : public QtTransferable
{
    // to detect in-flight QMimeData changes
    const QClipboard::Mode m_aMode;

    void ensureConsistencyWithSystemClipboard();

public:
    explicit QtClipboardTransferable(const QClipboard::Mode aMode, const QMimeData* pMimeData);

    // whether pMimeData are the current mime data
    bool hasMimeData(const QMimeData* pMimeData) const;

    // these are the same then QtTransferable, except they go through RunInMainThread
    css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors() override;
    sal_Bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor) override;
    css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;
};

/**
 * Convenience typedef for better code readability
 *
 * This just uses the QMimeData provided by the QWidgets D'n'D events.
 **/
typedef QtTransferable QtDnDTransferable;

/**
 * A lazy loading QMimeData for XTransferable reads
 *
 * This is an interface class to make a XTransferable read accessible as a
 * QMimeData. The mime data is just stored inside the XTransferable, never
 * in the QMimeData itself! It's objects are just used for QClipboard to read
 * the XTransferable data.
 *
 * Like XTransferable itself, this class should be considered an immutable
 * container for mime data. There is no need to ever set any of its data.
 *
 * LO will offer at least UTF-16, if there is a viable text representation.
 * If LO misses to offer a UTF-8 or a locale encoded string, these objects
 * will offer them themselves and convert from UTF-16 on demand.
 *
 * It's the "mirror" interface of the QtTransferable.
 **/
class QtMimeData final : public QMimeData
{
    friend class QtClipboardTransferable;

    const css::uno::Reference<css::datatransfer::XTransferable> m_aContents;
    mutable bool m_bHaveNoCharset; // = uses the locale charset
    mutable bool m_bHaveUTF8;
    mutable QStringList m_aMimeTypeList;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QVariant retrieveData(const QString& mimeType, QVariant::Type type) const override;
#else
    QVariant retrieveData(const QString& mimeType, QMetaType type) const override;
#endif

public:
    explicit QtMimeData(const css::uno::Reference<css::datatransfer::XTransferable>& aContents);

    bool hasFormat(const QString& mimeType) const override;
    QStringList formats() const override;

    bool deepCopy(QMimeData** const) const;

    css::datatransfer::XTransferable* xTransferable() const { return m_aContents.get(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
