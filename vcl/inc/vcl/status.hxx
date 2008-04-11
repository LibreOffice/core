/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: status.hxx,v $
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

#ifndef _SV_STATUS_HXX
#define _SV_STATUS_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/window.hxx>

class ImplStatusItemList;

// --------------------
// - Progress-Ausgabe -
// --------------------

void VCL_DLLPUBLIC DrawProgress( Window* pWindow, const Point& rPos,
                                 long nOffset, long nPrgsWidth, long nPrgsHeight,
                                 USHORT nPercent1, USHORT nPercent2, USHORT nPercentCount,
                                 const Rectangle& rFramePosSize
                                 );

// ---------------------
// - StatusBarItemBits -
// ---------------------

typedef USHORT StatusBarItemBits;

// ----------------------------
// - Bits fuer StatusBarItems -
// ----------------------------

#define SIB_LEFT                    ((StatusBarItemBits)0x0001)
#define SIB_CENTER                  ((StatusBarItemBits)0x0002)
#define SIB_RIGHT                   ((StatusBarItemBits)0x0004)
#define SIB_IN                      ((StatusBarItemBits)0x0008)
#define SIB_OUT                     ((StatusBarItemBits)0x0010)
#define SIB_FLAT                    ((StatusBarItemBits)0x0020)
#define SIB_AUTOSIZE                ((StatusBarItemBits)0x0040)
#define SIB_USERDRAW                ((StatusBarItemBits)0x0080)

// -------------------
// - StatusBar-Types -
// -------------------

#define STATUSBAR_APPEND            ((USHORT)0xFFFF)
#define STATUSBAR_ITEM_NOTFOUND     ((USHORT)0xFFFF)
#define STATUSBAR_OFFSET            ((long)5)

// -------------
// - StatusBar -
// -------------

class VCL_DLLPUBLIC StatusBar : public Window
{
    class   ImplData;
private:
    ImplStatusItemList* mpItemList;
    ImplData*           mpImplData;
    XubString           maPrgsTxt;
    Point               maPrgsTxtPos;
    Rectangle           maPrgsFrameRect;
    long                mnPrgsSize;
    long                mnItemsWidth;
    long                mnDX;
    long                mnDY;
    long                mnCalcHeight;
    long                mnTextY;
    long                mnItemY;
    USHORT              mnCurItemId;
    USHORT              mnPercent;
    USHORT              mnPercentCount;
    BOOL                mbVisibleItems;
    BOOL                mbFormat;
    BOOL                mbProgressMode;
    BOOL                mbInUserDraw;
    BOOL                mbBottomBorder;
    Link                maClickHdl;
    Link                maDoubleClickHdl;

    using Window::ImplInit;
    SAL_DLLPRIVATE void      ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void      ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );
    SAL_DLLPRIVATE void      ImplFormat();
    SAL_DLLPRIVATE BOOL      ImplIsItemUpdate();
//    #if 0 // _SOLAR__PRIVATE
    using OutputDevice::ImplDrawText;
//    #endif
    SAL_DLLPRIVATE void      ImplDrawText( BOOL bOffScreen, long nOldTextWidth );
    SAL_DLLPRIVATE void      ImplDrawItem( BOOL bOffScreen, USHORT nPos, BOOL bDrawText, BOOL bDrawFrame );
    SAL_DLLPRIVATE void      ImplDrawProgress( BOOL bPaint,
                                               USHORT nOldPerc, USHORT nNewPerc );
    SAL_DLLPRIVATE void      ImplCalcProgressRect();
    SAL_DLLPRIVATE Rectangle ImplGetItemRectPos( USHORT nPos ) const;
    SAL_DLLPRIVATE void      ImplCalcBorder();

