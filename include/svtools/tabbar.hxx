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

#ifndef INCLUDED_SVTOOLS_TABBAR_HXX
#define INCLUDED_SVTOOLS_TABBAR_HXX

#include <svtools/svtdllapi.h>
#include <tools/link.hxx>
#include <vcl/window.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <memory>

class StyleSettings;

/*

Allowed StylbeBits
------------------

WB_SCROLL       - The tabs can be scrolled via an extra field
WB_MINSCROLL    - The tabs can be scrolled via 2 additional buttons
WB_RANGESELECT  - Connected ranges can be selected
WB_MULTISELECT  - single tabs can be selected
WB_BORDER       - a border is drawn in the top and in the bottom
WB_DRAG         - A StartDrag handler is called by the TabBar, if drag
                  and drop should be started. In addition, drag and drop
                  is activated in the TabBar with EnableDrop().
WB_SIZEABLE     - a Split handler is called by the TabBar, if the user
                  wants to change the width of the TabBar
WB_STDTABBAR    - WB_BORDER


Allowed PageBits
-----------------

Setting page bits modify the display attributes of the tab name

TabBarPageBits::Blue
                - Display tab name in light blue, used in draw for
                  invisible layers and in calc for scenario pages
TabBarPageBits::Italic
                - Display tab name italic, used in draw for
                  locked layers
TabBarPageBits::Underline
                - Display tab name underlined, used in draw for
                  non-printable layers


Handlers
-------

Select          - is called when a tab is selected or unselected
DoubleClick     - Is called when a DoubleClick has been fired in the
                  TabBar. Inside of the handler, GetCurPageId() returns
                  the clicked tab or 0, if no tab has been clicked.
ActivatePage    - Is called, if another page is activated.
                  GetCurPageId() returns the activated page.
DeactivatePage  - Is called, when a page is deactivated. If another page
                  may be activated, true must be returned; if another
                  page shall be excluded from the activation, false must
                  be returned. GetCurPageId() returns the page to be
                  deactivated.


Drag and Drop
-------------

For Drag and Drop, the WinBit WB_DRAG must be set. In addition, the
Command handler, the QueryDrop handler and the Drop handler must be overlaid.
In doing so, the following must be implemented in the handlers:

Command         - If dragging should be started in this handler,
                  StartDrag() must be called. This method
                  then selects the respective entry or returns
                  false, if dragging cannot be carried out.

QueryDrop       - This handler is always called by StarView, when the
                  mouse is pulled over the window while dragging
                  (s.a. SV documentation). In this handler, it must be
                  determined whether a drop is possible. The drop
                  position can be shown in TabBar using ShowDropPos().
                  When calling, the position of the Event must be passed.
                  If the position is at the left or right border,
                  scrolling automatically takes place in the TabBar.
                  This method also returns the respective drop position,
                  which is also needed for a drop. If the window is left
                  while dragging, the drop position can be taken back
                  using HideDropPos(). Thus, it is also possible to handle
                  a drag which was triggered from outside the TabBar.

Drop            - In the Drop handler, the pages have to be moved, or
                  the new pages have to be inserted. The respective
                  drop position can be determined using ShowDropPos().

The following methods are needed for Drag and Drop and must be called
by the handlers:

StartDrag       - Must be called from the Command handler. As parameters,
                  the CommandEvent and a reference to a Region must be
                  passed. This vcl::Region then must be passed in
                  ExecuteDrag(), if the return value indicates that
                  ExecuteDrag shall be carried out. If the entry is not
                  selected, it is set as the current entry beforehand.
                  Because of this, attention must be paid that the Select
                  handler can be called from this method.

ShowDropPos     - This method must be called by the QueryDrop handler,
                  so that the TabBar shows where the Tabs are
                  inserted. This method can also be used in the Drop
                  handler, in order to determine the position at which
                  the Tabs shall be inserted. In the method, the
                  position of the Event must be passed. This method
                  returns the position, at which the Tabs shall be inserted.

HideDropPos     - This method takes back the DropPosition previously
                  displayed using ShowDropPos(). This method should be
                  called, when the window is left in the QueryDrop()
                  handler or the drag process has been ended.

The following methods can be used if the pages should be switched
in the Drag and Drop:

SwitchPage      - This method must be called by the QueryDrop handler
                  if the page, over which the mouse pointer resides,
                  should be switched. This method should be called
                  each time the QueryDrop-Handler is called.
                  Switching the page happens with a delay (500 ms) and
                  is automatically managed by this method.
                  The Position of the Event must be passed in the method.
                  This method returns true if the page has been switched.

EndSwitchPage   - This method resets the data for the switching of the
                  page. This method should be called when the window
                  is left in QueryDrop() or the drag process has been
                  ended.

IsInSwitching   - With this method, it can be queried in
                  ActivatePage()/DeactivatePage() whether this has been
                  caused by SwitchPage(). Thus, for example, switching
                  can be avoided in DeactivatePage() without an error
                  box.


Window Resize
--------------

If the window width can be changed by the user, the WinBit WB_SIZEABLE
must be set. In this case, the following handler must be overlaid:

Split           - When this handler is called, the window should be
                  adapted to the width that is returned by GetSplitSize().
                  In doing so, no minimal or maximum width is taken into
                  account. A minimal size can be queried using
                  GetMinSize() and the maximum width must be calculated
                  by the application itself. As only Online Resize is
                  supported, the window width must be changed inside
                  this handler and possibly the width of dependent windows
                  as well. For this handler, a link can also be set using
                  SetSplitHdl().

The following methods deliver more information while Splitting:

GetSplitSize()  - Returns the width of the TabBar, to which the user
                  wants to resize the window. No minimum or maximum
                  width is taken into account. However, a width < 5
                  is never returned. This method only returns valid
                  values as long as splitting is active.

GetMinSize()    - With this method, a minimum window width can be
                  queried, so that at least something of a Tab is
                  visible. Still, the TabBar can be set more narrow
                  then the width that this method returns.
                  This method can also be called, when no splitting
                  is active.


Edit Mode
----------

The TabBar also offers the user the possibility to change the names
in the Tabs.

EnableEditMode  - With this, it can be configured that on Alt+LeftClick,
                  StartEditMode() is automatically called by the TabBar.
                  In the StartRenaming() handler, the renaming can still
                  be rejected.
StartEditMode   - With this method, the EditMode is started on a Tab.
                  false is returned, if the EditMode is already
                  active, the mode is rejected with StartRenaming()
                  or no space is available for editing.
EndEditMode     - With this method, the EditMode is ended.
SetEditText     - With this method, the text in the AllowRenaming()
                  handler can still be replaced by another text.
GetEditText     - With this method, the text, which the user has typed
                  in, can be queried in the AllowRenaming() handler.
IsInEditMode    - This method is used to query whether the EditMode
                  is active.
IsEditModeCanceled      - This method can be used in the EndRenaming()
                          handler to query whether the renaming has
                          been canceled.
GetEditPageId   - With this method, the tab that is being/has been
                  renamed is queried in the Renaming handlers.

StartRenaming() - This handler is called when the EditMode hast been
                  started using StartEditMode(). GetEditPageId()
                  can be used to query which Tab should be renamed.
                  false should be returned if the EditMod should
                  not be started.
AllowRenaming() - This handler is called when the EditMode is ended
                  (not in case of Cancel). Within this handler, it
                  can then be tested whether the text is OK.
                  The Tab which was renamed can be queried using
                  GetEditPageId().
                  One of the following values should be returned:
                  TAB_RENAMING_YES
                  The Tab is renamed.
                  TAB_RENAMING_NO
                  The Tab is not renamed, but the EditMode remains
                  active, so that the user can adapt the name
                  accordingly.
                  TAB_RENAMING_CANCEL
                  The EditMode was cancelled and the old text
                  is restored.
EndRenaming()   - This handler is called when the EditMode has been
                  ended. The tab that has been renamed can be
                  queried using GetEditPageId(). Using
                  IsEditModeCanceled(), it can be queried whether
                  the mode has been cancelled and the name has
                  thus not been changed.


Maximum Page width
-------------------

The Page width of the tabs can be limited in order to make an easier
navigation by them possible. If then, the text cannot be displayed
completely, it is abbreviated with "..." and the whole text is
displayed in the Tip or in the active help (if no help text is set).
Using EnableAutoMaxPageWidth(), it can be configured whether the
maximum page width should be based on the currently visible width
(which is the default). Otherwise, the maximum page width can
also be set using SetMaxPageWidth() (in pixels) (in this case, the
AutoMaxPageWidth is ignored).

ContextMenu
-----------

If a context-sensitive PopupMenu should be displayed, the Command
handler must be overlaid. Using GetPageId() and when passing the
mouse position, it can be determined whether the mouse click has been
carried out over an item resp. over which item the mouse click has
been carried out.
*/

