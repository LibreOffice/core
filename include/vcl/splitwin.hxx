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

#ifndef INCLUDED_VCL_SPLITWIN_HXX
#define INCLUDED_VCL_SPLITWIN_HXX

#include <vcl/dllapi.h>
#include <vcl/dockwin.hxx>
#include <o3tl/typed_flags_set.hxx>

class ImplSplitSet;

enum class SplitWindowItemFlags
{
    NONE           = 0x0000,
    Fixed          = 0x0001,
    RelativeSize   = 0x0002,
    PercentSize    = 0x0004,
    ColSet         = 0x0008,
};
namespace o3tl
{
    template<> struct typed_flags<SplitWindowItemFlags> : is_typed_flags<SplitWindowItemFlags, 0x0f> {};
}

#define SPLITWINDOW_APPEND          (sal_uInt16(0xFFFF))
#define SPLITWINDOW_ITEM_NOTFOUND   (sal_uInt16(0xFFFF))

class VCL_DLLPUBLIC SplitWindow : public DockingWindow
{
private:
    std::unique_ptr<ImplSplitSet> mpMainSet;
    ImplSplitSet*       mpBaseSet;
    ImplSplitSet*       mpSplitSet;
    long*               mpLastSizes;
    tools::Rectangle           maDragRect;
    long                mnDX;
    long                mnDY;
    long                mnLeftBorder;
    long                mnTopBorder;
    long                mnRightBorder;
    long                mnBottomBorder;
    long                mnMaxSize;
    long                mnMouseOff;
    long                mnMStartPos;
    long                mnMSplitPos;
    WinBits             mnWinStyle;
    WindowAlign         meAlign;
    sal_uInt16          mnSplitTest;
    sal_uInt16          mnSplitPos;
    sal_uInt16          mnMouseModifier;
    bool                mbDragFull:1,
                        mbHorz:1,
                        mbBottomRight:1,
                        mbCalc:1,
                        mbRecalc:1,
                        mbInvalidate:1,
                        mbFadeIn:1,
                        mbFadeOut:1,
                        mbFadeInDown:1,
                        mbFadeOutDown:1,
                        mbFadeInPressed:1,
                        mbFadeOutPressed:1,
                        mbFadeNoButtonMode:1;
    Link<SplitWindow*,void>  maSplitHdl;

    using Window::ImplInit;
    SAL_DLLPRIVATE void ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void ImplInitSettings();
    SAL_DLLPRIVATE void ImplCalcLayout();
    SAL_DLLPRIVATE void ImplUpdate();
    SAL_DLLPRIVATE void ImplSetWindowSize( long nDelta );
    SAL_DLLPRIVATE void ImplSplitMousePos( Point& rMousePos );
    SAL_DLLPRIVATE void ImplGetButtonRect( tools::Rectangle& rRect, bool bTest ) const;
    SAL_DLLPRIVATE void ImplGetFadeInRect( tools::Rectangle& rRect, bool bTest = false ) const;
    SAL_DLLPRIVATE void ImplGetFadeOutRect( tools::Rectangle& rRect ) const;
    SAL_DLLPRIVATE void ImplDrawFadeIn(vcl::RenderContext& rRenderContext);
    SAL_DLLPRIVATE void ImplDrawFadeOut(vcl::RenderContext& rRenderContext);
    SAL_DLLPRIVATE void ImplNewAlign();
    SAL_DLLPRIVATE void ImplDrawGrip(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect, bool bHorz, bool bLeft);
    SAL_DLLPRIVATE void ImplStartSplit( const MouseEvent& rMEvt );