public:
                        StatusBar( Window* pParent,
                                   WinBits nWinStyle = WB_BORDER | WB_RIGHT );
                        StatusBar( Window* pParent, const ResId& rResId );
                        ~StatusBar();

    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        Paint( const Rectangle& rRect );
    virtual void        Move();
    virtual void        Resize();
    virtual void        RequestHelp( const HelpEvent& rHEvt );
    virtual void        StateChanged( StateChangedType nType );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    virtual void        Click();
    virtual void        DoubleClick();
    virtual void        UserDraw( const UserDrawEvent& rUDEvt );

    void                InsertItem( USHORT nItemId, ULONG nWidth,
                                    StatusBarItemBits nBits = SIB_CENTER | SIB_IN,
                                    long nOffset = STATUSBAR_OFFSET,
                                    USHORT nPos = STATUSBAR_APPEND );
    void                RemoveItem( USHORT nItemId );

    void                ShowItem( USHORT nItemId );
    void                HideItem( USHORT nItemId );
    BOOL                IsItemVisible( USHORT nItemId ) const;

    void                ShowItems();
    void                HideItems();
    BOOL                AreItemsVisible() const { return mbVisibleItems; }

    void                CopyItems( const StatusBar& rStatusBar );
    void                Clear();

    USHORT              GetItemCount() const;
    USHORT              GetItemId( USHORT nPos ) const;
    USHORT              GetItemId( const Point& rPos ) const;
    USHORT              GetItemPos( USHORT nItemId ) const;
    Rectangle           GetItemRect( USHORT nItemId ) const;
    Point               GetItemTextPos( USHORT nItemId ) const;
    USHORT              GetCurItemId() const { return mnCurItemId; }

    ULONG               GetItemWidth( USHORT nItemId ) const;
    StatusBarItemBits   GetItemBits( USHORT nItemId ) const;
    long                GetItemOffset( USHORT nItemId ) const;

    void                SetItemText( USHORT nItemId, const XubString& rText );
    const XubString&    GetItemText( USHORT nItemId ) const;

    void                SetItemData( USHORT nItemId, void* pNewData );
    void*               GetItemData( USHORT nItemId ) const;

    void                SetItemCommand( USHORT nItemId, const XubString& rCommand );
    const XubString&    GetItemCommand( USHORT nItemId );

    void                SetHelpText( USHORT nItemId, const XubString& rText );
    const XubString&    GetHelpText( USHORT nItemId ) const;

    using Window::SetQuickHelpText;
    void                SetQuickHelpText( USHORT nItemId, const XubString& rText );
    using Window::GetQuickHelpText;
    const XubString&    GetQuickHelpText( USHORT nItemId ) const;

    void                SetHelpId( USHORT nItemId, ULONG nHelpId );
    ULONG               GetHelpId( USHORT nItemId ) const;

    void                SetBottomBorder( BOOL bBottomBorder = TRUE );
    BOOL                IsBottomBorder() const { return mbBottomBorder; }

    void                SetTopBorder( BOOL bTopBorder = TRUE );
    BOOL                IsTopBorder() const;

    void                StartProgressMode( const XubString& rText );
    void                SetProgressValue( USHORT nPercent );
    void                EndProgressMode();
    BOOL                IsProgressMode() const { return mbProgressMode; }
    void                ResetProgressMode();

    void                SetText( const XubString& rText );

    void                SetHelpText( const XubString& rText )
                            { Window::SetHelpText( rText ); }
    const XubString&    GetHelpText() const
                            { return Window::GetHelpText(); }

    void                SetHelpId( ULONG nId )
                            { Window::SetHelpId( nId ); }
    ULONG               GetHelpId() const
                            { return Window::GetHelpId(); }

    Size                CalcWindowSizePixel() const;

    void                SetClickHdl( const Link& rLink ) { maClickHdl = rLink; }
    const Link&         GetClickHdl() const { return maClickHdl; }
    void                SetDoubleClickHdl( const Link& rLink ) { maDoubleClickHdl = rLink; }
    const Link&         GetDoubleClickHdl() const { return maDoubleClickHdl; }

    using Window::SetAccessibleName;
    void                SetAccessibleName( USHORT nItemId, const XubString& rName );
    using Window::GetAccessibleName;
    const XubString&    GetAccessibleName( USHORT nItemId ) const;
};

#endif  // _SV_STATUS_HXX