class Button;

#define WB_RANGESELECT      (WinBits(0x00200000))
#define WB_MULTISELECT      (WinBits(0x00400000))
#define WB_MINSCROLL        (WinBits(0x20000000))
#define WB_INSERTTAB        (WinBits(0x40000000))
#define WB_STDTABBAR        WB_BORDER

// Page bits

enum class TabBarPageBits {
    NONE       = 0x00,
    Blue       = 0x01,
    Italic     = 0x02,
    Underline  = 0x04,
};
namespace o3tl {
    template<> struct typed_flags<TabBarPageBits> : is_typed_flags<TabBarPageBits, 0x07> {};
};

    // interface checks only, do not use in regular control flow

#define TPB_DISPLAY_NAME_ALLFLAGS  (TabBarPageBits::Blue | TabBarPageBits::Italic | TabBarPageBits::Underline)

// - TabBar-Types - used in TabBar::AllowRenaming

enum TabBarAllowRenamingReturnCode {
   TABBAR_RENAMING_NO,
   TABBAR_RENAMING_YES,
   TABBAR_RENAMING_CANCEL
};

class MouseEvent;
class DataChangedEvent;

struct ImplTabBarItem;
struct TabBar_Impl;


class SVT_DLLPUBLIC TabBar : public vcl::Window
{
    friend class    ImplTabButton;
    friend class    ImplTabSizer;

private:
    std::unique_ptr<TabBar_Impl> mpImpl;