    SAL_DLLPRIVATE void ImplDrawBorder(vcl::RenderContext& rRenderContext);
    SAL_DLLPRIVATE void ImplDrawBorderLine(vcl::RenderContext& rRenderContext);
    static SAL_DLLPRIVATE void ImplCalcSet2( SplitWindow* pWindow, ImplSplitSet* pSet, bool bHide,
                                             bool bRows );
    SAL_DLLPRIVATE void ImplDrawBack(vcl::RenderContext& rRenderContext, ImplSplitSet* pSet );
    static SAL_DLLPRIVATE sal_uInt16 ImplTestSplit( ImplSplitSet* pSet, const Point& rPos,
                                                long& rMouseOff, ImplSplitSet** ppFoundSet, sal_uInt16& rFoundPos,
                                                bool bRows );
    static SAL_DLLPRIVATE sal_uInt16 ImplTestSplit( const SplitWindow* pWindow, const Point& rPos,
                                                long& rMouseOff, ImplSplitSet** ppFoundSet, sal_uInt16& rFoundPos );
    SAL_DLLPRIVATE void ImplDrawSplitTracking(const Point& rPos);

                        SplitWindow (const SplitWindow &) = delete;
                        SplitWindow & operator= (const SplitWindow &) = delete;
public:
                        SplitWindow( vcl::Window* pParent, WinBits nStyle = 0 );
    virtual             ~SplitWindow() override;
    virtual void        dispose() override;

    virtual void        StartSplit();
    virtual void        Split();
    virtual void        SplitResize();
    virtual void        FadeIn();
    virtual void        FadeOut();

    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        MouseMove( const MouseEvent& rMEvt ) override;
    virtual void        Tracking( const TrackingEvent& rTEvt ) override;
    virtual void        Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void        Resize() override;
    virtual void        RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void        StateChanged( StateChangedType nType ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual bool        PreNotify( NotifyEvent& rNEvt ) override;

    void                InsertItem( sal_uInt16 nId, vcl::Window* pWindow, long nSize,
                                    sal_uInt16 nPos, sal_uInt16 nIntoSetId,
                                    SplitWindowItemFlags nBits );
    void                InsertItem( sal_uInt16 nId, long nSize,
                                    sal_uInt16 nPos, sal_uInt16 nIntoSetId,
                                    SplitWindowItemFlags nBits );
    void                RemoveItem( sal_uInt16 nId );
    void                Clear();

    void                SplitItem( sal_uInt16 nId, long nNewSize,
                                   bool bPropSmall,
                                   bool bPropGreat );
    void                SetItemSize( sal_uInt16 nId, long nNewSize );
    long                GetItemSize( sal_uInt16 nId ) const;
    /** Set a range that limits the (variable part of the) size with an
        upper and a lower bound (both are valid values themselves.)
        @param nId
            Id of the item for which the size limits are set.
        @param rRange
            Values of -1 define missing bounds, thus setting a range (-1,-1)
            (the default) removes the size limit.
    */
    void                SetItemSizeRange (sal_uInt16 nId, const Range& rRange);
    /** Return the current size limits for the specified item.
    */
    long                GetItemSize( sal_uInt16 nId, SplitWindowItemFlags nBits ) const;
    sal_uInt16          GetSet( sal_uInt16 nId ) const;
    sal_uInt16          GetItemId( vcl::Window* pWindow ) const;
    sal_uInt16          GetItemId( const Point& rPos ) const;
    sal_uInt16          GetItemPos( sal_uInt16 nId, sal_uInt16 nSetId = 0 ) const;
    sal_uInt16          GetItemId( sal_uInt16 nPos ) const;
    sal_uInt16          GetItemCount( sal_uInt16 nSetId = 0 ) const;
    bool                IsItemValid( sal_uInt16 nId ) const;

    void                SetAlign( WindowAlign eNewAlign );
    WindowAlign         GetAlign() const { return meAlign; }
    bool                IsHorizontal() const { return mbHorz; }

    void                SetMaxSizePixel( long nNewMaxSize ) { mnMaxSize = nNewMaxSize; }

    Size                CalcLayoutSizePixel( const Size& aNewSize );

    void                ShowFadeInHideButton();
    void                ShowFadeOutButton();
    long                GetFadeInSize() const;
    bool                IsFadeNoButtonMode() const { return mbFadeNoButtonMode; }

    void                SetSplitHdl( const Link<SplitWindow*,void>& rLink ) { maSplitHdl = rLink; }
};

#endif // INCLUDED_VCL_SPLITWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
