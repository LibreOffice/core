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

#include <QtWidgets/QApplication>
#include <QtCore/QBuffer>
#include <QtCore/QMimeData>
#include <QtCore/QUuid>

#include <Qt5Clipboard.hxx>
#include <Qt5Clipboard.moc>
#include <Qt5Tools.hxx>

#include <map>

namespace
{
std::map<OUString, QClipboard::Mode> g_nameToClipboardMap
    = { { "CLIPBOARD", QClipboard::Clipboard }, { "PRIMARY", QClipboard::Selection } };

QClipboard::Mode getClipboardTypeFromName(const OUString& aString)
{
    // use QClipboard::Clipboard as fallback if requested type isn't found
    QClipboard::Mode aMode = QClipboard::Clipboard;

    auto iter = g_nameToClipboardMap.find(aString);
    if (iter != g_nameToClipboardMap.end())
    {
        aMode = iter->second;
    }
    else
    {
        SAL_WARN("vcl.qt5", "Unrecognized clipboard type \""
                                << aString
                                << "\" is requested, falling back to QClipboard::Clipboard");
    }

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

Qt5Transferable::Qt5Transferable(QClipboard::Mode aMode)
    : m_aClipboardMode(aMode)
{
}

std::vector<css::datatransfer::DataFlavor> Qt5Transferable::getTransferDataFlavorsAsVector()
{
    std::vector<css::datatransfer::DataFlavor> aVector;

    const QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData(m_aClipboardMode);
    css::datatransfer::DataFlavor aFlavor;

    if (mimeData)
    {
        for (QString& rMimeType : mimeData->formats())
        {
            // filter out non-MIME types such as TARGETS, MULTIPLE, TIMESTAMP
            if (rMimeType.indexOf('/') == -1)
                continue;

            if (rMimeType.startsWith("text/plain"))
            {
                aFlavor.MimeType = "text/plain;charset=utf-16";
                aFlavor.DataType = cppu::UnoType<OUString>::get();
                aVector.push_back(aFlavor);
            }
            else
            {
                aFlavor.MimeType = toOUString(rMimeType);
                aFlavor.DataType = cppu::UnoType<Sequence<sal_Int8>>::get();
                aVector.push_back(aFlavor);
            }
        }
    }

    return aVector;
}

css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL Qt5Transferable::getTransferDataFlavors()
{
    return comphelper::containerToSequence(getTransferDataFlavorsAsVector());
}

sal_Bool SAL_CALL
Qt5Transferable::isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor)
{
    const std::vector<css::datatransfer::DataFlavor> aAll = getTransferDataFlavorsAsVector();

    return std::any_of(aAll.begin(), aAll.end(), [&](const css::datatransfer::DataFlavor& aFlavor) {
        return rFlavor.MimeType == aFlavor.MimeType;
    }); //FIXME
}

/*
 * XTransferable
 */

css::uno::Any SAL_CALL
Qt5Transferable::getTransferData(const css::datatransfer::DataFlavor& rFlavor)
{
    css::uno::Any aRet;
    const QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData(m_aClipboardMode);

    if (mimeData)
    {
        if (rFlavor.MimeType == "text/plain;charset=utf-16")
        {
            QString clipboardContent = mimeData->text();
            OUString sContent = toOUString(clipboardContent);

            aRet <<= sContent.replaceAll("\r\n", "\n");
        }
        else if (rFlavor.MimeType == "text/html")
        {
            QString clipboardContent = mimeData->html();
            std::string aStr = clipboardContent.toStdString();
            Sequence<sal_Int8> aSeq(reinterpret_cast<const sal_Int8*>(aStr.c_str()), aStr.length());
            aRet <<= aSeq;
        }
        else if (rFlavor.MimeType.startsWith("image") && mimeData->hasImage())
        {
            QImage image = qvariant_cast<QImage>(mimeData->imageData());
            QByteArray ba;
            QBuffer buffer(&ba);
            sal_Int32 nIndex = rFlavor.MimeType.indexOf('/');
            OUString sFormat(nIndex != -1 ? rFlavor.MimeType.copy(nIndex + 1) : "png");

            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, sFormat.toUtf8().getStr());

            Sequence<sal_Int8> aSeq(reinterpret_cast<const sal_Int8*>(ba.data()), ba.size());
            aRet <<= aSeq;
        }
    }

    return aRet;
}

VclQt5Clipboard::VclQt5Clipboard(const OUString& aModeString)
    : cppu::WeakComponentImplHelper<datatransfer::clipboard::XSystemClipboard,
                                    datatransfer::clipboard::XFlushableClipboard, XServiceInfo>(
          m_aMutex)
    , m_aClipboardName(aModeString)
    , m_aClipboardMode(getClipboardTypeFromName(aModeString))
    , m_aUuid(QUuid::createUuid().toByteArray())
{
    connect(QApplication::clipboard(), &QClipboard::changed, this,
            &VclQt5Clipboard::handleClipboardChange, Qt::DirectConnection);
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
        m_aContents = new Qt5Transferable(m_aClipboardMode);

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
        bool bHasHtml = false, bHasImage = false;
        lcl_peekFormats(aFormats, bHasHtml, bHasImage);

        std::unique_ptr<QMimeData> pMimeData(new QMimeData);

        // Add html data if present
        if (bHasHtml)
        {
            css::datatransfer::DataFlavor aFlavor;
            aFlavor.MimeType = "text/html";
            aFlavor.DataType = cppu::UnoType<Sequence<sal_Int8>>::get();

            Any aValue;
            try
            {
                aValue = xTrans->getTransferData(aFlavor);
            }
            catch (...)
            {
            }

            if (aValue.getValueType() == cppu::UnoType<Sequence<sal_Int8>>::get())
            {
                Sequence<sal_Int8> aData;
                aValue >>= aData;

                OUString aHtmlAsString(reinterpret_cast<const char*>(aData.getConstArray()),
                                       aData.getLength(), RTL_TEXTENCODING_UTF8);

                pMimeData->setHtml(toQString(aHtmlAsString));
            }
        }

        // Add text data
        // TODO: consider checking if text of suitable type is present
        {
            css::datatransfer::DataFlavor aFlavor;
            aFlavor.MimeType = "text/plain;charset=utf-16";
            aFlavor.DataType = cppu::UnoType<OUString>::get();

            Any aValue;
            try
            {
                aValue = xTrans->getTransferData(aFlavor);
            }
            catch (...)
            {
            }

            if (aValue.getValueTypeClass() == TypeClass_STRING)
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

OUString VclQt5Clipboard::getName() { return m_aClipboardName; }

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

void VclQt5Clipboard::handleClipboardChange(QClipboard::Mode aMode)
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
