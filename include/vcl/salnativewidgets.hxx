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

#include <vcl/dllapi.h>
#include <tools/gen.hxx>
#include <o3tl/typed_flags_set.hxx>

/* Control Types:
 *
 *   Specify the overall, whole control
 *   type (as opposed to parts of the
 *   control if it were composite).
 */

enum class ControlType {
// for use in general purpose ImplControlValue
    Generic            =   0,
// Normal PushButton/Command Button
    Pushbutton         =   1,
// Normal single radio button
    Radiobutton        =   2,
// Normal single checkbox
    Checkbox           =  10,
// Combobox, i.e. a ListBox
// that allows data entry by user
    Combobox           =  20,
// Control that allows text entry
    Editbox            =  30,
// Control that allows text entry, but without the usual border
// Has to be handled separately, because this one cannot handle
// ControlPart::HasBackgroundTexture, which is drawn in the edit box'es
// border window.
    EditboxNoBorder    =  31,
// Control that allows text entry
// ( some systems distinguish between single and multi line edit boxes )
    MultilineEditbox   =  32,
// Control that pops up a menu,
// but does NOT allow data entry
    Listbox            =  35,
// An edit field together with two little
// buttons on the side (aka spin field)
    Spinbox            =  40,
// Two standalone spin buttons
// without an edit field
    SpinButtons        =  45,
// A single tab
    TabItem            =  50,
// The border around a tab area,
// but without the tabs themselves.
// May have a gap at the top for
// the active tab
    TabPane            =  55,
// The background to the tab area
    TabHeader          =  56,
// Background of a Tab Pane
    TabBody            =  57,
// Normal scrollbar, including
// all parts like slider, buttons
    Scrollbar          =  60,
    Slider             =  65,
// A separator line
    Fixedline          =  80,
// A toolbar control with buttons and a grip
    Toolbar            = 100,
// The menubar
    Menubar            = 120,
// popup menu
    MenuPopup          = 121,
    Progress           = 131,
// Progress bar for the intro window
// (aka splash screen), in case some
// wants native progress bar in the
// application but not for the splash
// screen (used in desktop/)
    IntroProgress      = 132,
// tool tips
    Tooltip            = 140,
// to draw the implemented theme
    WindowBackground   = 150,
//to draw border of frames natively
    Frame              = 160,
// for nodes in listviews
// used in svtools/source/contnr/svtreebx.cxx
    ListNode           = 170,
// nets between elements of listviews
// with nodes
    ListNet            = 171,
// for list headers
    ListHeader         = 172,
};


/* Control Parts:
 *
 *   Uniquely identify a part of a control,
 *   for example the slider of a scroll bar.
 */

enum class ControlPart
{
    NONE               = 0,
    Entire             = 1,
    ListboxWindow      = 5,   // the static listbox window containing the list
    Button             = 100,
    ButtonUp           = 101,
    ButtonDown         = 102, // Also for ComboBoxes/ListBoxes
    ButtonLeft         = 103,
    ButtonRight        = 104,
    AllButtons         = 105,
    SeparatorHorz      = 106,
    SeparatorVert      = 107,
    TrackHorzLeft      = 200,
    TrackVertUpper     = 201,
    TrackHorzRight     = 202,
    TrackVertLower     = 203,
    TrackHorzArea      = 204,
    TrackVertArea      = 205,
    Arrow              = 220,
    ThumbHorz          = 210, // Also used as toolbar grip
    ThumbVert          = 211, // Also used as toolbar grip
    MenuItem           = 250,
    MenuItemCheckMark  = 251,
    MenuItemRadioMark  = 252,
    Separator          = 253,
    SubmenuArrow       = 254,

/*  #i77549#
    HACK: for scrollbars in case of thumb rect, page up and page down rect we
    abuse the HitTestNativeScrollbar interface. All theming engines but aqua
    are actually able to draw the thumb according to our internal representation.
    However aqua draws a little outside. The canonical way would be to enhance the
    HitTestNativeScrollbar passing a ScrollbarValue additionally so all necessary
    information is available in the call.
    .
    However since there is only this one small exception we will deviate a little and
    instead pass the respective rect as control region to allow for a small correction.

    So all places using HitTestNativeScrollbar on ControlPart::ThumbHorz, ControlPart::ThumbVert,
    ControlPart::TrackHorzLeft, ControlPart::TrackHorzRight, ControlPart::TrackVertUpper, ControlPart::TrackVertLower
    do not use the control rectangle as region but the actual part rectangle, making
    only small deviations feasible.
*/

/** The edit field part of a control, e.g. of the combo box.

    Currently used just for combo boxes and just for GetNativeControlRegion().
    It is valid only if GetNativeControlRegion() supports ControlPart::ButtonDown as
    well.
*/
    SubEdit                 = 300,

// For controls that require the entire background
// to be drawn first, and then other pieces over top.
// (GTK+ scrollbars for example).  Control region passed
// in to draw this part is expected to be the entire
// area of the control.
// A control may respond to one or both.
    DrawBackgroundHorz      = 1000,
    DrawBackgroundVert      = 1001,

// GTK+ also draws tabs right->left since there is a
// hardcoded 2 pixel overlap between adjacent tabs
    TabsDrawRtl             = 3000,

// For themes that do not want to have the focus
// rectangle part drawn by VCL but take care of the
// whole inner control part by themselves
// eg, listboxes or comboboxes or spinbuttons
    HasBackgroundTexture    = 4000,

// For scrollbars that have 3 buttons (most KDE themes)
    HasThreeButtons         = 5000,

