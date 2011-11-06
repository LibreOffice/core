/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
