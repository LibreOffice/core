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

#include <XPropertyTable.hxx>

#include <osl/diagnose.h>
#include <vcl/virdev.hxx>
#include <svx/xtable.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

using namespace com::sun::star;

XBitmapEntry* XBitmapList::GetBitmap(tools::Long nIndex) const
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

BitmapEx XBitmapList::CreateBitmap( tools::Long nIndex, const Size& rSize ) const
{
    OSL_ENSURE( nIndex < Count(), "Access out of range" );

    if(nIndex < Count())
    {
        BitmapEx rBitmapEx = GetBitmap( nIndex )->GetGraphicObject().GetGraphic().GetBitmapEx();
        ScopedVclPtrInstance< VirtualDevice > pVirtualDevice;
        pVirtualDevice->SetOutputSizePixel(rSize);

        if(rBitmapEx.IsAlpha())
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

            for(tools::Long y(0); y < rSize.Height(); y += aBitmapSize.Height())
            {
                for(tools::Long x(0); x < rSize.Width(); x += aBitmapSize.Width())
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

BitmapEx XBitmapList::CreateBitmapForUI( tools::Long nIndex )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Size& rSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();
    return CreateBitmap(nIndex, rSize);
}

BitmapEx XBitmapList::GetBitmapForPreview( tools::Long nIndex, const Size& rSize )
{
    return CreateBitmap(nIndex, rSize);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
