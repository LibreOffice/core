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

#include <QtCore/QString>
#include <QtCore/QRect>
#include <QtCore/QSize>
#include <QtGui/QImage>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <tools/gen.hxx>

#include <memory>

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

inline QSize toQSize(const Size& rSize) { return QSize(rSize.Width(), rSize.Height()); }

inline Size toSize(const QSize& rSize) { return Size(rSize.width(), rSize.height()); }

static constexpr QImage::Format Qt5_DefaultFormat32 = QImage::Format_ARGB32;

inline QImage::Format getBitFormat(sal_uInt16 nBitCount)
{
    switch (nBitCount)
    {
        case 1:
            return QImage::Format_Mono;
        case 8:
            return QImage::Format_Indexed8;
        case 16:
            return QImage::Format_RGB16;
        case 24:
            return QImage::Format_RGB888;
        case 32:
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
        case QImage::Format_RGB16:
            return 16;
        case QImage::Format_RGB888:
            return 24;
        case Qt5_DefaultFormat32:
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
