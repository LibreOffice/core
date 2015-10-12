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

#ifndef INCLUDED_VCL_STATUS_HXX
#define INCLUDED_VCL_STATUS_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/window.hxx>
#include <vector>

struct ImplStatusItem;
typedef ::std::vector< ImplStatusItem* > ImplStatusItemList;


// - Progress-Ausgabe -

void VCL_DLLPUBLIC DrawProgress(vcl::Window* pWindow, vcl::RenderContext& rRenderContext, const Point& rPos,
                                long nOffset, long nPrgsWidth, long nPrgsHeight,
                                sal_uInt16 nPercent1, sal_uInt16 nPercent2, sal_uInt16 nPercentCount,
                                const Rectangle& rFramePosSize);


// - StatusBarItemBits -


typedef sal_uInt16 StatusBarItemBits;


// - Bits fuer StatusBarItems -


#define SIB_LEFT                    ((StatusBarItemBits)0x0001)
#define SIB_CENTER                  ((StatusBarItemBits)0x0002)
#define SIB_RIGHT                   ((StatusBarItemBits)0x0004)
#define SIB_IN                      ((StatusBarItemBits)0x0008)
#define SIB_OUT                     ((StatusBarItemBits)0x0010)
#define SIB_FLAT                    ((StatusBarItemBits)0x0020)
#define SIB_AUTOSIZE                ((StatusBarItemBits)0x0040)
#define SIB_USERDRAW                ((StatusBarItemBits)0x0080)


// - StatusBar-Types -


#define STATUSBAR_APPEND            ((sal_uInt16)0xFFFF)
#define STATUSBAR_ITEM_NOTFOUND     ((sal_uInt16)0xFFFF)
#define STATUSBAR_OFFSET            ((long)5)


// - StatusBar -


class VCL_DLLPUBLIC StatusBar : public vcl::Window
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
    sal_uInt16          mnCurItemId;
    sal_uInt16          mnPercent;
    sal_uInt16          mnPercentCount;
    bool                mbVisibleItems;
    bool                mbFormat;
    bool                mbProgressMode;
    bool                mbInUserDraw;
    bool                mbAdjustHiDPI;
    Link<StatusBar*,void>  maClickHdl;
    Link<StatusBar*,void>  maDoubleClickHdl;

    using Window::ImplInit;
    SAL_DLLPRIVATE void      ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void      ImplInitSettings();
    SAL_DLLPRIVATE void      ImplFormat();
    SAL_DLLPRIVATE bool      ImplIsItemUpdate();

    SAL_DLLPRIVATE void      ImplDrawText(vcl::RenderContext& rRenderContext, bool bOffScreen,
                                          long nOldTextWidth);
    SAL_DLLPRIVATE void      ImplDrawItem(vcl::RenderContext& rRenderContext, bool bOffScreen,
                                          sal_uInt16 nPos, bool bDrawText, bool bDrawFrame);
    SAL_DLLPRIVATE void      ImplDrawProgress(vcl::RenderContext& rRenderContext, bool bPaint,
                                              sal_uInt16 nOldPerc, sal_uInt16 nNewPerc);
    SAL_DLLPRIVATE void      ImplCalcProgressRect();
    SAL_DLLPRIVATE Rectangle ImplGetItemRectPos( sal_uInt16 nPos ) const;
    SAL_DLLPRIVATE sal_uInt16    ImplGetFirstVisiblePos() const;

protected:
    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

public:
                        StatusBar( vcl::Window* pParent,
                                   WinBits nWinStyle = WB_BORDER | WB_RIGHT );
    virtual             ~StatusBar();
    virtual void        dispose() override;

    void                AdjustItemWidthsForHiDPI(bool bAdjustHiDPI);

    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void        Move() override;
    virtual void        Resize() override;
    virtual void        RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void        StateChanged( StateChangedType nType ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;

    void                Click();
    void                DoubleClick();
    virtual void        UserDraw( const UserDrawEvent& rUDEvt );

    void                InsertItem( sal_uInt16 nItemId, sal_uLong nWidth,
                                    StatusBarItemBits nBits = SIB_CENTER | SIB_IN,
                                    long nOffset = STATUSBAR_OFFSET,
                                    sal_uInt16 nPos = STATUSBAR_APPEND );
    void                RemoveItem( sal_uInt16 nItemId );

    void                ShowItem( sal_uInt16 nItemId );
    void                HideItem( sal_uInt16 nItemId );
    bool                IsItemVisible( sal_uInt16 nItemId ) const;

    bool                AreItemsVisible() const { return mbVisibleItems; }

    void                RedrawItem( sal_uInt16 nItemId );

    void                Clear();

    sal_uInt16          GetItemCount() const;
    sal_uInt16          GetItemId( sal_uInt16 nPos ) const;
    sal_uInt16          GetItemId( const Point& rPos ) const;
    sal_uInt16          GetItemPos( sal_uInt16 nItemId ) const;
    Rectangle           GetItemRect( sal_uInt16 nItemId ) const;
    Point               GetItemTextPos( sal_uInt16 nItemId ) const;
    sal_uInt16          GetCurItemId() const { return mnCurItemId; }

    sal_uLong           GetItemWidth( sal_uInt16 nItemId ) const;
    StatusBarItemBits   GetItemBits( sal_uInt16 nItemId ) const;

    long                GetItemOffset( sal_uInt16 nItemId ) const;

    void                SetItemText( sal_uInt16 nItemId, const OUString& rText );
    const OUString&     GetItemText( sal_uInt16 nItemId ) const;

    void                SetItemData( sal_uInt16 nItemId, void* pNewData );
    void*               GetItemData( sal_uInt16 nItemId ) const;

    void                SetItemCommand( sal_uInt16 nItemId, const OUString& rCommand );
    const OUString      GetItemCommand( sal_uInt16 nItemId );

    void                SetHelpText( sal_uInt16 nItemId, const OUString& rText );
    const OUString&     GetHelpText( sal_uInt16 nItemId ) const;

    using Window::SetQuickHelpText;
    void                SetQuickHelpText( sal_uInt16 nItemId, const OUString& rText );
    using Window::GetQuickHelpText;
    const OUString&     GetQuickHelpText( sal_uInt16 nItemId ) const;

    void                SetHelpId( sal_uInt16 nItemId, const OString& rHelpId );
    OString             GetHelpId( sal_uInt16 nItemId ) const;

    void                StartProgressMode( const OUString& rText );
    void                SetProgressValue( sal_uInt16 nPercent );
    void                EndProgressMode();
    bool                IsProgressMode() const { return mbProgressMode; }

    void                SetText( const OUString& rText ) override;

    Size                CalcWindowSizePixel() const;

    void                SetClickHdl( const Link<StatusBar*,void>& rLink ) { maClickHdl = rLink; }
    void                SetDoubleClickHdl( const Link<StatusBar*,void>& rLink ) { maDoubleClickHdl = rLink; }

    using Window::SetAccessibleName;
    void                SetAccessibleName( sal_uInt16 nItemId, const OUString& rName );
    using Window::GetAccessibleName;
    const OUString&     GetAccessibleName( sal_uInt16 nItemId ) const;
};

#endif // INCLUDED_VCL_STATUS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
