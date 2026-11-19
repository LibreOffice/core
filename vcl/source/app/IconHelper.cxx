/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <IconHelper.hxx>
#include <svdata.hxx>

#include <comphelper/flatpak.hxx>
#include <vcl/bitmaps.hlst>

OUString IconHelper::GetAppName(sal_uInt16 nIcon)
{
    switch (nIcon)
    {
        case SV_ICON_ID_TEXT:
            return u"writer"_ustr;
        case SV_ICON_ID_SPREADSHEET:
            return u"calc"_ustr;
        case SV_ICON_ID_DRAWING:
            return u"draw"_ustr;
        case SV_ICON_ID_PRESENTATION:
            return u"impress"_ustr;
        case SV_ICON_ID_DATABASE:
            return u"base"_ustr;
        case SV_ICON_ID_FORMULA:
            return u"math"_ustr;
        default:
            return u"startcenter"_ustr;
    }
}

OUString IconHelper::GetAppIconName(sal_uInt16 nIcon)
{
    const OUString sAppName = GetAppName(nIcon);
    if (comphelper::flatpak::isFlatpak())
    {
        // Flatpak uses Flatpak ID as prefix and drops "startcenter" in icon and desktop file names
        if (sAppName == u"startcenter"_ustr)
            return u"org.libreoffice.LibreOffice"_ustr;
        return u"org.libreoffice.LibreOffice."_ustr + sAppName;
    }

    return u"libreoffice-"_ustr + sAppName;
}

OUString IconHelper::GetInternalAppIconName(sal_uInt16 nIcon)
{
    switch (nIcon)
    {
        case SV_ICON_ID_TEXT:
            return RID_FILE_THUMBNAIL_TEXT;
        case SV_ICON_ID_SPREADSHEET:
            return RID_FILE_THUMBNAIL_SHEET;
        case SV_ICON_ID_DRAWING:
            return RID_FILE_THUMBNAIL_DRAWING;
        case SV_ICON_ID_PRESENTATION:
            return RID_FILE_THUMBNAIL_PRESENTATION;
        case SV_ICON_ID_DATABASE:
            return RID_FILE_THUMBNAIL_DATABASE;
        case SV_ICON_ID_FORMULA:
            return RID_FILE_THUMBNAIL_MATH;
        default:
            return RID_FILE_THUMBNAIL_DEFAULT;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
