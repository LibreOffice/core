/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: readonlyimage.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif
#include <readonlyimage.hxx>
#include <vcl/help.hxx>
#include <svx/dialmgr.hxx>
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
/*-- 26.02.2004 13:31:04---------------------------------------------------

  -----------------------------------------------------------------------*/
ReadOnlyImage::ReadOnlyImage(Window* pParent, const ResId rResId) :
            FixedImage(pParent, rResId)
{
    sal_Bool bHighContrast = pParent->GetDisplayBackground().GetColor().IsDark();
    SetImage( Image(SVX_RES(bHighContrast ? RID_SVXBMP_LOCK_HC : RID_SVXBMP_LOCK )));
}

/*-- 26.02.2004 13:31:04---------------------------------------------------

  -----------------------------------------------------------------------*/
ReadOnlyImage::~ReadOnlyImage()
{
}
/*-- 26.02.2004 13:31:04---------------------------------------------------

  -----------------------------------------------------------------------*/
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

/*-- 26.02.2004 14:20:21---------------------------------------------------

  -----------------------------------------------------------------------*/
const String& ReadOnlyImage::GetHelpTip()
{
     static String  aStr(SVX_RES(RID_SVXSTR_READONLY_CONFIG_TIP));
     return aStr;
}
