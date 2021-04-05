/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QSize>
#include <QtCore/QString>
#include <QtGui/QImage>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <tools/gen.hxx>
#include <vcl/bitmap/BitmapTypes.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>

#include <memory>

class Image;
class QImage;

inline OUString toOUString(const QString& s)
{
    // QString stores UTF16, just like OUString
    return OUString(reinterpret_cast<const sal_Unicode*>(s.data()), s.length());
}

inline QString toQString(const OUString& s)
{
    return QString::fromUtf16(reinterpret_cast<ushort const*>(s.getStr()), s.getLength());
}

inline QRect toQRect(const tools::Rectangle& rRect)
{
    return QRect(rRect.Left(), rRect.Top(), rRect.GetWidth(), rRect.GetHeight());
}

inline QRect toQRect(const tools::Rectangle& rRect, const qreal fScale)
{
    return QRect(floor(rRect.Left() * fScale), floor(rRect.Top() * fScale),
                 ceil(rRect.GetWidth() * fScale), ceil(rRect.GetHeight() * fScale));
}

inline QRect scaledQRect(const QRect& rRect, const qreal fScale)
{
    return QRect(floor(rRect.x() * fScale), floor(rRect.y() * fScale), ceil(rRect.width() * fScale),
                 ceil(rRect.height() * fScale));
}

inline tools::Rectangle toRectangle(const QRect& rRect)
{
    return tools::Rectangle(rRect.left(), rRect.top(), rRect.right(), rRect.bottom());
}

inline QSize toQSize(const Size& rSize) { return QSize(rSize.Width(), rSize.Height()); }

inline Size toSize(const QSize& rSize) { return Size(rSize.width(), rSize.height()); }

inline Point toPoint(const QPoint& rPoint) { return Point(rPoint.x(), rPoint.y()); }

inline QColor toQColor(const Color& rColor)
{
    return QColor(rColor.GetRed(), rColor.GetGreen(), rColor.GetBlue(), rColor.GetAlpha());
}

Qt::DropActions toQtDropActions(sal_Int8 dragOperation);
sal_Int8 toVclDropActions(Qt::DropActions dragOperation);
sal_Int8 toVclDropAction(Qt::DropAction dragOperation);
Qt::DropAction getPreferredDropAction(sal_Int8 dragOperation);

inline QList<int> toQList(const css::uno::Sequence<sal_Int32>& aSequence)
{
    QList<int> aList;
    for (sal_Int32 i : aSequence)
    {
        aList.append(i);
    }
    return aList;
}

constexpr QImage::Format Qt5_DefaultFormat32 = QImage::Format_ARGB32;

inline QImage::Format getBitFormat(vcl::PixelFormat ePixelFormat)
{
    switch (ePixelFormat)
    {
        case vcl::PixelFormat::N1_BPP:
            return QImage::Format_Mono;
        case vcl::PixelFormat::N8_BPP:
            return QImage::Format_Indexed8;
        case vcl::PixelFormat::N24_BPP:
            return QImage::Format_RGB888;
        case vcl::PixelFormat::N32_BPP:
            return Qt5_DefaultFormat32;
        default:
            std::abort();
            break;
    }
    return QImage::Format_Invalid;
}

inline sal_uInt16 getFormatBits(QImage::Format eFormat)
{
    switch (eFormat)
    {
        case QImage::Format_Mono:
            return 1;
        case QImage::Format_Indexed8:
            return 8;
        case QImage::Format_RGB888:
            return 24;
        case Qt5_DefaultFormat32:
        case QImage::Format_ARGB32_Premultiplied:
            return 32;
        default:
            std::abort();
            return 0;
    }
}

typedef struct _cairo_surface cairo_surface_t;
struct CairoDeleter
{
    void operator()(cairo_surface_t* pSurface) const;
};

typedef std::unique_ptr<cairo_surface_t, CairoDeleter> UniqueCairoSurface;

sal_uInt16 GetKeyModCode(Qt::KeyboardModifiers eKeyModifiers);
sal_uInt16 GetMouseModCode(Qt::MouseButtons eButtons);

QImage toQImage(const Image& rImage);

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     const QString& rString)
{
    return stream << toOUString(rString);
}

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     const QRect& rRect)
{
    return stream << toRectangle(rRect);
}

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     const QSize& rSize)
{
    return stream << toSize(rSize);
}

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     const QPoint& rPoint)
{
    return stream << toPoint(rPoint);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
