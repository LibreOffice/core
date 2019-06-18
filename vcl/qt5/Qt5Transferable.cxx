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

static bool lcl_textMimeInfo(const OUString& rMimeString, bool& bHaveNoCharset, bool& bHaveUTF16,
                             bool& bHaveUTF8)
{
    sal_Int32 nIndex = 0;
    if (rMimeString.getToken(0, ';', nIndex) == "text/plain")
    {
        OUString aToken(rMimeString.getToken(0, ';', nIndex));
        if (aToken == "charset=utf-16")
            bHaveUTF16 = true;
        else if (aToken == "charset=utf-8")
            bHaveUTF8 = true;
        else if (aToken.isEmpty())
            bHaveNoCharset = true;
        else // we just handle UTF-16 and UTF-8, everything else is "bytes"
            return false;
        return true;
    }
    return false;
}

Qt5Transferable::Qt5Transferable(const QMimeData* pMimeData)
    : m_pMimeData(pMimeData)
    , m_bConvertFromLocale(false)
{
    assert(pMimeData);
}

css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL Qt5Transferable::getTransferDataFlavors()
{
    // it's just filled once, ever, so just try to get it without locking first
    if (m_aMimeTypeSeq.hasElements())
        return m_aMimeTypeSeq;

    // better safe then sorry; preventing broken usage
    // DnD should not be shared and Clipboard access runs in the GUI thread
    osl::MutexGuard aGuard(m_aMutex);
    if (m_aMimeTypeSeq.hasElements())
        return m_aMimeTypeSeq;

    QStringList aFormatList(m_pMimeData->formats());
    // we might add the UTF-16 mime text variant later
    const int nMimeTypeSeqSize = aFormatList.size() + 1;
    bool bHaveNoCharset = false, bHaveUTF16 = false;
    css::uno::Sequence<css::datatransfer::DataFlavor> aMimeTypeSeq(nMimeTypeSeqSize);

    css::datatransfer::DataFlavor aFlavor;
    int nMimeTypeCount = 0;

    for (const QString& rMimeType : aFormatList)
    {
        // filter out non-MIME types such as TARGETS, MULTIPLE, TIMESTAMP
        if (rMimeType.indexOf('/') == -1)
            continue;

        // gtk3 thinks it is not well defined - skip too
        if (rMimeType == QStringLiteral("text/plain;charset=unicode"))
            continue;

        // LO doesn't like 'text/plain', so we have to provide UTF-16
        bool bIsNoCharset = false, bIsUTF16 = false, bIsUTF8 = false;
        if (lcl_textMimeInfo(toOUString(rMimeType), bIsNoCharset, bIsUTF16, bIsUTF8))
        {
            bHaveNoCharset |= bIsNoCharset;
            bHaveUTF16 |= bIsUTF16;
            if (bIsUTF16)
                aFlavor.DataType = cppu::UnoType<OUString>::get();
            else
                aFlavor.DataType = cppu::UnoType<css::uno::Sequence<sal_Int8>>::get();
        }
        else
            aFlavor.DataType = cppu::UnoType<css::uno::Sequence<sal_Int8>>::get();

        aFlavor.MimeType = toOUString(rMimeType);
        assert(nMimeTypeCount < nMimeTypeSeqSize);
        aMimeTypeSeq[nMimeTypeCount] = aFlavor;
        nMimeTypeCount++;
    }

    m_bConvertFromLocale = bHaveNoCharset && !bHaveUTF16;
    if (m_bConvertFromLocale)
    {
        aFlavor.MimeType = "text/plain;charset=utf-16";
        aFlavor.DataType = cppu::UnoType<OUString>::get();
        assert(nMimeTypeCount < nMimeTypeSeqSize);
        aMimeTypeSeq[nMimeTypeCount] = aFlavor;
        nMimeTypeCount++;
    }

    aMimeTypeSeq.realloc(nMimeTypeCount);

    m_aMimeTypeSeq = aMimeTypeSeq;
    return m_aMimeTypeSeq;
}

sal_Bool SAL_CALL
Qt5Transferable::isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor)
{
    const auto aSeq = getTransferDataFlavors();
    return std::any_of(aSeq.begin(), aSeq.end(), [&](const css::datatransfer::DataFlavor& aFlavor) {
        return rFlavor.MimeType == aFlavor.MimeType;
    });
}

