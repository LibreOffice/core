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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_3D_SCENEAPPEARANCE_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_3D_SCENEAPPEARANCE_HXX

#include <vcl/tabpage.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include "ControllerLockGuard.hxx"

namespace chart
{

class ThreeD_SceneAppearance_TabPage : public TabPage
{
public:
    ThreeD_SceneAppearance_TabPage(
        vcl::Window* pWindow,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xChartModel,
        ControllerLockHelper & rControllerLockHelper );
    virtual ~ThreeD_SceneAppearance_TabPage();
    virtual void dispose() override;

    virtual void ActivatePage() override;

private:
    DECL_LINK_TYPED( SelectSchemeHdl, ListBox&, void );
    DECL_LINK_TYPED( SelectShading, CheckBox&, void );
    DECL_LINK_TYPED( SelectRoundedEdgeOrObjectLines, CheckBox&, void );

    void initControlsFromModel();
    void applyShadeModeToModel();
    void applyRoundedEdgeAndObjectLinesToModel();
    void updateScheme();

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
        m_xChartModel;

    VclPtr<ListBox>         m_pLB_Scheme;

    VclPtr<CheckBox>        m_pCB_Shading;
    VclPtr<CheckBox>        m_pCB_ObjectLines;
    VclPtr<CheckBox>        m_pCB_RoundedEdge;

    bool            m_bUpdateOtherControls;
    bool            m_bCommitToModel;
    OUString        m_aCustom;

    ControllerLockHelper & m_rControllerLockHelper;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
