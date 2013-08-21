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

#include "svx/XPropertyTable.hxx"

#include <vcl/virdev.hxx>
#include <svl/itemset.hxx>
#include <sfx2/docfile.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <svx/xbtmpit.hxx>

using namespace com::sun::star;

XBitmapEntry* XBitmapList::Remove(long nIndex)
{
    return (XBitmapEntry*) XPropertyList::Remove(nIndex);
}

XBitmapEntry* XBitmapList::GetBitmap(long nIndex) const
{
    return (XBitmapEntry*) XPropertyList::Get(nIndex);
}

uno::Reference< container::XNameContainer > XBitmapList::createInstance()
{
    return uno::Reference< container::XNameContainer >(
        SvxUnoXBitmapTable_createInstance( this ), uno::UNO_QUERY );
}

bool XBitmapList::Create()
{
    String aStr(SVX_RESSTR(RID_SVXSTR_BITMAP));
    sal_uInt16 aArray[64];
    Bitmap aBitmap;
    const xub_StrLen nLen(aStr.Len() - 1);

    memset(aArray, 0, sizeof(aArray));

    // white/white bitmap
    aStr.AppendAscii(" 1");
    aBitmap = createHistorical8x8FromArray(aArray, RGB_Color(COL_WHITE), RGB_Color(COL_WHITE));
    Insert(new XBitmapEntry(Graphic(aBitmap), aStr));

    // black/white bitmap
    aArray[ 0] = 1; aArray[ 9] = 1; aArray[18] = 1; aArray[27] = 1;
    aArray[36] = 1; aArray[45] = 1; aArray[54] = 1; aArray[63] = 1;
    aStr.SetChar(nLen, sal_Unicode('2'));
    aBitmap = createHistorical8x8FromArray(aArray, RGB_Color(COL_BLACK), RGB_Color(COL_WHITE));
    Insert(new XBitmapEntry(Graphic(aBitmap), aStr));

    // lightred/white bitmap
    aArray[ 7] = 1; aArray[14] = 1; aArray[21] = 1; aArray[28] = 1;
    aArray[35] = 1; aArray[42] = 1; aArray[49] = 1; aArray[56] = 1;
    aStr.SetChar(nLen, sal_Unicode('3'));
    aBitmap = createHistorical8x8FromArray(aArray, RGB_Color(COL_LIGHTRED), RGB_Color(COL_WHITE));
    Insert(new XBitmapEntry(Graphic(aBitmap), aStr));

    // lightblue/white bitmap
    aArray[24] = 1; aArray[25] = 1; aArray[26] = 1;
    aArray[29] = 1; aArray[30] = 1; aArray[31] = 1;
    aStr.SetChar(nLen, sal_Unicode('4'));
    aBitmap = createHistorical8x8FromArray(aArray, RGB_Color(COL_LIGHTBLUE), RGB_Color(COL_WHITE));
    Insert(new XBitmapEntry(Graphic(aBitmap), aStr));

    return sal_True;
}

Bitmap XBitmapList::CreateBitmapForUI( long /*nIndex*/ )
{
    return Bitmap();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
