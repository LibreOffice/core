/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <readonlyimage.hxx>
#include <vcl/help.hxx>
#include <dialmgr.hxx>
#include <cuires.hrc>

ReadOnlyImage::ReadOnlyImage(Window* pParent, const ResId rResId) :
            FixedImage(pParent, rResId)
{
    SetImage( Image(CUI_RES( RID_SVXBMP_LOCK )));
}

ReadOnlyImage::~ReadOnlyImage()
{
}

void ReadOnlyImage::RequestHelp( const HelpEvent& rHEvt )
{
    if( Help::IsBalloonHelpEnabled() || Help::IsQuickHelpEnabled() )
    {
        Rectangle   aLogicPix( LogicToPixel( Rectangle( Point(), GetOutputSize() ) ) );
        Rectangle   aScreenRect( OutputToScreenPixel( aLogicPix.TopLeft() ),
                                     OutputToScreenPixel( aLogicPix.BottomRight() ) );

        String aStr(ReadOnlyImage::GetHelpTip());
        if ( Help::IsBalloonHelpEnabled() )
            Help::ShowBalloon( this, rHEvt.GetMousePosPixel(), aScreenRect,
            aStr );
        else if ( Help::IsQuickHelpEnabled() )
            Help::ShowQuickHelp( this, aScreenRect, aStr );
    }
    else
        Window::RequestHelp( rHEvt );
}

const String& ReadOnlyImage::GetHelpTip()
{
     static String  aStr(CUI_RES(RID_SVXSTR_READONLY_CONFIG_TIP));
     return aStr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
