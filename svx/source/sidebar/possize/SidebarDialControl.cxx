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
#include <svx/sidebar/SidebarDialControl.hxx>

#include <vcl/builder.hxx>
#include <vcl/svapp.hxx>

namespace svx { namespace sidebar {

SidebarDialControl::SidebarDialControl (Window* pParent, WinBits nBits)
    : svx::DialControl(pParent, nBits)
{
    Init(GetOutputSizePixel());
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSidebarDialControl(Window *pParent, VclBuilder::stringmap &)
{
    return new SidebarDialControl(pParent, WB_TABSTOP);
}

SidebarDialControl::~SidebarDialControl (void)
{
}

Size SidebarDialControl::GetOptimalSize() const
{
    return LogicToPixel(Size(10, 10), MAP_APPFONT);
}

void SidebarDialControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() )
    {
        GrabFocus();
        CaptureMouse();
        mpImpl->mnOldAngle = mpImpl->mnAngle;
        HandleMouseEvent( rMEvt.GetPosPixel(), true );
    }
}

void SidebarDialControl::HandleMouseEvent( const Point& rPos, bool bInitial )
{
    long nX = rPos.X() - mpImpl->mnCenterX;
    long nY = mpImpl->mnCenterY - rPos.Y();
    double fH = sqrt( static_cast< double >( nX ) * nX + static_cast< double >( nY ) * nY );
    if( fH != 0.0 )
    {
        double fAngle = acos( nX / fH );
        sal_Int32 nAngle = static_cast< sal_Int32 >( fAngle / F_PI180 * 100.0 );
        if( nY < 0 )
            nAngle = 36000 - nAngle;
        if( bInitial )  // round to entire 15 degrees
            nAngle = ((nAngle + 750) / 1500) * 1500;

        if (Application::GetSettings().GetLayoutRTL())
            nAngle = 18000 - nAngle;
        SetRotation( nAngle, true );
    }
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
