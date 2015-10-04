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
#include <vcl/idle.hxx>
#include <svtools/valueset.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <svx/xenum.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/svxdllapi.h>
#include <svx/xtextit0.hxx>

// forward ---------------------------------------------------------------

class SdrView;
class SdrPageView;
class SdrObject;

class XFormTextAdjustItem;
class XFormTextDistanceItem;
class XFormTextStartItem;
class XFormTextMirrorItem;
class XFormTextHideFormItem;
class XFormTextOutlineItem;
class XFormTextShadowItem;
class XFormTextShadowColorItem;
class XFormTextShadowXValItem;
class XFormTextShadowYValItem;

/*************************************************************************
|*
|* ControllerItem for Fontwork
|*
\************************************************************************/

class SvxFontWorkDialog;

class SvxFontWorkControllerItem : public SfxControllerItem
{
    SvxFontWorkDialog  &rFontWorkDlg;

protected:
    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) SAL_OVERRIDE;

public:
    SvxFontWorkControllerItem(sal_uInt16 nId, SvxFontWorkDialog&, SfxBindings&);
};

/*************************************************************************
|*
|* Derived from SfxChildWindow as "container" for fontwork dialog
|*
\************************************************************************/

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxFontWorkChildWindow : public SfxChildWindow
{
 public:
    SvxFontWorkChildWindow(vcl::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo*);
    SFX_DECL_CHILDWINDOW_WITHID(SvxFontWorkChildWindow);
};

/*************************************************************************
|*
|* Floating window for setting attributes of text effects
|*
\************************************************************************/

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxFontWorkDialog : public SfxDockingWindow
{
#define CONTROLLER_COUNT 11

    SvxFontWorkControllerItem* pCtrlItems[CONTROLLER_COUNT];

    VclPtr<ToolBox>         m_pTbxStyle;
    VclPtr<ToolBox>         m_pTbxAdjust;

    VclPtr<FixedImage>      m_pFbDistance;
    VclPtr<MetricField>     m_pMtrFldDistance;
    VclPtr<FixedImage>      m_pFbTextStart;
    VclPtr<MetricField>     m_pMtrFldTextStart;

    VclPtr<ToolBox>         m_pTbxShadow;

    VclPtr<FixedImage>      m_pFbShadowX;
    VclPtr<MetricField>     m_pMtrFldShadowX;
    VclPtr<FixedImage>      m_pFbShadowY;
    VclPtr<MetricField>     m_pMtrFldShadowY;

    VclPtr<ColorLB>         m_pShadowColorLB;

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

    XColorListRef   pColorList;

 friend class SvxFontWorkChildWindow;
 friend class SvxFontWorkControllerItem;

    DECL_LINK_TYPED( SelectStyleHdl_Impl, ToolBox *, void );
    DECL_LINK_TYPED( SelectAdjustHdl_Impl, ToolBox *, void );
    DECL_LINK_TYPED( SelectShadowHdl_Impl, ToolBox *, void );

    DECL_LINK( ModifyInputHdl_Impl, void * );
    DECL_LINK_TYPED( InputTimoutHdl_Impl, Idle *, void );

    DECL_LINK_TYPED( ColorSelectHdl_Impl, ListBox&, void );

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
                                              SfxChildAlignment eAlign ) SAL_OVERRIDE;

 public:
    SvxFontWorkDialog(SfxBindings *pBinding, SfxChildWindow *pCW,
                      vcl::Window* pParent);
    virtual ~SvxFontWorkDialog();
    virtual void dispose() SAL_OVERRIDE;

    void SetColorList(const XColorListRef &pTable);
};

#endif // INCLUDED_SVX_FONTWORK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
