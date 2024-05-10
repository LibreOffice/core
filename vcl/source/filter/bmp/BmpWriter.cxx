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

#include <filter/BmpWriter.hxx>
#include <vcl/dibtools.hxx>

bool BmpWriter(SvStream& rStream, const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem)
{
    BitmapEx aBitmap = rGraphic.GetBitmapEx();
    sal_Int32 nColor = pFilterConfigItem->ReadInt32(u"Color"_ustr, 0);

    auto nColorRes = static_cast<BmpConversion>(nColor);
    if (nColorRes != BmpConversion::NNONE && nColorRes <= BmpConversion::N24Bit)
    {
        if (!aBitmap.Convert(nColorRes))
            aBitmap = rGraphic.GetBitmapEx();
    }
    bool bRleCoding = pFilterConfigItem->ReadBool(u"RLE_Coding"_ustr, true);
    WriteDIB(aBitmap, rStream, bRleCoding);

    return rStream.good();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
