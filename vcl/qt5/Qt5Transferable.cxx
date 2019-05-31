/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <Qt5Transferable.hxx>

#include <comphelper/solarmutex.hxx>
#include <comphelper/sequence.hxx>
#include <sal/log.hxx>

#include <QtCore/QBuffer>
#include <QtCore/QUrl>
#include <QtWidgets/QApplication>

#include <Qt5Clipboard.hxx>
#include <Qt5Tools.hxx>

Qt5Transferable::Qt5Transferable() {}

std::vector<css::datatransfer::DataFlavor> Qt5Transferable::getTransferDataFlavorsAsVector()
{
    std::vector<css::datatransfer::DataFlavor> aVector;

    css::datatransfer::DataFlavor aFlavor;
    for (QString& rMimeType : mimeData()->formats())
    {
        // filter out non-MIME types such as TARGETS, MULTIPLE, TIMESTAMP
        if (rMimeType.indexOf('/') == -1)
            continue;

        aFlavor.MimeType = toOUString(rMimeType);
        if (rMimeType.startsWith("text/plain"))
        {
            aFlavor.MimeType = "text/plain;charset=utf-16";
            aFlavor.DataType = cppu::UnoType<OUString>::get();
            aVector.push_back(aFlavor);
        }
        else
        {
            aFlavor.MimeType = toOUString(rMimeType);
            aFlavor.DataType = cppu::UnoType<css::uno::Sequence<sal_Int8>>::get();
            aVector.push_back(aFlavor);
        }
    }

    return aVector;
}

css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL Qt5Transferable::getTransferDataFlavors()
{
    css::uno::Reference<css::datatransfer::XTransferable> xTrans(xTransferable());
    if (xTrans.is())
        return xTrans->getTransferDataFlavors();
    return comphelper::containerToSequence(getTransferDataFlavorsAsVector());
}

sal_Bool SAL_CALL
Qt5Transferable::isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor)
{
    css::uno::Reference<css::datatransfer::XTransferable> xTrans(xTransferable());
    if (xTrans.is())
        return xTrans->isDataFlavorSupported(rFlavor);

    const std::vector<css::datatransfer::DataFlavor> aAll = getTransferDataFlavorsAsVector();
    return std::any_of(aAll.begin(), aAll.end(), [&](const css::datatransfer::DataFlavor& aFlavor) {
        return rFlavor.MimeType == aFlavor.MimeType;
    });
}

Qt5ClipboardTransferable::Qt5ClipboardTransferable(QClipboard::Mode aMode)
    : m_aMode(aMode)
{
}

const QMimeData* Qt5ClipboardTransferable::mimeData() const
{
    return QApplication::clipboard()->mimeData(m_aMode);
}

const css::uno::Reference<css::datatransfer::XTransferable>
Qt5ClipboardTransferable::xTransferable() const
{
    const Qt5MimeData* pQt5MimeData = dynamic_cast<const Qt5MimeData*>(mimeData());
    if (pQt5MimeData)
        return pQt5MimeData->m_aContents;
    return css::uno::Reference<css::datatransfer::XTransferable>();
}

css::uno::Any SAL_CALL
Qt5ClipboardTransferable::getTransferData(const css::datatransfer::DataFlavor& rFlavor)
{
    css::uno::Any aAny;
    css::uno::Reference<css::datatransfer::XTransferable> xTrans(xTransferable());
    if (xTrans.is())
    {
        aAny = xTrans->getTransferData(rFlavor);
        return aAny;
    }

    const QMimeData* pMimeData = mimeData();
    if (rFlavor.MimeType == "text/plain;charset=utf-16")
    {
        QString clipboardContent = pMimeData->text();
        OUString sContent = toOUString(clipboardContent);

        aAny <<= sContent.replaceAll("\r\n", "\n");
    }
    else if (rFlavor.MimeType == "text/html")
    {
        QString clipboardContent = pMimeData->html();
        std::string aStr = clipboardContent.toStdString();
        css::uno::Sequence<sal_Int8> aSeq(reinterpret_cast<const sal_Int8*>(aStr.c_str()),
                                          aStr.length());
        aAny <<= aSeq;
    }
    else if (rFlavor.MimeType.startsWith("image") && pMimeData->hasImage())
    {
        QImage image = qvariant_cast<QImage>(pMimeData->imageData());
        QByteArray ba;
        QBuffer buffer(&ba);
        sal_Int32 nIndex = rFlavor.MimeType.indexOf('/');
        OUString sFormat(nIndex != -1 ? rFlavor.MimeType.copy(nIndex + 1) : "png");

        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, sFormat.toUtf8().getStr());

        css::uno::Sequence<sal_Int8> aSeq(reinterpret_cast<const sal_Int8*>(ba.data()), ba.size());
        aAny <<= aSeq;
    }

    return aAny;
}

Qt5DnDTransferable::Qt5DnDTransferable(const QMimeData* pMimeData)
    : m_pMimeData(pMimeData)
{
}

const QMimeData* Qt5DnDTransferable::mimeData() const { return m_pMimeData; }

const css::uno::Reference<css::datatransfer::XTransferable>
Qt5DnDTransferable::xTransferable() const
{
    return nullptr;
}

css::uno::Any Qt5DnDTransferable::getTransferData(const css::datatransfer::DataFlavor&)
{
    css::uno::Any aAny;
    assert(m_pMimeData);

    // FIXME: not sure if we should support more mimetypes here
    // (how to carry out external DnD with anything else than [file] URL?)
    if (m_pMimeData->hasUrls())
    {
        QList<QUrl> urlList = m_pMimeData->urls();

        if (urlList.size() > 0)
        {
            std::string aStr;

            // transfer data is list of URLs
            for (int i = 0; i < urlList.size(); ++i)
            {
                QString url = urlList.at(i).path();
                aStr += url.toStdString();
                // separated by newline if more than 1
                if (i < urlList.size() - 1)
                    aStr += "\n";
            }

            css::uno::Sequence<sal_Int8> aSeq(reinterpret_cast<const sal_Int8*>(aStr.c_str()),
                                              aStr.length());
            aAny <<= aSeq;
        }
    }
    return aAny;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
