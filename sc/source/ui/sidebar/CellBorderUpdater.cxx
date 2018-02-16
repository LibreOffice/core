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

#include "CellBorderUpdater.hxx"
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

namespace sc { namespace sidebar {

CellBorderUpdater::CellBorderUpdater(
    sal_uInt16 nTbxBtnId,
    ToolBox& rTbx)
:   mnBtnId(nTbxBtnId),
    mrTbx(rTbx)
{
}

CellBorderUpdater::~CellBorderUpdater()
{
}

void CellBorderUpdater::UpdateCellBorder(bool bTop, bool bBot, bool bLeft, bool bRight, Image const & aImg, bool bVer, bool bHor)
{
    const Size aBmpSize = aImg.GetBitmapEx().GetSizePixel();
    if(aBmpSize.Width() != 43 || aBmpSize.Height() != 43)
        return;

    ScopedVclPtr<VirtualDevice> pVirDev(VclPtr<VirtualDevice>::Create());
    pVirDev->SetOutputSizePixel(aBmpSize);
    pVirDev->SetLineColor( ::Application::GetSettings().GetStyleSettings().GetFieldTextColor() ) ;
    pVirDev->SetFillColor( COL_BLACK);

    Point aTL(2, 1), aTR(42,1), aBL(2, 41), aBR(42, 41), aHL(2,21), aHR(42, 21), aVT(22,1), aVB(22, 41);
    if(bLeft)
        pVirDev->DrawLine( aTL,aBL );
    if(bRight)
        pVirDev->DrawLine( aTR,aBR );
    if(bTop)
        pVirDev->DrawLine( aTL,aTR );
    if(bBot)
        pVirDev->DrawLine( aBL,aBR );
    if(bVer)
        pVirDev->DrawLine( aVT,aVB );
    if(bHor)
        pVirDev->DrawLine( aHL,aHR );

    mrTbx.SetItemOverlayImage( mnBtnId, Image( pVirDev->GetBitmapEx(Point(0,0), aBmpSize) ) );
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
