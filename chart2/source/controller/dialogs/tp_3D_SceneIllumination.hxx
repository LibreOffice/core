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

#include <TimerTriggeredControllerLock.hxx>
#include <vcl/weld.hxx>
#include <svx/dlgctl3d.hxx>

namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }

class ColorListBox;

namespace chart
{

class LightButton
{
public:
    explicit LightButton(std::unique_ptr<weld::ToggleButton> xButton);

    void switchLightOn(bool bOn);
    bool isLightOn() const { return m_bLightOn;}

    bool get_active() const { return m_xButton->get_active(); }
    void set_active(bool bActive) { m_xButton->set_active(bActive); }

    weld::ToggleButton* get_widget() const { return m_xButton.get(); }

    void connect_clicked(const Link<weld::Button&, void>& rLink)
    {
        m_xButton->connect_clicked(rLink);
    }

private:
    std::unique_ptr<weld::ToggleButton> m_xButton;
    bool m_bLightOn;
};

struct LightSourceInfo;

class ThreeD_SceneIllumination_TabPage
{
public:
    ThreeD_SceneIllumination_TabPage(
        weld::Container* pParent, weld::Window* pTopLevel,
        const css::uno::Reference< css::beans::XPropertySet > & xSceneProperties,
        const css::uno::Reference< css::frame::XModel >& xChartModel );
    ~ThreeD_SceneIllumination_TabPage();

private:
    DECL_LINK( ClickLightSourceButtonHdl, weld::Button&, void );
    DECL_LINK( SelectColorHdl, ColorListBox&, void );
    DECL_LINK( ColorDialogHdl, weld::Button&, void );
    DECL_LINK( PreviewChangeHdl, LightCtl3D*, void );
    DECL_LINK( PreviewSelectHdl, LightCtl3D*, void );

    void updatePreview();

private:
    DECL_LINK(fillControlsFromModel, void *, void);

    void applyLightSourceToModel( sal_uInt32 nLightNumber );
    void applyLightSourcesToModel();

    std::unique_ptr<LightSourceInfo[]> m_pLightSourceInfoList;

    css::uno::Reference< css::beans::XPropertySet > m_xSceneProperties;

    TimerTriggeredControllerLock    m_aTimerTriggeredControllerLock;

    bool            m_bInCommitToModel;

    css::uno::Reference<css::frame::XModel> m_xChartModel;

    weld::Window* m_pTopLevel;
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<LightButton> m_xBtn_Light1;
    std::unique_ptr<LightButton> m_xBtn_Light2;
    std::unique_ptr<LightButton> m_xBtn_Light3;
    std::unique_ptr<LightButton> m_xBtn_Light4;
    std::unique_ptr<LightButton> m_xBtn_Light5;
    std::unique_ptr<LightButton> m_xBtn_Light6;
    std::unique_ptr<LightButton> m_xBtn_Light7;
    std::unique_ptr<LightButton> m_xBtn_Light8;
    std::unique_ptr<ColorListBox> m_xLB_LightSource;
    std::unique_ptr<weld::Button> m_xBtn_LightSource_Color;
    std::unique_ptr<ColorListBox> m_xLB_AmbientLight;
    std::unique_ptr<weld::Button> m_xBtn_AmbientLight_Color;
    std::unique_ptr<weld::Scale> m_xHoriScale;
    std::unique_ptr<weld::Scale> m_xVertScale;
    std::unique_ptr<weld::Button> m_xBtn_Corner;
    std::unique_ptr<LightControl3D> m_xPreview;
    std::unique_ptr<weld::CustomWeld> m_xPreviewWnd;
    std::unique_ptr<LightCtl3D> m_xCtl_Preview;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
