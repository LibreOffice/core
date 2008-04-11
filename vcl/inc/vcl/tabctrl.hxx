/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tabctrl.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _SV_TABCTRL_HXX
#define _SV_TABCTRL_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>

struct ImplTabItem;
struct ImplTabCtrlData;
class ImplTabItemList;
class TabPage;
class PushButton;

// --------------------
// - TabControl-Types -
// --------------------

#define TAB_APPEND          ((USHORT)0xFFFF)
#define TAB_PAGE_NOTFOUND   ((USHORT)0xFFFF)

// --------------
// - TabControl -
// --------------

class VCL_DLLPUBLIC TabControl : public Control
{
private:
    ImplTabItemList*    mpItemList;
    ImplTabCtrlData*    mpTabCtrlData;
    long                mnLastWidth;
    long                mnLastHeight;
    long                mnBtnSize;
    long                mnMaxPageWidth;
    USHORT              mnActPageId;
    USHORT              mnCurPageId;
    USHORT              mnFirstPagePos;
    USHORT              mnLastFirstPagePos;
    BOOL                mbFormat;
    BOOL                mbRestoreHelpId;
    BOOL                mbRestoreUnqId;
    BOOL                mbSingleLine;
    BOOL                mbScroll;
    BOOL                mbColored;
    BOOL                mbSmallInvalidate;
    BOOL                mbExtraSpace;
    Link                maActivateHdl;
    Link                maDeactivateHdl;

    SAL_DLLPRIVATE void         ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );
    SAL_DLLPRIVATE ImplTabItem* ImplGetItem( USHORT nId ) const;
    SAL_DLLPRIVATE void         ImplScrollBtnsColor();
    SAL_DLLPRIVATE void         ImplSetScrollBtnsState();
    SAL_DLLPRIVATE void         ImplPosScrollBtns();
    SAL_DLLPRIVATE Size         ImplGetItemSize( ImplTabItem* pItem, long nMaxWidth );
    SAL_DLLPRIVATE Rectangle    ImplGetTabRect( USHORT nPos, long nWidth = -1, long nHeight = -1 );
    SAL_DLLPRIVATE void         ImplChangeTabPage( USHORT nId, USHORT nOldId );
    SAL_DLLPRIVATE BOOL         ImplPosCurTabPage();
    SAL_DLLPRIVATE void         ImplActivateTabPage( BOOL bNext );
    SAL_DLLPRIVATE void         ImplSetFirstPagePos( USHORT nPagePos );
    SAL_DLLPRIVATE void         ImplShowFocus();
    SAL_DLLPRIVATE void         ImplDrawItem( ImplTabItem* pItem, const Rectangle& rCurRect, bool bLayout = false, bool bFirstInGroup = false, bool bLastInGroup = false, bool bIsCurrentItem = false );
    SAL_DLLPRIVATE void         ImplPaint( const Rectangle& rRect, bool bLayout = false );
    SAL_DLLPRIVATE void         ImplFreeLayoutData();
    DECL_DLLPRIVATE_LINK(       ImplScrollBtnHdl, PushButton* pBtn );

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void         ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void         ImplLoadRes( const ResId& rResId );

    virtual void                FillLayoutData() const;
    SAL_DLLPRIVATE Rectangle*   ImplFindPartRect( const Point& rPt );

public:
                        TabControl( Window* pParent,
                                    WinBits nStyle = WB_STDTABCONTROL );
                        TabControl( Window* pParent, const ResId& rResId );
                        ~TabControl();

    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        Paint( const Rectangle& rRect );
    virtual void        Resize();
    virtual void        GetFocus();
    virtual void        LoseFocus();
    virtual void        RequestHelp( const HelpEvent& rHEvt );
    virtual void        Command( const CommandEvent& rCEvt );
    virtual long        Notify( NotifyEvent& rNEvt );
    virtual void        StateChanged( StateChangedType nType );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
    virtual long        PreNotify( NotifyEvent& rNEvt );

    virtual void        ActivatePage();
    virtual long        DeactivatePage();

    void                SetTabPageSizePixel( const Size& rSize );
    Size                GetTabPageSizePixel() const;

    //  pixel offset for the tab items, default is (0,0)
    void                SetItemsOffset( const Point& rOffs );
    Point               GetItemsOffset() const;

    void                InsertPage( const ResId& rResId,
                                    USHORT nPos = TAB_APPEND );
    void                InsertPage( USHORT nPageId, const XubString& rText,
                                    USHORT nPos = TAB_APPEND );
    void                RemovePage( USHORT nPageId );
    void                Clear();

    USHORT              GetPageCount() const;
    USHORT              GetPageId( USHORT nPos ) const;
    USHORT              GetPagePos( USHORT nPageId ) const;
    USHORT              GetPageId( const Point& rPos ) const;

    void                SetCurPageId( USHORT nPageId );
    USHORT              GetCurPageId() const;

    void                SetFirstPageId( USHORT nPageId );
    USHORT              GetFirstPageId() const { return GetPageId( mnFirstPagePos ); }

    void                SelectTabPage( USHORT nPageId );

    void                SetMaxPageWidth( long nMaxWidth ) { mnMaxPageWidth = nMaxWidth; }
    long                GetMaxPageWidth() const { return mnMaxPageWidth; }
    void                ResetMaxPageWidth() { SetMaxPageWidth( 0 ); }
    BOOL                IsMaxPageWidth() const { return mnMaxPageWidth != 0; }

    void                SetTabPage( USHORT nPageId, TabPage* pPage );
    TabPage*            GetTabPage( USHORT nPageId ) const;
    USHORT              GetTabPageResId( USHORT nPageId ) const;

    void                SetPageText( USHORT nPageId, const XubString& rText );
    XubString           GetPageText( USHORT nPageId ) const;

    void                SetHelpText( USHORT nPageId, const XubString& rText );
    const XubString&    GetHelpText( USHORT nPageId ) const;

    void                SetHelpId( USHORT nPageId, ULONG nHelpId );
    ULONG               GetHelpId( USHORT nPageId ) const;

    void                SetHelpText( const XubString& rText )
                            { Control::SetHelpText( rText ); }
    const XubString&    GetHelpText() const
                            { return Control::GetHelpText(); }

    void                SetHelpId( ULONG nId )
                            { Control::SetHelpId( nId ); }
    ULONG               GetHelpId() const
                            { return Control::GetHelpId(); }

    void                SetActivatePageHdl( const Link& rLink ) { maActivateHdl = rLink; }
    const Link&         GetActivatePageHdl() const { return maActivateHdl; }
    void                SetDeactivatePageHdl( const Link& rLink ) { maDeactivateHdl = rLink; }
    const Link&         GetDeactivatePageHdl() const { return maDeactivateHdl; }

    // returns (control relative) bounding rectangle for the
    // character at index nIndex relative to the text of page nPageId
    using Control::GetCharacterBounds;
    Rectangle GetCharacterBounds( USHORT nPageId, long nIndex ) const;

    // returns the index relative to the text of page nPageId (also returned)
    // at position rPoint (control relative)
    using Control::GetIndexForPoint;
    long GetIndexForPoint( const Point& rPoint, USHORT& rPageId ) const;

    // returns the bounding rectangle of the union of tab page area and the
    // corresponding tab
    Rectangle GetTabPageBounds( USHORT nPageId ) const;

    // returns the rectangle of the tab for page nPageId
    Rectangle GetTabBounds( USHORT nPageId ) const;
};

#endif  // _SV_TABCTRL_HXX
