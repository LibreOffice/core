/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CHART2_TP_3D_SCENE_APPEARANCE_HXX
#define _CHART2_TP_3D_SCENE_APPEARANCE_HXX

// header for class TabPage
#include <vcl/tabpage.hxx>
// header for class FixedLine
#include <vcl/fixed.hxx>
// header for class MetricField
#include <vcl/field.hxx>
// header for class CheckBox
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for class ListBox
#include <vcl/lstbox.hxx>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include "ControllerLockGuard.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

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

    FixedLine       m_aFL_Seperator;

    CheckBox        m_aCB_Shading;
    CheckBox        m_aCB_ObjectLines;
    CheckBox        m_aCB_RoundedEdge;

    bool            m_bUpdateOtherControls;
    bool            m_bCommitToModel;

    ControllerLockHelper & m_rControllerLockHelper;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
