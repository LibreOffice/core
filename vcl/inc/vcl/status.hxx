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

#ifndef _SV_STATUS_HXX
#define _SV_STATUS_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/window.hxx>
#include <vector>

struct ImplStatusItem;
typedef ::std::vector< ImplStatusItem* > ImplStatusItemList;

// --------------------
// - Progress-Ausgabe -
// --------------------

void VCL_DLLPUBLIC DrawProgress( Window* pWindow, const Point& rPos,
                                 long nOffset, long nPrgsWidth, long nPrgsHeight,
                                 sal_uInt16 nPercent1, sal_uInt16 nPercent2, sal_uInt16 nPercentCount,
                                 const Rectangle& rFramePosSize
                                 );

// ---------------------
// - StatusBarItemBits -
// ---------------------

typedef sal_uInt16 StatusBarItemBits;

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

#define STATUSBAR_APPEND            ((sal_uInt16)0xFFFF)
#define STATUSBAR_ITEM_NOTFOUND     ((sal_uInt16)0xFFFF)
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
    sal_uInt16              mnCurItemId;
    sal_uInt16              mnPercent;
    sal_uInt16              mnPercentCount;
    sal_Bool                mbVisibleItems;
    sal_Bool                mbFormat;
    sal_Bool                mbProgressMode;
    sal_Bool                mbInUserDraw;
    Link                maClickHdl;
    Link                maDoubleClickHdl;

    using Window::ImplInit;
    SAL_DLLPRIVATE void      ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void      ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    SAL_DLLPRIVATE void      ImplFormat();
    SAL_DLLPRIVATE sal_Bool      ImplIsItemUpdate();
    using OutputDevice::ImplDrawText;
    SAL_DLLPRIVATE void      ImplDrawText( sal_Bool bOffScreen, long nOldTextWidth );
    SAL_DLLPRIVATE void      ImplDrawItem( sal_Bool bOffScreen, sal_uInt16 nPos, sal_Bool bDrawText, sal_Bool bDrawFrame );
    SAL_DLLPRIVATE void      ImplDrawProgress( sal_Bool bPaint,
                                               sal_uInt16 nOldPerc, sal_uInt16 nNewPerc );
    SAL_DLLPRIVATE void      ImplCalcProgressRect();
    SAL_DLLPRIVATE Rectangle ImplGetItemRectPos( sal_uInt16 nPos ) const;
    SAL_DLLPRIVATE sal_uInt16    ImplGetFirstVisiblePos() const;

public:
                        StatusBar( Window* pParent,
                                   WinBits nWinStyle = WB_BORDER | WB_RIGHT );
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

    void                InsertItem( sal_uInt16 nItemId, sal_uLong nWidth,
                                    StatusBarItemBits nBits = SIB_CENTER | SIB_IN,
                                    long nOffset = STATUSBAR_OFFSET,
                                    sal_uInt16 nPos = STATUSBAR_APPEND );

    sal_Bool                IsItemVisible( sal_uInt16 nItemId ) const;
    sal_Bool                AreItemsVisible() const { return mbVisibleItems; }

    void                Clear();

    sal_uInt16              GetItemCount() const;
    sal_uInt16              GetItemId( sal_uInt16 nPos ) const;
    sal_uInt16              GetItemId( const Point& rPos ) const;
    sal_uInt16              GetItemPos( sal_uInt16 nItemId ) const;
    Rectangle           GetItemRect( sal_uInt16 nItemId ) const;
    Point               GetItemTextPos( sal_uInt16 nItemId ) const;
    sal_uInt16              GetCurItemId() const { return mnCurItemId; }

    void                SetItemText( sal_uInt16 nItemId, const XubString& rText );
    const XubString&    GetItemText( sal_uInt16 nItemId ) const;

    void                SetItemData( sal_uInt16 nItemId, void* pNewData );

    void                SetItemCommand( sal_uInt16 nItemId, const XubString& rCommand );
    const XubString&    GetItemCommand( sal_uInt16 nItemId );

    void                SetHelpText( sal_uInt16 nItemId, const XubString& rText );
    const XubString&    GetHelpText( sal_uInt16 nItemId ) const;

    using Window::SetQuickHelpText;
    void                SetQuickHelpText( sal_uInt16 nItemId, const XubString& rText );
    using Window::GetQuickHelpText;
    const XubString&    GetQuickHelpText( sal_uInt16 nItemId ) const;

    void                SetHelpId( sal_uInt16 nItemId, const rtl::OString& rHelpId );
    rtl::OString        GetHelpId( sal_uInt16 nItemId ) const;

    void                StartProgressMode( const XubString& rText );
    void                SetProgressValue( sal_uInt16 nPercent );
    void                EndProgressMode();
    sal_Bool                IsProgressMode() const { return mbProgressMode; }

    void                SetText( const XubString& rText );

    void                SetHelpText( const XubString& rText )
                            { Window::SetHelpText( rText ); }
    const XubString&    GetHelpText() const
                            { return Window::GetHelpText(); }

    void                SetHelpId( const rtl::OString& rId )
                            { Window::SetHelpId( rId ); }
    const rtl::OString& GetHelpId() const
                            { return Window::GetHelpId(); }

    Size                CalcWindowSizePixel() const;

    void                SetClickHdl( const Link& rLink ) { maClickHdl = rLink; }
    const Link&         GetClickHdl() const { return maClickHdl; }
    void                SetDoubleClickHdl( const Link& rLink ) { maDoubleClickHdl = rLink; }
    const Link&         GetDoubleClickHdl() const { return maDoubleClickHdl; }

    using Window::SetAccessibleName;
    void                SetAccessibleName( sal_uInt16 nItemId, const XubString& rName );
    using Window::GetAccessibleName;
    const XubString&    GetAccessibleName( sal_uInt16 nItemId ) const;
};

#endif  // _SV_STATUS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
