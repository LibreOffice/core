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


#ifndef _CHART2_DLG_VIEW3D_DIAGRAM_HXX
#define _CHART2_DLG_VIEW3D_DIAGRAM_HXX

#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/frame/XModel.hpp>

// header for class TabDialog
#include <vcl/tabdlg.hxx>
// header for class TabControl
#include <vcl/tabctrl.hxx>
// header for class OKButton
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for class XColorList
#include <svx/xtable.hxx>

#include "ControllerLockGuard.hxx"

//.............................................................................
namespace chart
{
//.............................................................................
class ThreeD_SceneGeometry_TabPage;
class ThreeD_SceneAppearance_TabPage;
class ThreeD_SceneIllumination_TabPage;

class View3DDialog : public TabDialog
{
public:
    View3DDialog( Window* pWindow,
                  const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xChartModel,
                  XColorList* pColorTable=0 );
    ~View3DDialog();

    // from Dialog (base of TabDialog)
    virtual short Execute();

private:
    TabControl      m_aTabControl;
    OKButton        m_aBtnOK;
    CancelButton    m_aBtnCancel;
    HelpButton      m_aBtnHelp;

    ThreeD_SceneGeometry_TabPage*       m_pGeometry;
    ThreeD_SceneAppearance_TabPage*     m_pAppearance;
    ThreeD_SceneIllumination_TabPage*   m_pIllumination;

    ControllerLockHelper                m_aControllerLocker;

    static sal_uInt16 m_nLastPageId;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
