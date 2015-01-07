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

#ifndef INCLUDED_VCL_SALNATIVEWIDGETS_HXX
#define INCLUDED_VCL_SALNATIVEWIDGETS_HXX

#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>
#include <tools/gen.hxx>
#include <o3tl/typed_flags_set.hxx>

/* Control Types:
 *
 *   Specify the overall, whole control
 *   type (as opposed to parts of the
 *   control if it were composite).
 */

typedef sal_uInt32      ControlType;

// for use in general purpose ImplControlValue
#define CTRL_GENERIC            0

// Normal PushButton/Command Button
#define CTRL_PUSHBUTTON         1

// Normal single radio button
#define CTRL_RADIOBUTTON            2

// Normal single checkbox
#define CTRL_CHECKBOX           10

// Combobox, i.e. a ListBox
// that allows data entry by user
#define CTRL_COMBOBOX           20

// Control that allows text entry
#define CTRL_EDITBOX            30

// Control that allows text entry, but without the usual border
// Has to be handled separately, because this one cannot handle
// HAS_BACKGROUND_TEXTURE, which is drawn in the edit box'es
// border window.
#define CTRL_EDITBOX_NOBORDER   31

// Control that allows text entry
// ( some systems distinguish between single and multi line edit boxes )
#define CTRL_MULTILINE_EDITBOX 32

// Control that pops up a menu,
// but does NOT allow data entry
#define CTRL_LISTBOX            35

// An edit field together with two little
// buttons on the side (aka spin field)
#define CTRL_SPINBOX            40

// Two standalone spin buttons
// without an edit field
#define CTRL_SPINBUTTONS        45

// A single tab
#define CTRL_TAB_ITEM           50

// The border around a tab area,
// but without the tabs themselves.
// May have a gap at the top for
// the active tab
#define CTRL_TAB_PANE           55

// Background of a Tab Pane
#define CTRL_TAB_BODY           56

// Normal scrollbar, including
// all parts like slider, buttons
#define CTRL_SCROLLBAR          60

#define CTRL_SLIDER             65

// Border around a group of related
// items, perhaps also displaying
// a label of identification
#define CTRL_GROUPBOX           70

// A separator line
#define CTRL_FIXEDLINE          80

// A toolbar control with buttons and a grip
#define CTRL_TOOLBAR            100

// The menubar
#define CTRL_MENUBAR            120
// popup menu
#define CTRL_MENU_POPUP         121

#define CTRL_PROGRESS           131
// Progress bar for the intro window
// (aka splash screen), in case some
// wants native progress bar in the
// application but not for the splash
// screen (used in desktop/)
#define CTRL_INTROPROGRESS      132

// tool tips
#define CTRL_TOOLTIP            140

// to draw the implemented theme
#define CTRL_WINDOW_BACKGROUND  150

//to draw border of frames natively
#define CTRL_FRAME              160

// for nodes in listviews
// used in svtools/source/contnr/svtreebx.cxx
#define CTRL_LISTNODE           170
// nets between elements of listviews
// with nodes
#define CTRL_LISTNET            171
// for list headers
#define CTRL_LISTHEADER         172


/* Control Parts:
 *
 *   Uniquely identify a part of a control,
 *   for example the slider of a scroll bar.
 */

typedef sal_uInt32      ControlPart;

#define PART_ENTIRE_CONTROL         1
#define PART_WINDOW                 5       // the static listbox window containing the list
#define PART_BUTTON                 100
#define PART_BUTTON_UP              101
#define PART_BUTTON_DOWN            102 // Also for ComboBoxes/ListBoxes
#define PART_BUTTON_LEFT            103
#define PART_BUTTON_RIGHT           104
#define PART_ALL_BUTTONS            105
#define PART_SEPARATOR_HORZ         106
#define PART_SEPARATOR_VERT         107
#define PART_TRACK_HORZ_LEFT        200
#define PART_TRACK_VERT_UPPER       201
#define PART_TRACK_HORZ_RIGHT       202
#define PART_TRACK_VERT_LOWER       203
#define PART_TRACK_HORZ_AREA        204
#define PART_TRACK_VERT_AREA        205
#define PART_THUMB_HORZ             210 // Also used as toolbar grip
#define PART_THUMB_VERT             211 // Also used as toolbar grip
#define PART_ARROW                  220
#define PART_MENU_ITEM              250
#define PART_MENU_ITEM_CHECK_MARK   251
#define PART_MENU_ITEM_RADIO_MARK   252
#define PART_MENU_SEPARATOR         253
#define PART_MENU_SUBMENU_ARROW     254

