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

#include <config_options.h>
#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/EnumContext.hxx>
#include <vcl/NotebookbarContextControl.hxx>
#include <boost/property_tree/json_parser.hpp>

class ImplTabItem;
struct ImplTabCtrlData;
class TabPage;
class Button;
class PushButton;
class ListBox;

#ifndef TAB_APPEND
#define TAB_APPEND          (sal_uInt16(0xFFFF))
#define TAB_PAGE_NOTFOUND   (sal_uInt16(0xFFFF))
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
    std::unique_ptr<ImplTabCtrlData> mpTabCtrlData;
    long                mnLastWidth;
    long                mnLastHeight;
    sal_uInt16          mnActPageId;
    sal_uInt16          mnCurPageId;
    bool                mbFormat;
    bool                mbRestoreHelpId;
    bool                mbSmallInvalidate;
    bool                mbLayoutDirty;
    Link<TabControl*,void> maActivateHdl;
    Link<TabControl*,bool> maDeactivateHdl;

    using Control::ImplInitSettings;
    SAL_DLLPRIVATE void         ImplInitSettings( bool bBackground );
    SAL_DLLPRIVATE ImplTabItem* ImplGetItem( sal_uInt16 nId ) const;
    SAL_DLLPRIVATE ImplTabItem* ImplGetItem(const Point& rPt) const;
    SAL_DLLPRIVATE Size         ImplGetItemSize( ImplTabItem* pItem, long nMaxWidth );
    SAL_DLLPRIVATE tools::Rectangle    ImplGetTabRect( sal_uInt16 nPos, long nWidth = -1, long nHeight = -1 );
    SAL_DLLPRIVATE tools::Rectangle ImplGetTabRect(const ImplTabItem*, long nWidth = -1, long nHeight = -1);
    SAL_DLLPRIVATE void         ImplChangeTabPage( sal_uInt16 nId, sal_uInt16 nOldId );
    SAL_DLLPRIVATE bool         ImplPosCurTabPage();
    virtual void                ImplActivateTabPage( bool bNext );
    SAL_DLLPRIVATE void         ImplShowFocus();
    SAL_DLLPRIVATE void         ImplDrawItem(vcl::RenderContext& rRenderContext, ImplTabItem const * pItem,
                                             const tools::Rectangle& rCurRect, bool bFirstInGroup,
                                             bool bLastInGroup);
    SAL_DLLPRIVATE void         ImplFreeLayoutData();
    SAL_DLLPRIVATE bool         ImplHandleKeyEvent( const KeyEvent& rKeyEvent );

    DECL_DLLPRIVATE_LINK( ImplListBoxSelectHdl, ListBox&, void );
    DECL_DLLPRIVATE_LINK( ImplWindowEventListener, VclWindowEvent&, void );

    using Window::ImplInit;
    SAL_DLLPRIVATE void         ImplInit( vcl::Window* pParent, WinBits nStyle );

    virtual void                FillLayoutData() const override;
    virtual const vcl::Font&    GetCanonicalFont( const StyleSettings& _rStyle ) const override;
    virtual const Color&        GetCanonicalTextColor( const StyleSettings& _rStyle ) const override;
    virtual bool                ImplPlaceTabs( long nWidth );
    SAL_DLLPRIVATE Size ImplCalculateRequisition(sal_uInt16& nHeaderHeight) const;