css::uno::Any SAL_CALL
Qt5Transferable::getTransferData(const css::datatransfer::DataFlavor& rFlavor)
{
    css::uno::Any aAny;
    if (!isDataFlavorSupported(rFlavor))
        return aAny;

    if (rFlavor.MimeType == "text/plain;charset=utf-16")
    {
        OUString aString;
        if (m_bConvertFromLocale)
        {
            QByteArray aByteData(m_pMimeData->data(QStringLiteral("text/plain")));
            aString = OUString(reinterpret_cast<const sal_Char*>(aByteData.data()),
                               aByteData.size(), osl_getThreadTextEncoding());
        }
        else
        {
            QByteArray aByteData(m_pMimeData->data(toQString(rFlavor.MimeType)));
            aString = OUString(reinterpret_cast<const sal_Unicode*>(aByteData.data()),
                               aByteData.size() / 2);
        }
        aAny <<= aString;
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

Qt5ClipboardTransferable::Qt5ClipboardTransferable(const QClipboard::Mode aMode,
                                                   const QMimeData* pMimeData)
    : Qt5Transferable(pMimeData)
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
    , m_bHaveNoCharset(false)
    , m_bHaveUTF8(false)
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

QStringList Qt5MimeData::formats() const
{
    if (!m_aMimeTypeList.isEmpty())
        return m_aMimeTypeList;

    css::uno::Sequence<css::datatransfer::DataFlavor> aFormats
        = m_aContents->getTransferDataFlavors();
    QStringList aList;
    bool bHaveUTF16 = false;

    for (const auto& rFlavor : aFormats)
    {
        aList << toQString(rFlavor.MimeType);
        lcl_textMimeInfo(rFlavor.MimeType, m_bHaveNoCharset, bHaveUTF16, m_bHaveUTF8);
    }

    // we provide a locale encoded and an UTF-8 variant, if missing
    if (m_bHaveNoCharset || bHaveUTF16 || m_bHaveUTF8)
    {
        // if there is a text representation from LO point of view, it'll be UTF-16
        assert(bHaveUTF16);
        if (!m_bHaveUTF8)
            aList << QStringLiteral("text/plain;charset=utf-8");
        if (!m_bHaveNoCharset)
            aList << QStringLiteral("text/plain");
    }

    m_aMimeTypeList = aList;
    return m_aMimeTypeList;
}

QVariant Qt5MimeData::retrieveData(const QString& mimeType, QVariant::Type) const
{
    if (!hasFormat(mimeType))
        return QVariant();

    css::datatransfer::DataFlavor aFlavor;
    aFlavor.MimeType = toOUString(mimeType);
    aFlavor.DataType = cppu::UnoType<css::uno::Sequence<sal_Int8>>::get();

    bool bWantNoCharset = false, bWantUTF16 = false, bWantUTF8 = false;
    if (lcl_textMimeInfo(aFlavor.MimeType, bWantNoCharset, bWantUTF16, bWantUTF8))
    {
        if ((bWantNoCharset && !m_bHaveNoCharset) || (bWantUTF8 && !m_bHaveUTF8))
        {
            aFlavor.MimeType = "text/plain;charset=utf-16";
            aFlavor.DataType = cppu::UnoType<OUString>::get();
        }
        else if (bWantUTF16)
            aFlavor.DataType = cppu::UnoType<OUString>::get();
    }

    css::uno::Any aValue;

    try
    {
        aValue = m_aContents->getTransferData(aFlavor);
    }
    catch (...)
    {
    }

    QByteArray aByteArray;
    if (aValue.getValueTypeClass() == css::uno::TypeClass_STRING)
    {
        OUString aString;
        aValue >>= aString;

        if (bWantUTF8)
        {
            OString aUTF8String(OUStringToOString(aString, RTL_TEXTENCODING_UTF8));
            aByteArray = QByteArray(reinterpret_cast<const char*>(aUTF8String.getStr()),
                                    aUTF8String.getLength());
        }
        else if (bWantNoCharset)
        {
            OString aLocaleString(OUStringToOString(aString, osl_getThreadTextEncoding()));
            aByteArray = QByteArray(reinterpret_cast<const char*>(aLocaleString.getStr()),
                                    aLocaleString.getLength());
        }
        else
            return QVariant(toQString(aString));
    }
    else
    {
        css::uno::Sequence<sal_Int8> aData;
        aValue >>= aData;
        aByteArray
            = QByteArray(reinterpret_cast<const char*>(aData.getConstArray()), aData.getLength());
    }
    return QVariant::fromValue(aByteArray);
}

bool Qt5MimeData::hasFormat(const QString& mimeType) const { return formats().contains(mimeType); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
