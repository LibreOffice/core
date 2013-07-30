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
    OUString            maPrgsTxt;
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
    void                RemoveItem( sal_uInt16 nItemId );

    void                ShowItem( sal_uInt16 nItemId );
    void                HideItem( sal_uInt16 nItemId );
    sal_Bool                IsItemVisible( sal_uInt16 nItemId ) const;

    void                ShowItems();
    void                HideItems();
    sal_Bool                AreItemsVisible() const { return mbVisibleItems; }

    void                RedrawItem( sal_uInt16 nItemId );

    void                CopyItems( const StatusBar& rStatusBar );
    void                Clear();

    sal_uInt16              GetItemCount() const;
    sal_uInt16              GetItemId( sal_uInt16 nPos ) const;
    sal_uInt16              GetItemId( const Point& rPos ) const;
    sal_uInt16              GetItemPos( sal_uInt16 nItemId ) const;
    Rectangle           GetItemRect( sal_uInt16 nItemId ) const;
    Point               GetItemTextPos( sal_uInt16 nItemId ) const;
    sal_uInt16              GetCurItemId() const { return mnCurItemId; }

    sal_uLong               GetItemWidth( sal_uInt16 nItemId ) const;
    StatusBarItemBits   GetItemBits( sal_uInt16 nItemId ) const;

    long                GetItemOffset( sal_uInt16 nItemId ) const;

    void                SetItemText( sal_uInt16 nItemId, const OUString& rText );
    const OUString&    GetItemText( sal_uInt16 nItemId ) const;

    void                SetItemData( sal_uInt16 nItemId, void* pNewData );
    void*               GetItemData( sal_uInt16 nItemId ) const;

    void                SetItemCommand( sal_uInt16 nItemId, const OUString& rCommand );
    const OUString      GetItemCommand( sal_uInt16 nItemId );

    void                SetHelpText( sal_uInt16 nItemId, const OUString& rText );
    const OUString&    GetHelpText( sal_uInt16 nItemId ) const;

    using Window::SetQuickHelpText;
    void                SetQuickHelpText( sal_uInt16 nItemId, const OUString& rText );
    using Window::GetQuickHelpText;
    const OUString&     GetQuickHelpText( sal_uInt16 nItemId ) const;

    void                SetHelpId( sal_uInt16 nItemId, const OString& rHelpId );
    OString             GetHelpId( sal_uInt16 nItemId ) const;

    void                StartProgressMode( const OUString& rText );
    void                SetProgressValue( sal_uInt16 nPercent );
    void                EndProgressMode();
    sal_Bool                IsProgressMode() const { return mbProgressMode; }

    void                SetText( const OUString& rText );

    void                SetHelpText( const OUString& rText )
                            { Window::SetHelpText( rText ); }
    const OUString&    GetHelpText() const
                            { return Window::GetHelpText(); }

    void                SetHelpId( const OString& rId )
                            { Window::SetHelpId( rId ); }
    const OString&      GetHelpId() const
                            { return Window::GetHelpId(); }

    Size                CalcWindowSizePixel() const;

    void                SetClickHdl( const Link& rLink ) { maClickHdl = rLink; }
    const Link&         GetClickHdl() const { return maClickHdl; }
    void                SetDoubleClickHdl( const Link& rLink ) { maDoubleClickHdl = rLink; }
    const Link&         GetDoubleClickHdl() const { return maDoubleClickHdl; }

    using Window::SetAccessibleName;
    void                SetAccessibleName( sal_uInt16 nItemId, const OUString& rName );
    using Window::GetAccessibleName;
    const OUString&     GetAccessibleName( sal_uInt16 nItemId ) const;
};

#endif  // _SV_STATUS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
