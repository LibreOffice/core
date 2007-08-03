/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: status.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 14:05:11 $
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

#ifndef _SV_STATUS_HXX
#define _SV_STATUS_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

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
