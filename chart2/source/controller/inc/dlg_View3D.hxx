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
// header for class XColorTable
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
                  XColorTable* pColorTable=0 );
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
