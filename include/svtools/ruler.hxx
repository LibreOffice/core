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

#ifndef INCLUDED_SVTOOLS_RULER_HXX
#define INCLUDED_SVTOOLS_RULER_HXX

#include <memory>
#include <map>
#include <svtools/svtdllapi.h>
#include <rtl/ref.hxx>
#include <tools/fldunit.hxx>
#include <tools/link.hxx>
#include <tools/fract.hxx>
#include <vcl/window.hxx>
#include <vcl/glyphitem.hxx>

class MouseEvent;
class TrackingEvent;
class DataChangedEvent;
class SvtRulerAccessible;

/*************************************************************************

Description
============

class Ruler

This class is used for displaying a ruler, but it can also be used
for setting or moving tabs and margins.

--------------------------------------------------------------------------

WinBits

WB_HORZ             ruler is displayed horizontally
WB_VERT             ruler is displayed vertically
WB_3DLOOK           3D look
WB_BORDER           border at the bottom/right margin
WB_EXTRAFIELD       Field in the upper left corner for
                    displaying and selecting tabs, origin of coordinates, ...
WB_RIGHT_ALIGNED    Marks the vertical ruler as right aligned

--------------------------------------------------------------------------

All ruler parameters are set in pixel units. This way double conversions
and rounding errors are avoided and the ruler displays the margins
at their actual position in the document. Because of this, the application can,
for example in tables, do its own roundings and the positions on the ruler will
still match those in the document. However, for the ruler to know how the
document is displayed on the screen, some additional values have to be configured

SetWinPos() sets the offset of the ruler's edit window. In doing so,
the width of the window can also be configured. If there is a 0 among the
values passed to the function, the position/width is automatically set to
the width of the ruler.

SetPagePos() sets the offset of the page relative to the edit window and the
width of the page. If there is a 0 among the values passed to the function,
the position/width is automatically set as if the page filled the whole edit window.

SetBorderPos() sets the offset of the border. The position is relative to
the upper/left margin of the window. This is needed when there are a horizontal
and a vertical ruler visible at the same time. Example:
        aHRuler.SetBorderPos( aVRuler.GetSizePixel().Width()-1 );

SetNullOffset() sets the origin relative to the page.

All the other values (margins, indentation, tabs, ...) refer to the origin,
which is set with SetNullOffset().

The values are computed as described below:

- WinPos (if both windows have the same parent)

    Point aHRulerPos = aHRuler.GetPosPixel();
    Point aEditWinPos = aEditWin.GetPosPixel();
    aHRuler.SetWinPos( aEditWinPos().X() - aHRulerPos.X() );

- PagePos

    Point aPagePos = aEditWin.LogicToPixel( aEditWin.GetPagePos() );
    aHRuler.SetPagePos( aPagePos().X() );

- All other values

    Add the logical values, recompute as position and subtract the
    previously saved pixel positions (of PagePos and Null Offset).

--------------------------------------------------------------------------

SetUnit() and SetZoom() configure which unit is used to display
the values on the ruler. The following units are accepted:

    FieldUnit::MM
    FieldUnit::CM (Default)
    FieldUnit::M
    FieldUnit::KM
    FieldUnit::INCH
    FieldUnit::FOOT
    FieldUnit::MILE
    FieldUnit::POINT
    FieldUnit::PICA

--------------------------------------------------------------------------

SetMargin1() sets the upper/left margin and SetMargin2() sets the
bottom/right margin. If these methods are called without arguments,
no margins are displayed. Otherwise, the following arguments can be passed:

    long    nPos            - offset in pixels relative to the origin
    sal_uInt16 nStyle       - bit style:
                                RULER_MARGIN_SIZEABLE
                                margin size can be changed

                                The following bits can be set in addition
                                to these styles:
                                RULER_STYLE_INVISIBLE


SetBorders() sets an array of margins. To do this, an array of type RulerBorder
has to be passed. In the array, the following values have to be initialized:

    long    nPos            - offset in pixels relative to the origin
    long    nWidth          - column spacing in pixels (can also be 0, for example,
                              for table columns)
    sal_uInt16 nStyle       - bit style:
                                RulerBorderStyle::Sizeable
                                Column spacing can be changed. This flag should
                                only be set if the size of the spacing is changed,
                                not that of a cell.
                                RulerBorderStyle::Moveable
                                Column spacing/border can be moved. Whenever
                                table borders are to be moved, this flag should
                                be set instead of SIZEABLE (SIZEABLE indicates
                                that the size of a spacing, not that of a single
                                cell can be changed).
                                RulerBorderStyle::Variable
                                Not all of the column spacings are equal
                                RulerBorderStyle::Table
                                Table border. Whenever this style is set, the column
                                width must be 0.
                                RulerBorderStyle::Snap
                                Auxiliary line. Whenever this style is set, the
                                column width must be 0.
                                RulerBorderStyle::Margin
                                Margin. Whenever this style is set, the column
                                width must be 0.

                                The following bits can be set in addition
                                to these styles:
                                RULER_STYLE_INVISIBLE

SetIndents() sets an array of indents. This method may only be used for horizontal
rulers. A Ruler Indent must be passed as an argument, with the following values
initialized:

    long    nPos            - offset relative to the origin in pixels
    sal_uInt16 nStyle       - bit style:
                                RulerIndentStyle::Top    (indent of the first line)
                                RulerIndentStyle::Bottom (left/right indent)
                                RulerIndentStyle::Border (Vertical line that shows the border distance)
                                The following bits can be set in addition
                                to these styles:
                                RULER_STYLE_DONTKNOW (for old position or for
                                                     ambiguity)
                                RULER_STYLE_INVISIBLE

SetTabs() sets an array of tabs. This method may only be used for horizontal rulers.
An array of type RulerTab must be passed as an argument, with the following values
initialized:

    long    nPos            - offset relative to the origin in pixels
    sal_uInt16 nStyle       - bit style:
                                RULER_TAB_DEFAULT (can't be selected)
                                RULER_TAB_LEFT
                                RULER_TAB_CENTER
                                RULER_TAB_RIGHT
                                RULER_TAB_DECIMAL
                                The following bits can be set in addition
                                to these styles:
                                RULER_STYLE_DONTKNOW (for old position of for
                                                     ambiguity)
                                RULER_STYLE_INVISIBLE

SetLines() displays position lines in the ruler. An array of type RulerLine must be passed, with
the following values initialized:

    long    nPos            - offset relative to the origin in pixels
    sal_uInt16 nStyle       - bit style (has to be 0 currently)

--------------------------------------------------------------------------

If the user should also be able to change the margins tabs, borders, ...
in the ruler, a bit more effort is necessary. In this case, the StartDrag(),
Drag() and EndDrag() methods have to be overridden. For the StartDrag() method
it is possible to prevent dragging by returning FALSE. In the drag handler,
the drag position must be queried and the values must be moved to the new
position. This is done by calling the particular Set methods. While in the
drag handler, the values are just cached and only afterward the ruler is redrawn.
All the handlers can also be set as links with the particular Set..Hdl() methods.

    - StartDrag()
        Is called when dragging is started. If FALSE is returned, the dragging.
        won't be executed. If TRUE is returned, the dragging will be permitted.
        If the handler isn't overridden, FALSE will be returned.

    - EndDrag()
        Is called at the end of dragging.

    - Drag()
        Is called when dragging takes place.

    - Click()
        This handler is called when no element has been clicked on.
        The position can be queried with GetClickPos(). This way it is possible
        to, for example, ser tabs in the ruler. After calling the click handler,
        the drag, if any, is immediately triggered. This makes it possible to
        set a new tab in the click handler and then immediately move it.

    - DoubleClick()
        This handler is called when a double-click has been performed outside
        the special panel. The methods GetClickType(), GetClickAryPos() and
        GetClickPos() can be used to query what has been clicked on.
        This way you can, for example, show the tab dialog when a double-click
        is performed on a tab.

In the drag handler it is possible to query what has been dragged and where
it has been dragged. There are the following query methods:

    - GetDragType()
        Returns what has been dragged.
            RulerType::Margin1
            RulerType::Margin2
            RulerType::Border
            RulerType::Indent
            RulerType::Tab

    - GetDragPos()
        Returns the pixel position to which the user has moved the mouse
        relative to the set zero-offset.

    - GetDragAryPos()
        Returns the index in the array if a border, an indent or a tab
        is being dragged. Attention: During a drag process, the array position
        of the item that has been set before the drag is returned.
        Therefore, it is for example also possible, to no longer show a tab
        if the mouse is dragged out of the ruler in bottom/right direction.

    - GetDragSize()
        If Borders are dragged, this can be used to query whether the size
        resp. which side or the position should be changed.
            RulerDragSize::Move or 0      - Move
            RulerDragSize::N1                - left/upper border
            RulerDragSize::N2                - right/bottom border

    - IsDragDelete()
        This method can be used to query whether the mouse has been
        moved out of the window at the bottom/right while dragging.
        By this, it can for example be determined whether the user
        wants to delete a tab.

    - IsDragCanceled()
        Using this Handler, it can be queried in the EndDrag handler
        whether the action was canceled by the user releasing the
        mouse at the top/left of the window or by pressing ESC.
        In this case, the values are not applied. If during the
        dragging, the mouse is dragged out of the window at the
        top/left, the old values are displayed automatically without
        the Drag handler being called.
        But if the user has moved the value to the old position, the
        method returns 'false' nevertheless.
        If this should be avoided, the application must remember the
        old value in the StartDrag handler and compare the value in the
        EndDrag handler.

    - GetDragModifier()
        Returns the modifier keys that were pressed when the Drag process
        was started. See MouseEvent.

    - GetClickPos()
        Returns the pixel position at which the user has pressed the mouse
        with respect to the configured null-offset.

    - GetClickType()
        Returns what is applied by double click:
            RulerType::DontKnow             (no element in the ruler area)
            RulerType::Outside              (outside of the ruler area)
            RulerType::Margin1              (only Margin1 border)
            RulerType::Margin2              (only Margin2 border)
            RulerType::Border               (Border: GetClickAryPos())
            RulerType::Indent               (indent: GetClickAryPos())
            RulerType::Tab                  (Tab: GetClickAryPos())

    - GetClickAryPos()
        Returns the index in the array if a Border, an Indent or a Tab
        is applied via DoubleClick.

    - GetType()
        This method can be used to carry out a HitTest,
        in order to possibly also apply sth to an Item using the right
        mouse button by catching the MouseButtonDown handler. As
        parameters, the window position and possibly a pointer to a
        sal_uInt16 are passed, in order to determine the array position
        of a Tab, an Indent, or a Border. The following values are
        returned as type:
            RulerType::DontKnow             (no element in the ruler area)
            RulerType::Outside              (outside of the ruler area)
            RulerType::Margin1              (only Margin1 border)
            RulerType::Margin2              (only Margin2 border)
            RulerType::Border               (Border: GetClickAryPos())
            RulerType::Indent               (indent: GetClickAryPos())
            RulerType::Tab                  (Tab: GetClickAryPos())

If the drag process should be canceled, this can be done using CancelDrag().
There are the following methods for controlling the Drag:

    - IsDrag()
        Returns 'true' if the ruler is in a drag process.

    - CancelDrag()
        Cancels the drag process, if one is being carried out. During this,
        the old values are restored and the Drag and the EndDrag handlers
        are called.

In order to trigger a Drag from the document, there are the following
methods:

    - StartDocDrag()
        This method is passed the MouseEvent of the document window
        and what should be dragged. If RulerType::DontKnow is passed
        as DragType, the ruler decides what should be dragged. In case
        of the other types, the Drag is only started if a respective
        element was found at the given position.
        This is for example necessary if indents and columns are located
        at the same X position.
        The return value indicates whether the Drag has been triggered.
        If a Drag is triggered, the ruler takes over the normal drag
        control and behaves as if the ruler had been clicked directly.
        Thus, the ruler captures the mouse and also takes over control
        of the Cancel (via keyboard, or if the mouse is moved outside
        of the ruler above it or left of it). All handlers are called,
        too (including the StartDrag handler). If a MouseEvent with
        ClickCount 2 is passed, the DoubleClick handler is also called,
        respectively.

--------------------------------------------------------------------------

For the extra field, the content can be determined and there are handlers
that can be used to handle specific actions.

    - ExtraDown()
        This handler is called when the mouse is pressed in the extra field.

    - SetExtraType()
        With this method, it can be defined what should be displayed in
        the extra field.
            - ExtraType         what should be displayed in the extra field
                                RulerExtra::DontKnow        (nothing)
                                RulerExtra::NullOffset      (coordinate axes)
                                RulerExtra::Tab             (Tab)
            - sal_uInt16 nStyle     bit field as style:
                                    RULER_STYLE_HIGHLIGHT   (selected)
                                    RULER_TAB_...           (a Tab style)

    - GetExtraClick()
        Returns the number of mouse clicks. By this, it is for example
        also possible to trigger an action by a DoubleClick in the
        extra field.

    - GetExtraModifier()
        Returns the modifier keys that were pressed when the extra field
        was clicked. See MouseEvent.

--------------------------------------------------------------------------

Further helper functions:

- static Ruler::DrawTab()
    With this method, a Tab can be output on an OutputDevice.
    By this, it is also possible to show the Tabs in dialogs like
    they are drawn in the ruler.

    This method outputs the Tab centred at the given position. The size
    of the tabs can be defined by the defines RULER_TAB_WIDTH and
    RULER_TAB_HEIGHT.

--------------------------------------------------------------------------

Tips for the use of the ruler:

- For the ruler, neither in the Drag mode nor elsewhere, the setting
  of the values must be bracketed in SetUpdateMode(). The ruler itself
  takes care that, if multiple values are set, they are automatically
  grouped together and output flicker-free.

- Initially, the sizes, positions and values should be set first for the
  ruler, before it is displayed. This is important because otherwise
  many values are calculated unnecessarily.

- When the document window, in which the ruler resides, becomes active
  resp. unactive, the methods Activate() and Deactivate() should be
  called by the ruler. That is so because the display is switched according
  to the settings and the system.

- For example, while dragging Tabs and Indents, the old positions should
  also be shown if possible. For that, while setting the Tabs and Indents,
  the old positions should be inserted first into the array in addition
  and be linked with the style RULER_STYLE_DONTKNOW. After that, the
  remaining values should be set in the array.

- In case of multiple selected paragraphs and table cells, the Tabs and
  Indents should be displayed in grey in front of the first cell resp.
  the first paragraph. This can also be achieved by the style
  RULER_STYLE_DONTKNOW.

- The measuring arrow should always be shown when the Alt key (WW-Like)
  is pressed during the drag. Maybe, this setting should be configurable
  always and possibly the measuring arrows always be shown while
  dragging. For all settings, the values should always be rounded to the
  multiple of one value because the screen resolution is very imprecise.

- DoubleClicks should be handled in the following way (GetClickType()):
    - RulerType::DontKnow
      RulerType::Margin1
      RulerType::Margin2
        If the conditions GetClickPos() <= GetMargin1() or
        GetClickPos() >= GetMargin2() are met or the type is equal to
        RulerType::Margin1 or RulerType::Margin2, a side dialog should
        be displayed in which the focus is at the respective border.
    - RulerType::Border
        A column or table dialog should be shown in which the focus
        is at the respective column that can be queried using
        GetClickAryPos().
    - RulerType::Indent
        The dialog, in which the indents can be configured, should be
        shown. In this, the focus should be on the indent which can
        be queried using GetClickAryPos().
    - RulerType::Tab
        A TabDialog should be displayed in which the Tab, that can be
        queried using GetClickAryPos(), should be selected.

*************************************************************************/


