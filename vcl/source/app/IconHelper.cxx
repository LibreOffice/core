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

OUString IconHelper::GetAppIconName(sal_uInt16 nIcon)
{
    switch (nIcon)
    {
        case SV_ICON_ID_TEXT:
            return u"libreoffice-writer"_ustr;
        case SV_ICON_ID_SPREADSHEET:
            return u"libreoffice-calc"_ustr;
        case SV_ICON_ID_DRAWING:
            return u"libreoffice-draw"_ustr;
        case SV_ICON_ID_PRESENTATION:
            return u"libreoffice-impress"_ustr;
        case SV_ICON_ID_DATABASE:
            return u"libreoffice-base"_ustr;
        case SV_ICON_ID_FORMULA:
            return u"libreoffice-math"_ustr;
        default:
            return u"libreoffice-startcenter"_ustr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
