/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salnativewidgets.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-05 08:28:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_SALCONST_H
#include <salconst.h>
#endif

#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif

#ifndef _SV_NATIVEWIDGETS_HXX
#include <salnativewidgets.hxx>
#endif

#ifndef _SV_NATIVEWIDGETS_H
#include <salnativewidgets.h>
#endif

#include "saldata.hxx"

using ::rtl::OUString;

/*
 *  AquaGet*  are helpers for native controls
 */

// Helper returns an HIRect

HIRect ImplGetHIRectFromRectangle(Rectangle aRect)
{
    HIRect aHIRect;
    aHIRect.origin.x = static_cast<float>(aRect.Left());
    aHIRect.origin.y = static_cast<float>(aRect.Top());
    aHIRect.size.width = static_cast<float>(aRect.Right()) - static_cast<float>(aRect.Left() +1);
    aHIRect.size.height = static_cast<float>(aRect.Bottom()) - static_cast<float>(aRect.Top() +1);
    return aHIRect;
}

// Helper returns a Rectangle

Rectangle ImplGetRectangleFromHIRect( HIRect aHIRect )
{
    Rectangle aRect;
    aRect.Top() = static_cast<int>(aHIRect.origin.y);
    aRect.Left() = static_cast<int>(aHIRect.origin.x);
    aRect.Bottom() = static_cast<int>(aHIRect.size.height) + static_cast<short>(aHIRect.origin.y);
    aRect.Right() = static_cast<int>(aHIRect.origin.x) + static_cast<short>(aHIRect.size.width);
    return aRect;
}

// Helper returns a QD Rect

Rect ImplGetRectFromHIRect(HIRect aHIRect)
{
    Rect aRect;
    aRect.top = static_cast<short>(aHIRect.origin.y);
    aRect.left = static_cast<short>(aHIRect.origin.x);
    aRect.right= static_cast<short>(aHIRect.origin.x) + static_cast<short>(aHIRect.size.width);
    aRect.bottom = static_cast<short>(aHIRect.size.height) + static_cast<short>(aHIRect.origin.y);
    return aRect;
}


static bool isAppleScrollBarVariantDoubleMax(void)
{
    bool isDoubleMax = true;  // default is DoubleMax
    CFStringRef ScrollBarVariant;

    ScrollBarVariant = ( (CFStringRef)CFPreferencesCopyAppValue(CFSTR("AppleScrollBarVariant"), kCFPreferencesCurrentApplication) );
    if ( ScrollBarVariant )
    {
        if ( !CFStringCompare(ScrollBarVariant, CFSTR("DoubleMax"), kCFCompareCaseInsensitive) )
            isDoubleMax = true;
        else
            isDoubleMax = false;
        CFRelease( ScrollBarVariant );
    }

    return isDoubleMax;
}

static ThemeButtonValue ImplGetButtonValue( ButtonValue aButtonValue )
{
    switch( aButtonValue )
    {
        case BUTTONVALUE_ON:
            return kThemeButtonOn;
            break;

        case BUTTONVALUE_OFF:
            return kThemeButtonOff;
            break;

        case BUTTONVALUE_MIXED:
        case BUTTONVALUE_DONTKNOW:
        default:
            return kThemeButtonMixed;
            break;
    }
}

static Rectangle AquaGetScrollButtonRect(   /*implement me : int nScreen, */  ControlPart nPart, Rectangle aAreaRect )
{
    Rectangle  buttonRect;

    switch (nPart)
    {
        case PART_BUTTON_UP:
            buttonRect.setX( aAreaRect.Left() );
            buttonRect.setY( aAreaRect.Top() );
            break;

        case PART_BUTTON_LEFT:
            buttonRect.setX( aAreaRect.Left() );
            buttonRect.setY( aAreaRect.Top() );
            break;

        case PART_BUTTON_DOWN:
            buttonRect.setX( aAreaRect.Left() );
            buttonRect.setY( aAreaRect.Top() + aAreaRect.GetHeight() - BUTTON_HEIGHT );
            break;

        case PART_BUTTON_RIGHT:
            buttonRect.setX( aAreaRect.Left() + aAreaRect.GetWidth() - BUTTON_WIDTH );
            buttonRect.setY( aAreaRect.Top() );
            break;
    }

    buttonRect.SetSize( Size( BUTTON_WIDTH, BUTTON_HEIGHT ) );

    return( buttonRect );
}