#define WB_EXTRAFIELD     (WinBits(0x00004000))
#define WB_RIGHT_ALIGNED  (WinBits(0x00008000))
#define WB_STDRULER       WB_HORZ


enum class RulerType { DontKnow, Outside,
                 Margin1, Margin2,
                 Border, Indent, Tab };

enum class RulerExtra { DontKnow, NullOffset, Tab };

#define RULER_STYLE_HIGHLIGHT   (sal_uInt16(0x8000))
#define RULER_STYLE_DONTKNOW    (sal_uInt16(0x4000))
#define RULER_STYLE_INVISIBLE   (sal_uInt16(0x2000))

enum class RulerDragSize {
    Move,
    N1,
    N2
};

#define RULER_MOUSE_BORDERMOVE  5
#define RULER_MOUSE_BORDERWIDTH 5
#define RULER_MOUSE_MARGINWIDTH 3


enum class RulerMarginStyle {
    NONE       = 0x0000,
    Sizeable   = 0x0001,
    Invisible  = 0x0002
};
namespace o3tl {
    template<> struct typed_flags<RulerMarginStyle> : is_typed_flags<RulerMarginStyle, 0x0003> {};
}


enum class RulerBorderStyle {
    Sizeable   = 0x0001,
    Moveable   = 0x0002,
    Variable   = 0x0004,
    Invisible  = 0x0008
};
namespace o3tl {
    template<> struct typed_flags<RulerBorderStyle> : is_typed_flags<RulerBorderStyle, 0x000f> {};
}

