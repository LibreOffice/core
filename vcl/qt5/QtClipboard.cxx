/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <QtClipboard.hxx>
#include <QtClipboard.moc>

#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>

#include <QtWidgets/QApplication>

#include <QtInstance.hxx>
#include <QtTransferable.hxx>
#include <QtTools.hxx>

#include <cassert>
#include <map>
#include <utility>

QtClipboard::QtClipboard(OUString aModeString, const QClipboard::Mode aMode)
    : cppu::WeakComponentImplHelper<css::datatransfer::clipboard::XSystemClipboard,
                                    css::datatransfer::clipboard::XFlushableClipboard,
                                    XServiceInfo>(m_aMutex)
    , m_aClipboardName(std::move(aModeString))
    , m_aClipboardMode(aMode)
    , m_bOwnClipboardChange(false)
    , m_bDoClear(false)
{
    assert(isSupported(m_aClipboardMode));
    // DirectConnection guarantees the changed slot runs in the same thread as the QClipboard
    connect(QApplication::clipboard(), &QClipboard::changed, this, &QtClipboard::handleChanged,
            Qt::DirectConnection);

    // explicitly queue an event, so we can eventually ignore it
    connect(this, &QtClipboard::clearClipboard, this, &QtClipboard::handleClearClipboard,
            Qt::QueuedConnection);
}

css::uno::Reference<css::uno::XInterface> QtClipboard::create(const OUString& aModeString)
{
    static const std::map<OUString, QClipboard::Mode> aNameToClipboardMap
        = { { "CLIPBOARD", QClipboard::Clipboard }, { "PRIMARY", QClipboard::Selection } };

    assert(QApplication::clipboard()->thread() == qApp->thread());

    auto iter = aNameToClipboardMap.find(aModeString);
    if (iter != aNameToClipboardMap.end() && isSupported(iter->second))
        return cppu::getXWeak(new QtClipboard(aModeString, iter->second));
    SAL_WARN("vcl.qt", "Ignoring unrecognized clipboard type: '" << aModeString << "'");
    return css::uno::Reference<css::uno::XInterface>();
}

void QtClipboard::flushClipboard()
{
    auto* pSalInst(GetQtInstance());
    SolarMutexGuard g;
    pSalInst->RunInMainThread([this]() {
        if (!isOwner(m_aClipboardMode))
            return;

        QClipboard* pClipboard = QApplication::clipboard();
        const QtMimeData* pQtMimeData
            = qobject_cast<const QtMimeData*>(pClipboard->mimeData(m_aClipboardMode));
        assert(pQtMimeData);

        QMimeData* pMimeCopy = nullptr;
        if (pQtMimeData && pQtMimeData->deepCopy(&pMimeCopy))
        {
            m_bOwnClipboardChange = true;
            pClipboard->setMimeData(pMimeCopy, m_aClipboardMode);
            m_bOwnClipboardChange = false;
        }
    });
}

css::uno::Reference<css::datatransfer::XTransferable> QtClipboard::getContents()
{
#if defined(EMSCRIPTEN)
    static QMimeData aMimeData;
#endif
    osl::MutexGuard aGuard(m_aMutex);

    // if we're the owner, we might have the XTransferable from setContents. but
    // maybe a non-LO clipboard change from within LO, like some C'n'P in the
    // QFileDialog, might have invalidated m_aContents, so we need to check it too.
    if (isOwner(m_aClipboardMode) && m_aContents.is())
        return m_aContents;

    // check if we can still use the shared QtClipboardTransferable
    const QMimeData* pMimeData = QApplication::clipboard()->mimeData(m_aClipboardMode);
#if defined(EMSCRIPTEN)
    if (!pMimeData)
        pMimeData = &aMimeData;
#endif
    if (m_aContents.is())
    {
        const auto* pTrans = dynamic_cast<QtClipboardTransferable*>(m_aContents.get());
        assert(pTrans);
        if (pTrans && pTrans->hasMimeData(pMimeData))
            return m_aContents;
    }

    m_aContents = new QtClipboardTransferable(m_aClipboardMode, pMimeData);
    return m_aContents;
}

void QtClipboard::handleClearClipboard()
{
    if (!m_bDoClear)
        return;
    QApplication::clipboard()->clear(m_aClipboardMode);
}

void QtClipboard::setContents(
    const css::uno::Reference<css::datatransfer::XTransferable>& xTrans,
    const css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner>& xClipboardOwner)
{
    // it's actually possible to get a non-empty xTrans and an empty xClipboardOwner!
    osl::ClearableMutexGuard aGuard(m_aMutex);

    css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner> xOldOwner(m_aOwner);
    css::uno::Reference<css::datatransfer::XTransferable> xOldContents(m_aContents);
    m_aContents = xTrans;
    m_aOwner = xClipboardOwner;

    m_bDoClear = !m_aContents.is();
    if (!m_bDoClear)
    {
        m_bOwnClipboardChange = true;
        QApplication::clipboard()->setMimeData(new QtMimeData(m_aContents), m_aClipboardMode);
        m_bOwnClipboardChange = false;
    }
    else
    {
        assert(!m_aOwner.is());
        Q_EMIT clearClipboard();
    }

    aGuard.clear();

    // we have to notify only an owner change, since handleChanged can't
    // access the previous owner anymore and can just handle lost ownership.
    if (xOldOwner.is() && xOldOwner != xClipboardOwner)
        xOldOwner->lostOwnership(this, xOldContents);
}

