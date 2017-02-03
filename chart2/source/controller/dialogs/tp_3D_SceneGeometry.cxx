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

#include "tp_3D_SceneGeometry.hxx"

#include "BaseGFXHelper.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "ThreeDHelper.hxx"
#include <rtl/math.hxx>
#include <editeng/unoprnms.hxx>
#include <com/sun/star/drawing/ProjectionMode.hpp>

namespace chart
{

using namespace ::com::sun::star;

namespace
{

void lcl_shiftAngleToValidRange( sal_Int64& rnAngleDegree )
{
    //valid range:  ]-180,180]
    while( rnAngleDegree<=-180 )
        rnAngleDegree+=360;
    while( rnAngleDegree>180 )
        rnAngleDegree-=360;
}

void lcl_SetMetricFieldLimits( MetricField& rField, sal_Int64 nLimit )
{
    rField.SetMin(-1*nLimit);
    rField.SetFirst(-1*nLimit);
    rField.SetMax(nLimit);
    rField.SetLast(nLimit);
}

}
ThreeD_SceneGeometry_TabPage::ThreeD_SceneGeometry_TabPage( vcl::Window* pWindow
                , const uno::Reference< beans::XPropertySet > & xSceneProperties
                , ControllerLockHelper & rControllerLockHelper )
                : TabPage ( pWindow
                          , "tp_3DSceneGeometry"
                          , "modules/schart/ui/tp_3D_SceneGeometry.ui")
                , m_xSceneProperties( xSceneProperties )
                , m_nXRotation(0)
                , m_nYRotation(0)
                , m_nZRotation(0)
                , m_bAngleChangePending( false )
                , m_bPerspectiveChangePending( false )
                , m_rControllerLockHelper( rControllerLockHelper )
{
    get(m_pCbxRightAngledAxes,"CBX_RIGHT_ANGLED_AXES");
    get(m_pMFXRotation, "MTR_FLD_X_ROTATION");
    get(m_pMFYRotation, "MTR_FLD_Y_ROTATION");
    get(m_pFtZRotation, "FT_Z_ROTATION");
    get(m_pMFZRotation, "MTR_FLD_Z_ROTATION");
    get(m_pCbxPerspective,"CBX_PERSPECTIVE");
    get(m_pMFPerspective, "MTR_FLD_PERSPECTIVE");

    double fXAngle, fYAngle, fZAngle;
    ThreeDHelper::getRotationAngleFromDiagram( m_xSceneProperties, fXAngle, fYAngle, fZAngle );

    fXAngle = basegfx::rad2deg(fXAngle);
    fYAngle = basegfx::rad2deg(fYAngle);
    fZAngle = basegfx::rad2deg(fZAngle);

    OSL_ENSURE( fZAngle>=-90 && fZAngle<=90, "z angle is out of valid range" );

    lcl_SetMetricFieldLimits( *m_pMFZRotation, 90 );

    m_nXRotation = ::basegfx::fround(fXAngle*pow(10.0,m_pMFXRotation->GetDecimalDigits()));
    m_nYRotation = ::basegfx::fround(-1.0*fYAngle*pow(10.0,m_pMFYRotation->GetDecimalDigits()));
    m_nZRotation = ::basegfx::fround(-1.0*fZAngle*pow(10.0,m_pMFZRotation->GetDecimalDigits()));

    lcl_shiftAngleToValidRange( m_nXRotation );
    lcl_shiftAngleToValidRange( m_nYRotation );
    lcl_shiftAngleToValidRange( m_nZRotation );

    m_pMFXRotation->SetValue(m_nXRotation);
    m_pMFYRotation->SetValue(m_nYRotation);
    m_pMFZRotation->SetValue(m_nZRotation);

    const sal_uLong nTimeout = 4*EDIT_UPDATEDATA_TIMEOUT;
    Link<Edit&,void> aAngleChangedLink( LINK( this, ThreeD_SceneGeometry_TabPage, AngleChanged ));
    Link<Edit&,void> aAngleEditedLink( LINK( this, ThreeD_SceneGeometry_TabPage, AngleEdited ));

    m_pMFXRotation->EnableUpdateData( nTimeout );
    m_pMFXRotation->SetUpdateDataHdl( aAngleChangedLink );
    m_pMFXRotation->SetModifyHdl( aAngleEditedLink );

    m_pMFYRotation->EnableUpdateData( nTimeout );
    m_pMFYRotation->SetUpdateDataHdl( aAngleChangedLink );
    m_pMFYRotation->SetModifyHdl( aAngleEditedLink );

    m_pMFZRotation->EnableUpdateData( nTimeout );
    m_pMFZRotation->SetUpdateDataHdl( aAngleChangedLink );
    m_pMFZRotation->SetModifyHdl( aAngleEditedLink );

    drawing::ProjectionMode aProjectionMode = drawing::ProjectionMode_PERSPECTIVE;
    m_xSceneProperties->getPropertyValue( "D3DScenePerspective" ) >>= aProjectionMode;
    m_pCbxPerspective->Check( aProjectionMode == drawing::ProjectionMode_PERSPECTIVE );
    m_pCbxPerspective->SetToggleHdl( LINK( this, ThreeD_SceneGeometry_TabPage, PerspectiveToggled ));

    sal_Int32 nPerspectivePercentage = 20;
    m_xSceneProperties->getPropertyValue( "Perspective" ) >>= nPerspectivePercentage;
    m_pMFPerspective->SetValue( nPerspectivePercentage );

    m_pMFPerspective->EnableUpdateData( nTimeout );
    m_pMFPerspective->SetUpdateDataHdl( LINK( this, ThreeD_SceneGeometry_TabPage, PerspectiveChanged ) );
    m_pMFPerspective->SetModifyHdl( LINK( this, ThreeD_SceneGeometry_TabPage, PerspectiveEdited ) );
    m_pMFPerspective->Enable( m_pCbxPerspective->IsChecked() );

    //RightAngledAxes
    uno::Reference< chart2::XDiagram > xDiagram( m_xSceneProperties, uno::UNO_QUERY );
    if( ChartTypeHelper::isSupportingRightAngledAxes(
            DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) ) )
    {
        bool bRightAngledAxes = false;
        m_xSceneProperties->getPropertyValue( "RightAngledAxes" ) >>= bRightAngledAxes;
        m_pCbxRightAngledAxes->SetToggleHdl( LINK( this, ThreeD_SceneGeometry_TabPage, RightAngledAxesToggled ));
        m_pCbxRightAngledAxes->Check( bRightAngledAxes );
    }
    else
    {
        m_pCbxRightAngledAxes->Enable(false);
    }
}

