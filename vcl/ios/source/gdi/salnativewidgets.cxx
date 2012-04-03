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

#include "vcl/salnativewidgets.hxx"
#include "vcl/decoview.hxx"
#include "vcl/svapp.hxx"
#include "vcl/timer.hxx"

#include "ios/salconst.h"
#include "ios/salgdi.h"
#include "ios/saldata.hxx"
#include "ios/salframe.h"

#include "premac.h"
#include <UIKit/UIKit.h>
#include "postmac.h"


/*
 * IsNativeControlSupported()
 * --------------------------
 * Returns sal_True if the platform supports native
 * drawing of the control defined by nPart.
 *
 */
sal_Bool IosSalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    bool bOk = sal_False;

    // Native controls are now defaults
    // If you want to disable experimental native controls code,
    // just set the environment variable SAL_NO_NWF to something
    // and vcl controls will be used as default again.

    switch( nType )
    {
        case CTRL_PUSHBUTTON:
        case CTRL_RADIOBUTTON:
        case CTRL_CHECKBOX:
        case CTRL_LISTNODE:
            if( nPart == PART_ENTIRE_CONTROL )
                return true;
            break;

        case CTRL_SCROLLBAR:
            if( nPart == PART_DRAW_BACKGROUND_HORZ ||
                nPart == PART_DRAW_BACKGROUND_VERT ||
                nPart == PART_ENTIRE_CONTROL       ||
                nPart == HAS_THREE_BUTTONS )
                return true;
            break;

        case CTRL_SLIDER:
            if( nPart == PART_TRACK_HORZ_AREA || nPart == PART_TRACK_VERT_AREA )
                return true;
            break;

        case CTRL_EDITBOX:
            if( nPart == PART_ENTIRE_CONTROL ||
                nPart == HAS_BACKGROUND_TEXTURE )
                return true;
            break;

        case CTRL_MULTILINE_EDITBOX:
            if( nPart == PART_ENTIRE_CONTROL ||
                nPart == HAS_BACKGROUND_TEXTURE )
                return true;
            break;

        case CTRL_SPINBOX:
            if( nPart == PART_ENTIRE_CONTROL ||
                nPart == PART_ALL_BUTTONS    ||
                nPart == HAS_BACKGROUND_TEXTURE )
                return true;
            break;

        case CTRL_SPINBUTTONS:
                return false;
            break;

        case CTRL_COMBOBOX:
            if( nPart == PART_ENTIRE_CONTROL ||
                nPart == HAS_BACKGROUND_TEXTURE )
                return true;
            break;

        case CTRL_LISTBOX:
            if( nPart == PART_ENTIRE_CONTROL    ||
                nPart == PART_WINDOW            ||
                nPart == HAS_BACKGROUND_TEXTURE ||
                nPart == PART_SUB_EDIT
                )
                return true;
            break;

        case CTRL_TAB_ITEM:
        case CTRL_TAB_PANE:
        case CTRL_TAB_BODY:  // see vcl/source/window/tabpage.cxx
        case CTRL_FIXEDBORDER:
            if( nPart == PART_ENTIRE_CONTROL ||
                nPart == PART_TABS_DRAW_RTL ||
                nPart == HAS_BACKGROUND_TEXTURE )
                return true;
            break;

        // when PART_BUTTON is used, toolbar icons are not highlighted when mouse rolls over.
        // More Ios compliant
        case CTRL_TOOLBAR:
            if( nPart == PART_ENTIRE_CONTROL       ||
                nPart == PART_DRAW_BACKGROUND_HORZ ||
                nPart == PART_DRAW_BACKGROUND_VERT)
                return true;
            break;

        case  CTRL_WINDOW_BACKGROUND:
            if ( nPart == PART_BACKGROUND_WINDOW ||
                 nPart == PART_BACKGROUND_DIALOG )
                 return true;
            break;

        case CTRL_MENUBAR:
            if( nPart == PART_ENTIRE_CONTROL )
                return true;
            break;

        case CTRL_TOOLTIP: // ** TO DO
            break;

        case CTRL_MENU_POPUP:
            if( nPart == PART_ENTIRE_CONTROL       ||
                nPart == PART_MENU_ITEM            ||
                nPart == PART_MENU_ITEM_CHECK_MARK ||
                nPart == PART_MENU_ITEM_RADIO_MARK)
                return true;
            break;
        case CTRL_PROGRESS:
        case CTRL_INTROPROGRESS:
            if( nPart == PART_ENTIRE_CONTROL )
                return true;
            break;
        case CTRL_FRAME:
            if( nPart == PART_BORDER )
                return true;
            break;
        case CTRL_LISTNET:
            if( nPart == PART_ENTIRE_CONTROL )
                return true;
            break;
    }

    return bOk;
}

/*
 * HitTestNativeControl()
 *
 *  If the return value is sal_True, bIsInside contains information whether
 *  aPos was or was not inside the native widget specified by the
 *  nType/nPart combination.
 */
sal_Bool IosSalGraphics::hitTestNativeControl( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion,
                        const Point& rPos, sal_Bool& rIsInside )
{
    (void) nType;
    (void) nPart;
    (void) rControlRegion;
    (void) rPos;
    (void) rIsInside;
    return sal_False;
}

UInt32 IosSalGraphics::getState( ControlState nState )
{
    (void) nState;
    return 0;
}

UInt32 IosSalGraphics::getTrackState( ControlState nState )
{
    (void) nState;
    return 0;
}

/*
 * DrawNativeControl()
 *
 *  Draws the requested control described by nPart/nState.
 *
 *  rControlRegion: The bounding region of the complete control in VCL frame coordinates.
 *  aValue:     An optional value (tristate/numerical/string)
 *  aCaption:   A caption or title string (like button text etc)
 */
sal_Bool IosSalGraphics::drawNativeControl(ControlType nType,
                    ControlPart nPart,
                    const Rectangle& rControlRegion,
                    ControlState nState,
                    const ImplControlValue& aValue,
                    const rtl::OUString& )
{
    (void) nType;
    (void) nPart;
    (void) rControlRegion;
    (void) nState;
    (void) aValue;
    return sal_False;
}

/*
 * GetNativeControlRegion()
 *
 *  If the return value is sal_True, rNativeBoundingRegion
 *  contains the true bounding region covered by the control
 *  including any adornment, while rNativeContentRegion contains the area
 *  within the control that can be safely drawn into without drawing over
 *  the borders of the control.
 *
 *  rControlRegion: The bounding region of the control in VCL frame coordinates.
 *  aValue:     An optional value (tristate/numerical/string)
 *  aCaption:       A caption or title string (like button text etc)
 */
sal_Bool IosSalGraphics::getNativeControlRegion( ControlType nType, ControlPart nPart, const Rectangle& rControlRegion, ControlState /*nState*/,
                                               const ImplControlValue& aValue, const rtl::OUString&,
                                                Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion )

{
    (void) nType;
    (void) nPart;
    (void) rControlRegion;
    (void) aValue;
    (void) rNativeBoundingRegion;
    (void) rNativeContentRegion;
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
