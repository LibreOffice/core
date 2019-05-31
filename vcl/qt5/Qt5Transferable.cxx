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

#include <comphelper/sequence.hxx>
#include <sal/log.hxx>

#include <QtWidgets/QApplication>

#include <Qt5Instance.hxx>
#include <Qt5Tools.hxx>

#include <cassert>

Qt5Transferable::Qt5Transferable(const QMimeData* pMimeData)
    : m_pMimeData(pMimeData)
{
    assert(pMimeData);
}

std::vector<css::datatransfer::DataFlavor>& Qt5Transferable::getTransferDataFlavorsAsVector()
{
    if (!m_aMimeTypes.empty())
        return m_aMimeTypes;

    css::datatransfer::DataFlavor aFlavor;
    for (QString& rMimeType : m_pMimeData->formats())
    {
        // filter out non-MIME types such as TARGETS, MULTIPLE, TIMESTAMP
        if (rMimeType.indexOf('/') == -1)
            continue;

        if (rMimeType.startsWith("text/plain"))
        {
            rMimeType = "text/plain;charset=utf-16";
            aFlavor.DataType = cppu::UnoType<OUString>::get();
        }
        else if (rMimeType == "text/uri-list")
            aFlavor.DataType = cppu::UnoType<css::uno::Sequence<OUString>>::get();
        else
            aFlavor.DataType = cppu::UnoType<css::uno::Sequence<sal_Int8>>::get();

        aFlavor.MimeType = toOUString(rMimeType);
        m_aMimeTypes.push_back(aFlavor);
    }

    return m_aMimeTypes;
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
    });
}

css::uno::Any SAL_CALL
Qt5Transferable::getTransferData(const css::datatransfer::DataFlavor& rFlavor)
{
    css::uno::Any aAny;

    if (rFlavor.MimeType == "text/plain;charset=utf-16" && m_pMimeData->hasText())
        aAny <<= toOUString(m_pMimeData->text());
    else if (rFlavor.MimeType == "text/uri-list" && m_pMimeData->hasUrls())
    {
        QList<QUrl> aUrlList(m_pMimeData->urls());
        css::uno::Sequence<OUString> aSeq(aUrlList.size());
        int i = 0;
        for (auto it = aUrlList.begin(); it != aUrlList.end(); i++, it++)
            aSeq[i] = toOUString(it->toString());
        aAny <<= aSeq;
    }
    else
    {
        QByteArray aByteData(m_pMimeData->data(toQString(rFlavor.MimeType)));
        css::uno::Sequence<sal_Int8> aSeq(reinterpret_cast<const sal_Int8*>(aByteData.data()),
                                          aByteData.size());
        aAny <<= aSeq;
    }

    return aAny;
}

Qt5ClipboardTransferable::Qt5ClipboardTransferable(const QClipboard::Mode aMode)
    : Qt5Transferable(QApplication::clipboard()->mimeData(aMode))
    , m_aMode(aMode)
{
}

bool Qt5ClipboardTransferable::hasInFlightChanged() const
{
    const bool bChanged(mimeData() != QApplication::clipboard()->mimeData(m_aMode));
    SAL_WARN_IF(bChanged, "vcl.qt5",
                "In flight clipboard change detected - broken clipboard read!");
    return bChanged;
}

css::uno::Any SAL_CALL
Qt5ClipboardTransferable::getTransferData(const css::datatransfer::DataFlavor& rFlavor)
{
    css::uno::Any aAny;
    auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
    SolarMutexGuard g;
    pSalInst->RunInMainThread([&, this]() {
        if (!hasInFlightChanged())
            aAny = Qt5Transferable::getTransferData(rFlavor);
    });
    return aAny;
}

css::uno::Sequence<css::datatransfer::DataFlavor>
    SAL_CALL Qt5ClipboardTransferable::getTransferDataFlavors()
{
    css::uno::Sequence<css::datatransfer::DataFlavor> aSeq;
    auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
    SolarMutexGuard g;
    pSalInst->RunInMainThread([&, this]() {
        if (!hasInFlightChanged())
            aSeq = Qt5Transferable::getTransferDataFlavors();
    });
    return aSeq;
}

