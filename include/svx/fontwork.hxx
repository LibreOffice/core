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

#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/idle.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <svx/svxdllapi.h>

class SvxColorListBox;

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

class SvxFontWorkControllerItem : public SfxControllerItem
{
    SvxFontWorkDialog  &rFontWorkDlg;

protected:
    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) override;

public:
    SvxFontWorkControllerItem(sal_uInt16 nId, SvxFontWorkDialog&, SfxBindings&);
};

/** Derived from SfxChildWindow as "container" for fontwork dialog
 */

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxFontWorkChildWindow : public SfxChildWindow
{
 public:
    SvxFontWorkChildWindow(vcl::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo*);
    SFX_DECL_CHILDWINDOW_WITHID(SvxFontWorkChildWindow);
};

/** Floating window for setting attributes of text effects
  */

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxFontWorkDialog : public SfxDockingWindow
{
#define CONTROLLER_COUNT 11

    SvxFontWorkControllerItem* pCtrlItems[CONTROLLER_COUNT];

    VclPtr<ToolBox>         m_pTbxStyle;
    VclPtr<ToolBox>         m_pTbxAdjust;

    VclPtr<MetricField>     m_pMtrFldDistance;
    VclPtr<MetricField>     m_pMtrFldTextStart;

    VclPtr<ToolBox>         m_pTbxShadow;

    VclPtr<FixedImage>      m_pFbShadowX;
    VclPtr<MetricField>     m_pMtrFldShadowX;
    VclPtr<FixedImage>      m_pFbShadowY;
    VclPtr<MetricField>     m_pMtrFldShadowY;

    VclPtr<SvxColorListBox> m_pShadowColorLB;

    SfxBindings&    rBindings;
    Idle            aInputIdle;

    sal_uInt16      nLastStyleTbxId;
    sal_uInt16      nStyleOffId;
    sal_uInt16      nStyleRotateId;
    sal_uInt16      nStyleUprightId;
    sal_uInt16      nStyleSlantXId;
    sal_uInt16      nStyleSlantYId;

    sal_uInt16      nLastAdjustTbxId;
    sal_uInt16      nAdjustMirrorId;
    sal_uInt16      nAdjustLeftId;
    sal_uInt16      nAdjustCenterId;
    sal_uInt16      nAdjustRightId;
    sal_uInt16      nAdjustAutoSizeId;

    sal_uInt16      nLastShadowTbxId;
    sal_uInt16      nShowFormId;
    sal_uInt16      nOutlineId;
    sal_uInt16      nShadowOffId;
    sal_uInt16      nShadowNormalId;
    sal_uInt16      nShadowSlantId;

    long            nSaveShadowX;
    long            nSaveShadowY;
    long            nSaveShadowAngle;
    long            nSaveShadowSize;

 friend class SvxFontWorkChildWindow;
 friend class SvxFontWorkControllerItem;

    DECL_LINK( SelectStyleHdl_Impl, ToolBox *, void );
    DECL_LINK( SelectAdjustHdl_Impl, ToolBox *, void );
    DECL_LINK( SelectShadowHdl_Impl, ToolBox *, void );

    DECL_LINK( ModifyInputHdl_Impl, Edit&, void );
    DECL_LINK( InputTimoutHdl_Impl, Timer *, void );

    DECL_LINK( ColorSelectHdl_Impl, SvxColorListBox&, void );

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

 protected:
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