    BackgroundWindow        = 6000,
    BackgroundDialog        = 6001,

//to draw natively the border of frames
    Border                  = 7000,

//to draw natively the focus rects
    Focus                   = 8000
};

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
    NONE            = 0,
    ENABLED         = 0x0001,
    FOCUSED         = 0x0002,
    PRESSED         = 0x0004,
    ROLLOVER        = 0x0008,
    DEFAULT         = 0x0020,
    SELECTED        = 0x0040,
    DOUBLEBUFFERING = 0x4000,  ///< Set when the control is painted using double-buffering via VirtualDevice.
    CACHING_ALLOWED = 0x8000,  ///< Set when the control is completely visible (i.e. not clipped).
};
namespace o3tl
{
    template<> struct typed_flags<ControlState> : is_typed_flags<ControlState, 0xc06f> {};
}

/* ButtonValue:
 *
 *   Identifies the tri-state value options
 *   that buttons allow
 */

enum class ButtonValue {
    DontKnow,
    On,
    Off,
    Mixed
};

/* ImplControlValue:
 *
 *   Generic value container for all control parts.
 */

class VCL_DLLPUBLIC ImplControlValue
{
    friend class SalFrame;

    private:
        ControlType const mType;
        ButtonValue     mTristate;    // Tristate value: on, off, mixed
        long            mNumber;      // numeric value
    protected:
        ImplControlValue( ControlType i_eType, long i_nNumber )
        : mType( i_eType )
        , mTristate( ButtonValue::DontKnow )
        , mNumber( i_nNumber )
        {}

    public:
        explicit ImplControlValue( ButtonValue nTristate )
            : mType( ControlType::Generic ), mTristate(nTristate), mNumber(0) {}
        explicit ImplControlValue( long nNumeric )
            : mType( ControlType::Generic ), mTristate(ButtonValue::DontKnow), mNumber( nNumeric) {}
        ImplControlValue()
            : mType( ControlType::Generic ), mTristate(ButtonValue::DontKnow), mNumber(0) {}

        virtual ~ImplControlValue();

        ImplControlValue(ImplControlValue const &) = default;
        ImplControlValue(ImplControlValue &&) = default;
        ImplControlValue & operator =(ImplControlValue const &) = delete; // due to const mType
        ImplControlValue & operator =(ImplControlValue &&) = delete; // due to const mType

        virtual ImplControlValue* clone() const;

        ControlType getType() const { return mType; }

        ButtonValue  getTristateVal() const { return mTristate; }
        void         setTristateVal( ButtonValue nTristate ) { mTristate = nTristate; }