void QtClipboard::handleChanged(QClipboard::Mode aMode)
{
    if (aMode != m_aClipboardMode)
        return;

    osl::ClearableMutexGuard aGuard(m_aMutex);

    if (!m_bOwnClipboardChange && isOwner(aMode))
    {
        auto const mimeData = QApplication::clipboard()->mimeData(aMode);

        // QtWayland will send a second change notification (seemingly without any
        // trigger). And any C'n'P operation in the Qt file picker emits a signal,
        // with LO still holding the clipboard ownership, but internally having lost
        // it. So ignore any signal, which still delivers the internal QtMimeData
        // as the clipboard content and is no "advertised" change.
        if (qobject_cast<const QtMimeData*>(mimeData))
            return;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && defined EMSCRIPTEN
        // At least for the Qt5 Wasm backend, copying text from LO and then pasting it back into LO
        // will, for whatever reason, call here with an empty text/plain mimeData; while that
        // doesn't seem to be an issue at least with the current upstream Qt6 dev branch (again, for
        // whatever reason), for at least our Qt 5.15.2+wasm branch the below m_aContents.clear()
        // would make us lose the text just copied from LO into the clipboard and thus make us paste
        // an empty text into LO, so, as a hack, filter out this unhelpful event here:
        if (mimeData != nullptr && mimeData->hasText() && mimeData->text().isEmpty())
        {
            return;
        }
#endif
    }

    css::uno::Reference<css::datatransfer::clipboard::XClipboardOwner> xOldOwner(m_aOwner);
    css::uno::Reference<css::datatransfer::XTransferable> xOldContents(m_aContents);
    // ownership change from LO POV is handled in setContents
    if (!m_bOwnClipboardChange)
    {
        m_aContents.clear();
        m_aOwner.clear();
    }

    std::vector<css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>> aListeners(
        m_aListeners);
    css::datatransfer::clipboard::ClipboardEvent aEv;
    aEv.Contents = getContents();

    aGuard.clear();

    if (!m_bOwnClipboardChange && xOldOwner.is())
        xOldOwner->lostOwnership(this, xOldContents);
    for (auto const& listener : aListeners)
        listener->changedContents(aEv);
}

OUString QtClipboard::getImplementationName() { return "com.sun.star.datatransfer.QtClipboard"; }

css::uno::Sequence<OUString> QtClipboard::getSupportedServiceNames()
{
    return { "com.sun.star.datatransfer.clipboard.SystemClipboard" };
}

sal_Bool QtClipboard::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

OUString QtClipboard::getName() { return m_aClipboardName; }

sal_Int8 QtClipboard::getRenderingCapabilities() { return 0; }

void QtClipboard::addClipboardListener(
    const css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>& listener)
{
    osl::MutexGuard aGuard(m_aMutex);
    m_aListeners.push_back(listener);
}

void QtClipboard::removeClipboardListener(
    const css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>& listener)
{
    osl::MutexGuard aGuard(m_aMutex);
    std::erase(m_aListeners, listener);
}

bool QtClipboard::isSupported(const QClipboard::Mode aMode)
{
    const QClipboard* pClipboard = QApplication::clipboard();
    switch (aMode)
    {
        case QClipboard::Selection:
            return pClipboard->supportsSelection();

        case QClipboard::FindBuffer:
            return pClipboard->supportsFindBuffer();

        case QClipboard::Clipboard:
            return true;
    }
    return false;
}

bool QtClipboard::isOwner(const QClipboard::Mode aMode)
{
    if (!isSupported(aMode))
        return false;

    const QClipboard* pClipboard = QApplication::clipboard();
    switch (aMode)
    {
        case QClipboard::Selection:
            return pClipboard->ownsSelection();

        case QClipboard::FindBuffer:
            return pClipboard->ownsFindBuffer();

        case QClipboard::Clipboard:
            if (pClipboard->ownsClipboard())
            {
                return true;
            }
#if defined EMSCRIPTEN
            // QWasmClipboard::ownsMode (in qtbase/src/plugins/platforms/wasm/qwasmclipboard.cpp,
            // which is the actual implementation of the above ownsClipboard call) unconditionally
            // returns false, so as a hack use "m_aContents is a LO-internal XTransferable type" as
            // a poor approximation of "LO owns the clipboard":
            if (m_aContents.is()
                && dynamic_cast<QtClipboardTransferable*>(m_aContents.get()) == nullptr)
            {
                return true;
            }
#endif
            return false;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
