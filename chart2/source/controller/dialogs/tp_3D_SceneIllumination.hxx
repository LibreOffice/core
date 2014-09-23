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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_3D_SCENEILLUMINATION_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_3D_SCENEILLUMINATION_HXX

#include "ModifyListenerCallBack.hxx"
#include "TimerTriggeredControllerLock.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>

#include <vcl/tabpage.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/dlgctl3d.hxx>

namespace chart
{

class LightButton : public ImageButton
{
public:
    LightButton( vcl::Window* pParent);
    virtual ~LightButton();

    void switchLightOn(bool bOn);
    bool isLightOn() const { return m_bLightOn;}

private:
    bool m_bLightOn;
};

struct LightSourceInfo;

class ThreeD_SceneIllumination_TabPage : public TabPage
{
public:
    ThreeD_SceneIllumination_TabPage(
        vcl::Window* pWindow,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & xSceneProperties,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel,
        const XColorListRef &pColorTable );
    virtual ~ThreeD_SceneIllumination_TabPage();

    // has to be called in case the dialog was closed with OK
    void commitPendingChanges();

private:
    DECL_LINK( ClickLightSourceButtonHdl, LightButton* );
    DECL_LINK( SelectColorHdl, ColorLB* );
    DECL_LINK( ColorDialogHdl, Button* );
    DECL_LINK( PreviewChangeHdl, void* );
    DECL_LINK( PreviewSelectHdl, void* );

    void updatePreview();

private:
    DECL_LINK(fillControlsFromModel, void *);

    void applyLightSourceToModel( sal_uInt32 nLightNumber );
    void applyLightSourcesToModel();

    LightButton* m_pBtn_Light1;
    LightButton* m_pBtn_Light2;
    LightButton* m_pBtn_Light3;
    LightButton* m_pBtn_Light4;
    LightButton* m_pBtn_Light5;
    LightButton* m_pBtn_Light6;
    LightButton* m_pBtn_Light7;
    LightButton* m_pBtn_Light8;

    ColorLB*     m_pLB_LightSource;
    PushButton*  m_pBtn_LightSource_Color;

    ColorLB*     m_pLB_AmbientLight;
    PushButton*  m_pBtn_AmbientLight_Color;

    SvxLightCtl3D*   m_pCtl_Preview;

    LightSourceInfo* m_pLightSourceInfoList;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_xSceneProperties;

    TimerTriggeredControllerLock    m_aTimerTriggeredControllerLock;

    bool            m_bInCommitToModel;

    ModifyListenerCallBack m_aModelChangeListener;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel >       m_xChartModel;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