        long         getNumericVal() const { return mNumber; }
        void         setNumericVal( long nNumeric ) { mNumber = nNumeric; }
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
        tools::Rectangle       maThumbRect;
        tools::Rectangle       maButton1Rect;
        tools::Rectangle       maButton2Rect;
        ControlState    mnButton1State;
        ControlState    mnButton2State;
        ControlState    mnThumbState;

        ScrollbarValue()
        : ImplControlValue( ControlType::Scrollbar, 0 )
        {
            mnMin = 0; mnMax = 0; mnCur = 0; mnVisibleSize = 0;
            mnButton1State = ControlState::NONE; mnButton2State = ControlState::NONE;
            mnThumbState = ControlState::NONE;
        };
        virtual ~ScrollbarValue() override;
        virtual ScrollbarValue* clone() const override;

        ScrollbarValue(ScrollbarValue const &) = default;
        ScrollbarValue(ScrollbarValue &&) = default;
        ScrollbarValue & operator =(ScrollbarValue const &) = delete; // due to ImplControlValue
        ScrollbarValue & operator =(ScrollbarValue &&) = delete; // due to ImplControlValue
};

class VCL_DLLPUBLIC SliderValue : public ImplControlValue
{
    public:
        long            mnMin;
        long            mnMax;
        long            mnCur;
        tools::Rectangle       maThumbRect;
        ControlState    mnThumbState;

        SliderValue()
        : ImplControlValue( ControlType::Slider, 0 )
        , mnMin( 0 ), mnMax( 0 ), mnCur( 0 ), mnThumbState( ControlState::NONE )
        {}
        virtual ~SliderValue() override;
        virtual SliderValue* clone() const override;

        SliderValue(SliderValue const &) = default;
        SliderValue(SliderValue &&) = default;
        SliderValue & operator =(SliderValue const &) = delete; // due to ImplControlValue
        SliderValue & operator =(SliderValue &&) = delete; // due to ImplControlValue
};

/* TabitemValue:
 *
 *   Value container for tabitems.
 */

/* TABITEM constants are OR-ed together */
enum class TabitemFlags
{
    NONE           = 0x00,
    LeftAligned    = 0x01,   // the tabitem is aligned with the left  border of the TabControl
    RightAligned   = 0x02,   // the tabitem is aligned with the right border of the TabControl
    FirstInGroup   = 0x04,   // the tabitem is the first in group of tabitems
    LastInGroup    = 0x08,   // the tabitem is the last in group of tabitems
};
namespace o3tl
{
    template<> struct typed_flags<TabitemFlags> : is_typed_flags<TabitemFlags, 0x0f> {};
}

class VCL_DLLPUBLIC TabitemValue : public ImplControlValue
{
    public:
        TabitemFlags    mnAlignment;
        tools::Rectangle       maContentRect;

        TabitemValue(const tools::Rectangle &rContentRect)
            : ImplControlValue( ControlType::TabItem, 0 )
            , mnAlignment(TabitemFlags::NONE)
            , maContentRect(rContentRect)
        {
        }
        virtual ~TabitemValue() override;
        virtual TabitemValue* clone() const override;

        TabitemValue(TabitemValue const &) = default;
        TabitemValue(TabitemValue &&) = default;
        TabitemValue & operator =(TabitemValue const &) = delete; // due to ImplControlValue
        TabitemValue & operator =(TabitemValue &&) = delete; // due to ImplControlValue

        bool isLeftAligned() const  { return bool(mnAlignment & TabitemFlags::LeftAligned); }
        bool isRightAligned() const { return bool(mnAlignment & TabitemFlags::RightAligned); }
        bool isBothAligned() const  { return isLeftAligned() && isRightAligned(); }
        bool isNotAligned() const   { return !(mnAlignment & (TabitemFlags::LeftAligned | TabitemFlags::RightAligned)); }
        bool isFirst() const        { return bool(mnAlignment & TabitemFlags::FirstInGroup); }
        bool isLast() const         { return bool(mnAlignment & TabitemFlags::LastInGroup); }
        const tools::Rectangle& getContentRect() const { return maContentRect; }
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
        tools::Rectangle       maUpperRect;
        tools::Rectangle       maLowerRect;
        ControlState    mnUpperState;
        ControlState    mnLowerState;
        ControlPart     mnUpperPart;
        ControlPart     mnLowerPart;

