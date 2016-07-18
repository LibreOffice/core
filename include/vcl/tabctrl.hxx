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

#ifndef INCLUDED_VCL_TABCTRL_HXX
#define INCLUDED_VCL_TABCTRL_HXX

#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>
#include <vcl/EnumContext.hxx>

struct ImplTabItem;
struct ImplTabCtrlData;
class TabPage;
class PushButton;
class ListBox;

#ifndef TAB_APPEND
#define TAB_APPEND          ((sal_uInt16)0xFFFF)
#define TAB_PAGE_NOTFOUND   ((sal_uInt16)0xFFFF)
#endif /* !TAB_APPEND */

#define TAB_OFFSET          3
#define TAB_TABOFFSET_X     3
#define TAB_TABOFFSET_Y     3
#define TAB_EXTRASPACE_X    6
#define TAB_BORDER_LEFT     1
#define TAB_BORDER_TOP      1
#define TAB_BORDER_RIGHT    2
#define TAB_BORDER_BOTTOM   2

class VCL_DLLPUBLIC TabControl : public Control
{
protected:
    ImplTabCtrlData*    mpTabCtrlData;
    long                mnLastWidth;
    long                mnLastHeight;
    long                mnMaxPageWidth;
    sal_uInt16          mnActPageId;
    sal_uInt16          mnCurPageId;
    bool                mbFormat;
    bool                mbRestoreHelpId;
    bool                mbSmallInvalidate;
    bool                mbLayoutDirty;
    Link<TabControl*,void> maActivateHdl;
    Link<TabControl*,bool> maDeactivateHdl;

    using Control::ImplInitSettings;
    SAL_DLLPRIVATE void         ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    SAL_DLLPRIVATE ImplTabItem* ImplGetItem( sal_uInt16 nId ) const;
    SAL_DLLPRIVATE Size         ImplGetItemSize( ImplTabItem* pItem, long nMaxWidth );
    SAL_DLLPRIVATE Rectangle    ImplGetTabRect( sal_uInt16 nPos, long nWidth = -1, long nHeight = -1 );
    SAL_DLLPRIVATE void         ImplChangeTabPage( sal_uInt16 nId, sal_uInt16 nOldId );
    SAL_DLLPRIVATE bool         ImplPosCurTabPage();
    SAL_DLLPRIVATE void         ImplActivateTabPage( bool bNext );
    SAL_DLLPRIVATE void         ImplShowFocus();
    SAL_DLLPRIVATE void         ImplDrawItem(vcl::RenderContext& rRenderContext, ImplTabItem* pItem,
                                             const Rectangle& rCurRect, bool bFirstInGroup = false,
                                             bool bLastInGroup = false, bool bIsCurrentItem = false);
    SAL_DLLPRIVATE void         ImplFreeLayoutData();
    SAL_DLLPRIVATE bool         ImplHandleKeyEvent( const KeyEvent& rKeyEvent );

    DECL_DLLPRIVATE_LINK_TYPED( ImplListBoxSelectHdl, ListBox&, void );
    DECL_DLLPRIVATE_LINK_TYPED( ImplWindowEventListener, VclWindowEvent&, void );

    using Window::ImplInit;
    SAL_DLLPRIVATE void         ImplInit( vcl::Window* pParent, WinBits nStyle );