struct RulerBorder
{
    long             nPos;
    long             nWidth;
    RulerBorderStyle nStyle;
    long             nMinPos; //minimum/maximum position, supported for table borders/rows
    long             nMaxPos;
};

enum class RulerIndentStyle {
    Top, Bottom
};

struct RulerIndent
{
    long              nPos;
    RulerIndentStyle  nStyle;
    bool              bInvisible;
};


#define RULER_TAB_LEFT          (sal_uInt16(0x0000))
#define RULER_TAB_RIGHT         (sal_uInt16(0x0001))
#define RULER_TAB_CENTER        (sal_uInt16(0x0002))
#define RULER_TAB_DECIMAL       (sal_uInt16(0x0003))
#define RULER_TAB_DEFAULT       (sal_uInt16(0x0004))
#define RULER_TAB_STYLE         (sal_uInt16(0x000F))
#define RULER_TAB_RTL           (sal_uInt16(0x0010))

struct RulerTab
{
    long        nPos;
    sal_uInt16  nStyle;
};


struct RulerLine
{
    long    nPos;
};


struct RulerSelection
{
    long          nPos;
    RulerType     eType;
    sal_uInt16    nAryPos;
    RulerDragSize mnDragSize;
    bool          bSize;
    bool          bSizeBar;
    bool          bExpandTest;