        SpinbuttonValue()
            : ImplControlValue( ControlType::SpinButtons, 0 )
            , mnUpperState(ControlState::NONE)
            , mnLowerState(ControlState::NONE)
            , mnUpperPart(ControlPart::NONE)
            , mnLowerPart(ControlPart::NONE)
        {
        }

        virtual ~SpinbuttonValue() override;
        virtual SpinbuttonValue* clone() const override;

        SpinbuttonValue(SpinbuttonValue const &) = default;
        SpinbuttonValue(SpinbuttonValue &&) = default;
        SpinbuttonValue & operator =(SpinbuttonValue const &) = delete; // due to ImplControlValue
        SpinbuttonValue & operator =(SpinbuttonValue &&) = delete; // due to ImplControlValue
};

/*  Toolbarvalue:
 *
 *  Value container for toolbars detailing the grip position
 */
class VCL_DLLPUBLIC ToolbarValue : public ImplControlValue
{
public:
    ToolbarValue() : ImplControlValue( ControlType::Toolbar, 0 )
    { mbIsTopDockingArea = false; }
    virtual ~ToolbarValue() override;
    virtual ToolbarValue* clone() const override;

    ToolbarValue(ToolbarValue const &) = default;
    ToolbarValue(ToolbarValue &&) = default;
    ToolbarValue & operator =(ToolbarValue const &) = delete; // due to ImplControlValue
    ToolbarValue & operator =(ToolbarValue &&) = delete; // due to ImplControlValue

    tools::Rectangle           maGripRect;
    bool                mbIsTopDockingArea; // indicates that this is the top aligned dockingarea
                                            // adjacent to the menubar, only used on Windows
};

/*  MenubarValue:
 *
 *  Value container for menubars specifying height of adjacent docking area
 */
class VCL_DLLPUBLIC MenubarValue : public ImplControlValue
{
public:
    MenubarValue() : ImplControlValue( ControlType::Menubar, 0 )
    { maTopDockingAreaHeight=0; }
    virtual ~MenubarValue() override;
    virtual MenubarValue* clone() const override;
    MenubarValue(MenubarValue const &) = default;
    MenubarValue(MenubarValue &&) = default;
    MenubarValue & operator =(MenubarValue const &) = delete; // due to ImplControlValue
    MenubarValue & operator =(MenubarValue &&) = delete; // due to ImplControlValue
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
    MenupopupValue( long i_nGutterWidth, const tools::Rectangle& i_rItemRect )
    : ImplControlValue( ControlType::MenuPopup, i_nGutterWidth )
    , maItemRect( i_rItemRect )
    {}
    virtual ~MenupopupValue() override;
    virtual MenupopupValue* clone() const override;
    MenupopupValue(MenupopupValue const &) = default;
    MenupopupValue(MenupopupValue &&) = default;
    MenupopupValue & operator =(MenupopupValue const &) = delete; // due to ImplControlValue
    MenupopupValue & operator =(MenupopupValue &&) = delete; // due to ImplControlValue
    tools::Rectangle       maItemRect;
};

/*  PushButtonValue:
 *
 *  Value container for pushbuttons specifying additional drawing hints
 */
class VCL_DLLPUBLIC PushButtonValue : public ImplControlValue
{
public:
    PushButtonValue()
        : ImplControlValue( ControlType::Pushbutton, 0 )
        , mbBevelButton(false)
        , mbSingleLine(true)
        , mbIsAction(false)
    {}

    virtual ~PushButtonValue() override;
    virtual PushButtonValue* clone() const override;

    PushButtonValue(PushButtonValue const &) = default;
    PushButtonValue(PushButtonValue &&) = default;
    PushButtonValue & operator =(PushButtonValue const &) = delete; // due to ImplControlValue
    PushButtonValue & operator =(PushButtonValue &&) = delete; // due to ImplControlValue

    bool mbBevelButton:1; // only used on OSX
    bool mbSingleLine:1;  // only used on OSX
    bool mbIsAction:1;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