ThreeD_SceneGeometry_TabPage::~ThreeD_SceneGeometry_TabPage()
{
    disposeOnce();
}

void ThreeD_SceneGeometry_TabPage::dispose()
{
    m_pCbxRightAngledAxes.clear();
    m_pMFXRotation.clear();
    m_pMFYRotation.clear();
    m_pFtZRotation.clear();
    m_pMFZRotation.clear();
    m_pCbxPerspective.clear();
    m_pMFPerspective.clear();
    TabPage::dispose();
}


void ThreeD_SceneGeometry_TabPage::commitPendingChanges()
{
    ControllerLockHelperGuard aGuard( m_rControllerLockHelper );

    if( m_bAngleChangePending )
        applyAnglesToModel();
    if( m_bPerspectiveChangePending )
        applyPerspectiveToModel();
}

void ThreeD_SceneGeometry_TabPage::applyAnglesToModel()
{
    ControllerLockHelperGuard aGuard( m_rControllerLockHelper );

    double fXAngle = 0.0, fYAngle = 0.0, fZAngle = 0.0;

    if( !m_pMFZRotation->IsEmptyFieldValue() )
        m_nZRotation = m_pMFZRotation->GetValue();

    fXAngle = double(m_nXRotation)/double(pow(10.0,m_pMFXRotation->GetDecimalDigits()));
    fYAngle = double(-1.0*m_nYRotation)/double(pow(10.0,m_pMFYRotation->GetDecimalDigits()));
    fZAngle = double(-1.0*m_nZRotation)/double(pow(10.0,m_pMFZRotation->GetDecimalDigits()));

    fXAngle = basegfx::deg2rad(fXAngle);
    fYAngle = basegfx::deg2rad(fYAngle);
    fZAngle = basegfx::deg2rad(fZAngle);

    ThreeDHelper::setRotationAngleToDiagram( m_xSceneProperties, fXAngle, fYAngle, fZAngle );

    m_bAngleChangePending = false;
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, AngleEdited, Edit&, void)
{
    m_nXRotation = m_pMFXRotation->GetValue();
    m_nYRotation = m_pMFYRotation->GetValue();

    m_bAngleChangePending = true;
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, AngleChanged, Edit&, void)
{
    applyAnglesToModel();
}

