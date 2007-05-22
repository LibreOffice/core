/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_View3D.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:38:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
#include "Pseudo3DHelper.hxx"
#include "ControllerLockGuard.hxx"

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

// for RET_OK
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

//-----------------------------------------------------------------------------
//-------------------------------------------------------------------
//-------------------------------------------------------------------

// static
USHORT View3DDialog::m_nLastPageId = 0;

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