    virtual void                FillLayoutData() const override;
    virtual const vcl::Font&    GetCanonicalFont( const StyleSettings& _rStyle ) const override;
    virtual const Color&        GetCanonicalTextColor( const StyleSettings& _rStyle ) const override;
    SAL_DLLPRIVATE Rectangle*   ImplFindPartRect( const Point& rPt );
    virtual bool                ImplPlaceTabs( long nWidth );
    virtual void                ImplPaint(vcl::RenderContext& rRenderContext, const Rectangle& rRect);

public:
                        TabControl( vcl::Window* pParent,
                                    WinBits nStyle = WB_STDTABCONTROL );
                        virtual ~TabControl();
    virtual void        dispose() override;

    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        KeyInput( const KeyEvent& rKEvt ) override;
    virtual void        Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void        Resize() override;
    virtual void        GetFocus() override;
    virtual void        LoseFocus() override;
    virtual void        RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void        Command( const CommandEvent& rCEvt ) override;
    virtual bool        Notify( NotifyEvent& rNEvt ) override;
    virtual void        StateChanged( StateChangedType nType ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual bool        PreNotify( NotifyEvent& rNEvt ) override;

    virtual void        ActivatePage();
    virtual bool        DeactivatePage();

    virtual Size GetOptimalSize() const override;

    void                SetTabPageSizePixel( const Size& rSize );
    Size                GetTabPageSizePixel() const;

    //  pixel offset for the tab items, default is (0,0)
    void                SetItemsOffset( const Point& rOffs );
    Point               GetItemsOffset() const;

    void                InsertPage( sal_uInt16 nPageId, const OUString& rText,
                                    sal_uInt16 nPos = TAB_APPEND );
    void                RemovePage( sal_uInt16 nPageId );
    void                Clear();
    void                EnablePage( sal_uInt16 nPageId, bool bEnable = true );

    sal_uInt16          GetPagePos( sal_uInt16 nPageId ) const;
    sal_uInt16          GetPageCount() const;
    sal_uInt16          GetPageId( sal_uInt16 nPos ) const;
    virtual sal_uInt16  GetPageId( const Point& rPos ) const;
    sal_uInt16          GetPageId( const TabPage& rPage ) const;
    sal_uInt16          GetPageId( const OString& rName ) const;

    void                SetCurPageId( sal_uInt16 nPageId );
    sal_uInt16          GetCurPageId() const;

    void                SelectTabPage( sal_uInt16 nPageId );

    void                SetTabPage( sal_uInt16 nPageId, TabPage* pPage );
    TabPage*            GetTabPage( sal_uInt16 nPageId ) const;

    void                SetPageText( sal_uInt16 nPageId, const OUString& rText );
    OUString const &    GetPageText( sal_uInt16 nPageId ) const;

    void                SetHelpText( sal_uInt16 nPageId, const OUString& rText );
    const OUString&     GetHelpText( sal_uInt16 nPageId ) const;

    void                SetHelpId( sal_uInt16 nPageId, const OString& rId ) const;
    OString             GetHelpId( sal_uInt16 nPageId ) const;

    void                SetPageName( sal_uInt16 nPageId, const OString& rName ) const;
    OString             GetPageName( sal_uInt16 nPageId ) const;

    void                SetPageImage( sal_uInt16 nPageId, const Image& rImage );

    void                SetHelpId( const OString& rId )
                            { Control::SetHelpId( rId ); }
    const OString&      GetHelpId() const
                            { return Control::GetHelpId(); }

    void                SetActivatePageHdl( const Link<TabControl*,void>& rLink ) { maActivateHdl = rLink; }
    void                SetDeactivatePageHdl( const Link<TabControl*, bool>& rLink ) { maDeactivateHdl = rLink; }

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

    virtual void SetPosPixel(const Point& rPos) override;
    virtual void SetSizePixel(const Size& rNewSize) override;
    virtual void SetPosSizePixel(const Point& rNewPos, const Size& rNewSize) override;

    Size calculateRequisition() const;
    void setAllocation(const Size &rAllocation);

    void markLayoutDirty()
    {
        mbLayoutDirty = true;
    }

    std::vector<sal_uInt16> GetPageIDs() const;

    virtual FactoryFunction GetUITestFactory() const override;

    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout) override;
};

class VCL_DLLPUBLIC NotebookbarTabControl : public TabControl
{
public:
    NotebookbarTabControl( vcl::Window* pParent, WinBits nStyle = WB_STDTABCONTROL );

    void SetContext( vcl::EnumContext::Context eContext );

    virtual sal_uInt16  GetPageId( const Point& rPos ) const override;

protected:
    virtual bool ImplPlaceTabs( long nWidth ) override;
    virtual void ImplPaint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;

private:
    vcl::EnumContext::Context eLastContext;
};

#endif // INCLUDED_VCL_TABCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
