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

#include <svx/XPropertyTable.hxx>

#include <vcl/virdev.hxx>
#include <svl/itemset.hxx>
#include <sfx2/docfile.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <svx/xbtmpit.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/BitmapTools.hxx>

using namespace com::sun::star;

XBitmapEntry* XPatternList::GetBitmap(long nIndex) const
{
    return static_cast<XBitmapEntry*>( XPropertyList::Get(nIndex) );
}

uno::Reference< container::XNameContainer > XPatternList::createInstance()
{
    return uno::Reference< container::XNameContainer >(
        SvxUnoXBitmapTable_createInstance( this ), uno::UNO_QUERY );
}

bool XPatternList::Create()
{
    OUStringBuffer aStr(SvxResId(RID_SVXSTR_PATTERN));
    std::array<sal_uInt8,64> aArray;
    BitmapEx aBitmap;
    const sal_Int32 nLen(aStr.getLength() - 1);

    aArray.fill(0);

    // white/white bitmap
    aStr.append(" 1");
    aBitmap = vcl::bitmap::createHistorical8x8FromArray(aArray, COL_WHITE, COL_WHITE);
    Insert(std::make_unique<XBitmapEntry>(Graphic(aBitmap), aStr.toString()));

    // black/white bitmap
    aArray[ 0] = 1; aArray[ 9] = 1; aArray[18] = 1; aArray[27] = 1;
    aArray[36] = 1; aArray[45] = 1; aArray[54] = 1; aArray[63] = 1;
    aStr[nLen] = '2';
    aBitmap = vcl::bitmap::createHistorical8x8FromArray(aArray, COL_BLACK, COL_WHITE);
    Insert(std::make_unique<XBitmapEntry>(Graphic(aBitmap), aStr.toString()));

    // lightred/white bitmap
    aArray[ 7] = 1; aArray[14] = 1; aArray[21] = 1; aArray[28] = 1;
    aArray[35] = 1; aArray[42] = 1; aArray[49] = 1; aArray[56] = 1;
    aStr[nLen] = '3';
    aBitmap = vcl::bitmap::createHistorical8x8FromArray(aArray, COL_LIGHTRED, COL_WHITE);
    Insert(std::make_unique<XBitmapEntry>(Graphic(aBitmap), aStr.toString()));

    // lightblue/white bitmap
    aArray[24] = 1; aArray[25] = 1; aArray[26] = 1;
    aArray[29] = 1; aArray[30] = 1; aArray[31] = 1;
    aStr[nLen] = '4';
    aBitmap = vcl::bitmap::createHistorical8x8FromArray(aArray, COL_LIGHTBLUE, COL_WHITE);
    Insert(std::make_unique<XBitmapEntry>(Graphic(aBitmap), aStr.toString()));

    return true;
}

BitmapEx XPatternList::CreateBitmap( long nIndex, const Size& rSize ) const
{
    assert( nIndex < Count() );

    if(nIndex < Count())
    {
        BitmapEx rBitmapEx = GetBitmap( nIndex )->GetGraphicObject().GetGraphic().GetBitmapEx();
        ScopedVclPtrInstance< VirtualDevice > pVirtualDevice;
        pVirtualDevice->SetOutputSizePixel(rSize);

        if(rBitmapEx.IsTransparent())
        {
            const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

            if(rStyleSettings.GetPreviewUsesCheckeredBackground())
            {
                const Point aNull(0, 0);
                static const sal_uInt32 nLen(8);
                static const Color aW(COL_WHITE);
                static const Color aG(0xef, 0xef, 0xef);

                pVirtualDevice->DrawCheckered(aNull, rSize, nLen, aW, aG);
            }
            else
            {
                pVirtualDevice->SetBackground(rStyleSettings.GetFieldColor());
                pVirtualDevice->Erase();
            }
        }

        if(rBitmapEx.GetSizePixel().Width() >= rSize.Width() && rBitmapEx.GetSizePixel().Height() >= rSize.Height())
        {
            rBitmapEx.Scale(rSize);
            pVirtualDevice->DrawBitmapEx(Point(0, 0), rBitmapEx);
        }
        else
        {
            const Size aBitmapSize(rBitmapEx.GetSizePixel());

            for(long y(0); y < rSize.Height(); y += aBitmapSize.Height())
            {
                for(long x(0); x < rSize.Width(); x += aBitmapSize.Width())
                {
                    pVirtualDevice->DrawBitmapEx(
                        Point(x, y),
                        rBitmapEx);
                }
            }
        }
        rBitmapEx = pVirtualDevice->GetBitmapEx(Point(0, 0), rSize);
        return rBitmapEx;
    }
    else
        return BitmapEx();
}

BitmapEx XPatternList::CreateBitmapForUI( long nIndex )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Size& rSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();
    return CreateBitmap(nIndex, rSize);
}

BitmapEx XPatternList::GetBitmapForPreview( long nIndex, const Size& rSize )
{
    return CreateBitmap(nIndex, rSize);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
