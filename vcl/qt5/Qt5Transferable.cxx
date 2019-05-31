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

#include <o3tl/typed_flags_set.hxx>

namespace o3tl
{
template <>
struct typed_flags<::StringConverter::PlainTextType>
    : is_typed_flags<::StringConverter::PlainTextType, 0x7>
{
};
};

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

StringConverter::StringConverter()
    : m_eHaveType(PlainTextType::NONE)
{
}

void StringConverter::setHaveType(bool bHaveNoCharset, bool bHaveUTF16, bool bHaveUTF8)
{
    m_eHaveType = PlainTextType::NONE;
    if (bHaveNoCharset)
        m_eHaveType |= PlainTextType::NoCharset;
    if (bHaveUTF16)
        m_eHaveType |= PlainTextType::UTF16;
    if (bHaveUTF8)
        m_eHaveType |= PlainTextType::UTF8;
}

bool StringConverter::getFetchType(const OUString& aMimeWant, bool& bWantUTF8, OUString& aMimeFetch,
                                   bool& bFetchUTF16)
{
    if (m_eHaveType == PlainTextType::NONE) // no string available
        return false;

    bool bWantNoCharset = false, bWantUTF16 = false;
    bWantUTF8 = false;
    if (!lcl_textMimeInfo(aMimeWant, bWantNoCharset, bWantUTF16, bWantUTF8))
        return false;

    bFetchUTF16 = false;
    if (bWantNoCharset || bWantUTF16)
    {
        if (m_eHaveType & (PlainTextType::NoCharset | PlainTextType::UTF16))
        {
            bFetchUTF16 = true;
            if (bool(m_eHaveType & PlainTextType::NoCharset) == bWantNoCharset)
                aMimeFetch = OUStringLiteral("text/plain");
            else
                aMimeFetch = OUStringLiteral("text/plain;charset=utf-16");
        }
        else
            aMimeFetch = OUStringLiteral("text/plain;charset=utf-8");
    }
    else
    {
        if (m_eHaveType & PlainTextType::UTF8)
            aMimeFetch = OUStringLiteral("text/plain;charset=utf-8");
        else
        {
            bFetchUTF16 = true;
            if (m_eHaveType & PlainTextType::UTF16)
                aMimeFetch = OUStringLiteral("text/plain;charset=utf-16");
            else
                aMimeFetch = OUStringLiteral("text/plain");
        }
    }
    return true;
}

bool StringConverter::getStringAsAny(const QMimeData* pMimeData, const OUString& aMimeWant,
                                     css::uno::Any& aAny)
{
    OUString aMimeFetch;
    bool bWantUTF8, bFetchUTF16;
    if (!getFetchType(aMimeWant, bWantUTF8, aMimeFetch, bFetchUTF16))
        return false;

    QByteArray aByteData(pMimeData->data(toQString(aMimeFetch)));

    if (bWantUTF8)
    {
        if (bFetchUTF16)
        {
            OString aOString(reinterpret_cast<const sal_Unicode*>(aByteData.data()),
                             aByteData.size() / 2, RTL_TEXTENCODING_UTF8);
            aAny <<= css::uno::Sequence<sal_Int8>(
                reinterpret_cast<const sal_Int8*>(aOString.getStr()), aOString.getLength());
        }
        else
            aAny <<= css::uno::Sequence<sal_Int8>(
                reinterpret_cast<const sal_Int8*>(aByteData.data()), aByteData.size());
    }
    else
    {
        if (bFetchUTF16)
            aAny <<= OUString(reinterpret_cast<const sal_Unicode*>(aByteData.data()),
                              aByteData.size() / 2);
        else
            aAny <<= OUString(reinterpret_cast<const sal_Char*>(aByteData.data()), aByteData.size(),
                              RTL_TEXTENCODING_UTF8);
    }

    return true;
}

Qt5Transferable::Qt5Transferable(const QMimeData* pMimeData)
    : m_pMimeData(pMimeData)
{
    assert(pMimeData);
}

