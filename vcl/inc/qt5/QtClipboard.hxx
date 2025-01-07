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

#include <QtGui/QClipboard>

/**
 * This implementation has two main functions, which handle the clipboard content:
 * the XClipboard::setContent function and the QClipboard::change signal handler.
 *
 * The first just sets the respective clipboard to the expected content from LO,
 * the latter will handle any reported changes.
 **/
class QtClipboard final
    : public QObject,
      public cppu::WeakComponentImplHelper<css::datatransfer::clipboard::XSystemClipboard,
                                           css::datatransfer::clipboard::XFlushableClipboard,
                                           css::lang::XServiceInfo>
{
    Q_OBJECT

    osl::Mutex m_aMutex;
    const OUString m_aClipboardName;
    const QClipboard::Mode m_aClipboardMode;
    // has to be set, if LO changes the QClipboard itself, so it won't instantly lose
    // ownership by it's self-triggered QClipboard::changed handler
    bool m_bOwnClipboardChange;
    // true, if LO really wants to give up clipboard ownership
    bool m_bDoClear;

    // if not empty, this holds the setContents provided XTransferable or a QtClipboardTransferable
    css::uno::Reference<css::datatransfer::XTransferable> m_aContents;
    // the owner of the current contents, which must be informed on content change
    css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner> m_aOwner;
    std::vector<css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>> m_aListeners;

    bool isOwner(const QClipboard::Mode aMode);
    static bool isSupported(const QClipboard::Mode aMode);

    explicit QtClipboard(OUString aModeString, const QClipboard::Mode aMode);

private Q_SLOTS:
    void handleChanged(QClipboard::Mode mode);
    void handleClearClipboard();

signals:
    void clearClipboard();

public:
    // factory function to construct only valid QtClipboard objects by name
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
