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
#include <sal/log.hxx>

#include <QtCore/QMimeData>
#include <QtCore/QUuid>
#include <QtWidgets/QApplication>

#include <Qt5Clipboard.hxx>
#include <Qt5Clipboard.moc>
#include <Qt5Transferable.hxx>
#include <Qt5Tools.hxx>

#include <map>

using namespace com::sun::star;

namespace
{
QClipboard::Mode getClipboardTypeFromName(const OUString& aString)
{
    static const std::map<OUString, QClipboard::Mode> aNameToClipboardMap
        = { { "CLIPBOARD", QClipboard::Clipboard }, { "PRIMARY", QClipboard::Selection } };

    // default to QClipboard::Clipboard as fallback
    QClipboard::Mode aMode = QClipboard::Clipboard;

    auto iter = aNameToClipboardMap.find(aString);
    if (iter != aNameToClipboardMap.end())
        aMode = iter->second;
    else
        SAL_WARN("vcl.qt5", "Unrecognized clipboard type \""
                                << aString << "\"; falling back to QClipboard::Clipboard");
    return aMode;
}

void lcl_peekFormats(const css::uno::Sequence<css::datatransfer::DataFlavor>& rFormats,
                     bool& bHasHtml, bool& bHasImage)
{
    for (int i = 0; i < rFormats.getLength(); ++i)
    {
        const css::datatransfer::DataFlavor& rFlavor = rFormats[i];

        if (rFlavor.MimeType == "text/html")
            bHasHtml = true;
        else if (rFlavor.MimeType.startsWith("image"))
            bHasImage = true;
    }
}
}

Qt5Clipboard::Qt5Clipboard(const OUString& aModeString)
    : cppu::WeakComponentImplHelper<datatransfer::clipboard::XSystemClipboard,
                                    datatransfer::clipboard::XFlushableClipboard, XServiceInfo>(
          m_aMutex)
    , m_aClipboardName(aModeString)
    , m_aClipboardMode(getClipboardTypeFromName(aModeString))
    , m_aUuid(QUuid::createUuid().toByteArray())
{
    connect(QApplication::clipboard(), &QClipboard::changed, this,
            &Qt5Clipboard::handleClipboardChange, Qt::DirectConnection);
}

void Qt5Clipboard::flushClipboard()
{
    SolarMutexGuard aGuard;
    return;
}

Qt5Clipboard::~Qt5Clipboard() {}

OUString Qt5Clipboard::getImplementationName()
{
    return OUString("com.sun.star.datatransfer.Qt5Clipboard");
}

uno::Sequence<OUString> Qt5Clipboard::getSupportedServiceNames()
{
    return { "com.sun.star.datatransfer.clipboard.SystemClipboard" };
}

sal_Bool Qt5Clipboard::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Reference<css::datatransfer::XTransferable> Qt5Clipboard::getContents()
{
    if (!m_aContents.is())
        m_aContents = new Qt5ClipboardTransferable(m_aClipboardMode);
    return m_aContents;
}

