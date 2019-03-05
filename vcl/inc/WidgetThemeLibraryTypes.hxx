/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_WIDGETTHEMETYPES_HXX
#define INCLUDED_VCL_INC_WIDGETTHEMETYPES_HXX

#include <cstdint>

#include <o3tl/typed_flags_set.hxx>

/**
 * These types are all based on the supported variants
 * vcl/salnativewidgets.hxx and must be kept in-sync.
 **/

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

template<> struct o3tl::typed_flags<ControlState>: o3tl::is_typed_flags<ControlState, 0xC06F> {};

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
