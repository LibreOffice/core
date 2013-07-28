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
#ifndef _CHART2_TP_3D_SCENE_APPEARANCE_HXX
#define _CHART2_TP_3D_SCENE_APPEARANCE_HXX

// header for class TabPage
#include <vcl/tabpage.hxx>
// header for class FixedLine
#include <vcl/fixed.hxx>
// header for class MetricField
#include <vcl/field.hxx>
// header for class CheckBox
#include <vcl/button.hxx>
// header for class ListBox
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
        Window* pWindow,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xChartModel,
        ControllerLockHelper & rControllerLockHelper );
    virtual ~ThreeD_SceneAppearance_TabPage();

    // has to be called in case the dialog was closed with OK
    void commitPendingChanges();

    virtual void ActivatePage();

private:
    DECL_LINK( SelectSchemeHdl, void* );
    DECL_LINK( SelectShading, void* );
    DECL_LINK( SelectRoundedEdgeOrObjectLines, CheckBox* );

    void initControlsFromModel();
    void applyShadeModeToModel();
    void applyRoundedEdgeAndObjectLinesToModel();
    void updateScheme();

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
        m_xChartModel;

    FixedText       m_aFT_Scheme;
    ListBox         m_aLB_Scheme;

    FixedLine       m_aFL_Separator;

    CheckBox        m_aCB_Shading;
    CheckBox        m_aCB_ObjectLines;
    CheckBox        m_aCB_RoundedEdge;

    bool            m_bUpdateOtherControls;
    bool            m_bCommitToModel;

    ControllerLockHelper & m_rControllerLockHelper;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
