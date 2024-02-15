/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <QtTransferable.hxx>

#include <comphelper/sequence.hxx>
#include <sal/log.hxx>
#include <o3tl/string_view.hxx>

#include <QtWidgets/QApplication>

#include <QtInstance.hxx>
#include <QtTools.hxx>

#include <cassert>

static bool lcl_textMimeInfo(std::u16string_view rMimeString, bool& bHaveNoCharset,
                             bool& bHaveUTF16, bool& bHaveUTF8)
{
    sal_Int32 nIndex = 0;
    if (o3tl::getToken(rMimeString, 0, ';', nIndex) == u"text/plain")
    {
        std::u16string_view aToken(o3tl::getToken(rMimeString, 0, ';', nIndex));
        if (aToken == u"charset=utf-16")
            bHaveUTF16 = true;
        else if (aToken == u"charset=utf-8")
            bHaveUTF8 = true;
        else if (aToken.empty())
            bHaveNoCharset = true;
        else // we just handle UTF-16 and UTF-8, everything else is "bytes"
            return false;
        return true;
    }
    return false;
}

QtTransferable::QtTransferable(const QMimeData* pMimeData)
    : m_pMimeData(pMimeData)
    , m_bProvideUTF16FromOtherEncoding(false)
{
    assert(pMimeData);
}

css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL QtTransferable::getTransferDataFlavors()
{
    if (!m_pMimeData)
        return css::uno::Sequence<css::datatransfer::DataFlavor>();

    QStringList aFormatList(m_pMimeData->formats());
    // we might add the UTF-16 mime text variant later
    const int nMimeTypeSeqSize = aFormatList.size() + 1;
    bool bHaveNoCharset = false, bHaveUTF16 = false, bHaveUTF8 = false;
    css::uno::Sequence<css::datatransfer::DataFlavor> aMimeTypeSeq(nMimeTypeSeqSize);
    auto pMimeTypeSeq = aMimeTypeSeq.getArray();

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
            bHaveUTF8 |= bIsUTF8;
            if (bIsUTF16)
                aFlavor.DataType = cppu::UnoType<OUString>::get();
            else
                aFlavor.DataType = cppu::UnoType<css::uno::Sequence<sal_Int8>>::get();
        }
        else
            aFlavor.DataType = cppu::UnoType<css::uno::Sequence<sal_Int8>>::get();

        aFlavor.MimeType = toOUString(rMimeType);
        assert(nMimeTypeCount < nMimeTypeSeqSize);
        pMimeTypeSeq[nMimeTypeCount] = aFlavor;
        nMimeTypeCount++;
    }

    m_bProvideUTF16FromOtherEncoding = (bHaveNoCharset || bHaveUTF8) && !bHaveUTF16;
    if (m_bProvideUTF16FromOtherEncoding)
    {
        aFlavor.MimeType = "text/plain;charset=utf-16";
        aFlavor.DataType = cppu::UnoType<OUString>::get();
        assert(nMimeTypeCount < nMimeTypeSeqSize);
        pMimeTypeSeq[nMimeTypeCount] = aFlavor;
        nMimeTypeCount++;
    }

    aMimeTypeSeq.realloc(nMimeTypeCount);

    return aMimeTypeSeq;
}

sal_Bool SAL_CALL
QtTransferable::isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor)
{
    const auto aSeq = getTransferDataFlavors();
    return std::any_of(aSeq.begin(), aSeq.end(), [&](const css::datatransfer::DataFlavor& aFlavor) {
        return rFlavor.MimeType == aFlavor.MimeType;
    });
}