public:
                        TabControl( vcl::Window* pParent,
                                    WinBits nStyle = WB_STDTABCONTROL );
                        virtual ~TabControl() override;
    virtual void        dispose() override;

    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        KeyInput( const KeyEvent& rKEvt ) override;
    virtual void        Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void        Resize() override;
    virtual void        GetFocus() override;
    virtual void        LoseFocus() override;
    virtual void        RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void        Command( const CommandEvent& rCEvt ) override;
    virtual bool        EventNotify( NotifyEvent& rNEvt ) override;
    virtual void        StateChanged( StateChangedType nType ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual bool        PreNotify( NotifyEvent& rNEvt ) override;

    virtual void        ActivatePage();
    bool                DeactivatePage();

    virtual Size GetOptimalSize() const override;

    void                SetTabPageSizePixel( const Size& rSize );

    //  pixel offset for the tab items, default is (0,0)
    void                SetItemsOffset( const Point& rOffs );
    Point               GetItemsOffset() const;

    void                InsertPage( sal_uInt16 nPageId, const OUString& rText,
                                    sal_uInt16 nPos = TAB_APPEND );
    void                RemovePage( sal_uInt16 nPageId );

    void SetPageEnabled(sal_uInt16 nPageId, bool bEnable = true);
    void SetPageVisible(sal_uInt16 nPageId, bool bVisible = true);

    sal_uInt16          GetPagePos( sal_uInt16 nPageId ) const;
    sal_uInt16          GetPageCount() const;
    sal_uInt16          GetPageId( sal_uInt16 nPos ) const;
    sal_uInt16 GetPageId(const Point& rPos) const;
    sal_uInt16          GetPageId( const OString& rName ) const;

    void SetCurPageId(sal_uInt16 nPageId);
    sal_uInt16          GetCurPageId() const;

    void                SelectTabPage( sal_uInt16 nPageId );

    void SetTabPage(sal_uInt16 nPageId, TabPage* pPage);
    TabPage*            GetTabPage( sal_uInt16 nPageId ) const;

    void                SetPageText( sal_uInt16 nPageId, const OUString& rText );
    OUString const &    GetPageText( sal_uInt16 nPageId ) const;

    void                SetHelpText( sal_uInt16 nPageId, const OUString& rText );
    const OUString&     GetHelpText( sal_uInt16 nPageId ) const;

    void                SetPageName( sal_uInt16 nPageId, const OString& rName ) const;
    OString             GetPageName( sal_uInt16 nPageId ) const;

    void                SetPageImage( sal_uInt16 nPageId, const Image& rImage );

    using Control::SetHelpId;
    using Control::GetHelpId;

    void                SetActivatePageHdl( const Link<TabControl*,void>& rLink ) { maActivateHdl = rLink; }
    void                SetDeactivatePageHdl( const Link<TabControl*, bool>& rLink ) { maDeactivateHdl = rLink; }

    // returns (control relative) bounding rectangle for the
    // character at index nIndex relative to the text of page nPageId
    using Control::GetCharacterBounds;
    tools::Rectangle GetCharacterBounds( sal_uInt16 nPageId, long nIndex ) const;

    // returns the index relative to the text of page nPageId (also returned)
    // at position rPoint (control relative)
    using Control::GetIndexForPoint;
    long GetIndexForPoint( const Point& rPoint, sal_uInt16& rPageId ) const;

    // returns the rectangle of the tab for page nPageId
    tools::Rectangle GetTabBounds( sal_uInt16 nPageId ) const;

    virtual void SetPosPixel(const Point& rPos) override;
    virtual void SetSizePixel(const Size& rNewSize) override;
    virtual void SetPosSizePixel(const Point& rNewPos, const Size& rNewSize) override;

    virtual Size calculateRequisition() const;
    void setAllocation(const Size &rAllocation);

    std::vector<sal_uInt16> GetPageIDs() const;

    virtual FactoryFunction GetUITestFactory() const override;

    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout) override;
};

class NotebookBar;

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) NotebookbarTabControlBase : public TabControl,
                                            public NotebookbarContextControl
{
public:
    NotebookbarTabControlBase( vcl::Window* pParent );
    ~NotebookbarTabControlBase() override;
    void dispose() override;

    void SetContext( vcl::EnumContext::Context eContext ) override;
    void SetIconClickHdl( Link<NotebookBar*, void> aHdl );
    void SetToolBox( ToolBox* pToolBox );
    ToolBox* GetToolBox() { return m_pShortcuts; }
    PushButton* GetOpenMenu() { return m_pOpenMenu; }

    virtual Size        calculateRequisition() const override;
    static sal_uInt16   GetHeaderHeight();

protected:
    virtual bool ImplPlaceTabs( long nWidth ) override;
    virtual void ImplActivateTabPage( bool bNext ) override;

private:
    bool bLastContextWasSupported;
    vcl::EnumContext::Context eLastContext;
    Link<NotebookBar*,void> m_aIconClickHdl;
    static sal_uInt16 m_nHeaderHeight;
    VclPtr<ToolBox> m_pShortcuts;
    VclPtr<PushButton> m_pOpenMenu;
    DECL_LINK(OpenMenu, Button*, void);
};

#endif // INCLUDED_VCL_TABCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
