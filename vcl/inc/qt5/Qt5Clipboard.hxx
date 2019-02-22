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

#include <QtGui/QClipboard>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

class Qt5Transferable : public cppu::WeakImplHelper<css::datatransfer::XTransferable>
{
public:
    explicit Qt5Transferable(QClipboard::Mode aMode);

    virtual css::uno::Any SAL_CALL
    getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;

    virtual std::vector<css::datatransfer::DataFlavor> getTransferDataFlavorsAsVector();

    virtual css::uno::Sequence<css::datatransfer::DataFlavor>
        SAL_CALL getTransferDataFlavors() override;
    virtual sal_Bool SAL_CALL
    isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor) override;

private:
    QClipboard::Mode m_aClipboardMode;
};

class VclQt5Clipboard
    : public QObject,
      public cppu::WeakComponentImplHelper<datatransfer::clipboard::XSystemClipboard,
                                           datatransfer::clipboard::XFlushableClipboard,
                                           XServiceInfo>
{
    Q_OBJECT

private Q_SLOTS:
    void handleClipboardChange(QClipboard::Mode mode);

private:
    osl::Mutex m_aMutex;
    Reference<css::datatransfer::XTransferable> m_aContents;
    Reference<css::datatransfer::clipboard::XClipboardOwner> m_aOwner;
    std::vector<Reference<css::datatransfer::clipboard::XClipboardListener>> m_aListeners;
    OUString m_aClipboardName;
    QClipboard::Mode m_aClipboardMode;
    // custom MIME type to detect whether clipboard content was added by self or externally
    const QString m_sMimeTypeUuid = "application/x-libreoffice-clipboard-uuid";
    const QByteArray m_aUuid;

public:
    explicit VclQt5Clipboard(const OUString& aModeString);
    virtual ~VclQt5Clipboard() override;

    /*
     * XServiceInfo
     */

    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    /*
     * XClipboard
     */

    virtual Reference<css::datatransfer::XTransferable> SAL_CALL getContents() override;

    virtual void SAL_CALL setContents(
        const Reference<css::datatransfer::XTransferable>& xTrans,
        const Reference<css::datatransfer::clipboard::XClipboardOwner>& xClipboardOwner) override;

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
        const Reference<css::datatransfer::clipboard::XClipboardListener>& listener) override;

    virtual void SAL_CALL removeClipboardListener(
        const Reference<css::datatransfer::clipboard::XClipboardListener>& listener) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