    RulerSelection()
        : nPos(0)
        , eType(RulerType::DontKnow)
        , nAryPos(0)
        , mnDragSize(RulerDragSize::Move)
        , bSize(false)
        , bSizeBar(false)
        , bExpandTest( false )
    {}
};


struct RulerUnitData
{
    MapUnit const         eMapUnit;           // MAP_UNIT for calculation
    long const            nTickUnit;          // Unit divider
    double const          nTick1;             // Minimal step
    double const          nTick2;             // Tick quarter unit
    double const          nTick3;             // Tick half unit
    double const          nTick4;             // Tick whole unit
    sal_Char const        aUnitStr[8];        // Unit string
};


// Data for drawing ruler tabstops
struct RulerTabData
{
    sal_uInt16  DPIScaleFactor;
    sal_uInt16  width;
    sal_uInt16  height;
    sal_uInt16  height2;
    sal_uInt16  width2;
    sal_uInt16  cwidth;
    sal_uInt16  cwidth2;
    sal_uInt16  cwidth3;
    sal_uInt16  cwidth4;
    sal_uInt16  dheight;
    sal_uInt16  dheight2;
    sal_uInt16  dwidth;
    sal_uInt16  dwidth2;
    sal_uInt16  dwidth3;
    sal_uInt16  dwidth4;
    sal_uInt16  textoff;
};


