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

Qt5Transferable::Qt5Transferable(const QMimeData* pMimeData)
    : m_pMimeData(pMimeData)
{
}

std::vector<css::datatransfer::DataFlavor> Qt5Transferable::getTransferDataFlavorsAsVector()
{
    std::vector<css::datatransfer::DataFlavor> aVector;
    assert(m_pMimeData);
    if (!m_pMimeData)
        return aVector;

    css::datatransfer::DataFlavor aFlavor;
    for (QString& rMimeType : m_pMimeData->formats())
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

bool Qt5Transferable::getXTransferableData(const css::datatransfer::DataFlavor& rFlavor,
                                           css::uno::Any& rAny)
{
    assert(m_pMimeData);
    if (!m_pMimeData)
        return true;

    const Qt5MimeData* pMimeData = dynamic_cast<const Qt5MimeData*>(m_pMimeData);
    if (pMimeData && pMimeData->m_aContents.is())
    {
        rAny = pMimeData->m_aContents->getTransferData(rFlavor);
        return true;
    }

    return false;
}

Qt5ClipboardTransferable::Qt5ClipboardTransferable(QClipboard::Mode aMode)
    : Qt5Transferable(QApplication::clipboard()->mimeData(aMode))
{
}

css::uno::Any SAL_CALL
Qt5ClipboardTransferable::getTransferData(const css::datatransfer::DataFlavor& rFlavor)
{
    css::uno::Any aAny;
    if (getXTransferableData(rFlavor, aAny))
        return aAny;

    if (rFlavor.MimeType == "text/plain;charset=utf-16")
    {
        QString clipboardContent = m_pMimeData->text();
        OUString sContent = toOUString(clipboardContent);

        aAny <<= sContent.replaceAll("\r\n", "\n");
    }
    else if (rFlavor.MimeType == "text/html")
    {
        QString clipboardContent = m_pMimeData->html();
        std::string aStr = clipboardContent.toStdString();
        css::uno::Sequence<sal_Int8> aSeq(reinterpret_cast<const sal_Int8*>(aStr.c_str()),
                                          aStr.length());
        aAny <<= aSeq;
    }
    else if (rFlavor.MimeType.startsWith("image") && m_pMimeData->hasImage())
    {
        QImage image = qvariant_cast<QImage>(m_pMimeData->imageData());
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
    : Qt5Transferable(pMimeData)
{
}

css::uno::Any Qt5DnDTransferable::getTransferData(const css::datatransfer::DataFlavor& rFlavor)
{
    css::uno::Any aAny;
    if (getXTransferableData(rFlavor, aAny))
        return aAny;

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