void ThreeD_SceneGeometry_TabPage::applyPerspectiveToModel()
{
    ControllerLockHelperGuard aGuard( m_rControllerLockHelper );

    drawing::ProjectionMode aMode = m_pCbxPerspective->IsChecked()
        ? drawing::ProjectionMode_PERSPECTIVE
        : drawing::ProjectionMode_PARALLEL;

    try
    {
        m_xSceneProperties->setPropertyValue( "D3DScenePerspective" , uno::Any( aMode ));
        m_xSceneProperties->setPropertyValue( "Perspective" , uno::Any( (sal_Int32)m_pMFPerspective->GetValue() ));
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    m_bPerspectiveChangePending = false;
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, PerspectiveEdited, Edit&, void)
{
    m_bPerspectiveChangePending = true;
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, PerspectiveChanged, Edit&, void)
{
    applyPerspectiveToModel();
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, PerspectiveToggled, CheckBox&, void)
{
    m_pMFPerspective->Enable( m_pCbxPerspective->IsChecked() );
    applyPerspectiveToModel();
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, RightAngledAxesToggled, CheckBox&, void)
{
    ControllerLockHelperGuard aGuard( m_rControllerLockHelper );

    bool bEnableZ = !m_pCbxRightAngledAxes->IsChecked();
    m_pFtZRotation->Enable( bEnableZ );
    m_pMFZRotation->Enable( bEnableZ );
    m_pMFZRotation->EnableEmptyFieldValue( !bEnableZ );
    if( !bEnableZ )
    {
        m_nXRotation = m_pMFXRotation->GetValue();
        m_nYRotation = m_pMFYRotation->GetValue();
        m_nZRotation = m_pMFZRotation->GetValue();

        m_pMFXRotation->SetValue(static_cast<sal_Int64>(ThreeDHelper::getValueClippedToRange(static_cast<double>(m_nXRotation), ThreeDHelper::getXDegreeAngleLimitForRightAngledAxes())));
        m_pMFYRotation->SetValue(static_cast<sal_Int64>(ThreeDHelper::getValueClippedToRange(static_cast<double>(m_nYRotation), ThreeDHelper::getYDegreeAngleLimitForRightAngledAxes())));
        m_pMFZRotation->SetEmptyFieldValue();

        lcl_SetMetricFieldLimits( *m_pMFXRotation, static_cast<sal_Int64>(ThreeDHelper::getXDegreeAngleLimitForRightAngledAxes()));
        lcl_SetMetricFieldLimits( *m_pMFYRotation, static_cast<sal_Int64>(ThreeDHelper::getYDegreeAngleLimitForRightAngledAxes()));
    }
    else
    {
        lcl_SetMetricFieldLimits( *m_pMFXRotation, 180 );
        lcl_SetMetricFieldLimits( *m_pMFYRotation, 180 );

        m_pMFXRotation->SetValue(m_nXRotation);
        m_pMFYRotation->SetValue(m_nYRotation);
        m_pMFZRotation->SetValue(m_nZRotation);
    }

    ThreeDHelper::switchRightAngledAxes( m_xSceneProperties, m_pCbxRightAngledAxes->IsChecked() );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
