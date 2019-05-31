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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XSystemClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>

#include <QtCore/QMimeData>
#include <QtGui/QClipboard>

class Qt5MimeData : public QMimeData
{
    Q_OBJECT

    mutable css::uno::Reference<css::datatransfer::XTransferable> m_aContents;

    QVariant retrieveData(const QString& mimeType, QVariant::Type type) const override;

public:
    explicit Qt5MimeData(css::uno::Reference<css::datatransfer::XTransferable>& aContents);
    explicit Qt5MimeData(const QMimeData& obj);

    bool hasFormat(const QString& mimeType) const override;
    QStringList formats() const override;

    void releaseXTransferable() const { m_aContents.clear(); }
};

class Qt5Transferable : public cppu::WeakImplHelper<css::datatransfer::XTransferable>
{
    Qt5MimeData m_aContent;

public:
    explicit Qt5Transferable(QClipboard::Mode aMode);

    virtual css::uno::Any SAL_CALL
    getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;

    virtual std::vector<css::datatransfer::DataFlavor> getTransferDataFlavorsAsVector();

    virtual css::uno::Sequence<css::datatransfer::DataFlavor>
        SAL_CALL getTransferDataFlavors() override;
    virtual sal_Bool SAL_CALL
    isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor) override;
};

class Qt5Clipboard
    : public QObject,
      public cppu::WeakComponentImplHelper<css::datatransfer::clipboard::XSystemClipboard,
                                           css::datatransfer::clipboard::XFlushableClipboard,
                                           css::lang::XServiceInfo>
{
    Q_OBJECT

    osl::Mutex m_aMutex;
    const OUString m_aClipboardName;
    const QClipboard::Mode m_aClipboardMode;

    css::uno::Reference<css::datatransfer::XTransferable> m_aContents;
    css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner> m_aOwner;
    std::vector<css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>> m_aListeners;

private Q_SLOTS:
    void handleChanged(QClipboard::Mode mode);

public:
    explicit Qt5Clipboard(const OUString& aModeString);
    virtual ~Qt5Clipboard() override;

    /*
     * XServiceInfo
     */

    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    /*
     * XClipboard
     */

    virtual css::uno::Reference<css::datatransfer::XTransferable> SAL_CALL getContents() override;

    virtual void SAL_CALL setContents(
        const css::uno::Reference<css::datatransfer::XTransferable>& xTrans,
        const css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner>& xClipboardOwner)
        override;

    virtual OUString SAL_CALL getName() override;

    /*
     * XClipboardEx
     */

    virtual sal_Int8 SAL_CALL getRenderingCapabilities() override;

    /*
     * XFlushableClipboard
     */
    virtual void SAL_CALL flushClipboard() override;

    /*
     * XClipboardNotifier
     */
    virtual void SAL_CALL addClipboardListener(
        const css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>& listener)
        override;

    virtual void SAL_CALL removeClipboardListener(
        const css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>& listener)
        override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