/*  #i77549#
    HACK: for scrollbars in case of thumb rect, page up and page down rect we
    abuse the HitTestNativeControl interface. All theming engines but aqua
    are actually able to draw the thumb according to our internal representation.
    However aqua draws a little outside. The canonical way would be to enhance the
    HitTestNativeControl passing a ScrollbarValue additionally so all necessary
    information is available in the call.
    .
    However since there is only this one small exception we will deviate a little and
    instead pass the respective rect as control region to allow for a small correction.

    So all places using HitTestNativeControl on PART_THUMB_HORZ, PART_THUMB_VERT,
    PART_TRACK_HORZ_LEFT, PART_TRACK_HORZ_RIGHT, PART_TRACK_VERT_UPPER, PART_TRACK_VERT_LOWER
    do not use the control rectangle as region but the actuall part rectangle, making
    only small deviations feasible.
*/

/** The edit field part of a control, e.g. of the combo box.

    Currently used just for combo boxes and just for GetNativeControlRegion().
    It is valid only if GetNativeControlRegion() supports PART_BUTTON_DOWN as
    well.
*/
#define PART_SUB_EDIT           300

// For controls that require the entire background
// to be drawn first, and then other pieces over top.
// (GTK+ scrollbars for example).  Control region passed
// in to draw this part is expected to be the entire
// area of the control.
// A control may respond to one or both.
#define PART_DRAW_BACKGROUND_HORZ       1000
#define PART_DRAW_BACKGROUND_VERT       1001

// GTK+ also draws tabs right->left since there is a
// hardcoded 2 pixel overlap between adjacent tabs
#define PART_TABS_DRAW_RTL          3000

// For themes that do not want to have the focus
// rectangle part drawn by VCL but take care of the
// whole inner control part by themselves
// eg, listboxes or comboboxes or spinbuttons
#define HAS_BACKGROUND_TEXTURE  4000

// For scrollbars that have 3 buttons (most KDE themes)
#define HAS_THREE_BUTTONS       5000

#define PART_BACKGROUND_WINDOW  6000
#define PART_BACKGROUND_DIALOG  6001

//to draw natively the border of frames
#define PART_BORDER             7000

/* Control State:
 *
 *   Specify how a particular part of the control
 *   is to be drawn.  Constants are bitwise OR-ed
 *   together to compose a final drawing state.
 *   A _disabled_ state is assumed by the drawing
 *   functions until an ENABLED or HIDDEN is passed
 *   in the ControlState.
 */
enum class ControlState {
    NONE         = 0,
    ENABLED      = 0x0001,
    FOCUSED      = 0x0002,
    PRESSED      = 0x0004,
    ROLLOVER     = 0x0008,
    HIDDEN       = 0x0010,
    DEFAULT      = 0x0020,
    SELECTED     = 0x0040,
    CACHING_ALLOWED  = 0x8000,  // set when the control is completely visible (i.e. not clipped)
};
namespace o3tl
{
    template<> struct typed_flags<ControlState> : is_typed_flags<ControlState, 0x8007f> {};
}

/* ButtonValue:
 *
 *   Identifies the tri-state value options
 *   that buttons allow
 */

enum ButtonValue {
    BUTTONVALUE_DONTKNOW,
    BUTTONVALUE_ON,
    BUTTONVALUE_OFF,
    BUTTONVALUE_MIXED
};

/* ImplControlValue:
 *
 *   Generic value container for all control parts.
 */

class VCL_DLLPUBLIC ImplControlValue
{
    friend class SalFrame;