css::uno::Sequence<css::datatransfer::DataFlavor>& Qt5Transferable::filledMimeTypeSeq()
{
    // it's just filled once, ever, so just try to get it unlocked first
    if (m_aMimeTypeSeq.hasElements())
        return m_aMimeTypeSeq;

    // better save then sorry; preventing broken usage
    // DnD should not be shared and Clipboard access runs in the GUI thread
    osl::MutexGuard aGuard(m_aMutex);
    if (m_aMimeTypeSeq.hasElements())
        return m_aMimeTypeSeq;

    QStringList aFormatList(m_pMimeData->formats());
    // we might add two mime text variants later
    const int nMimeTypeSeqSize = aFormatList.size() + 2;
    css::uno::Sequence<css::datatransfer::DataFlavor> aMimeTypeSeq(nMimeTypeSeqSize);

    css::datatransfer::DataFlavor aFlavor;
    bool bHaveNoCharset = false, bHaveUTF16 = false, bHaveUTF8 = false;
    int i = 0;

    for (const QString& rMimeType : aFormatList)
    {
        // filter out non-MIME types such as TARGETS, MULTIPLE, TIMESTAMP
        if (rMimeType.indexOf('/') == -1)
            continue;

        if (lcl_textMimeInfo(toOUString(rMimeType), bHaveNoCharset, bHaveUTF16, bHaveUTF8))
        {
            if (bHaveUTF8)
                aFlavor.DataType = cppu::UnoType<css::uno::Sequence<sal_Int8>>::get();
            else
                aFlavor.DataType = cppu::UnoType<OUString>::get();
        }
        else if (rMimeType == "text/uri-list")
            aFlavor.DataType = cppu::UnoType<css::uno::Sequence<OUString>>::get();
        else
            aFlavor.DataType = cppu::UnoType<css::uno::Sequence<sal_Int8>>::get();

        aFlavor.MimeType = toOUString(rMimeType);
        assert(i < nMimeTypeSeqSize);
        aMimeTypeSeq[i] = aFlavor;
        i++;
    }

    auto addMimeText = [&](const bool bHave, const char* sType, const css::uno::Type aUnoType) {
        if (bHave)
            return;
        aFlavor.MimeType = "text/plain";
        aFlavor.MimeType += OStringToOUString(sType, RTL_TEXTENCODING_UTF8);
        aFlavor.DataType = aUnoType;
        assert(i < nMimeTypeSeqSize);
        aMimeTypeSeq[i] = aFlavor;
        i++;
    };

    if (bHaveNoCharset || bHaveUTF16 || bHaveUTF8)
    {
        addMimeText(bHaveNoCharset, "", cppu::UnoType<OUString>::get());
        addMimeText(bHaveUTF16, ";charset=utf-16", cppu::UnoType<OUString>::get());
        addMimeText(bHaveUTF8, ";charset=utf-8",
                    cppu::UnoType<css::uno::Sequence<sal_Int8>>::get());
    }

    m_aConverter.setHaveType(bHaveNoCharset, bHaveUTF16, bHaveUTF8);
    m_aMimeTypeSeq = aMimeTypeSeq;
    return m_aMimeTypeSeq;
}

css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL Qt5Transferable::getTransferDataFlavors()
{
    return filledMimeTypeSeq();
}

sal_Bool SAL_CALL
Qt5Transferable::isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor)
{
    const auto& aSeq = filledMimeTypeSeq();
    return std::any_of(aSeq.begin(), aSeq.end(), [&](const css::datatransfer::DataFlavor& aFlavor) {
        return rFlavor.MimeType == aFlavor.MimeType;
    });
}

css::uno::Any SAL_CALL
Qt5Transferable::getTransferData(const css::datatransfer::DataFlavor& rFlavor)
{
    css::uno::Any aAny;
    filledMimeTypeSeq();

    if (m_aConverter.getStringAsAny(m_pMimeData, rFlavor.MimeType, aAny))
        return aAny;
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

QStringList& Qt5MimeData::filledMimeTypeList() const
{
    if (!m_aMimeTypeList.isEmpty())
        return m_aMimeTypeList;

    css::uno::Sequence<css::datatransfer::DataFlavor> aFormats
        = m_aContents->getTransferDataFlavors();
    QStringList aList;
    bool bHaveNoCharset = false, bHaveUTF16 = false, bHaveUTF8 = false;

    for (const auto& rFlavor : aFormats)
    {
        aList << toQString(rFlavor.MimeType);
        lcl_textMimeInfo(rFlavor.MimeType, bHaveNoCharset, bHaveUTF16, bHaveUTF8);
    }

    // if we have known UTF text formats, add the direct convertibles
    if (bHaveNoCharset || bHaveUTF16 || bHaveUTF8)
    {
        if (!bHaveUTF16)
            aList << toQString("text/plain;charset=utf-16");
        if (!bHaveUTF8)
            aList << toQString("text/plain;charset=utf-8");
        // Qt expects plain "text/plain" for UTF-16
        if (!bHaveNoCharset)
            aList << "text/plain";
    }

    m_aConverter.setHaveType(bHaveNoCharset, bHaveUTF16, bHaveUTF8);
    m_aMimeTypeList = aList;
    return m_aMimeTypeList;
}

QVariant Qt5MimeData::retrieveData(const QString& mimeType, QVariant::Type) const
{
    css::datatransfer::DataFlavor aFlavor;
    aFlavor.MimeType = toOUString(mimeType);
    aFlavor.DataType = cppu::UnoType<css::uno::Sequence<sal_Int8>>::get();

    filledMimeTypeList();

    OUString aMimeFetch;
    bool bWantUTF8, bFetchUTF16;
    if (m_aConverter.getFetchType(aFlavor.MimeType, bWantUTF8, aMimeFetch, bFetchUTF16))
    {
        aFlavor.MimeType = aMimeFetch;
        if (!bWantUTF8)
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

    if (aValue.getValueTypeClass() == css::uno::TypeClass_STRING)
    {
        OUString aString;
        aValue >>= aString;
        return QVariant(toQString(aString));
    }
    else
    {
        css::uno::Sequence<sal_Int8> aData;
        aValue >>= aData;
        const QByteArray aQByteArray(reinterpret_cast<const char*>(aData.getConstArray()),
                                     aData.getLength());
        if (bWantUTF8)
            return QVariant(QString::fromUtf8(aQByteArray));
        return QVariant::fromValue(aQByteArray);
    }
}

bool Qt5MimeData::hasFormat(const QString& mimeType) const
{
    const auto& aList = filledMimeTypeList();
    return aList.contains(mimeType);
}

QStringList Qt5MimeData::formats() const { return filledMimeTypeList(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
