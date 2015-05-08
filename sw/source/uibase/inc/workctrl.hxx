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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_WORKCTRL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_WORKCTRL_HXX

#include <sfx2/tbxctrl.hxx>
#include <vcl/toolbox.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/button.hxx>

class PopupMenu;
class SwView;

// double entry! hrc and hxx
// these Ids say what the buttons below the scrollbar are doing
#define NID_START   20000
#define NID_NEXT    20000
#define NID_PREV    20001
#define NID_TBL     20002
#define NID_FRM     20003
#define NID_PGE     20004
#define NID_DRW     20005
#define NID_CTRL    20006
#define NID_REG     20007
#define NID_BKM     20008
#define NID_GRF     20009
#define NID_OLE     20010
#define NID_OUTL    20011
#define NID_SEL     20012
#define NID_FTN     20013
#define NID_MARK    20014
#define NID_POSTIT  20015
#define NID_SRCH_REP 20016
#define NID_INDEX_ENTRY  20017
#define NID_TABLE_FORMULA   20018
#define NID_TABLE_FORMULA_ERROR     20019
#define NID_COUNT  20

class SwTbxInsertCtrl : public SfxToolBoxControl
{
    sal_uInt16                  nLastSlotId;

    virtual void            Select(sal_uInt16 nSelectModifier) SAL_OVERRIDE;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SwTbxInsertCtrl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    virtual ~SwTbxInsertCtrl();

    virtual SfxPopupWindowType  GetPopupWindowType() const SAL_OVERRIDE;
    virtual VclPtr<SfxPopupWindow> CreatePopupWindow() SAL_OVERRIDE;
    virtual void                StateChanged( sal_uInt16 nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState ) SAL_OVERRIDE;

    virtual void SAL_CALL update() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

class SwTbxAutoTextCtrl : public SfxToolBoxControl
{
    PopupMenu*              pPopup;

    void                    DelPopup();
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SwTbxAutoTextCtrl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    virtual ~SwTbxAutoTextCtrl();

    virtual SfxPopupWindowType  GetPopupWindowType() const SAL_OVERRIDE;
    virtual VclPtr<SfxPopupWindow> CreatePopupWindow() SAL_OVERRIDE;
    virtual void                StateChanged( sal_uInt16 nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState ) SAL_OVERRIDE;

    DECL_STATIC_LINK(SwTbxAutoTextCtrl, PopupHdl, PopupMenu*);
};

class SwTbxFieldCtrl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SwTbxFieldCtrl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    virtual ~SwTbxFieldCtrl();

    virtual SfxPopupWindowType  GetPopupWindowType() const SAL_OVERRIDE;
    virtual VclPtr<SfxPopupWindow> CreatePopupWindow() SAL_OVERRIDE;
    virtual void                StateChanged( sal_uInt16 nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState ) SAL_OVERRIDE;

    DECL_LINK(PopupHdl, PopupMenu*);
};

class SwScrollNaviPopup;

class SwScrollNaviToolBox : public ToolBox
{
    VclPtr<SwScrollNaviPopup> m_pNaviPopup;

    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;

public:
    SwScrollNaviToolBox(vcl::Window* pParent, SwScrollNaviPopup* pNaviPopup, WinBits nWinStyle)
        : ToolBox(pParent, nWinStyle)
        , m_pNaviPopup(pNaviPopup)
    {
    }
    virtual ~SwScrollNaviToolBox();
    virtual void dispose() SAL_OVERRIDE;
};

class SwScrollNaviPopup : public SfxPopupWindow
{
    VclPtr<SwScrollNaviToolBox> m_pToolBox;
    VclPtr<FixedText>           m_pInfoField;
    ImageList       aIList;

    OUString        sQuickHelp[2 * NID_COUNT];

    void            ApplyImageList();

    using Window::GetQuickHelpText;

protected:
        DECL_LINK_TYPED(SelectHdl, ToolBox*, void);
        virtual void        DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

public:
        SwScrollNaviPopup( sal_uInt16 nId, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, vcl::Window *pParent );
        virtual ~SwScrollNaviPopup();
    virtual void dispose() SAL_OVERRIDE;

    static OUString         GetQuickHelpText(bool bNext);

    virtual VclPtr<SfxPopupWindow> Clone() const SAL_OVERRIDE;
    void                GrabFocus() { m_pToolBox->GrabFocus(); }
};

//  ImageButtons have to set the HelpText themselves if needed

class SwHlpImageButton : public ImageButton
{
    bool        bUp;
    public:
        SwHlpImageButton(vcl::Window* pParent, const ResId& rResId, bool bUpBtn) :
            ImageButton(pParent, rResId), bUp(bUpBtn){}

    virtual void    RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;

};

class SwPreviewZoomControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SwPreviewZoomControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    virtual ~SwPreviewZoomControl();

    virtual void            StateChanged( sal_uInt16 nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState ) SAL_OVERRIDE;

    virtual VclPtr<vcl::Window> CreateItemWindow( vcl::Window *pParent ) SAL_OVERRIDE;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