char *ImplDbgGetStringControlType(ControlType nType)
{
    switch (nType)
    {
        case CTRL_PUSHBUTTON: return "PUSHBUTTON";
        case CTRL_RADIOBUTTON: return "RADIOBUTTON";
        case CTRL_CHECKBOX: return "CHECKBOX";
        case CTRL_COMBOBOX: return "COMBOBOX";
        case CTRL_EDITBOX: return "EDITBOX";
        case CTRL_EDITBOX_NOBORDER: return "EDITBOX_NOBORDER";
        case CTRL_MULTILINE_EDITBOX: return "MULTILINE_EDITBOX";
        case CTRL_LISTBOX: return "LISTBOX";
        case CTRL_SPINBOX: return "SPINBOX";
        case CTRL_SPINBUTTONS: return "SPINBUTTONS";
        case CTRL_TAB_ITEM: return "TAB_ITEM";
        case CTRL_TAB_PANE: return "TAB_PANE";
        case CTRL_TAB_BODY: return "TAB_BODY";
        case CTRL_SCROLLBAR: return "SCROLLBAR";
        case CTRL_GROUPBOX: return "GROUPBOX";
        case CTRL_FIXEDLINE: return "FIXEDLINE";
        case CTRL_FIXEDBORDER: return "FIXEDBORDER";
        case CTRL_TOOLBAR: return "TOOLBAR";
        case CTRL_MENUBAR: return "MENUBAR";
        case CTRL_MENU_POPUP: return "MENU_POPUP";
        case CTRL_STATUSBAR: return "STATUSBAR";
        case CTRL_TOOLTIP: return "TOOLTIP";
        case CTRL_WINDOW_BACKGROUND: return "WINDOW_BACKGROUND";
    }

    AquaLog( "UNKNOWN ControlType %d!\n", (int) nType);
    return "UNKNOWN";
}

char *ImplDbgGetStringControlPart(ControlPart nPart)
{
    switch (nPart)
    {
        case PART_ENTIRE_CONTROL: return "ENTIRE_CONTROL";
        case PART_WINDOW: return "WINDOW";
        case PART_BUTTON: return "BUTTON";
        case PART_BUTTON_UP: return "BUTTON_UP";
        case PART_BUTTON_DOWN: return "BUTTON_DOWN";
        case PART_BUTTON_LEFT: return "BUTTON_LEFT";
        case PART_BUTTON_RIGHT: return "BUTTON_RIGHT";
        case PART_ALL_BUTTONS: return "ALL_BUTTONS";
        case PART_TRACK_HORZ_LEFT: return "TRACK_HORZ_LEFT";
        case PART_TRACK_VERT_UPPER: return "TRACK_VERT_UPPER";
        case PART_TRACK_HORZ_RIGHT: return "TRACK_HORZ_RIGHT";
        case PART_TRACK_VERT_LOWER: return "TRACK_VERT_LOWER";
        case PART_THUMB_HORZ: return "THUMB_HORZ";
        case PART_THUMB_VERT: return "THUMB_VERT";
        case PART_MENU_ITEM: return "MENU_ITEM";
        case PART_MENU_ITEM_CHECK_MARK: return "MENU_ITEM_CHECK_MARK";
        case PART_MENU_ITEM_RADIO_MARK: return "MENU_ITEM_RADIO_MARK";
        case PART_SUB_EDIT: return "SUB_EDIT";
        case PART_DRAW_BACKGROUND_HORZ: return "DRAW_BACKGROUND_HORZ";
        case PART_DRAW_BACKGROUND_VERT: return "DRAW_BACKGROUND_VERT";
        case PART_TABS_DRAW_RTL: return "TABS_DRAW_RTL";
        case HAS_BACKGROUND_TEXTURE: return "HAS_BACKGROUND_TEXTURE";
        case HAS_THREE_BUTTONS: return "HAS_THREE_BUTTONS";
        case PART_BACKGROUND_WINDOW: return "BACKGROUND_WINDOW";
        case PART_BACKGROUND_DIALOG: return "BACKGROUND_DIALOG";
}

    AquaLog( "UNKNOWN ControlPart %d!\n", (int) nPart);
    return "UNKNOWN";
}