class ImplRulerData;

class SVT_DLLPUBLIC Ruler : public vcl::Window
{
private:
    ScopedVclPtr<VirtualDevice>   maVirDev;
    MapMode         maMapMode;
    long            mnBorderOff;
    long            mnWinOff;
    long            mnWinWidth;
    long            mnWidth;
    long            mnHeight;
    long            mnVirOff;
    long            mnVirWidth;
    long            mnVirHeight;
    long            mnBorderWidth;
    long            mnStartDragPos;
    long            mnDragPos;
    std::unique_ptr<ImplRulerData>  mpSaveData;
    ImplRulerData*  mpData;
    std::unique_ptr<ImplRulerData>  mpDragData;
    tools::Rectangle       maExtraRect;
    WinBits         mnWinStyle;
    sal_uInt16      mnUnitIndex;
    sal_uInt16      mnDragAryPos;
    RulerDragSize   mnDragSize;
    sal_uInt16      mnDragModifier;
    sal_uInt16      mnExtraStyle;
    long            mnCharWidth;
    long            mnLineHeight;

    RulerExtra      meExtraType;
    RulerType       meDragType;
    FieldUnit       meUnit;
    Fraction        maZoom;
    bool            mbCalc;
    bool            mbFormat;
    bool            mbDrag;
    bool            mbDragDelete;
    bool            mbDragCanceled;
    bool            mbAutoWinWidth;
    bool            mbActive;
    sal_uInt8       mnUpdateFlags;

