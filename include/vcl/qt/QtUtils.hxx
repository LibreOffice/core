/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

/**
 * Utilities/helpers for use in Qt-specific LO code used in multiple modules
 * (e.g. avmedia and vcl).
 *
 * Helpers only needed in a specific module should be defined
 * in that module instead, e.g. helper functions only relevant for the Qt-based
 * VCL plugins are defined in `vcl/inc/qt5/QtTools.hxx`.
 **/

#include <rtl/ustring.hxx>
#include <tools/stream.hxx>
#include <vcl/graph.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#include <vcl/image.hxx>
#include <vcl/outdev.hxx>

#include <QtCore/QString>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyle>

inline QString toQString(const OUString& rStr)
{
    return QString::fromUtf16(rStr.getStr(), rStr.getLength());
}

inline OUString toOUString(const QString& s)
{
    // QString stores UTF16, just like OUString
    return OUString(reinterpret_cast<const sal_Unicode*>(s.data()), s.length());
}

inline QPixmap toQPixmap(const Bitmap& rBitmap)
{
    SvMemoryStream aMemoryStream;
    vcl::PngImageWriter aWriter(aMemoryStream);
    aWriter.write(rBitmap);
    QPixmap aPixmap;
    aPixmap.loadFromData(static_cast<const uchar*>(aMemoryStream.GetData()),
                         aMemoryStream.TellEnd());
    assert(!aPixmap.isNull() && "Failed to create icon pixmap");
    return aPixmap;
}

inline QPixmap toQPixmap(const Image& rImage) { return toQPixmap(rImage.GetBitmap()); }

inline QPixmap toQPixmap(const css::uno::Reference<css::graphic::XGraphic>& rImage)
{
    if (!rImage.is())
        return QPixmap();

    Image aImage(rImage);
    return toQPixmap(aImage);
}

inline QPixmap toQPixmap(const OutputDevice& rDevice)
{
    return toQPixmap(Image(rDevice.GetBitmap(Point(), rDevice.GetOutputSize())));
}

inline QPixmap loadQPixmapIcon(const OUString& rIconName)
{
    const Bitmap aBitmap(rIconName);
    if (!aBitmap.IsEmpty())
        return toQPixmap(aBitmap);

    const QIcon aIcon = QIcon::fromTheme(toQString(rIconName));
    assert(!aIcon.isNull() && "No icon found for that icon name");
    const int nIconSize = QApplication::style()->pixelMetric(QStyle::PM_ButtonIconSize);
    return aIcon.pixmap(nIconSize);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