/*
 * IsNativeControlSupported()
 * --------------------------
 * Returns TRUE if the platform supports native
 * drawing of the control defined by nPart.
 *
 */
BOOL AquaSalGraphics::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    bool bOk = FALSE;

    // Native controls are now defaults
    // If you want to disable experimental native controls code,
    // just set the environment variable SAL_NO_NWF to something
    // and vcl controls will be used as default again.

    AquaLog( "%s (%s, %s)\n", __func__, ImplDbgGetStringControlType(nType), ImplDbgGetStringControlPart(nPart));

    switch( nType )
    {
        case CTRL_PUSHBUTTON:
        case CTRL_RADIOBUTTON:
        case CTRL_CHECKBOX:
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

        case CTRL_EDITBOX: // ** TO DO **
            if( nPart == PART_ENTIRE_CONTROL ||
                nPart == HAS_BACKGROUND_TEXTURE )
                return false;
            break;

        case CTRL_MULTILINE_EDITBOX: // ** TO DO **
            if( nPart == PART_ENTIRE_CONTROL ||
                nPart == HAS_BACKGROUND_TEXTURE )
                return false;
            break;

        case CTRL_SPINBOX: // ** TO DO **
            if( nPart == PART_ENTIRE_CONTROL ||
                nPart == PART_ALL_BUTTONS    ||
                nPart == HAS_BACKGROUND_TEXTURE )
                return false;
            break;

        case CTRL_SPINBUTTONS: // ** TO DO **
            if( nPart == PART_ENTIRE_CONTROL ||
                nPart == PART_ALL_BUTTONS )
                return false;
            break;

        case CTRL_COMBOBOX: // ** TO DO **
            if( nPart == PART_ENTIRE_CONTROL ||
                nPart == HAS_BACKGROUND_TEXTURE )
                return true;
            break;

        case CTRL_LISTBOX: // ** TO DO **
            if( nPart == PART_ENTIRE_CONTROL ||
                nPart == PART_WINDOW         ||
                nPart == HAS_BACKGROUND_TEXTURE
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
        // More Aqua compliant
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

        case CTRL_MENUBAR: // ** TO DO + CHECK IF NEEDED **
            if( nPart == PART_ENTIRE_CONTROL )
                return true;
            break;

        case CTRL_TOOLTIP: // ** TO DO + CHECK IF NEEDED **
            #if 0
            if( nPart == PART_ENTIRE_CONTROL ) // we don't currently support the tooltip
                return true;
            #endif
            break;

        case CTRL_MENU_POPUP:
            if( nPart == PART_ENTIRE_CONTROL       ||
                nPart == PART_MENU_ITEM            )
                return true;
            break;
    }

    return bOk;
}

/*
 * HitTestNativeControl()
 *
 *  If the return value is TRUE, bIsInside contains information whether
 *  aPos was or was not inside the native widget specified by the
 *  nType/nPart combination.
 */
