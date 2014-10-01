/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <numformat.hxx>
#include <patattr.hxx>
#include <scitems.hxx>

#include <svl/zforlist.hxx>
#include <svl/intitem.hxx>

namespace sc {

bool NumFmtUtil::isGeneral( sal_uLong nFormat )
{
    return (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0;
}

bool NumFmtUtil::isGeneral( const ScPatternAttr& rPat )
{
    const SfxPoolItem* pItem = NULL;
    if (!rPat.GetItemSet().HasItem(ATTR_VALUE_FORMAT, &pItem))
        // Assume it's 'General' when the number format is not explicitly set.
        return true;

    sal_uInt32 nNumFmt = static_cast<const SfxUInt32Item*>(pItem)->GetValue();
    return isGeneral(nNumFmt);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
