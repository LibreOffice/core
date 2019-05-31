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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XSystemClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#include <cppuhelper/compbase.hxx>

#include <QtCore/QMimeData>
#include <QtGui/QClipboard>

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

/**
 * This implementation has two main function, which handle the clipboard content:
 * the XClipboard::setContent function and the QClipboard::change signal handler.
 *
 * The first just sets the respective clipboard to the expected content from LO,
 * the latter will handle any reported changes.
 **/
class Qt5Clipboard final
    : public QObject,
      public cppu::WeakComponentImplHelper<css::datatransfer::clipboard::XSystemClipboard,
                                           css::datatransfer::clipboard::XFlushableClipboard,
                                           css::lang::XServiceInfo>
{
    Q_OBJECT

    osl::Mutex m_aMutex;
    const OUString m_aClipboardName;
    const QClipboard::Mode m_aClipboardMode;
    // constant object returened as the volatile QClipboard data
    const css::uno::Reference<css::datatransfer::XTransferable> m_aVolatileContents;

    css::uno::Reference<css::datatransfer::XTransferable> m_aContents;
    css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner> m_aOwner;
    std::vector<css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>> m_aListeners;

    static bool isOwner(const QClipboard::Mode aMode);
    static bool isSupported(const QClipboard::Mode aMode);

    explicit Qt5Clipboard(const OUString& aModeString, const QClipboard::Mode aMode);

private Q_SLOTS:
    void handleChanged(QClipboard::Mode mode);

public:
    static css::uno::Reference<css::uno::XInterface> create(const OUString& aModeString);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XClipboard
    virtual css::uno::Reference<css::datatransfer::XTransferable> SAL_CALL getContents() override;
    virtual void SAL_CALL setContents(
        const css::uno::Reference<css::datatransfer::XTransferable>& xTrans,
        const css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner>& xClipboardOwner)
        override;
    virtual OUString SAL_CALL getName() override;

    // XClipboardEx
    virtual sal_Int8 SAL_CALL getRenderingCapabilities() override;

    // XFlushableClipboard
    virtual void SAL_CALL flushClipboard() override;

    // XClipboardNotifier
    virtual void SAL_CALL addClipboardListener(
        const css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>& listener)
        override;
    virtual void SAL_CALL removeClipboardListener(
        const css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>& listener)
        override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