css::uno::Any SAL_CALL QtTransferable::getTransferData(const css::datatransfer::DataFlavor& rFlavor)
{
    css::uno::Any aAny;
    if (!isDataFlavorSupported(rFlavor))
        return aAny;

    if (rFlavor.MimeType == "text/plain;charset=utf-16")
    {
        OUString aString;
        if (m_bProvideUTF16FromOtherEncoding)
        {
            if (m_pMimeData->hasFormat("text/plain;charset=utf-8"))
            {
                QByteArray aByteData(m_pMimeData->data(QStringLiteral("text/plain;charset=utf-8")));
                aString = OUString::fromUtf8(reinterpret_cast<const char*>(aByteData.data()));
            }
            else
            {
                QByteArray aByteData(m_pMimeData->data(QStringLiteral("text/plain")));
                aString = OUString(reinterpret_cast<const char*>(aByteData.data()),
                                   aByteData.size(), osl_getThreadTextEncoding());
            }
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

QtClipboardTransferable::QtClipboardTransferable(const QClipboard::Mode aMode,
                                                 const QMimeData* pMimeData)
    : QtTransferable(pMimeData)
    , m_aMode(aMode)
{
}

void QtClipboardTransferable::ensureConsistencyWithSystemClipboard()
{
    const QMimeData* pCurrentClipboardData = QApplication::clipboard()->mimeData(m_aMode);
    if (mimeData() != pCurrentClipboardData)
    {
        SAL_WARN("vcl.qt", "In flight clipboard change detected - updating mime data with current "
                           "clipboard contents.");
        setMimeData(pCurrentClipboardData);
    }
}

css::uno::Any SAL_CALL
QtClipboardTransferable::getTransferData(const css::datatransfer::DataFlavor& rFlavor)
{
    css::uno::Any aAny;
    auto* pSalInst(GetQtInstance());
    SolarMutexGuard g;
    pSalInst->RunInMainThread([&, this]() {
        ensureConsistencyWithSystemClipboard();
        aAny = QtTransferable::getTransferData(rFlavor);
    });
    return aAny;
}

css::uno::Sequence<css::datatransfer::DataFlavor>
    SAL_CALL QtClipboardTransferable::getTransferDataFlavors()
{
    css::uno::Sequence<css::datatransfer::DataFlavor> aSeq;
    auto* pSalInst(GetQtInstance());
    SolarMutexGuard g;
    pSalInst->RunInMainThread([&, this]() {
        ensureConsistencyWithSystemClipboard();
        aSeq = QtTransferable::getTransferDataFlavors();
    });
    return aSeq;
}

sal_Bool SAL_CALL
QtClipboardTransferable::isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor)
{
    bool bIsSupported = false;
    auto* pSalInst(GetQtInstance());
    SolarMutexGuard g;
    pSalInst->RunInMainThread([&, this]() {
        ensureConsistencyWithSystemClipboard();
        bIsSupported = QtTransferable::isDataFlavorSupported(rFlavor);
    });
    return bIsSupported;
}

QtMimeData::QtMimeData(const css::uno::Reference<css::datatransfer::XTransferable>& xTrans)
    : m_aContents(xTrans)
    , m_bHaveNoCharset(false)
    , m_bHaveUTF8(false)
{
    assert(xTrans.is());
}

bool QtMimeData::deepCopy(QMimeData** const pMimeCopy) const
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

QStringList QtMimeData::formats() const
{
    if (!m_aMimeTypeList.isEmpty())
        return m_aMimeTypeList;

    const css::uno::Sequence<css::datatransfer::DataFlavor> aFormats
        = m_aContents->getTransferDataFlavors();
    QStringList aList;
    bool bHaveUTF16 = false;

    for (const auto& rFlavor : aFormats)
    {
        aList << toQString(rFlavor.MimeType);
        lcl_textMimeInfo(rFlavor.MimeType, m_bHaveNoCharset, bHaveUTF16, m_bHaveUTF8);
    }

    // we provide a locale encoded and a UTF-8 variant, if missing
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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QVariant QtMimeData::retrieveData(const QString& mimeType, QVariant::Type) const
#else
QVariant QtMimeData::retrieveData(const QString& mimeType, QMetaType) const
#endif
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
        // tdf#129809 take a reference in case m_aContents is replaced during this call
        css::uno::Reference<com::sun::star::datatransfer::XTransferable> xCurrentContents(
            m_aContents);
        aValue = xCurrentContents->getTransferData(aFlavor);
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
            aByteArray = QByteArray(aUTF8String.getStr(), aUTF8String.getLength());
        }
        else if (bWantNoCharset)
        {
            OString aLocaleString(OUStringToOString(aString, osl_getThreadTextEncoding()));
            aByteArray = QByteArray(aLocaleString.getStr(), aLocaleString.getLength());
        }
        else if (bWantUTF16)
        {
            aByteArray = QByteArray(reinterpret_cast<const char*>(aString.getStr()),
                                    aString.getLength() * 2);
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

bool QtMimeData::hasFormat(const QString& mimeType) const { return formats().contains(mimeType); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