    RulerSelection  maHoverSelection;

    Link<Ruler*,void>  maDoubleClickHdl;

    std::unique_ptr<RulerSelection> mxCurrentHitTest;
    std::unique_ptr<RulerSelection> mxPreviousHitTest;

    rtl::Reference<SvtRulerAccessible> mxAccContext;

    std::map<OUString, SalLayoutGlyphs> maTextGlyphs;

    SVT_DLLPRIVATE void ImplVDrawLine(vcl::RenderContext& rRenderContext,  long nX1, long nY1, long nX2, long nY2 );
    SVT_DLLPRIVATE void ImplVDrawRect(vcl::RenderContext& rRenderContext, long nX1, long nY1, long nX2, long nY2 );
    SVT_DLLPRIVATE void ImplVDrawText(vcl::RenderContext& rRenderContext, long nX, long nY, const OUString& rText,
                                      long nMin = LONG_MIN, long nMax = LONG_MAX );

    SVT_DLLPRIVATE void ImplDrawTicks(vcl::RenderContext& rRenderContext,
                                      long nMin, long nMax, long nStart, long nVirTop, long nVirBottom);
    SVT_DLLPRIVATE void ImplDrawBorders(vcl::RenderContext& rRenderContext,
                                        long nMin, long nMax, long nVirTop, long nVirBottom);
    SVT_DLLPRIVATE static void ImplDrawIndent(vcl::RenderContext& rRenderContext,
                                       const tools::Polygon& rPoly, bool bIsHit);
    SVT_DLLPRIVATE void ImplDrawIndents(vcl::RenderContext& rRenderContext,
                                        long nMin, long nMax, long nVirTop, long nVirBottom);
    SVT_DLLPRIVATE void ImplDrawTab(vcl::RenderContext& rRenderContext, const Point& rPos, sal_uInt16 nStyle);
    SVT_DLLPRIVATE void ImplDrawTabs(vcl::RenderContext& rRenderContext,
                                     long nMin, long nMax, long nVirTop, long nVirBottom);

    using Window::ImplInit;
    SVT_DLLPRIVATE void ImplInit( WinBits nWinBits );
    SVT_DLLPRIVATE void ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    SVT_DLLPRIVATE void ImplCalc();
    SVT_DLLPRIVATE void ImplFormat(vcl::RenderContext const & rRenderContext);
    SVT_DLLPRIVATE void ImplInitExtraField( bool bUpdate );
    SVT_DLLPRIVATE void ImplInvertLines(vcl::RenderContext& rRenderContext);
    SVT_DLLPRIVATE void ImplDraw(vcl::RenderContext& rRenderContext);
    SVT_DLLPRIVATE void ImplDrawExtra(vcl::RenderContext& rRenderContext);
    SVT_DLLPRIVATE void ImplUpdate( bool bMustCalc = false );

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

    using Window::ImplHitTest;
    SVT_DLLPRIVATE bool ImplHitTest( const Point& rPosition,
                                         RulerSelection* pHitTest,
                                         bool bRequiredStyle = false,
                                         RulerIndentStyle nRequiredStyle = RulerIndentStyle::Top ) const;
    SVT_DLLPRIVATE bool     ImplDocHitTest( const Point& rPos, RulerType eDragType, RulerSelection* pHitTest ) const;
    SVT_DLLPRIVATE bool     ImplStartDrag( RulerSelection const * pHitTest, sal_uInt16 nModifier );
    SVT_DLLPRIVATE void     ImplDrag( const Point& rPos );
    SVT_DLLPRIVATE void     ImplEndDrag();