    OUString        maEditText;
    Size            maWinSize;
    long            mnMaxPageWidth;
    long            mnCurMaxWidth;
    long            mnOffX;
    long            mnOffY;
    long            mnLastOffX;
    long            mnSplitSize;
    sal_uInt64      mnSwitchTime;
    WinBits         mnWinStyle;
    sal_uInt16      mnCurPageId;
    sal_uInt16      mnFirstPos;
    sal_uInt16      mnDropPos;
    sal_uInt16      mnSwitchId;
    sal_uInt16      mnEditId;

    bool            mbFormat : 1;
    bool            mbFirstFormat : 1;
    bool            mbSizeFormat : 1;
    bool            mbAutoEditMode : 1;
    bool            mbEditCanceled : 1;
    bool            mbDropPos : 1;
    bool            mbInSelect : 1;
    bool            mbMirrored : 1;
    bool            mbScrollAlwaysEnabled : 1;

    Link<TabBar*,void>              maSelectHdl;
    Link<TabBar*,void>              maSplitHdl;
    Link<const CommandEvent&, void> maScrollAreaContextHdl;
    size_t          maCurrentItemList;

    using Window::ImplInit;
    SVT_DLLPRIVATE void            ImplInit( WinBits nWinStyle );
    SVT_DLLPRIVATE void            ImplInitSettings( bool bFont, bool bBackground );
    SVT_DLLPRIVATE void            ImplGetColors(const StyleSettings& rStyleSettings,
                                                 Color& rFaceColor, Color& rFaceTextColor,
                                                 Color& rSelectColor, Color& rSelectTextColor);
    SVT_DLLPRIVATE void            ImplShowPage( sal_uInt16 nPos );
    SVT_DLLPRIVATE bool            ImplCalcWidth();
    SVT_DLLPRIVATE void            ImplFormat();
    SVT_DLLPRIVATE sal_uInt16      ImplGetLastFirstPos();
    SVT_DLLPRIVATE void            ImplInitControls();
    SVT_DLLPRIVATE void            ImplEnableControls();
    SVT_DLLPRIVATE void            ImplSelect();
    SVT_DLLPRIVATE void            ImplActivatePage();
    SVT_DLLPRIVATE bool            ImplDeactivatePage();
    SVT_DLLPRIVATE void            ImplPrePaint();
    SVT_DLLPRIVATE ImplTabBarItem* ImplGetLastTabBarItem( sal_uInt16 nItemCount );

    DECL_DLLPRIVATE_LINK(ImplClickHdl, Button*, void);

    DECL_DLLPRIVATE_LINK(ImplAddClickHandler, Button*, void);

    ImplTabBarItem* seek( size_t i );
    ImplTabBarItem* prev();
    ImplTabBarItem* next();

protected:
    virtual void AddTabClick();
    OUString     GetAuxiliaryText(sal_uInt16 nPageId) const; // needed in derived class LayerTabBar
    void         SetAuxiliaryText(sal_uInt16 nPageId, const OUString& rText );

public:
    static const sal_uInt16 APPEND;
    static const sal_uInt16 PAGE_NOT_FOUND;