    private:
        ControlType     mType;
        ButtonValue     mTristate;    // Tristate value: on, off, mixed
        long            mNumber;      // numeric value
    protected:
        ImplControlValue( ControlType i_eType, ButtonValue i_eTriState, long i_nNumber )
        : mType( i_eType )
        , mTristate( i_eTriState )
        , mNumber( i_nNumber )
        {}

    public:
        explicit ImplControlValue( ButtonValue nTristate )
            : mType( CTRL_GENERIC ), mTristate(nTristate), mNumber(0) {}
        explicit ImplControlValue( long nNumeric )
            : mType( CTRL_GENERIC ), mTristate(BUTTONVALUE_DONTKNOW), mNumber( nNumeric) {}
        inline ImplControlValue()
            : mType( CTRL_GENERIC ), mTristate(BUTTONVALUE_DONTKNOW), mNumber(0) {}

        virtual ~ImplControlValue();

        virtual ImplControlValue* clone() const;

        ControlType getType() const { return mType; }

        inline ButtonValue      getTristateVal( void ) const { return mTristate; }
        inline void         setTristateVal( ButtonValue nTristate ) { mTristate = nTristate; }

        inline long         getNumericVal( void ) const { return mNumber; }
        inline void         setNumericVal( long nNumeric ) { mNumber = nNumeric; }
};

/* ScrollbarValue:
 *
 *   Value container for scrollbars.
 */
class VCL_DLLPUBLIC ScrollbarValue : public ImplControlValue
{
    public:
        long            mnMin;
        long            mnMax;
        long            mnCur;
        long            mnVisibleSize;
        Rectangle       maThumbRect;
        Rectangle       maButton1Rect;
        Rectangle       maButton2Rect;
        ControlState    mnButton1State;
        ControlState    mnButton2State;
        ControlState    mnThumbState;
        ControlState    mnPage1State;
        ControlState    mnPage2State;

        inline ScrollbarValue()
        : ImplControlValue( CTRL_SCROLLBAR, BUTTONVALUE_DONTKNOW, 0 )
        {
            mnMin = 0; mnMax = 0; mnCur = 0; mnVisibleSize = 0;
            mnButton1State = ControlState::NONE; mnButton2State = ControlState::NONE;
            mnThumbState = ControlState::NONE; mnPage1State = ControlState::NONE; mnPage2State = ControlState::NONE;
        };
        virtual ~ScrollbarValue();
        virtual ScrollbarValue* clone() const SAL_OVERRIDE;
};

class VCL_DLLPUBLIC SliderValue : public ImplControlValue
{
    public:
        long            mnMin;
        long            mnMax;
        long            mnCur;
        Rectangle       maThumbRect;
        ControlState    mnThumbState;

        SliderValue()
        : ImplControlValue( CTRL_SLIDER, BUTTONVALUE_DONTKNOW, 0 )
        , mnMin( 0 ), mnMax( 0 ), mnCur( 0 ), mnThumbState( ControlState::NONE )
        {}
        virtual ~SliderValue();
        virtual SliderValue* clone() const SAL_OVERRIDE;
};

/* TabitemValue:
 *
 *   Value container for tabitems.
 */

/* TABITEM constants are OR-ed together */
#define TABITEM_LEFTALIGNED    0x001   // the tabitem is aligned with the left  border of the TabControl
#define TABITEM_RIGHTALIGNED   0x002   // the tabitem is aligned with the right border of the TabControl
#define TABITEM_FIRST_IN_GROUP 0x004   // the tabitem is the first in group of tabitems
#define TABITEM_LAST_IN_GROUP  0x008   // the tabitem is the last in group of tabitems

class VCL_DLLPUBLIC TabitemValue : public ImplControlValue
{
    public:
        unsigned int    mnAlignment;

        inline TabitemValue()
        : ImplControlValue( CTRL_TAB_ITEM, BUTTONVALUE_DONTKNOW, 0 )
        {
            mnAlignment = 0;
        };
        virtual ~TabitemValue();
        virtual TabitemValue* clone() const SAL_OVERRIDE;

