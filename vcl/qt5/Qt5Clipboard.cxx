/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <Qt5Clipboard.hxx>
#include <Qt5Clipboard.moc>

#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>

#include <QtWidgets/QApplication>

#include <Qt5Instance.hxx>
#include <Qt5Transferable.hxx>
#include <Qt5Tools.hxx>

#include <cassert>
#include <map>

Qt5Clipboard::Qt5Clipboard(const OUString& aModeString, const QClipboard::Mode aMode)
    : cppu::WeakComponentImplHelper<css::datatransfer::clipboard::XSystemClipboard,
                                    css::datatransfer::clipboard::XFlushableClipboard,
                                    XServiceInfo>(m_aMutex)
    , m_aClipboardName(aModeString)
    , m_aClipboardMode(aMode)
    , m_bOwnClipboardChange(false)
    , m_bDoClear(false)
{
    assert(isSupported(m_aClipboardMode));
    // DirectConnection guarantees the changed slot runs in the same thread as the QClipboard
    connect(QApplication::clipboard(), &QClipboard::changed, this, &Qt5Clipboard::handleChanged,
            Qt::DirectConnection);

    // explicitly queue an event, so we can eventually ignore it
    connect(this, &Qt5Clipboard::clearClipboard, this, &Qt5Clipboard::handleClearClipboard,
            Qt::QueuedConnection);
}

css::uno::Reference<css::uno::XInterface> Qt5Clipboard::create(const OUString& aModeString)
{
    static const std::map<OUString, QClipboard::Mode> aNameToClipboardMap
        = { { "CLIPBOARD", QClipboard::Clipboard }, { "PRIMARY", QClipboard::Selection } };

    assert(QApplication::clipboard()->thread() == qApp->thread());

    auto iter = aNameToClipboardMap.find(aModeString);
    if (iter != aNameToClipboardMap.end() && isSupported(iter->second))
        return static_cast<cppu::OWeakObject*>(new Qt5Clipboard(aModeString, iter->second));
    SAL_WARN("vcl.qt5", "Ignoring unrecognized clipboard type: '" << aModeString << "'");
    return css::uno::Reference<css::uno::XInterface>();
}

void Qt5Clipboard::flushClipboard()
{
    auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
    SolarMutexGuard g;
    pSalInst->RunInMainThread([&, this]() {
        if (!isOwner(m_aClipboardMode))
            return;

        QClipboard* pClipboard = QApplication::clipboard();
        const Qt5MimeData* pQt5MimeData
            = dynamic_cast<const Qt5MimeData*>(pClipboard->mimeData(m_aClipboardMode));
        assert(pQt5MimeData);

        QMimeData* pMimeCopy = nullptr;
        if (pQt5MimeData && pQt5MimeData->deepCopy(&pMimeCopy))
        {
            m_bOwnClipboardChange = true;
            pClipboard->setMimeData(pMimeCopy, m_aClipboardMode);
            m_bOwnClipboardChange = false;
        }
    });
}

css::uno::Reference<css::datatransfer::XTransferable> Qt5Clipboard::getContents()
{
    osl::MutexGuard aGuard(m_aMutex);

    // if we're the owner, we might have the XTransferable from setContents. but
    // maybe a non-LO clipboard change from within LO, like some C'n'P in the
    // QFileDialog, might have invalidated m_aContents, so we need to check it too.
    if (isOwner(m_aClipboardMode) && m_aContents.is())
        return m_aContents;

    // check if we can still use the shared Qt5ClipboardTransferable
    const QMimeData* pMimeData = QApplication::clipboard()->mimeData(m_aClipboardMode);
    if (m_aContents.is())
    {
        const auto* pTrans = dynamic_cast<Qt5ClipboardTransferable*>(m_aContents.get());
        assert(pTrans);
        if (pTrans && pTrans->mimeData() == pMimeData)
            return m_aContents;
    }

    m_aContents = new Qt5ClipboardTransferable(m_aClipboardMode, pMimeData);
    return m_aContents;
}

void Qt5Clipboard::handleClearClipboard()
{
    if (!m_bDoClear)
        return;
    QApplication::clipboard()->clear(m_aClipboardMode);
}

void Qt5Clipboard::setContents(
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
        QApplication::clipboard()->setMimeData(new Qt5MimeData(m_aContents), m_aClipboardMode);
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

void Qt5Clipboard::handleChanged(QClipboard::Mode aMode)
{
    if (aMode != m_aClipboardMode)
        return;

    osl::ClearableMutexGuard aGuard(m_aMutex);

    // QtWayland will send a second change notification (seemingly without any
    // trigger). And any C'n'P operation in the Qt file picker emits a signal,
    // with LO still holding the clipboard ownership, but internally having lost
    // it. So ignore any signal, which still delivers the internal Qt5MimeData
    // as the clipboard content and is no "advertised" change.
    if (!m_bOwnClipboardChange && isOwner(aMode)
        && dynamic_cast<const Qt5MimeData*>(QApplication::clipboard()->mimeData(aMode)))
        return;

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

OUString Qt5Clipboard::getImplementationName() { return "com.sun.star.datatransfer.Qt5Clipboard"; }

css::uno::Sequence<OUString> Qt5Clipboard::getSupportedServiceNames()
{
    return { "com.sun.star.datatransfer.clipboard.SystemClipboard" };
}

sal_Bool Qt5Clipboard::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

OUString Qt5Clipboard::getName() { return m_aClipboardName; }

sal_Int8 Qt5Clipboard::getRenderingCapabilities() { return 0; }

void Qt5Clipboard::addClipboardListener(
    const css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>& listener)
{
    osl::MutexGuard aGuard(m_aMutex);
    m_aListeners.push_back(listener);
}

void Qt5Clipboard::removeClipboardListener(
    const css::uno::Reference<css::datatransfer::clipboard::XClipboardListener>& listener)
{
    osl::MutexGuard aGuard(m_aMutex);
    m_aListeners.erase(std::remove(m_aListeners.begin(), m_aListeners.end(), listener),
                       m_aListeners.end());
}

bool Qt5Clipboard::isSupported(const QClipboard::Mode aMode)
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

bool Qt5Clipboard::isOwner(const QClipboard::Mode aMode)
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
            return pClipboard->ownsClipboard();
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