                    TabBar( vcl::Window* pParent, WinBits nWinStyle );
    virtual         ~TabBar() override;
    virtual void    dispose() override;

    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void    Resize() override;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void    StateChanged( StateChangedType nStateChange ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;

    virtual void    Select();
    virtual void    DoubleClick();
    void            Split();
    virtual void    ActivatePage();
    virtual bool    DeactivatePage();
    virtual bool    StartRenaming();
    virtual TabBarAllowRenamingReturnCode    AllowRenaming();
    virtual void    EndRenaming();
    virtual void    Mirror();

    virtual void    InsertPage( sal_uInt16 nPageId, const OUString& rText,
                                TabBarPageBits nBits = TabBarPageBits::NONE,
                                sal_uInt16 nPos = TabBar::APPEND );
    void            RemovePage( sal_uInt16 nPageId );
    void            MovePage( sal_uInt16 nPageId, sal_uInt16 nNewPos );

    Color           GetTabBgColor( sal_uInt16 nPageId ) const;
    void            SetTabBgColor( sal_uInt16 nPageId, const Color& aTabBgColor );

    void            Clear();

    bool            IsPageEnabled( sal_uInt16 nPageId ) const;

    void            SetPageBits( sal_uInt16 nPageId, TabBarPageBits nBits );
    TabBarPageBits  GetPageBits( sal_uInt16 nPageId ) const;

    sal_uInt16      GetPageCount() const;
    sal_uInt16      GetPageId( sal_uInt16 nPos ) const;
    sal_uInt16      GetPagePos( sal_uInt16 nPageId ) const;
    sal_uInt16      GetCurPagePos() const { return GetPagePos(GetCurPageId()); }
    sal_uInt16      GetPageId( const Point& rPos ) const;
    tools::Rectangle       GetPageRect( sal_uInt16 nPageId ) const;
    // returns the rectangle in which page tabs are drawn
    tools::Rectangle       GetPageArea() const;

    void            SetCurPageId( sal_uInt16 nPageId );
    sal_uInt16      GetCurPageId() const { return mnCurPageId; }

    void            SetFirstPageId( sal_uInt16 nPageId );
    void            MakeVisible( sal_uInt16 nPageId );

    void            SelectPage( sal_uInt16 nPageId, bool bSelect );
    sal_uInt16      GetSelectPageCount() const;
    bool            IsPageSelected( sal_uInt16 nPageId ) const;
    void            SetProtectionSymbol( sal_uInt16 nPageId, bool bProtection );

    void            SetMaxPageWidth( long nMaxWidth );

    void            EnableEditMode() { mbAutoEditMode = true; }
    bool            StartEditMode( sal_uInt16 nPageId );
    void            EndEditMode( bool bCancel = false );
    void            SetEditText( const OUString& rText ) { maEditText = rText; }
    const OUString& GetEditText() const { return maEditText; }
    bool            IsInEditMode() const;
    bool            IsEditModeCanceled() const { return mbEditCanceled; }
    sal_uInt16      GetEditPageId() const { return mnEditId; }

    /** Mirrors the entire control including position of buttons and splitter.
        Mirroring is done relative to the current direction of the GUI.
        @param bMirrored  sal_True = the control will draw itself RTL in LTR GUI,
            and vice versa; sal_False = the control behaves according to the
            current direction of the GUI. */
    void            SetMirrored(bool bMirrored);
    /** Returns true, if the control is set to mirrored mode (see SetMirrored()). */
    bool            IsMirrored() const { return mbMirrored; }

    /** Sets the control to LTR or RTL mode regardless of the GUI direction.
        @param bRTL  sal_False = the control will draw from left to right;
            sal_True = the control will draw from right to left. */
    void            SetEffectiveRTL( bool bRTL );
    /** Returns true, if the control draws from right to left (see SetEffectiveRTL()). */
    bool            IsEffectiveRTL() const;

    bool            StartDrag( const CommandEvent& rCEvt, vcl::Region& rRegion );
    sal_uInt16      ShowDropPos( const Point& rPos );
    void            HideDropPos();
    void            SwitchPage( const Point& rPos );
    void            EndSwitchPage();

    virtual void    SetPageText( sal_uInt16 nPageId, const OUString& rText );
    OUString        GetPageText( sal_uInt16 nPageId ) const;
    OUString        GetHelpText( sal_uInt16 nPageId ) const;

    long            GetSplitSize() const { return mnSplitSize; }

    using Window::SetHelpText;
    using Window::GetHelpText;
    using Window::SetHelpId;
    using Window::GetHelpId;

    void            SetStyle( WinBits nStyle );
    WinBits         GetStyle() const { return mnWinStyle; }

    void            SetScrollAlwaysEnabled(bool bScrollAlwaysEnabled);

    Size            CalcWindowSizePixel() const;

    void            SetSelectHdl( const Link<TabBar*,void>& rLink ) { maSelectHdl = rLink; }
    void            SetSplitHdl( const Link<TabBar*,void>& rLink ) { maSplitHdl = rLink; }
    void            SetScrollAreaContextHdl( const Link<const CommandEvent&,void>& rLink ) { maScrollAreaContextHdl = rLink; }

    // accessibility
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;
};

#endif // INCLUDED_SVTOOLS_TABBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