        bool isLeftAligned() const  { return (mnAlignment & TABITEM_LEFTALIGNED) != 0; }
        bool isRightAligned() const { return (mnAlignment & TABITEM_RIGHTALIGNED) != 0; }
        bool isBothAligned() const  { return isLeftAligned() && isRightAligned(); }
        bool isNotAligned() const   { return (mnAlignment & (TABITEM_LEFTALIGNED | TABITEM_RIGHTALIGNED)) == 0; }
        bool isFirst() const        { return (mnAlignment & TABITEM_FIRST_IN_GROUP) != 0; }
        bool isLast() const         { return (mnAlignment & TABITEM_LAST_IN_GROUP) != 0; }
};

/* SpinbuttonValue:
 *
 *   Value container for spinbuttons to paint both buttons at once.
 *   Note: the other parameters of DrawNativeControl will have no meaning
 *         all parameters for spinbuttons are carried here
 */
class VCL_DLLPUBLIC SpinbuttonValue : public ImplControlValue
{
    public:
        Rectangle       maUpperRect;
        Rectangle       maLowerRect;
        ControlState    mnUpperState;
        ControlState    mnLowerState;
        int         mnUpperPart;
        int         mnLowerPart;

        SpinbuttonValue()
            : ImplControlValue( CTRL_SPINBUTTONS, BUTTONVALUE_DONTKNOW, 0 )
            , mnUpperState(ControlState::NONE)
            , mnLowerState(ControlState::NONE)
            , mnUpperPart(0)
            , mnLowerPart(0)
        {
        }

        virtual ~SpinbuttonValue();
        virtual SpinbuttonValue* clone() const SAL_OVERRIDE;
};

/*  Toolbarvalue:
 *
 *  Value container for toolbars detailing the grip position
 */
class VCL_DLLPUBLIC ToolbarValue : public ImplControlValue
{
public:
    ToolbarValue() : ImplControlValue( CTRL_TOOLBAR, BUTTONVALUE_DONTKNOW, 0 )
    { mbIsTopDockingArea = false; }
    virtual ~ToolbarValue();
    virtual ToolbarValue* clone() const SAL_OVERRIDE;
    Rectangle           maGripRect;
    bool                mbIsTopDockingArea; // indicates that this is the top aligned dockingarea
                                            // adjacent to the menubar
};

/*  MenubarValue:
 *
 *  Value container for menubars specifying height of adjacent docking area
 */
class VCL_DLLPUBLIC MenubarValue : public ImplControlValue
{
public:
    MenubarValue() : ImplControlValue( CTRL_MENUBAR, BUTTONVALUE_DONTKNOW, 0 )
    { maTopDockingAreaHeight=0; }
    virtual ~MenubarValue();
    virtual MenubarValue* clone() const SAL_OVERRIDE;
    int             maTopDockingAreaHeight;
};

/* MenupopupValue:
 *
 * Value container for menu items; specifies the rectangle for the whole item which
 * may be useful when drawing parts with a smaller rectangle.
 */
class VCL_DLLPUBLIC MenupopupValue : public ImplControlValue
{
public:
    MenupopupValue() : ImplControlValue( CTRL_MENU_POPUP, BUTTONVALUE_DONTKNOW, 0 )
    {}
    MenupopupValue( long i_nGutterWidth, const Rectangle& i_rItemRect )
    : ImplControlValue( CTRL_MENU_POPUP, BUTTONVALUE_DONTKNOW, i_nGutterWidth )
    , maItemRect( i_rItemRect )
    {}
    virtual ~MenupopupValue();
    virtual MenupopupValue* clone() const SAL_OVERRIDE;
    Rectangle       maItemRect;
};

/*  PushButtonValue:
 *
 *  Value container for pushbuttons specifying additional drawing hints
 */
class VCL_DLLPUBLIC PushButtonValue : public ImplControlValue
{
public:
    PushButtonValue()
    : ImplControlValue( CTRL_PUSHBUTTON, BUTTONVALUE_DONTKNOW, 0 )
    , mbBevelButton( false ), mbSingleLine( true ) {}
    virtual ~PushButtonValue();
    virtual PushButtonValue* clone() const SAL_OVERRIDE;

    bool            mbBevelButton:1;
    bool            mbSingleLine:1;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
