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
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <svx/xbtmpit.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

using namespace com::sun::star;

XBitmapEntry* XBitmapList::GetBitmap(long nIndex) const
{
    return static_cast<XBitmapEntry*>( XPropertyList::Get(nIndex) );
}

uno::Reference< container::XNameContainer > XBitmapList::createInstance()
{
    return uno::Reference< container::XNameContainer >(
        SvxUnoXBitmapTable_createInstance( this ), uno::UNO_QUERY );
}

bool XBitmapList::Create()
{
    return true;
}

Bitmap XBitmapList::CreateBitmap( long nIndex, const Size& rSize ) const
{
    OSL_ENSURE( nIndex < Count(), "Access out of range" );

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
        rBitmapEx = pVirtualDevice->GetBitmap(Point(0, 0), rSize);
        return rBitmapEx.GetBitmap();
    }
    else
        return Bitmap();
}

Bitmap XBitmapList::CreateBitmapForUI( long nIndex )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Size& rSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();
    return CreateBitmap(nIndex, rSize);
}

Bitmap XBitmapList::GetBitmapForPreview( long nIndex, const Size& rSize )
{
    return CreateBitmap(nIndex, rSize);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
