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

#include <QtWidgets/QApplication>
#include <QtGui/QClipboard>
#include <QtCore/QBuffer>
#include <QtCore/QMimeData>

#include <Qt5Clipboard.hxx>
#include <Qt5Tools.hxx>
namespace
{
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

std::vector<css::datatransfer::DataFlavor> Qt5Transferable::getTransferDataFlavorsAsVector()
{
    std::vector<css::datatransfer::DataFlavor> aVector;

    const QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();
    css::datatransfer::DataFlavor aFlavor;

    if (mimeData->hasHtml())
    {
        aFlavor.MimeType = "text/html";
        aFlavor.DataType = cppu::UnoType<Sequence<sal_Int8>>::get();
        aVector.push_back(aFlavor);
    }

    if (mimeData->hasText())
    {
        aFlavor.MimeType = "text/plain;charset=utf-16";
        aFlavor.DataType = cppu::UnoType<OUString>::get();
        aVector.push_back(aFlavor);
    }

    if (mimeData->hasImage())
    {
        aFlavor.MimeType = "image/png";
        aFlavor.DataType = cppu::UnoType<Sequence<sal_Int8>>::get();
        aVector.push_back(aFlavor);
    }

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
Qt5Transferable::getTransferData(const css::datatransfer::DataFlavor& rFlavor)
{
    css::uno::Any aRet;
    const QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();

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
        m_aContents = new Qt5Transferable;

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

        // Add image data if present
        if (bHasImage)
        {
            css::datatransfer::DataFlavor aFlavor;
            //FIXME: other image formats?
            aFlavor.MimeType = "image/png";
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

        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setMimeData(pMimeData.release());
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
