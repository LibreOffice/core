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

#include "TimerTriggeredControllerLock.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>

#include <vcl/tabpage.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/dlgctl3d.hxx>

class SvxColorListBox;

namespace chart
{

class LightButton : public ImageButton
{
public:
    explicit LightButton( vcl::Window* pParent);

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
        const css::uno::Reference< css::beans::XPropertySet > & xSceneProperties,
        const css::uno::Reference< css::frame::XModel >& xChartModel );
    virtual ~ThreeD_SceneIllumination_TabPage() override;
    virtual void dispose() override;

private:
    DECL_LINK( ClickLightSourceButtonHdl, Button*, void );
    DECL_LINK( SelectColorHdl, SvxColorListBox&, void );
    DECL_LINK( ColorDialogHdl, Button*, void );
    DECL_LINK( PreviewChangeHdl, SvxLightCtl3D*, void );
    DECL_LINK( PreviewSelectHdl, SvxLightCtl3D*, void );

    void updatePreview();

private:
    DECL_LINK(fillControlsFromModel, void *, void);

    void applyLightSourceToModel( sal_uInt32 nLightNumber );
    void applyLightSourcesToModel();

    VclPtr<LightButton> m_pBtn_Light1;
    VclPtr<LightButton> m_pBtn_Light2;
    VclPtr<LightButton> m_pBtn_Light3;
    VclPtr<LightButton> m_pBtn_Light4;
    VclPtr<LightButton> m_pBtn_Light5;
    VclPtr<LightButton> m_pBtn_Light6;
    VclPtr<LightButton> m_pBtn_Light7;
    VclPtr<LightButton> m_pBtn_Light8;

    VclPtr<SvxColorListBox> m_pLB_LightSource;
    VclPtr<PushButton>  m_pBtn_LightSource_Color;

    VclPtr<SvxColorListBox> m_pLB_AmbientLight;
    VclPtr<PushButton>  m_pBtn_AmbientLight_Color;

    VclPtr<SvxLightCtl3D>   m_pCtl_Preview;

    LightSourceInfo* m_pLightSourceInfoList;

    css::uno::Reference< css::beans::XPropertySet > m_xSceneProperties;

    TimerTriggeredControllerLock    m_aTimerTriggeredControllerLock;

    bool            m_bInCommitToModel;

    css::uno::Reference< css::frame::XModel >       m_xChartModel;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
