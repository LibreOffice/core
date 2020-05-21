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

#ifndef INCLUDED_SVX_FILLCTRL_HXX
#define INCLUDED_SVX_FILLCTRL_HXX

#include <memory>
#include <vcl/InterimItemWindow.hxx>
#include <sfx2/tbxctrl.hxx>
#include <sfx2/weldutils.hxx>
#include <svx/svxdllapi.h>
#include <com/sun/star/drawing/FillStyle.hpp>

class XFillStyleItem;
class XFillColorItem;
class XFillGradientItem;
class XFillHatchItem;
class XFillBitmapItem;
class FillControl;

/*************************************************************************
|*
|* Class for surface attributes (controls and controller)
|*
\************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxFillToolBoxControl final : public SfxToolBoxControl
{
private:
    std::unique_ptr< XFillStyleItem >    mpStyleItem;
    std::unique_ptr< XFillColorItem >    mpColorItem;
    std::unique_ptr< XFillGradientItem > mpFillGradientItem;
    std::unique_ptr< XFillHatchItem >    mpHatchItem;
    std::unique_ptr< XFillBitmapItem >   mpBitmapItem;

    VclPtr<FillControl> mxFillControl;
    weld::ComboBox* mpLbFillType;
    weld::Toolbar* mpToolBoxColor;
    weld::ComboBox* mpLbFillAttr;

    css::drawing::FillStyle    meLastXFS;
    sal_Int32           mnLastPosGradient;
    sal_Int32           mnLastPosHatch;
    sal_Int32           mnLastPosBitmap;

    DECL_LINK(SelectFillTypeHdl, weld::ComboBox&, void);
    DECL_LINK(SelectFillAttrHdl, weld::ComboBox&, void);

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFillToolBoxControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx);
    virtual ~SvxFillToolBoxControl() override;

    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState) override;
    void Update();
    virtual VclPtr<InterimItemWindow> CreateItemWindow(vcl::Window* pParent) override;
};

class SAL_WARN_UNUSED FillControl final : public InterimItemWindow
{
private:
    friend class SvxFillToolBoxControl;

    std::unique_ptr<weld::ComboBox> mxLbFillType;
    std::unique_ptr<weld::Toolbar> mxToolBoxColor;
    std::unique_ptr<ToolbarUnoDispatcher> mxColorDispatch;
    std::unique_ptr<weld::ComboBox> mxLbFillAttr;
    int mnTypeCurPos;
    int mnAttrCurPos;

    DECL_LINK(AttrKeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(TypeKeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(ColorKeyInputHdl, const KeyEvent&, bool);
    DECL_STATIC_LINK(FillControl, DumpAsPropertyTreeHdl, boost::property_tree::ptree&, void);
    DECL_LINK(AttrFocusHdl, weld::Widget&, void);
    DECL_LINK(TypeFocusHdl, weld::Widget&, void);

    void SetOptimalSize();

    virtual void DataChanged(const DataChangedEvent& rDCEvt) override;

    static void ReleaseFocus_Impl();

public:
    FillControl(vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rFrame);
    virtual void dispose() override;
    virtual ~FillControl() override;

    virtual void GetFocus() override;
};

#endif // INCLUDED_SVX_FILLCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