void Qt5Clipboard::setContents(
    const uno::Reference<css::datatransfer::XTransferable>& xTrans,
    const uno::Reference<css::datatransfer::clipboard::XClipboardOwner>& xClipboardOwner)
{
    osl::ClearableMutexGuard aGuard(m_aMutex);
    uno::Reference<datatransfer::clipboard::XClipboardOwner> xOldOwner(m_aOwner);
    uno::Reference<datatransfer::XTransferable> xOldContents(m_aContents);
    m_aContents = xTrans;
    m_aOwner = xClipboardOwner;

    std::vector<uno::Reference<datatransfer::clipboard::XClipboardListener>> aListeners(
        m_aListeners);
    datatransfer::clipboard::ClipboardEvent aEv;

    QClipboard* clipboard = QApplication::clipboard();

    switch (m_aClipboardMode)
    {
        case QClipboard::Selection:
            if (!clipboard->supportsSelection())
            {
                return;
            }
            break;

        case QClipboard::FindBuffer:
            if (!clipboard->supportsFindBuffer())
            {
                return;
            }
            break;

        case QClipboard::Clipboard:
        default:
            break;
    }

    if (m_aContents.is())
    {
        css::uno::Sequence<css::datatransfer::DataFlavor> aFormats
            = xTrans->getTransferDataFlavors();
        // Do not add non-text formats for the selection buffer,
        // I don't think that one is ever used for anything else
        // besides text and this gets called whenever something
        // in LO gets selected (which may be e.g. an entire Calc sheet).
        bool bHasHtml = false, bHasImage = false;
        if (m_aClipboardMode != QClipboard::Selection)
            lcl_peekFormats(aFormats, bHasHtml, bHasImage);

        std::unique_ptr<QMimeData> pMimeData(new QMimeData);

        // Add html data if present
        if (bHasHtml)
        {
            css::datatransfer::DataFlavor aFlavor;
            aFlavor.MimeType = "text/html";
            aFlavor.DataType = cppu::UnoType<uno::Sequence<sal_Int8>>::get();

            uno::Any aValue;
            try
            {
                aValue = xTrans->getTransferData(aFlavor);
            }
            catch (...)
            {
            }

            if (aValue.getValueType() == cppu::UnoType<uno::Sequence<sal_Int8>>::get())
            {
                uno::Sequence<sal_Int8> aData;
                aValue >>= aData;

                OUString aHtmlAsString(reinterpret_cast<const char*>(aData.getConstArray()),
                                       aData.getLength(), RTL_TEXTENCODING_UTF8);

                pMimeData->setHtml(toQString(aHtmlAsString));
            }
        }

        // Add image data if present
        if (bHasImage)
        {
            css::datatransfer::DataFlavor aFlavor;
            //FIXME: other image formats?
            aFlavor.MimeType = "image/png";
            aFlavor.DataType = cppu::UnoType<uno::Sequence<sal_Int8>>::get();

            uno::Any aValue;
            try
            {
                aValue = xTrans->getTransferData(aFlavor);
            }
            catch (...)
            {
            }

            if (aValue.getValueType() == cppu::UnoType<uno::Sequence<sal_Int8>>::get())
            {
                uno::Sequence<sal_Int8> aData;
                aValue >>= aData;

                QImage image;
                image.loadFromData(reinterpret_cast<const uchar*>(aData.getConstArray()),
                                   aData.getLength());

                pMimeData->setImageData(image);
            }
        }

        // Add text data
        // TODO: consider checking if text of suitable type is present
        {
            css::datatransfer::DataFlavor aFlavor;
            aFlavor.MimeType = "text/plain;charset=utf-16";
            aFlavor.DataType = cppu::UnoType<OUString>::get();

            uno::Any aValue;
            try
            {
                aValue = xTrans->getTransferData(aFlavor);
            }
            catch (...)
            {
            }

            if (aValue.getValueTypeClass() == uno::TypeClass_STRING)
            {
                OUString aString;
                aValue >>= aString;
                pMimeData->setText(toQString(aString));
            }
        }

        // set value for custom MIME type to indicate that content was added by this clipboard
        pMimeData->setData(m_sMimeTypeUuid, m_aUuid);

        clipboard->setMimeData(pMimeData.release(), m_aClipboardMode);
    }

    aEv.Contents = getContents();

    aGuard.clear();

    if (xOldOwner.is() && xOldOwner != xClipboardOwner)
        xOldOwner->lostOwnership(this, xOldContents);
    for (auto const& listener : aListeners)
    {
        listener->changedContents(aEv);
    }
}

OUString Qt5Clipboard::getName() { return m_aClipboardName; }

sal_Int8 Qt5Clipboard::getRenderingCapabilities() { return 0; }

void Qt5Clipboard::addClipboardListener(
    const uno::Reference<datatransfer::clipboard::XClipboardListener>& listener)
{
    osl::ClearableMutexGuard aGuard(m_aMutex);

    m_aListeners.push_back(listener);
}

void Qt5Clipboard::removeClipboardListener(
    const uno::Reference<datatransfer::clipboard::XClipboardListener>& listener)
{
    osl::ClearableMutexGuard aGuard(m_aMutex);

    m_aListeners.erase(std::remove(m_aListeners.begin(), m_aListeners.end(), listener),
                       m_aListeners.end());
}

void Qt5Clipboard::handleClipboardChange(QClipboard::Mode aMode)
{
    // if system clipboard content has changed and current content was not created by
    // this clipboard itself, clear the own current content
    // (e.g. to take into account clipboard updates from other applications)
    if (aMode == m_aClipboardMode
        && QApplication::clipboard()->mimeData(aMode)->data(m_sMimeTypeUuid) != m_aUuid)
    {
        m_aContents.clear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
