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
#include <o3tl/typed_flags_set.hxx>
#include <memory>
#include <vector>

class DataChangedEvent;
class HelpEvent;
class MouseEvent;
class UserDrawEvent;
struct ImplStatusItem;

void VCL_DLLPUBLIC DrawProgress(vcl::Window* pWindow, vcl::RenderContext& rRenderContext, const Point& rPos,
                                long nOffset, long nPrgsWidth, long nPrgsHeight,
                                sal_uInt16 nPercent1, sal_uInt16 nPercent2, sal_uInt16 nPercentCount,
                                const tools::Rectangle& rFramePosSize);


enum class StatusBarItemBits {
    NONE            = 0x0000,
    Left            = 0x0001,
    Center          = 0x0002,
    Right           = 0x0004,
    In              = 0x0008,
    Out             = 0x0010,
    Flat            = 0x0020,
    AutoSize        = 0x0040,
    UserDraw        = 0x0080,
    Mandatory       = 0x0100,
};
namespace o3tl
{
    template<> struct typed_flags<StatusBarItemBits> : is_typed_flags<StatusBarItemBits, 0x01ff> {};
}

#define STATUSBAR_APPEND            (sal_uInt16(0xFFFF))
#define STATUSBAR_ITEM_NOTFOUND     (sal_uInt16(0xFFFF))
#define STATUSBAR_OFFSET            (long(5))


class VCL_DLLPUBLIC StatusBar : public vcl::Window
{
    class   ImplData;
private:
    std::vector<std::unique_ptr<ImplStatusItem>> mvItemList;
    std::unique_ptr<ImplData> mpImplData;
    OUString            maPrgsTxt;
    Point               maPrgsTxtPos;
    tools::Rectangle           maPrgsFrameRect;
    long                mnPrgsSize;
    long                mnItemsWidth;
    long                mnDX;
    long                mnDY;
    long                mnCalcHeight;
    long                mnTextY;
    sal_uInt16          mnCurItemId;
    sal_uInt16          mnPercent;
    sal_uInt16          mnPercentCount;
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

    SAL_DLLPRIVATE void      ImplDrawText(vcl::RenderContext& rRenderContext);
    SAL_DLLPRIVATE void      ImplDrawItem(vcl::RenderContext& rRenderContext, bool bOffScreen,
                                          sal_uInt16 nPos);
    SAL_DLLPRIVATE void      ImplDrawProgress(vcl::RenderContext& rRenderContext, sal_uInt16 nNewPerc);
    SAL_DLLPRIVATE void      ImplCalcProgressRect();
    SAL_DLLPRIVATE tools::Rectangle ImplGetItemRectPos( sal_uInt16 nPos ) const;
    SAL_DLLPRIVATE sal_uInt16    ImplGetFirstVisiblePos() const;

protected:
    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

public:
                        StatusBar( vcl::Window* pParent,
                                   WinBits nWinStyle = WB_BORDER | WB_RIGHT );
    virtual             ~StatusBar() override;
    virtual void        dispose() override;

    void                AdjustItemWidthsForHiDPI();

    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void        Resize() override;
    virtual void        RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void        StateChanged( StateChangedType nType ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;

    void                Click();
    void                DoubleClick();
    virtual void        UserDraw( const UserDrawEvent& rUDEvt );

    void                InsertItem( sal_uInt16 nItemId, sal_uLong nWidth,
                                    StatusBarItemBits nBits = StatusBarItemBits::Center | StatusBarItemBits::In,
                                    long nOffset = STATUSBAR_OFFSET,
                                    sal_uInt16 nPos = STATUSBAR_APPEND );
    void                RemoveItem( sal_uInt16 nItemId );

    void                ShowItem( sal_uInt16 nItemId );
    void                HideItem( sal_uInt16 nItemId );
    bool                IsItemVisible( sal_uInt16 nItemId ) const;

    void                RedrawItem( sal_uInt16 nItemId );

    void                Clear();

    sal_uInt16          GetItemCount() const;
    sal_uInt16          GetItemId( sal_uInt16 nPos ) const;
    sal_uInt16          GetItemId( const Point& rPos ) const;
    sal_uInt16          GetItemPos( sal_uInt16 nItemId ) const;
    tools::Rectangle           GetItemRect( sal_uInt16 nItemId ) const;
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