    Ruler (const Ruler &) = delete;
    Ruler& operator= (const Ruler &) = delete;

protected:
    long            GetRulerVirHeight() const { return mnVirHeight;}
    const MapMode&  GetCurrentMapMode() const { return maMapMode; }
    const RulerUnitData& GetCurrentRulerUnit() const;

public:
            Ruler( vcl::Window* pParent, WinBits nWinStyle = WB_STDRULER );
    virtual ~Ruler() override;
    virtual void dispose() override;

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    Tracking( const TrackingEvent& rTEvt ) override;
    virtual void    Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void    Resize() override;
    virtual void    StateChanged( StateChangedType nStateChange ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual bool    StartDrag();
    virtual void    Drag();
    virtual void    EndDrag();
    virtual void    Click();
    void            DoubleClick();
    virtual void    ExtraDown();

    void            Activate() override;
    void            Deactivate() override;

    void            SetWinPos( long nOff, long nWidth = 0 );
    long            GetWinOffset() const { return mnWinOff; }
    void            SetPagePos( long nOff = 0, long nWidth = 0 );
    long            GetPageOffset() const;
    void            SetBorderPos( long nOff = 0 );
    long            GetBorderOffset() const { return mnBorderOff; }
    const tools::Rectangle& GetExtraRect() const { return maExtraRect; }

    void            SetUnit( FieldUnit eNewUnit );
    FieldUnit       GetUnit() const { return meUnit; }
    void            SetZoom( const Fraction& rNewZoom );

    void            SetExtraType( RulerExtra eNewExtraType, sal_uInt16 nStyle = 0 );

    bool            StartDocDrag( const MouseEvent& rMEvt,
                                  RulerType eDragType );
    RulerType       GetDragType() const { return meDragType; }
    long            GetDragPos() const { return mnDragPos; }
    sal_uInt16      GetDragAryPos() const { return mnDragAryPos; }
    RulerDragSize   GetDragSize() const { return mnDragSize; }
    bool            IsDragDelete() const { return mbDragDelete; }
    bool            IsDragCanceled() const { return mbDragCanceled; }
    sal_uInt16      GetDragModifier() const { return mnDragModifier; }
    bool            IsDrag() const { return mbDrag; }
    void            CancelDrag();
    long            GetClickPos() const { return mnDragPos; }
    RulerType       GetClickType() const { return meDragType; }

    const RulerSelection& GetHoverSelection() const { return maHoverSelection; }

    using Window::GetType;
    RulerType       GetType( const Point& rPos, sal_uInt16* pAryPos = nullptr );

    void            SetNullOffset( long nPos );
    long            GetNullOffset() const;
    void            SetMargin1() { SetMargin1( 0, RulerMarginStyle::Invisible ); }
    void            SetMargin1( long nPos, RulerMarginStyle nMarginStyle = RulerMarginStyle::Sizeable );
    long            GetMargin1() const;
    void            SetMargin2() { SetMargin2( 0, RulerMarginStyle::Invisible ); }
    void            SetMargin2( long nPos, RulerMarginStyle nMarginStyle = RulerMarginStyle::Sizeable );
    long            GetMargin2() const;

    void            SetLeftFrameMargin( long nPos );
    void            SetRightFrameMargin( long nPos );
    void            SetLines( sal_uInt32 n = 0, const RulerLine* pLineAry = nullptr );
    void            SetBorders( sal_uInt32 n = 0, const RulerBorder* pBrdAry = nullptr );
    void            SetIndents( sal_uInt32 n = 0, const RulerIndent* pIndentAry = nullptr );

    void            SetTabs( sal_uInt32 n = 0, const RulerTab* pTabAry = nullptr );

    static void     DrawTab(vcl::RenderContext& rRenderContext, const Color &rFillColor,
                            const Point& rPos, sal_uInt16 nStyle);

    void            SetStyle( WinBits nStyle );
    WinBits         GetStyle() const { return mnWinStyle; }

    void            SetDoubleClickHdl( const Link<Ruler*,void>& rLink ) { maDoubleClickHdl = rLink; }

    void            SetTextRTL(bool bRTL);
    bool            GetTextRTL();
    void            SetCharWidth( long nWidth ) { mnCharWidth = nWidth ; }
    void            SetLineHeight( long nHeight ) { mnLineHeight = nHeight ; }

    void            DrawTicks();

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;
};

#endif // INCLUDED_SVTOOLS_RULER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
