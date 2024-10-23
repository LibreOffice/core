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
#include <vcl/filter/PngImageWriter.hxx>

#include <QtGui/QPixmap>

QPixmap loadQPixmapIcon(const OUString& rIconName)
{
    BitmapEx aIcon(rIconName);
    SvMemoryStream aMemoryStream;
    vcl::PngImageWriter aWriter(aMemoryStream);
    aWriter.write(aIcon);
    QPixmap aPixmap;
    aPixmap.loadFromData(static_cast<const uchar*>(aMemoryStream.GetData()),
                         aMemoryStream.TellEnd());
    assert(!aPixmap.isNull() && "Failed to create icon pixmap");
    return aPixmap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
