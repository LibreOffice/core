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
#include <vcl/InterimItemWindow.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

class Menu;
class PopupMenu;
class SwView;

using namespace ::com::sun::star;

// double entry! hrc and hxx
// these Ids say what the buttons below the scrollbar are doing
#define NID_START   20000
#define NID_TBL     20000
#define NID_FRM     20001
#define NID_PGE     20002
#define NID_DRW     20003
#define NID_CTRL    20004
#define NID_REG     20005
#define NID_BKM     20006
#define NID_GRF     20007
#define NID_OLE     20008
#define NID_OUTL    20009
#define NID_SEL     20010
#define NID_FTN     20011
#define NID_MARK    20012
#define NID_POSTIT  20013
#define NID_SRCH_REP 20014
#define NID_INDEX_ENTRY  20015
#define NID_TABLE_FORMULA   20016
#define NID_TABLE_FORMULA_ERROR     20017
#define NID_RECENCY 20018
#define NID_FIELD   20019
#define NID_FIELD_BYTYPE    20020
#define NID_COUNT  21

class SwTbxAutoTextCtrl final : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SwTbxAutoTextCtrl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx );
    virtual ~SwTbxAutoTextCtrl() override;

    virtual void CreatePopupWindow() override;
    virtual void                StateChangedAtToolBoxControl( sal_uInt16 nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState ) override;

    DECL_STATIC_LINK(SwTbxAutoTextCtrl, PopupHdl, Menu*, bool);
};

class SwPreviewZoomControl final : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SwPreviewZoomControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx );
    virtual ~SwPreviewZoomControl() override;

    virtual void            StateChangedAtToolBoxControl( sal_uInt16 nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState ) override;

    virtual VclPtr<InterimItemWindow> CreateItemWindow( vcl::Window *pParent ) override;
};

class SwJumpToSpecificPageControl final : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SwJumpToSpecificPageControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx );
    virtual ~SwJumpToSpecificPageControl() override;

    virtual VclPtr<InterimItemWindow> CreateItemWindow( vcl::Window *pParent ) override;
};

class NavElementToolBoxControl;

class NavElementBox_Base
{
public:
    NavElementBox_Base(std::unique_ptr<weld::ComboBox> xComboBox,
                       uno::Reference<frame::XFrame> xFrame);

    virtual ~NavElementBox_Base() {}

    void set_sensitive(bool bSensitive) {m_xComboBox->set_sensitive(bSensitive);}

    weld::ComboBox* GetComboBox() {return m_xComboBox.get();}

    void UpdateBox();


protected:
    std::unique_ptr<weld::ComboBox> m_xComboBox;
    uno::Reference< frame::XFrame > m_xFrame;

    virtual bool DoKeyInput(const KeyEvent& /*rKEvt*/);

    DECL_STATIC_LINK(NavElementBox_Base, SelectHdl, weld::ComboBox&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

    void ReleaseFocus_Impl();
};

class NavElementBox_Impl final : public InterimItemWindow, public NavElementBox_Base
{
public:
    NavElementBox_Impl(vcl::Window* pParent,
                       const uno::Reference<frame::XFrame>& xFrame);

    virtual void dispose() override
    {
        m_xComboBox.reset();
        InterimItemWindow::dispose();
    }

    virtual void GetFocus() override
    {
        if (m_xComboBox)
            m_xComboBox->grab_focus();
        InterimItemWindow::GetFocus();
    }

    virtual bool DoKeyInput(const KeyEvent& rKEvt) override;

    virtual ~NavElementBox_Impl() override
    {
        disposeOnce();
    }
};

typedef cppu::ImplInheritanceHelper< ::svt::ToolboxController, lang::XServiceInfo> NavElementToolBoxControl_Base;
class NavElementToolBoxControl final : public NavElementToolBoxControl_Base
{
public:
    explicit NavElementToolBoxControl(
        const css::uno::Reference< css::uno::XComponentContext >& rServiceManager );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 KeyModifier ) override;
    virtual void SAL_CALL click() override;
    virtual void SAL_CALL doubleClick() override;
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() override;
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) override;

    weld::ComboBox* GetComboBox() {return m_pBox->GetComboBox();}

private:
    VclPtr<NavElementBox_Impl> m_xVclBox;
    std::unique_ptr<NavElementBox_Base> m_xWeldBox;
    NavElementBox_Base* m_pBox;
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