BOOL AquaSalGraphics::hitTestNativeControl( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                        const Point& rPos, SalControlHandle& rControlHandle, BOOL& rIsInside )
{
    AquaLog( "%s (%s, %s)\n", __func__, ImplDbgGetStringControlType(nType), ImplDbgGetStringControlPart(nPart));

    if ( nType == CTRL_SCROLLBAR )
    {
        // outside by default
        rIsInside = FALSE;

        ControlPart nCounterPart = 0;
        switch (nPart)
        {
            case PART_BUTTON_UP:
                nCounterPart = PART_BUTTON_DOWN;
                break;
            case PART_BUTTON_DOWN:
                nCounterPart = PART_BUTTON_UP;
                break;
            case PART_BUTTON_LEFT:
                nCounterPart = PART_BUTTON_RIGHT;
                break;
            case PART_BUTTON_RIGHT:
                nCounterPart = PART_BUTTON_LEFT;
                break;
        }
        // make position relative to rControlRegion

        // [ericb] rControlRegion.GetBoundRect() returns the rectangle where vcl control is located
        // when a control is hit on the scrollbar (native control), the coordinates of the click
        // are translated to vcl control coordinates
        // rPos : the structure containing the cursor position
        // aPos = relative position of current cursor

        Point aPos;
        aPos = rPos - rControlRegion.GetBoundRect().TopLeft();

        // button_leftRect contains the left/top button hit in the scrollbar
        // button_rightRect contains the right/bottom button hit in the scrollbar
        Rectangle button_leftRect;
        Rectangle button_rightRect;

        // controlRect contains the scrollbar
        Rectangle controlRect = rControlRegion.GetBoundRect();

        // Get respective areas
        button_leftRect = AquaGetScrollButtonRect( /* m_nScreen, implement me */ nPart, rControlRegion.GetBoundRect() );
        button_rightRect =  AquaGetScrollButtonRect( /* m_nScreen, implement me */ nCounterPart, rControlRegion.GetBoundRect() );

        short offset_button_left  = 0; //  from left of scrollbar area to the left side of left button
        short offset_button_top  = 0; // from top of scrollbar area to top of top button

        // right / bottom butons have always the same position
        short offset_button_right = controlRect.GetWidth() - button_rightRect.GetWidth(); // from left of scrollbar area to left side of right button
        short offset_button_bottom = controlRect.GetHeight() - button_rightRect.GetHeight();   // from top of scrollbar area to top of bottom button

        if ( isAppleScrollBarVariantDoubleMax() == true )
        {
            offset_button_left = controlRect.GetWidth() - (button_leftRect.GetWidth() + button_rightRect.GetWidth());
            offset_button_top  = controlRect.GetHeight() - (button_leftRect.GetHeight() + button_rightRect.GetHeight());
        }

        // depending on nPart, we have to test different regions
        switch ( nPart )
        {
            case PART_BUTTON_LEFT:
                if ( (aPos.getY() > 0 ) && (aPos.getY() < controlRect.GetHeight() )
                     && (aPos.getX() > offset_button_left) && (aPos.getX() < (offset_button_left + button_leftRect.GetWidth()) ) )
                {
                    AquaLog( "button left \n");
                    rIsInside = TRUE;
                }
                break;

            case PART_BUTTON_RIGHT:
                if ( (aPos.getY() > 0 ) && (aPos.getY() < controlRect.GetHeight() )
                     && (aPos.getX() > offset_button_right ) && (aPos.getX() < controlRect.GetWidth() + 1))  // + 1 because of the borders
                {
                    AquaLog( "button right \n");
                    rIsInside = TRUE;
                }
                break;

            case PART_BUTTON_UP:
                if ( (aPos.getX() > 0 ) && (aPos.getX() < controlRect.GetWidth() )
                     && (aPos.getY() > offset_button_top) && (aPos.getY() < (offset_button_top + button_leftRect.GetHeight() )) )
                {
                    AquaLog( "button up \n");
                    rIsInside = TRUE;
                }
                break;

            case PART_BUTTON_DOWN:
                if ((aPos.getX() > 0 )  && (aPos.getX() < controlRect.GetWidth() )
                    && ( aPos.getY() > (offset_button_bottom)) && (aPos.getY() < (controlRect.GetHeight() + 1) ) )
                {
                    AquaLog( "button down \n");
                    rIsInside = TRUE;
                }
                break;

            default:
                rIsInside = FALSE;
                break;
        }

        return TRUE;

    }  //  CTRL_SCROLLBAR

    return FALSE;
}

/*
  kThemeStateInactive = 0,
   kThemeStateActive = 1,
   kThemeStatePressed = 2,
   kThemeStateRollover = 6,
   kThemeStateUnavailable = 7,
   kThemeStateUnavailableInactive = 8
   kThemeStatePressedUp = 2,
   kThemeStatePressedDown = 3

#define CTRL_STATE_ENABLED      0x0001
#define CTRL_STATE_FOCUSED      0x0002
#define CTRL_STATE_PRESSED      0x0004
#define CTRL_STATE_ROLLOVER     0x0008
#define CTRL_STATE_HIDDEN       0x0010
#define CTRL_STATE_DEFAULT      0x0020
#define CTRL_STATE_SELECTED     0x0040
#define CTRL_CACHING_ALLOWED    0x8000  // set when the control is completely visible (i.e. not clipped)
*/
static ThemeDrawState getState( ControlState nState )
{
    if( (nState & CTRL_STATE_ENABLED) == 0 )
    {
        if( (nState & CTRL_STATE_HIDDEN) == 0 )
            return kThemeStateInactive;
        else
            return kThemeStateUnavailableInactive;
    }

    if( (nState & CTRL_STATE_HIDDEN) != 0 )
        return kThemeStateUnavailable;

    if( (nState & CTRL_STATE_PRESSED) != 0 )
        return kThemeStatePressed;

    return kThemeStateActive;
}

