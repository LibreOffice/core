/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <comphelper/solarmutex.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>

#include <Qt5Clipboard.hxx>

std::vector<css::datatransfer::DataFlavor> Qt5Transferable::getTransferDataFlavorsAsVector()
{
    std::vector<css::datatransfer::DataFlavor> aVector;

    // FIXME: this is fake
    css::datatransfer::DataFlavor aFlavor;
    aFlavor.MimeType = "text/plain;charset=utf-16";
    aFlavor.DataType = cppu::UnoType<OUString>::get();
    aVector.push_back(aFlavor);

    return aVector;
}

css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL Qt5Transferable::getTransferDataFlavors()
{
    return comphelper::containerToSequence(getTransferDataFlavorsAsVector());
}

sal_Bool SAL_CALL
Qt5Transferable::isDataFlavorSupported(const css::datatransfer::DataFlavor& /*rFlavor*/)
{
    const std::vector<css::datatransfer::DataFlavor> aAll = getTransferDataFlavorsAsVector();

    return !aAll.empty(); //FIXME
}

/*
 * XTransferable
 */

css::uno::Any SAL_CALL
Qt5Transferable::getTransferData(const css::datatransfer::DataFlavor& /*rFlavor*/)
{
    css::uno::Any aRet;
    Sequence<sal_Int8> aSeq(0, 4); //FIXME
    aRet <<= aSeq;
    return aRet;
}

VclQt5Clipboard::VclQt5Clipboard()
    : cppu::WeakComponentImplHelper<datatransfer::clipboard::XSystemClipboard,
                                    datatransfer::clipboard::XFlushableClipboard, XServiceInfo>(
          m_aMutex)
{
}

void VclQt5Clipboard::flushClipboard()
{
    SolarMutexGuard aGuard;
    return;
}

VclQt5Clipboard::~VclQt5Clipboard() {}

OUString VclQt5Clipboard::getImplementationName()
{
    return OUString("com.sun.star.datatransfer.VclQt5Clipboard");
}

Sequence<OUString> VclQt5Clipboard::getSupportedServiceNames()
{
    Sequence<OUString> aRet{ "com.sun.star.datatransfer.clipboard.SystemClipboard" };
    return aRet;
}

sal_Bool VclQt5Clipboard::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Reference<css::datatransfer::XTransferable> VclQt5Clipboard::getContents()
{
    if (!m_aContents.is())
        m_aContents = new Qt5Transferable();

    return m_aContents;
}

void VclQt5Clipboard::setContents(
    const Reference<css::datatransfer::XTransferable>& xTrans,
    const Reference<css::datatransfer::clipboard::XClipboardOwner>& xClipboardOwner)
{
    osl::ClearableMutexGuard aGuard(m_aMutex);
    Reference<datatransfer::clipboard::XClipboardOwner> xOldOwner(m_aOwner);
    Reference<datatransfer::XTransferable> xOldContents(m_aContents);
    m_aContents = xTrans;
    m_aOwner = xClipboardOwner;

    std::vector<Reference<datatransfer::clipboard::XClipboardListener>> aListeners(m_aListeners);
    datatransfer::clipboard::ClipboardEvent aEv;

    aEv.Contents = getContents();

    aGuard.clear();

    if (xOldOwner.is() && xOldOwner != xClipboardOwner)
        xOldOwner->lostOwnership(this, xOldContents);
    for (auto const& listener : aListeners)
    {
        listener->changedContents(aEv);
    }
}

OUString VclQt5Clipboard::getName() { return OUString("CLIPBOARD"); }

sal_Int8 VclQt5Clipboard::getRenderingCapabilities() { return 0; }

void VclQt5Clipboard::addClipboardListener(
    const Reference<datatransfer::clipboard::XClipboardListener>& listener)
{
    osl::ClearableMutexGuard aGuard(m_aMutex);

    m_aListeners.push_back(listener);
}

void VclQt5Clipboard::removeClipboardListener(
    const Reference<datatransfer::clipboard::XClipboardListener>& listener)
{
    osl::ClearableMutexGuard aGuard(m_aMutex);

    m_aListeners.erase(std::remove(m_aListeners.begin(), m_aListeners.end(), listener),
                       m_aListeners.end());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
