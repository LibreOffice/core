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

#ifndef _SV_TABCTRL_HXX
#define _SV_TABCTRL_HXX

#include "tools/solar.h"
#include "vcl/dllapi.h"
#include "vcl/ctrl.hxx"

struct ImplTabItem;
struct ImplTabCtrlData;
class TabPage;
class PushButton;
class ListBox;

// --------------------
// - TabControl-Types -
// --------------------

#ifndef TAB_APPEND
#define TAB_APPEND          ((sal_uInt16)0xFFFF)
#define TAB_PAGE_NOTFOUND   ((sal_uInt16)0xFFFF)
#endif /* !TAB_APPEND */

// --------------
// - TabControl -
// --------------

class VCL_DLLPUBLIC TabControl : public Control
{
private:
    ImplTabCtrlData*    mpTabCtrlData;
    long                mnLastWidth;
    long                mnLastHeight;
    long                mnBtnSize;
    long                mnMaxPageWidth;
    sal_uInt16              mnActPageId;
    sal_uInt16              mnCurPageId;
    sal_Bool                mbFormat;
    sal_Bool                mbRestoreHelpId;
    sal_Bool                mbRestoreUnqId;
    sal_Bool                mbSmallInvalidate;
    bool                    mbLayoutDirty;
    Link                maActivateHdl;
    Link                maDeactivateHdl;

    using Control::ImplInitSettings;
    SAL_DLLPRIVATE void         ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    SAL_DLLPRIVATE ImplTabItem* ImplGetItem( sal_uInt16 nId ) const;
    SAL_DLLPRIVATE Size         ImplGetItemSize( ImplTabItem* pItem, long nMaxWidth );
    SAL_DLLPRIVATE Rectangle    ImplGetTabRect( sal_uInt16 nPos, long nWidth = -1, long nHeight = -1 );
    SAL_DLLPRIVATE void         ImplChangeTabPage( sal_uInt16 nId, sal_uInt16 nOldId );
    SAL_DLLPRIVATE sal_Bool         ImplPosCurTabPage();
    SAL_DLLPRIVATE void         ImplActivateTabPage( sal_Bool bNext );
    SAL_DLLPRIVATE void         ImplShowFocus();
    SAL_DLLPRIVATE void         ImplDrawItem( ImplTabItem* pItem, const Rectangle& rCurRect, bool bLayout = false, bool bFirstInGroup = false, bool bLastInGroup = false, bool bIsCurrentItem = false );
    SAL_DLLPRIVATE void         ImplPaint( const Rectangle& rRect, bool bLayout = false );
    SAL_DLLPRIVATE void         ImplFreeLayoutData();
    SAL_DLLPRIVATE long         ImplHandleKeyEvent( const KeyEvent& rKeyEvent );

    DECL_DLLPRIVATE_LINK(       ImplListBoxSelectHdl, void* );
    DECL_DLLPRIVATE_LINK(       ImplWindowEventListener, VclSimpleEvent* );


protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void         ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void         ImplLoadRes( const ResId& rResId );

    virtual void                FillLayoutData() const;
    virtual const Font&         GetCanonicalFont( const StyleSettings& _rStyle ) const;
    virtual const Color&        GetCanonicalTextColor( const StyleSettings& _rStyle ) const;
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

    virtual Size GetOptimalSize() const;

    void                SetTabPageSizePixel( const Size& rSize );
    Size                GetTabPageSizePixel() const;

    //  pixel offset for the tab items, default is (0,0)
    void                SetItemsOffset( const Point& rOffs );
    Point               GetItemsOffset() const;

    void                InsertPage( const ResId& rResId,
                                    sal_uInt16 nPos = TAB_APPEND );
    void                InsertPage( sal_uInt16 nPageId, const XubString& rText,
                                    sal_uInt16 nPos = TAB_APPEND );
    void                RemovePage( sal_uInt16 nPageId );
    void                Clear();
    void                EnablePage( sal_uInt16 nPageId, bool bEnable = true );

    sal_uInt16          GetPagePos( sal_uInt16 nPageId ) const;
    sal_uInt16          GetPageCount() const;
    sal_uInt16          GetPageId( sal_uInt16 nPos ) const;
    sal_uInt16          GetPageId( const Point& rPos ) const;
    sal_uInt16          GetPageId( const TabPage& rPage ) const;
    sal_uInt16          GetPageId( const OString& rName ) const;

    void                SetCurPageId( sal_uInt16 nPageId );
    sal_uInt16              GetCurPageId() const;

    void                SelectTabPage( sal_uInt16 nPageId );

    void                SetMaxPageWidth( long nMaxWidth ) { mnMaxPageWidth = nMaxWidth; }
    long                GetMaxPageWidth() const { return mnMaxPageWidth; }
    void                ResetMaxPageWidth() { SetMaxPageWidth( 0 ); }
    sal_Bool                IsMaxPageWidth() const { return mnMaxPageWidth != 0; }

    void                SetTabPage( sal_uInt16 nPageId, TabPage* pPage );
    TabPage*            GetTabPage( sal_uInt16 nPageId ) const;

    void                SetPageText( sal_uInt16 nPageId, const XubString& rText );
    XubString           GetPageText( sal_uInt16 nPageId ) const;

    void                SetHelpText( sal_uInt16 nPageId, const XubString& rText );
    const XubString&    GetHelpText( sal_uInt16 nPageId ) const;

    void                SetHelpId( sal_uInt16 nPageId, const OString& rId ) const;
    OString             GetHelpId( sal_uInt16 nPageId ) const;

    void                SetPageName( sal_uInt16 nPageId, const OString& rName ) const;
    OString             GetPageName( sal_uInt16 nPageId ) const;

    void                SetPageImage( sal_uInt16 nPageId, const Image& rImage );

    void                SetHelpText( const XubString& rText )
                            { Control::SetHelpText( rText ); }
    const XubString&    GetHelpText() const
                            { return Control::GetHelpText(); }

    void                SetHelpId( const OString& rId )
                            { Control::SetHelpId( rId ); }
    const OString& GetHelpId() const
                            { return Control::GetHelpId(); }

    void                SetActivatePageHdl( const Link& rLink ) { maActivateHdl = rLink; }
    const Link&         GetActivatePageHdl() const { return maActivateHdl; }
    void                SetDeactivatePageHdl( const Link& rLink ) { maDeactivateHdl = rLink; }
    const Link&         GetDeactivatePageHdl() const { return maDeactivateHdl; }

    // returns (control relative) bounding rectangle for the
    // character at index nIndex relative to the text of page nPageId
    using Control::GetCharacterBounds;
    Rectangle GetCharacterBounds( sal_uInt16 nPageId, long nIndex ) const;

    // returns the index relative to the text of page nPageId (also returned)
    // at position rPoint (control relative)
    using Control::GetIndexForPoint;
    long GetIndexForPoint( const Point& rPoint, sal_uInt16& rPageId ) const;

    // returns the rectangle of the tab for page nPageId
    Rectangle GetTabBounds( sal_uInt16 nPageId ) const;

    virtual void SetPosPixel(const Point& rPos);
    virtual void SetSizePixel(const Size& rNewSize);
    virtual void SetPosSizePixel(const Point& rNewPos, const Size& rNewSize);

    Size calculateRequisition() const;
    void setAllocation(const Size &rAllocation);

    void markLayoutDirty()
    {
        mbLayoutDirty = true;
    }
};

#endif  // _SV_TABCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