/*
 * DrawNativeControl()
 *
 *  Draws the requested control described by nPart/nState.
 *
 *  rControlRegion: The bounding region of the complete control in VCL frame coordinates.
 *  aValue:     An optional value (tristate/numerical/string)
 *  rControlHandle: Carries platform dependent data and is maintained by the AquaSalGraphics implementation.
 *  aCaption:   A caption or title string (like button text etc)
 */
BOOL AquaSalGraphics::drawNativeControl(ControlType nType,
                    ControlPart nPart,
                    const Region& rControlRegion,
                    ControlState nState,
                    const ImplControlValue& aValue,
                    SalControlHandle& rControlHandle,
                    const rtl::OUString& aCaption )
{
    AquaLog( "%s (%s, %s)\n", __func__, ImplDbgGetStringControlType(nType), ImplDbgGetStringControlPart(nPart));
    BOOL bOK = FALSE;

    if( ! CheckContext() )
        return false;

    CGContextSaveGState( mrContext );

    Rectangle buttonRect = rControlRegion.GetBoundRect();
    RefreshRect( buttonRect.Left(), buttonRect.Top(), buttonRect.GetWidth(), buttonRect.GetHeight() );
    HIRect rc = ImplGetHIRectFromRectangle(buttonRect);

    /** Scrollbar parts code equivalent **
    PART_BUTTON_UP 101
    PART_BUTTON_DOWN 102
    PART_THUMB_VERT 211
    PART_TRACK_VERT_UPPER 201
    PART_TRACK_VERT_LOWER 203

    PART_DRAW_BACKGROUND_HORZ 1000
    PART_DRAW_BACKGROUND_VERT 1001
    **/

    switch( nType )
    {
    case  CTRL_LISTBOX:
        switch( nPart)
        {
            case PART_BUTTON_DOWN:
            case HAS_BACKGROUND_TEXTURE:
            case PART_ENTIRE_CONTROL:
            {
                HIThemeButtonDrawInfo aListInfo;
                aListInfo.version = 0;
                aListInfo.kind = kThemePopupButton;
                aListInfo.state = kThemeStateActive;//getState( nState );//kThemeStateInactive -> greyed

                ButtonValue aListButtonValue = aValue.getTristateVal();
                aListInfo.value = ImplGetButtonValue( aListButtonValue );

                aListInfo.adornment = kThemeAdornmentDefault;

                HIThemeDrawButton(&rc, &aListInfo, mrContext, kHIThemeOrientationNormal,&rc);
                bOK = true;
            }
        }
        break;

    case  CTRL_COMBOBOX:
        if ( nPart == HAS_BACKGROUND_TEXTURE ||
             nPart == PART_ENTIRE_CONTROL )
        {
            HIThemeButtonDrawInfo aComboInfo;
            aComboInfo.version = 0;
            aComboInfo.kind = kThemeComboBox;
            aComboInfo.state = getState( nState );
            ButtonValue aComboButtonValue = aValue.getTristateVal();
            aComboInfo.value = ImplGetButtonValue( aComboButtonValue );
            aComboInfo.adornment = kThemeAdornmentNone;

            if( (nState & CTRL_STATE_FOCUSED) != 0 )
                aComboInfo.adornment |= kThemeAdornmentFocus;

            HIThemeDrawButton(&rc, &aComboInfo, mrContext, kHIThemeOrientationNormal,&rc);
            bOK = true;
        }
        break;

    case CTRL_FIXEDBORDER:
    case CTRL_TAB_PANE:
    case CTRL_TAB_BODY:
    case CTRL_TOOLBAR:
        {
            HIThemeMenuItemDrawInfo aMenuItemDrawInfo;
            aMenuItemDrawInfo.version = 0;
            aMenuItemDrawInfo.state = kThemeMenuActive;
            aMenuItemDrawInfo.itemType = kThemeMenuItemHierBackground;
            HIThemeDrawMenuItem(&rc,&rc,&aMenuItemDrawInfo,mrContext,kHIThemeOrientationNormal,NULL);
            bOK = true;
        }
        break;

        case CTRL_WINDOW_BACKGROUND:
        {
            HIThemeBackgroundDrawInfo aThemeBackgroundInfo;
            aThemeBackgroundInfo.version = 0;
            aThemeBackgroundInfo.state = getState( nState );
            aThemeBackgroundInfo.kind = kThemeBrushDialogBackgroundInactive;

            HIThemeApplyBackground( &rc, &aThemeBackgroundInfo, mrContext, kHIThemeOrientationNormal);
            CGContextFillRect( mrContext, rc );
            bOK = true;
        }
        break;

    case CTRL_MENUBAR:
    case CTRL_MENU_POPUP:
        {
            if ((nPart == PART_ENTIRE_CONTROL) || (nPart == PART_MENU_ITEM )|| (nPart == HAS_BACKGROUND_TEXTURE ))
            {
                // FIXME: without this magical offset there is a 2 pixel black border on the right
                rc.size.width += 2;

                HIThemeMenuDrawInfo aMenuInfo;
                aMenuInfo.version = 0;
                aMenuInfo.menuType = kThemeMenuTypePullDown;

                HIThemeMenuItemDrawInfo aMenuItemDrawInfo;
                // the Aqua grey theme when the item is selected is drawn here.
                aMenuItemDrawInfo.itemType = kThemeMenuItemPlain;

                if ((nPart == PART_MENU_ITEM ))
                {
                    // the blue theme when the item is selected is drawn here.
                    aMenuItemDrawInfo.state = kThemeMenuSelected;
                }
                else
                {
                    // normal color for non selected item
                    aMenuItemDrawInfo.state = kThemeMenuActive;
                }

                // repaints the background of the pull down menu
                HIThemeDrawMenuBackground(&rc,&aMenuInfo,mrContext,kHIThemeOrientationNormal);

                // repaints the item either blue (selected) and/or Aqua grey (active only)
                HIThemeDrawMenuItem(&rc,&rc,&aMenuItemDrawInfo,mrContext,kHIThemeOrientationNormal,&rc);

                bOK = true;
            }
        }
        break;

    case CTRL_PUSHBUTTON:
        {
            // [ FIXME] : instead of use a value, vcl can retrieve corect values on the fly (to be implemented)
            int max_height = 20;

            // [FIXME]: for width, better use (native)  text length to define width. How retrieve the text length ?
            int max_width = 68;

            // [FIXME] find a better criteria to differentiate several buttons who are square, but are not Bewel buttons.
            if((rc.size.width < 17)  && (rc.size.width == rc.size.height ) )
            {
                HIThemeButtonDrawInfo aBevelInfo;
                aBevelInfo.version = 0;
                aBevelInfo.kind = kThemeBevelButton;
                aBevelInfo.state = getState( nState );

                ButtonValue aButtonValue = aValue.getTristateVal();
                aBevelInfo.value = ImplGetButtonValue( aButtonValue );

                aBevelInfo.adornment = (( nState & CTRL_STATE_DEFAULT ) != 0) ?
                kThemeAdornmentDefault :
                kThemeAdornmentNone;
                if( (nState & CTRL_STATE_FOCUSED) != 0 )
                    aBevelInfo.adornment |= kThemeAdornmentFocus;

                HIThemeDrawButton( &rc, &aBevelInfo, mrContext, kHIThemeOrientationNormal, NULL );
                bOK = true;
            }
            else
            {
                HIThemeButtonDrawInfo aPushInfo;
                aPushInfo.version = 0;
                if(( rc.size.width >= max_width) || ( rc.size.height >= max_height ))
                {
                    aPushInfo.kind = kThemePushButton;
                    // just in case people don't like push button with iconn inside
                    if (rc.size.width > max_width )
                    {
                        // translate the origin of the button from the half of the offset
                        // translation value must be an integrer
                        int delta_y = floor((rc.size.height - max_height) / 2.0);
                        rc.origin.y += delta_y + 1;

                        // button height must be limited ( height value collected from OSXHIGuidelines )
                        rc.size.height = max_height;
                    }

                    aPushInfo.state = getState( nState );

                    aPushInfo.value = ImplGetButtonValue( aValue.getTristateVal() );

                    aPushInfo.adornment = (( nState & CTRL_STATE_DEFAULT ) != 0) ?
                    kThemeAdornmentDefault :
                    kThemeAdornmentNone;
                    if( (nState & CTRL_STATE_FOCUSED) != 0 )
                        aPushInfo.adornment |= kThemeAdornmentFocus;

                    // will show correct refresh once working
                    aPushInfo.animation.time.current = CFAbsoluteTimeGetCurrent();

                    HIThemeDrawButton( &rc, &aPushInfo, mrContext, kHIThemeOrientationNormal, NULL );
                    bOK = true;
                }
            }
        }
        break;

    case CTRL_RADIOBUTTON:
    case CTRL_CHECKBOX:
        {
            HIThemeButtonDrawInfo aInfo;
            aInfo.version = 0;
            switch( nType )
            {
            case CTRL_RADIOBUTTON: aInfo.kind = kThemeRadioButton;
                break;
            case CTRL_CHECKBOX:    aInfo.kind = kThemeCheckBox;
                break;
            }

            aInfo.state = getState( nState );

            ButtonValue aButtonValue = aValue.getTristateVal();
            aInfo.value = ImplGetButtonValue( aButtonValue );

            aInfo.adornment = (( nState & CTRL_STATE_DEFAULT ) != 0) ?
            kThemeAdornmentDefault :
            kThemeAdornmentNone;
            if( (nState & CTRL_STATE_FOCUSED) != 0 )
                aInfo.adornment |= kThemeAdornmentFocus;
            HIThemeDrawButton( &rc, &aInfo, mrContext, kHIThemeOrientationNormal, NULL );
            bOK = true;
        }
        break;

    case CTRL_SCROLLBAR:
        {
            ScrollbarValue* pScrollbarVal = (ScrollbarValue *)(aValue.getOptionalVal());

            if( nPart == PART_DRAW_BACKGROUND_VERT ||
                nPart == PART_DRAW_BACKGROUND_HORZ )
            {
                HIThemeTrackDrawInfo aTrackDraw;
                aTrackDraw.version = 0;
                aTrackDraw.kind = kThemeMediumScrollBar;
                aTrackDraw.bounds = rc;
                aTrackDraw.min = pScrollbarVal->mnMin;
                aTrackDraw.max = pScrollbarVal->mnMax - pScrollbarVal->mnVisibleSize;
                aTrackDraw.value = pScrollbarVal->mnCur;
                aTrackDraw.reserved = 0;
                aTrackDraw.attributes = kThemeTrackShowThumb;
                if( nPart == PART_DRAW_BACKGROUND_HORZ )
                    aTrackDraw.attributes |= kThemeTrackHorizontal;
                aTrackDraw.enableState = kThemeTrackActive;

                ScrollBarTrackInfo aScrollInfo;
                aScrollInfo.viewsize = pScrollbarVal->mnVisibleSize;
                aScrollInfo.pressState = 0;

                if ( pScrollbarVal->mnButton1State & CTRL_STATE_ENABLED )
                {
                    if ( pScrollbarVal->mnButton1State & CTRL_STATE_PRESSED )
                        aScrollInfo.pressState = kThemeTopOutsideArrowPressed;
                }

                if ( pScrollbarVal->mnButton2State & CTRL_STATE_ENABLED )
                {
                    if ( pScrollbarVal->mnButton2State & CTRL_STATE_PRESSED )
                        aScrollInfo.pressState = kThemeBottomOutsideArrowPressed;
                }

                if ( pScrollbarVal->mnThumbState & CTRL_STATE_ENABLED )
                {
                    if ( pScrollbarVal->mnThumbState & CTRL_STATE_PRESSED )
                        aScrollInfo.pressState = kThemeThumbPressed;
                }

                aTrackDraw.trackInfo.scrollbar = aScrollInfo;

                HIThemeDrawTrack( &aTrackDraw, NULL, mrContext, kHIThemeOrientationNormal );
                bOK = true;
            }
        }
    }

    CGContextRestoreGState( mrContext );

    return bOK;
}

