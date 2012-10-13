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
#include "tp_3D_SceneGeometry.hrc"
#include "ResId.hxx"
#include "NoWarningThisInCTOR.hxx"
#include "BaseGFXHelper.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "ThreeDHelper.hxx"
#include <rtl/math.hxx>
#include <editeng/unoprnms.hxx>
#include <com/sun/star/drawing/ProjectionMode.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

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
ThreeD_SceneGeometry_TabPage::ThreeD_SceneGeometry_TabPage( Window* pWindow
                , const uno::Reference< beans::XPropertySet > & xSceneProperties
                , ControllerLockHelper & rControllerLockHelper )
                : TabPage       ( pWindow, SchResId( TP_3D_SCENEGEOMETRY ) )
                , m_xSceneProperties( xSceneProperties )
                , m_aCbxRightAngledAxes( this, SchResId( CBX_RIGHT_ANGLED_AXES ) )
                , m_aFtXRotation    ( this, SchResId( FT_X_ROTATION ) )
                , m_aMFXRotation    ( this, SchResId( MTR_FLD_X_ROTATION ) )
                , m_aFtYRotation    ( this, SchResId( FT_Y_ROTATION ) )
                , m_aMFYRotation    ( this, SchResId( MTR_FLD_Y_ROTATION ) )
                , m_aFtZRotation    ( this, SchResId( FT_Z_ROTATION ) )
                , m_aMFZRotation    ( this, SchResId( MTR_FLD_Z_ROTATION ) )
                , m_aCbxPerspective ( this, SchResId( CBX_PERSPECTIVE ) )
                , m_aMFPerspective  ( this, SchResId( MTR_FLD_PERSPECTIVE ) )
                , m_nXRotation(0)
                , m_nYRotation(0)
                , m_nZRotation(0)
                , m_bAngleChangePending( false )
                , m_bPerspectiveChangePending( false )
                , m_rControllerLockHelper( rControllerLockHelper )
{
    FreeResource();

    double fXAngle, fYAngle, fZAngle;
    ThreeDHelper::getRotationAngleFromDiagram( m_xSceneProperties, fXAngle, fYAngle, fZAngle );

    fXAngle = BaseGFXHelper::Rad2Deg( fXAngle );
    fYAngle = BaseGFXHelper::Rad2Deg( fYAngle );
    fZAngle = BaseGFXHelper::Rad2Deg( fZAngle );

    OSL_ENSURE( fZAngle>=-90 && fZAngle<=90, "z angle is out of valid range" );

    lcl_SetMetricFieldLimits( m_aMFZRotation, 90 );

    m_nXRotation = ::basegfx::fround(fXAngle*pow(10.0,m_aMFXRotation.GetDecimalDigits()));
    m_nYRotation = ::basegfx::fround(-1.0*fYAngle*pow(10.0,m_aMFYRotation.GetDecimalDigits()));
    m_nZRotation = ::basegfx::fround(-1.0*fZAngle*pow(10.0,m_aMFZRotation.GetDecimalDigits()));

    lcl_shiftAngleToValidRange( m_nXRotation );
    lcl_shiftAngleToValidRange( m_nYRotation );
    lcl_shiftAngleToValidRange( m_nZRotation );

    m_aMFXRotation.SetValue(m_nXRotation);
    m_aMFYRotation.SetValue(m_nYRotation);
    m_aMFZRotation.SetValue(m_nZRotation);

    const sal_uLong nTimeout = 4*EDIT_UPDATEDATA_TIMEOUT;
    Link aAngleChangedLink( LINK( this, ThreeD_SceneGeometry_TabPage, AngleChanged ));
    Link aAngleEditedLink( LINK( this, ThreeD_SceneGeometry_TabPage, AngleEdited ));

    m_aMFXRotation.EnableUpdateData( nTimeout );
    m_aMFXRotation.SetUpdateDataHdl( aAngleChangedLink );
    m_aMFXRotation.SetModifyHdl( aAngleEditedLink );

    m_aMFYRotation.EnableUpdateData( nTimeout );
    m_aMFYRotation.SetUpdateDataHdl( aAngleChangedLink );
    m_aMFYRotation.SetModifyHdl( aAngleEditedLink );

    m_aMFZRotation.EnableUpdateData( nTimeout );
    m_aMFZRotation.SetUpdateDataHdl( aAngleChangedLink );
    m_aMFZRotation.SetModifyHdl( aAngleEditedLink );

    drawing::ProjectionMode aProjectionMode = drawing::ProjectionMode_PERSPECTIVE;
    m_xSceneProperties->getPropertyValue( "D3DScenePerspective" ) >>= aProjectionMode;
    m_aCbxPerspective.Check( aProjectionMode == drawing::ProjectionMode_PERSPECTIVE );
    m_aCbxPerspective.SetToggleHdl( LINK( this, ThreeD_SceneGeometry_TabPage, PerspectiveToggled ));

    sal_Int32 nPerspectivePercentage = 20;
    m_xSceneProperties->getPropertyValue( "Perspective" ) >>= nPerspectivePercentage;
    m_aMFPerspective.SetValue( nPerspectivePercentage );

    m_aMFPerspective.EnableUpdateData( nTimeout );
    m_aMFPerspective.SetUpdateDataHdl( LINK( this, ThreeD_SceneGeometry_TabPage, PerspectiveChanged ) );
    m_aMFPerspective.SetModifyHdl( LINK( this, ThreeD_SceneGeometry_TabPage, PerspectiveEdited ) );
    m_aMFPerspective.Enable( m_aCbxPerspective.IsChecked() );


    //RightAngledAxes
    sal_Bool bRightAngledAxes = false;

    uno::Reference< chart2::XDiagram > xDiagram( m_xSceneProperties, uno::UNO_QUERY );
    if( ChartTypeHelper::isSupportingRightAngledAxes(
            DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) ) )
    {
        m_xSceneProperties->getPropertyValue( "RightAngledAxes" ) >>= bRightAngledAxes;
        m_aCbxRightAngledAxes.SetToggleHdl( LINK( this, ThreeD_SceneGeometry_TabPage, RightAngledAxesToggled ));
        m_aCbxRightAngledAxes.Check( bRightAngledAxes );
    }
    else
    {
        m_aCbxRightAngledAxes.Enable(false);
    }
    m_aMFPerspective.SetAccessibleName(m_aCbxPerspective.GetText());
    m_aMFPerspective.SetAccessibleRelationLabeledBy(&m_aCbxPerspective);
}

