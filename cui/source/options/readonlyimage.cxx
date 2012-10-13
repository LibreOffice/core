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