/*
 * DrawNativeControlText()
 *
 *  OPTIONAL.  Draws the requested text for the control described by nPart/nState.
 *     Used if text not drawn by DrawNativeControl().
 *
 *  rControlRegion: The bounding region of the complete control in VCL frame coordinates.
 *  aValue:     An optional value (tristate/numerical/string)
 *  rControlHandle: Carries platform dependent data and is maintained by the AquaSalGraphics implementation.
 *  aCaption:   A caption or title string (like button text etc)
 */
BOOL AquaSalGraphics::drawNativeControlText( ControlType nType, ControlPart nPart, const Region& rControlRegion,
                                               ControlState nState, const ImplControlValue& aValue,
                                               SalControlHandle& rControlHandle, const rtl::OUString& aCaption )
{
    AquaLog("In text\n");
    return( FALSE );
}


/*
 * GetNativeControlRegion()
 *
 *  If the return value is TRUE, rNativeBoundingRegion
 *  contains the true bounding region covered by the control
 *  including any adornment, while rNativeContentRegion contains the area
 *  within the control that can be safely drawn into without drawing over
 *  the borders of the control.
 *
 *  rControlRegion: The bounding region of the control in VCL frame coordinates.
 *  aValue:     An optional value (tristate/numerical/string)
 *  rControlHandle: Carries platform dependent data and is maintained by the AquaSalGraphics implementation.
 *  aCaption:       A caption or title string (like button text etc)
 */
