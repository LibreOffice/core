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

View3DDialog::View3DDialog(Window* pParent, const uno::Reference< frame::XModel > & xChartModel, const XColorListRef &pColorTable )
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

    m_aTabControl.InsertPage( TP_3D_SCENEGEOMETRY, SCH_RESSTR(STR_PAGE_PERSPECTIVE) );
    m_aTabControl.InsertPage( TP_3D_SCENEAPPEARANCE, SCH_RESSTR(STR_PAGE_APPEARANCE) );
    m_aTabControl.InsertPage( TP_3D_SCENEILLUMINATION, SCH_RESSTR(STR_PAGE_ILLUMINATION) );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