sal_Bool SAL_CALL
Qt5ClipboardTransferable::isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor)
{
    bool bIsSupported = false;
    auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
    SolarMutexGuard g;
    pSalInst->RunInMainThread([&, this]() {
        if (!hasInFlightChanged())
            bIsSupported = Qt5Transferable::isDataFlavorSupported(rFlavor);
    });
    return bIsSupported;
}

Qt5MimeData::Qt5MimeData(const css::uno::Reference<css::datatransfer::XTransferable>& xTrans)
    : m_aContents(xTrans)
{
    assert(xTrans.is());
}

bool Qt5MimeData::deepCopy(QMimeData** const pMimeCopy) const
{
    if (!pMimeCopy)
        return false;

    QMimeData* pMimeData = new QMimeData();
    for (QString& format : formats())
    {
        QByteArray aData = data(format);
        // Checking for custom MIME types
        if (format.startsWith("application/x-qt"))
        {
            // Retrieving true format name
            int indexBegin = format.indexOf('"') + 1;
            int indexEnd = format.indexOf('"', indexBegin);
            format = format.mid(indexBegin, indexEnd - indexBegin);
        }
        pMimeData->setData(format, aData);
    }

    *pMimeCopy = pMimeData;
    return true;
}

QVariant Qt5MimeData::retrieveData(const QString& mimeType, QVariant::Type) const
{
    css::datatransfer::DataFlavor aFlavor;
    aFlavor.MimeType = toOUString(mimeType);
    aFlavor.DataType = cppu::UnoType<css::uno::Sequence<sal_Int8>>::get();
    if (aFlavor.MimeType == "text/plain")
        aFlavor.MimeType += ";charset=utf-16";

    css::uno::Sequence<sal_Int8> aData;
    css::uno::Any aValue;

    try
    {
        aValue = m_aContents->getTransferData(aFlavor);
    }
    catch (...)
    {
    }

    if (aValue.getValueTypeClass() == css::uno::TypeClass_STRING)
    {
        OUString aString;
        aValue >>= aString;
        return QVariant(toQString(aString));
    }
    else
    {
        aValue >>= aData;
        const QByteArray aQByteArray(reinterpret_cast<const char*>(aData.getConstArray()),
                                     aData.getLength());
        return QVariant::fromValue(aQByteArray);
    }
}

bool Qt5MimeData::hasFormat(const QString& mimeType) const
{
    OUString aSearchString = toOUString(mimeType);
    if (aSearchString == "text/plain")
        aSearchString += ";charset=utf-16";
    css::uno::Sequence<css::datatransfer::DataFlavor> aFormats
        = m_aContents->getTransferDataFlavors();

    return std::any_of(aFormats.begin(), aFormats.end(),
                       [&aSearchString](const auto& rFlavor) -> bool {
                           return rFlavor.MimeType == aSearchString;
                       });
}

QStringList Qt5MimeData::formats() const
{
    css::uno::Sequence<css::datatransfer::DataFlavor> aFormats
        = m_aContents->getTransferDataFlavors();
    QStringList aList;
    bool bHaveText = false, bHaveNoCharset = false, bHaveUTF16 = false;
    for (const auto& rFlavor : aFormats)
    {
        aList << toQString(rFlavor.MimeType);
        sal_Int32 nIndex(0);
        if (rFlavor.MimeType.getToken(0, ';', nIndex) == "text/plain")
        {
            bHaveText = true;
            OUString aToken(rFlavor.MimeType.getToken(0, ';', nIndex));
            if (aToken == "charset=utf-16")
                bHaveUTF16 = true;
            else if (aToken.isEmpty())
                bHaveNoCharset = true;
        }
    }

    if (bHaveText)
    {
        // If we have text, but no UTF-16 format which is basically the only
        // text-format LibreOffice supports for cnp then claim we do and we
        // will convert on demand
        if (!bHaveUTF16)
            aList << toQString("text/plain;charset=utf-16");
        // Qt expects plain "text/plain" for UTF-16
        if (!bHaveNoCharset)
            aList << "text/plain";
    }

    return aList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