BOOL AquaSalGraphics::getNativeControlRegion( ControlType nType, ControlPart nPart, const Region& rControlRegion, ControlState nState,
                                               const ImplControlValue& aValue, SalControlHandle& rControlHandle, const rtl::OUString& aCaption,
                                                Region &rNativeBoundingRegion, Region &rNativeContentRegion )

{
    AquaLog( "%s (%s, %s)\n", __func__, ImplDbgGetStringControlType(nType), ImplDbgGetStringControlPart(nPart));
    BOOL toReturn = FALSE;

    switch (nType)
    {
        case CTRL_COMBOBOX:
            short xcombo = rControlRegion.GetBoundRect().Left();
            short ycombo = rControlRegion.GetBoundRect().Top();
            short wcombo = rControlRegion.GetBoundRect().GetWidth();
            short hcombo = rControlRegion.GetBoundRect().GetHeight();
            rNativeContentRegion = Rectangle( Point( xcombo, ycombo ), Size( wcombo, hcombo) );
            rNativeBoundingRegion = Rectangle( Point( xcombo, ycombo ), Size( wcombo , hcombo ) );

            toReturn = TRUE;
            break;

        case CTRL_PUSHBUTTON:
        case CTRL_RADIOBUTTON:
        case CTRL_CHECKBOX:

            short x = rControlRegion.GetBoundRect().Left();
            short y = rControlRegion.GetBoundRect().Top();
            short w, h;

            sal_uInt8 nBorderCleanup = 0;

            if ( nType == CTRL_PUSHBUTTON )
            {
                w = rControlRegion.GetBoundRect().GetWidth();
                h = rControlRegion.GetBoundRect().GetHeight();
            }
            else
            {
                // checkbox and radio borders need cleanup after unchecking them
                nBorderCleanup = 4;

                // TEXT_SEPARATOR to respect Aqua HIG
                w = BUTTON_WIDTH + TEXT_SEPARATOR;
                h = BUTTON_HEIGHT;

            }

            rNativeContentRegion = Rectangle( Point( x, y ), Size( w, h + nBorderCleanup) );
            rNativeBoundingRegion = Rectangle( Point( x, y ), Size( w, h ) );

            toReturn = TRUE;
            break;
    }

    AquaLog( "AquaSalGraphics::getNativeControlRegion will return: %d\n", toReturn);
    return toReturn;
}