ThreeD_SceneGeometry_TabPage::~ThreeD_SceneGeometry_TabPage()
{
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

    if( !m_aMFZRotation.IsEmptyFieldValue() )
        m_nZRotation = m_aMFZRotation.GetValue();

    fXAngle = double(m_nXRotation)/double(pow(10.0,m_aMFXRotation.GetDecimalDigits()));
    fYAngle = double(-1.0*m_nYRotation)/double(pow(10.0,m_aMFYRotation.GetDecimalDigits()));
    fZAngle = double(-1.0*m_nZRotation)/double(pow(10.0,m_aMFZRotation.GetDecimalDigits()));

    fXAngle = BaseGFXHelper::Deg2Rad( fXAngle );
    fYAngle = BaseGFXHelper::Deg2Rad( fYAngle );
    fZAngle = BaseGFXHelper::Deg2Rad( fZAngle );

    ThreeDHelper::setRotationAngleToDiagram( m_xSceneProperties, fXAngle, fYAngle, fZAngle );

    m_bAngleChangePending = false;
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, AngleEdited)
{
    m_nXRotation = m_aMFXRotation.GetValue();
    m_nYRotation = m_aMFYRotation.GetValue();

    m_bAngleChangePending = true;
    return 0;
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, AngleChanged)
{
    applyAnglesToModel();
    return 0;
}

void ThreeD_SceneGeometry_TabPage::applyPerspectiveToModel()
{
    ControllerLockHelperGuard aGuard( m_rControllerLockHelper );

    drawing::ProjectionMode aMode = m_aCbxPerspective.IsChecked()
        ? drawing::ProjectionMode_PERSPECTIVE
        : drawing::ProjectionMode_PARALLEL;

    try
    {
        m_xSceneProperties->setPropertyValue( "D3DScenePerspective" , uno::makeAny( aMode ));
        m_xSceneProperties->setPropertyValue( "Perspective" , uno::makeAny( (sal_Int32)m_aMFPerspective.GetValue() ));
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    m_bPerspectiveChangePending = false;
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, PerspectiveEdited)
{
    m_bPerspectiveChangePending = true;
    return 0;
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, PerspectiveChanged)
{
    applyPerspectiveToModel();
    return 0;
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, PerspectiveToggled)
{
    m_aMFPerspective.Enable( m_aCbxPerspective.IsChecked() );
    applyPerspectiveToModel();
    return 0;
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, RightAngledAxesToggled)
{
    ControllerLockHelperGuard aGuard( m_rControllerLockHelper );

    bool bEnableZ = !m_aCbxRightAngledAxes.IsChecked();
    m_aFtZRotation.Enable( bEnableZ );
    m_aMFZRotation.Enable( bEnableZ );
    m_aMFZRotation.EnableEmptyFieldValue( !bEnableZ );
    if( !bEnableZ )
    {
        m_nXRotation = m_aMFXRotation.GetValue();
        m_nYRotation = m_aMFYRotation.GetValue();
        m_nZRotation = m_aMFZRotation.GetValue();

        m_aMFXRotation.SetValue(static_cast<sal_Int64>(ThreeDHelper::getValueClippedToRange(static_cast<double>(m_nXRotation), ThreeDHelper::getXDegreeAngleLimitForRightAngledAxes())));
        m_aMFYRotation.SetValue(static_cast<sal_Int64>(ThreeDHelper::getValueClippedToRange(static_cast<double>(m_nYRotation), ThreeDHelper::getYDegreeAngleLimitForRightAngledAxes())));
        m_aMFZRotation.SetEmptyFieldValue();

        lcl_SetMetricFieldLimits( m_aMFXRotation, static_cast<sal_Int64>(ThreeDHelper::getXDegreeAngleLimitForRightAngledAxes()));
        lcl_SetMetricFieldLimits( m_aMFYRotation, static_cast<sal_Int64>(ThreeDHelper::getYDegreeAngleLimitForRightAngledAxes()));
    }
    else
    {
        lcl_SetMetricFieldLimits( m_aMFXRotation, 180 );
        lcl_SetMetricFieldLimits( m_aMFYRotation, 180 );

        m_aMFXRotation.SetValue(m_nXRotation);
        m_aMFYRotation.SetValue(m_nYRotation);
        m_aMFZRotation.SetValue(m_nZRotation);
    }

    ThreeDHelper::switchRightAngledAxes( m_xSceneProperties, m_aCbxRightAngledAxes.IsChecked(), true /*bRotateLights*/ );

    return 0;
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
