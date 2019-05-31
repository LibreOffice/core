/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <cppuhelper/compbase.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>

#include <QtCore/QMimeData>
#include <QtCore/QStringList>
#include <QtGui/QClipboard>

/**
 * Helper class to provide the requested string values
 **/
class StringConverter
{
public: // so it can be accessed by o3tl::typed_flags (or find a working friend)
    enum class PlainTextType
    {
        NONE = 0x0,
        NoCharset = 0x1,
        UTF16 = 0x2,
        UTF8 = 0x4,
    };

private:
    PlainTextType m_eHaveType;

public:
    StringConverter();

    void setHaveType(bool bHaveNoCharset, bool bHaveUTF16, bool bHaveUTF8);

    bool getFetchType(const OUString& aMimeWant, bool& bWantUTF8, OUString& aMimeFetch,
                      bool& bFetchUTF16);
    bool getStringAsAny(const QMimeData*, const OUString& aMimeWant, css::uno::Any&);
};

/**
 * Qt5Transferable classes are used to read QMimeData via the XTransferable
 * interface. All the functionality is already implemented in the Qt5Transferable.
 *
 * The specialisations map to the two users, which provide QMimeData: the
 * Clipboard and the Drag'n'Drop functionality.
 *
 * It's the "mirror" interface of the Qt5MimeData, which is defined below.
 **/
class Qt5Transferable : public cppu::WeakImplHelper<css::datatransfer::XTransferable>
{
    Qt5Transferable(const Qt5Transferable&) = delete;

    const QMimeData* m_pMimeData;
    osl::Mutex m_aMutex;
    StringConverter m_aConverter;
    css::uno::Sequence<css::datatransfer::DataFlavor> m_aMimeTypeSeq;

    css::uno::Sequence<css::datatransfer::DataFlavor>& filledMimeTypeSeq();

public:
    Qt5Transferable(const QMimeData* pMimeData);
    const QMimeData* mimeData() const { return m_pMimeData; }

    css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors() override;
    sal_Bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor) override;
    css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;
};

/**
 * The QClipboard's QMimeData is volatile. As written in the QClipboard::mimeData
 * documentation, "the pointer returned might become invalidated when the contents
 * of the clipboard changes". Therefore it can just be accessed reliable inside
 * the QClipboard's object thread, which is the QApplication's thread, so all of
 * the access has to go through RunInMainThread().
 *
 * If we detect a QMimeData change, we simply drop reporting any content. In theory
 * we can recover in the case where there hadn't been any calls of the XTransferable
 * interface, but currently we don't. But we ensure to never report mixed content,
 * so we'll just cease operation on QMimeData change.
 **/
class Qt5ClipboardTransferable final : public Qt5Transferable
{
    // to detect in-flight QMimeData changes
    const QClipboard::Mode m_aMode;

    bool hasInFlightChanged() const;

public:
    explicit Qt5ClipboardTransferable(const QClipboard::Mode aMode, const QMimeData* pMimeData);

    // these are the same then Qt5Transferable, except they go through RunInMainThread
    css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors() override;
    sal_Bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& rFlavor) override;
    css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override;
};

/**
 * Convenience typedef for better code readability
 *
 * This just uses the QMimeData provided by the QWidgets D'n'D events.
 **/
typedef Qt5Transferable Qt5DnDTransferable;

/**
 * A lazy loading QMimeData for XTransferable reads
 *
 * This is an interface class to make a XTransferable read accessible as a
 * QMimeData. The mime data is just stored inside the XTransferable, never
 * in the QMimeData itself! It's objects are just used for QClipboard to read
 * the XTransferable data.
 *
 * Like XTransferable itself, this class should be considered an immutable
 * container for mime data. There is no need to ever set any of its data.
 *
 * It's the "mirror" interface of the Qt5Transferable.
 **/
class Qt5MimeData final : public QMimeData
{
    friend class Qt5ClipboardTransferable;

    const css::uno::Reference<css::datatransfer::XTransferable> m_aContents;
    mutable StringConverter m_aConverter;
    mutable QStringList m_aMimeTypeList;

    QStringList& filledMimeTypeList() const;
    QVariant retrieveData(const QString& mimeType, QVariant::Type type) const override;

public:
    explicit Qt5MimeData(const css::uno::Reference<css::datatransfer::XTransferable>& aContents);

    bool hasFormat(const QString& mimeType) const override;
    QStringList formats() const override;

    bool deepCopy(QMimeData** const) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
