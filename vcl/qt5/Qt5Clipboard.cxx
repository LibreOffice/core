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

#include <Qt5Clipboard.hxx>
#include <Qt5Clipboard.moc>
#include <Qt5Transferable.hxx>
#include <Qt5Tools.hxx>

#include <cassert>
#include <map>

using namespace com::sun::star;

Qt5MimeData::Qt5MimeData(uno::Reference<css::datatransfer::XTransferable>& xTrans)
    : m_aContents(xTrans)
{
}

bool Qt5MimeData::deepCopy(QMimeData** const pMimeCopy) const
{
    assert(m_aContents.is() && pMimeCopy);
    if (!m_aContents.is() || !pMimeCopy)
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

QVariant Qt5MimeData::retrieveData(const QString& mimeType, QVariant::Type type) const
{
    if (!m_aContents.is())
        return QMimeData::retrieveData(mimeType, type);

    css::datatransfer::DataFlavor aFlavor;
    aFlavor.MimeType = toOUString(mimeType);
    aFlavor.DataType = cppu::UnoType<uno::Sequence<sal_Int8>>::get();
    if (aFlavor.MimeType == "text/plain")
        aFlavor.MimeType += ";charset=utf-16";

    uno::Sequence<sal_Int8> aData;
    uno::Any aValue;

    try
    {
        aValue = m_aContents->getTransferData(aFlavor);
    }
    catch (...)
    {
    }

    if (aValue.getValueTypeClass() == uno::TypeClass_STRING)
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
    if (!m_aContents.is())
        return QMimeData::hasFormat(mimeType);

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
    if (!m_aContents.is())
        return QMimeData::formats();

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

Qt5Clipboard::Qt5Clipboard(const OUString& aModeString, const QClipboard::Mode aMode)
    : cppu::WeakComponentImplHelper<datatransfer::clipboard::XSystemClipboard,
                                    datatransfer::clipboard::XFlushableClipboard, XServiceInfo>(
          m_aMutex)
    , m_aClipboardName(aModeString)
    , m_aClipboardMode(aMode)
{
    assert(isSupported(m_aClipboardMode));
    connect(QApplication::clipboard(), &QClipboard::changed, this, &Qt5Clipboard::handleChanged,
            Qt::DirectConnection);
}

css::uno::Reference<css::uno::XInterface> Qt5Clipboard::create(const OUString& aModeString)
{
    static const std::map<OUString, QClipboard::Mode> aNameToClipboardMap
        = { { "CLIPBOARD", QClipboard::Clipboard }, { "PRIMARY", QClipboard::Selection } };

    auto iter = aNameToClipboardMap.find(aModeString);
    if (iter != aNameToClipboardMap.end() && isSupported(iter->second))
        return static_cast<cppu::OWeakObject*>(new Qt5Clipboard(aModeString, iter->second));
    SAL_WARN("vcl.qt5", "Ignoring unrecognized clipboard type: '" << aModeString << "'");
    return css::uno::Reference<css::uno::XInterface>();
}

void Qt5Clipboard::flushClipboard()
{
    if (!isOwner(m_aClipboardMode))
        return;

    QClipboard* pClipboard = QApplication::clipboard();
    const Qt5MimeData* pQt5MimeData
        = dynamic_cast<const Qt5MimeData*>(pClipboard->mimeData(m_aClipboardMode));
    assert(pQt5MimeData);
    QMimeData* pMimeCopy = nullptr;
    if (pQt5MimeData && pQt5MimeData->deepCopy(&pMimeCopy))
        pClipboard->setMimeData(pMimeCopy, m_aClipboardMode);
}

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
    if (!m_aContents.is() && isSupported(m_aClipboardMode))
        m_aContents = new Qt5ClipboardTransferable(m_aClipboardMode);
    return m_aContents;
}

void Qt5Clipboard::setContents(
    const uno::Reference<css::datatransfer::XTransferable>& xTrans,
    const uno::Reference<css::datatransfer::clipboard::XClipboardOwner>& xClipboardOwner)
{
    QClipboard* pClipboard = QApplication::clipboard();

    if (!isSupported(m_aClipboardMode))
        return;

    osl::ClearableMutexGuard aGuard(m_aMutex);
    uno::Reference<datatransfer::clipboard::XClipboardOwner> xOldOwner(m_aOwner);
    uno::Reference<datatransfer::XTransferable> xOldContents(m_aContents);
    m_aContents = xTrans;
    m_aOwner = xClipboardOwner;

    if (m_aContents.is())
        pClipboard->setMimeData(new Qt5MimeData(m_aContents), m_aClipboardMode);
    else if (isOwner(m_aClipboardMode))
        pClipboard->clear(m_aClipboardMode);

    std::vector<uno::Reference<datatransfer::clipboard::XClipboardListener>> aListeners(
        m_aListeners);
    datatransfer::clipboard::ClipboardEvent aEv;
    aEv.Contents = getContents();

    aGuard.clear();

    if (xOldOwner.is() && xOldOwner != xClipboardOwner)
        xOldOwner->lostOwnership(this, xOldContents);
    for (auto const& listener : aListeners)
        listener->changedContents(aEv);
}

OUString Qt5Clipboard::getName() { return m_aClipboardName; }

sal_Int8 Qt5Clipboard::getRenderingCapabilities() { return 0; }

void Qt5Clipboard::addClipboardListener(
    const uno::Reference<datatransfer::clipboard::XClipboardListener>& listener)
{
    osl::MutexGuard aGuard(m_aMutex);
    m_aListeners.push_back(listener);
}

void Qt5Clipboard::removeClipboardListener(
    const uno::Reference<datatransfer::clipboard::XClipboardListener>& listener)
{
    osl::MutexGuard aGuard(m_aMutex);
    m_aListeners.erase(std::remove(m_aListeners.begin(), m_aListeners.end(), listener),
                       m_aListeners.end());
}

bool Qt5Clipboard::isSupported(const QClipboard::Mode aMode)
{
    const QClipboard* pClipboard = QApplication::clipboard();
    switch (aMode)
    {
        case QClipboard::Selection:
            if (pClipboard->supportsSelection())
                return true;
            break;

        case QClipboard::FindBuffer:
            if (pClipboard->supportsFindBuffer())
                return true;
            break;

        case QClipboard::Clipboard:
            return true;
    }
    return false;
}

bool Qt5Clipboard::isOwner(const QClipboard::Mode aMode)
{
    if (!isSupported(aMode))
        return false;

    const QClipboard* pClipboard = QApplication::clipboard();
    switch (aMode)
    {
        case QClipboard::Selection:
            if (pClipboard->ownsSelection())
                return true;
            break;

        case QClipboard::FindBuffer:
            if (pClipboard->ownsFindBuffer())
                return true;
            break;

        case QClipboard::Clipboard:
            if (pClipboard->ownsClipboard())
                return true;
            break;
    }
    return false;
}

void Qt5Clipboard::handleChanged(QClipboard::Mode aMode)
{
    if (aMode != m_aClipboardMode)
        return;

    // if we're still the owner there is nothing to update
    if (isOwner(aMode))
        return;

    setContents(uno::Reference<css::datatransfer::XTransferable>(),
                uno::Reference<css::datatransfer::clipboard::XClipboardOwner>());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
