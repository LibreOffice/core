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
#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGESTYLESPANEL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGESTYLESPANEL_HXX

#include <memory>
#include <com/sun/star/frame/XFrame.hpp>

#include <sfx2/sidebar/PanelLayout.hxx>

#include <sfx2/sidebar/ControllerItem.hxx>

#include <svx/pageitem.hxx>

#include <svl/intitem.hxx>
#include <svl/poolitem.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/pagenumberlistbox.hxx>

class List;
class ColorListBox;
namespace sw::sidebar {

class PageStylesPanel:
    public PanelLayout,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static VclPtr<PanelLayout> Create(
        vcl::Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
        SfxBindings* pBindings);

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState) override;

    virtual void GetControlState(
        const sal_uInt16 /*nSId*/,
        boost::property_tree::ptree& /*rState*/) override {};

    SfxBindings* GetBindings() const { return mpBindings; }
    PageStylesPanel(
        vcl::Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
        SfxBindings* pBindings);
    virtual ~PageStylesPanel() override;
    virtual void dispose() override;

private:

    SfxBindings* mpBindings;

    ::std::unique_ptr<SfxInt16Item>      mpPageColumnItem;
    ::std::unique_ptr<SvxPageItem>       mpPageItem;
    ::std::unique_ptr<XFillColorItem>    mpBgColorItem;
    ::std::unique_ptr<XFillGradientItem> mpBgGradientItem;
    ::std::unique_ptr<XFillHatchItem>    mpBgHatchItem;
    ::std::unique_ptr<XFillBitmapItem>   mpBgBitmapItem;

    ::sfx2::sidebar::ControllerItem maPageColumnControl;
    ::sfx2::sidebar::ControllerItem maPageNumFormatControl;
    ::sfx2::sidebar::ControllerItem maBgColorControl;
    ::sfx2::sidebar::ControllerItem maBgHatchingControl;
    ::sfx2::sidebar::ControllerItem maBgGradientControl;
    ::sfx2::sidebar::ControllerItem maBgBitmapControl;
    ::sfx2::sidebar::ControllerItem maBgFillStyleControl;

    std::unique_ptr<ColorListBox> mxBgColorLB;
    std::unique_ptr<weld::ComboBox> mxBgHatchingLB;
    std::unique_ptr<ColorListBox> mxBgGradientLB;
    std::unique_ptr<weld::ComboBox> mxBgBitmapLB;
    std::unique_ptr<weld::ComboBox> mxLayoutSelectLB;
    std::unique_ptr<weld::ComboBox> mxColumnCount;
    std::unique_ptr<SvxPageNumberListBox> mxNumberSelectLB;
    std::unique_ptr<weld::ComboBox> mxBgFillType;
    std::unique_ptr<weld::Label> mxCustomEntry;
    OUString aCustomEntry;

    void Initialize();
    void Update();
    Color const & GetColorSetOrDefault();
    XGradient const & GetGradientSetOrDefault();
    OUString const & GetHatchingSetOrDefault();
    OUString const & GetBitmapSetOrDefault();
    OUString const & GetPatternSetOrDefault();

    void ModifyFillColor();

    DECL_LINK( ModifyColumnCountHdl, weld::ComboBox&, void );
    DECL_LINK( ModifyNumberingHdl, weld::ComboBox&, void );
    DECL_LINK( ModifyLayoutHdl, weld::ComboBox&, void );
    DECL_LINK( ModifyFillStyleHdl, weld::ComboBox&, void );
    DECL_LINK( ModifyFillColorHdl, weld::ComboBox&, void );
    DECL_LINK( ModifyFillColorListHdl, ColorListBox&, void );
};

} //end of namespace sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
