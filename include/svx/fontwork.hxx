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
#ifndef INCLUDED_SVX_FONTWORK_HXX
#define INCLUDED_SVX_FONTWORK_HXX

#include <sfx2/dockwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <svx/svxdllapi.h>
#include <vcl/idle.hxx>
#include <vcl/weld.hxx>

class ColorListBox;

class XFormTextAdjustItem;
class XFormTextDistanceItem;
class XFormTextStartItem;
class XFormTextStyleItem;
class XFormTextMirrorItem;
class XFormTextHideFormItem;
class XFormTextOutlineItem;
class XFormTextShadowItem;
class XFormTextShadowColorItem;
class XFormTextShadowXValItem;
class XFormTextShadowYValItem;

/** ControllerItem for Fontwork
 */


class SvxFontWorkDialog;

class SvxFontWorkControllerItem final : public SfxControllerItem
{
    SvxFontWorkDialog  &rFontWorkDlg;

    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) override;

public:
    SvxFontWorkControllerItem(sal_uInt16 nId, SvxFontWorkDialog&, SfxBindings&);
};

/** Derived from SfxChildWindow as "container" for fontwork dialog
 */

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxFontWorkChildWindow final : public SfxChildWindow
{
 public:
    SvxFontWorkChildWindow(vcl::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo*);
    SFX_DECL_CHILDWINDOW_WITHID(SvxFontWorkChildWindow);
};

/** Floating window for setting attributes of text effects
  */

class SAL_WARN_UNUSED SvxFontWorkDialog final : public SfxDockingWindow
{
#define CONTROLLER_COUNT 11

    SfxBindings&    rBindings;
    Idle            aInputIdle;

    OString         m_sLastStyleTbxId;
    OString         m_sLastAdjustTbxId;
    OString         m_sLastShadowTbxId;

    tools::Long            nSaveShadowX;
    tools::Long            nSaveShadowY;
    tools::Long            nSaveShadowAngle;
    tools::Long            nSaveShadowSize;

    SvxFontWorkControllerItem* pCtrlItems[CONTROLLER_COUNT];

    std::unique_ptr<weld::Toolbar> m_xTbxStyle;
    std::unique_ptr<weld::Toolbar> m_xTbxAdjust;

    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldDistance;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldTextStart;

    std::unique_ptr<weld::Toolbar> m_xTbxShadow;

    std::unique_ptr<weld::Image> m_xFbShadowX;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldShadowX;
    std::unique_ptr<weld::Image> m_xFbShadowY;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldShadowY;

    std::unique_ptr<ColorListBox> m_xShadowColorLB;

    friend class SvxFontWorkChildWindow;
    friend class SvxFontWorkControllerItem;

    DECL_LINK( SelectStyleHdl_Impl, const OString&, void );
    DECL_LINK( SelectAdjustHdl_Impl, const OString&, void );
    DECL_LINK( SelectShadowHdl_Impl, const OString&, void );

    DECL_LINK( ModifyInputHdl_Impl, weld::MetricSpinButton&, void );
    DECL_LINK( InputTimeoutHdl_Impl, Timer*, void );

    DECL_LINK( ColorSelectHdl_Impl, ColorListBox&, void );

    void SetStyle_Impl(const XFormTextStyleItem*);
    void SetAdjust_Impl(const XFormTextAdjustItem*);
    void SetDistance_Impl(const XFormTextDistanceItem*);
    void SetStart_Impl(const XFormTextStartItem*);
    void SetMirror_Impl(const XFormTextMirrorItem*);
    void SetShowForm_Impl(const XFormTextHideFormItem*);
    void SetOutline_Impl(const XFormTextOutlineItem*);
    void SetShadow_Impl(const XFormTextShadowItem*, bool bRestoreValues = false);
    void SetShadowColor_Impl(const XFormTextShadowColorItem*);
    void SetShadowXVal_Impl(const XFormTextShadowXValItem*);
    void SetShadowYVal_Impl(const XFormTextShadowYValItem*);

    void ApplyImageList();

    virtual SfxChildAlignment CheckAlignment( SfxChildAlignment eActAlign,
                                              SfxChildAlignment eAlign ) override;
public:
    SvxFontWorkDialog(SfxBindings *pBinding, SfxChildWindow *pCW,
                      vcl::Window* pParent);
    virtual ~SvxFontWorkDialog() override;
    virtual void dispose() override;
};

#endif // INCLUDED_SVX_FONTWORK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
