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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "dlg_View3D.hxx"
#include "dlg_View3D.hrc"
#include "Strings.hrc"
#include "TabPages.hrc"
#include "ResId.hxx"
#include "NoWarningThisInCTOR.hxx"
#include "tp_3D_SceneGeometry.hxx"
#include "tp_3D_SceneAppearance.hxx"
#include "tp_3D_SceneIllumination.hxx"
#include "ChartModelHelper.hxx"
#include "macros.hxx"
#include "ControllerLockGuard.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>

// for RET_OK
#include <vcl/msgbox.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

//-----------------------------------------------------------------------------
//-------------------------------------------------------------------
//-------------------------------------------------------------------

sal_uInt16 View3DDialog::m_nLastPageId = 0;

View3DDialog::View3DDialog(Window* pParent, const uno::Reference< frame::XModel > & xChartModel, XColorTable* pColorTable )
    : TabDialog(pParent,SchResId(DLG_3D_VIEW))
    , m_aTabControl(this,SchResId(TABCTRL))
    , m_aBtnOK(this,SchResId(BTN_OK))
    , m_aBtnCancel(this,SchResId(BTN_CANCEL))
    , m_aBtnHelp(this,SchResId(BTN_HELP))
    , m_pGeometry(0)
    , m_pAppearance(0)
    , m_pIllumination(0)
    , m_aControllerLocker(xChartModel)
{
    FreeResource();

    uno::Reference< beans::XPropertySet > xSceneProperties( ChartModelHelper::findDiagram( xChartModel ), uno::UNO_QUERY );
    m_pGeometry   = new ThreeD_SceneGeometry_TabPage(&m_aTabControl,xSceneProperties,m_aControllerLocker);
    m_pAppearance = new ThreeD_SceneAppearance_TabPage(&m_aTabControl,xChartModel,m_aControllerLocker);
    m_pIllumination = new ThreeD_SceneIllumination_TabPage(&m_aTabControl,xSceneProperties,xChartModel,pColorTable);

    m_aTabControl.InsertPage( TP_3D_SCENEGEOMETRY, String(SchResId(STR_PAGE_PERSPECTIVE)) );
    m_aTabControl.InsertPage( TP_3D_SCENEAPPEARANCE, String(SchResId(STR_PAGE_APPEARANCE)) );
    m_aTabControl.InsertPage( TP_3D_SCENEILLUMINATION, String(SchResId(STR_PAGE_ILLUMINATION)) );

    m_aTabControl.SetTabPage( TP_3D_SCENEGEOMETRY, m_pGeometry );
    m_aTabControl.SetTabPage( TP_3D_SCENEAPPEARANCE, m_pAppearance );
    m_aTabControl.SetTabPage( TP_3D_SCENEILLUMINATION, m_pIllumination );

    m_aTabControl.SelectTabPage( m_nLastPageId );
}

View3DDialog::~View3DDialog()
{
    delete m_pGeometry;
    delete m_pAppearance;
    delete m_pIllumination;

    m_nLastPageId = m_aTabControl.GetCurPageId();
}

short View3DDialog::Execute()
{
    short nResult = TabDialog::Execute();
    if( nResult == RET_OK )
    {
        if( m_pGeometry )
            m_pGeometry->commitPendingChanges();
        if( m_pAppearance )
            m_pAppearance->commitPendingChanges();
        if( m_pIllumination )
            m_pIllumination->commitPendingChanges();
    }
    return nResult;
}

//.............................................................................
} //namespace chart
//.............................................................................
