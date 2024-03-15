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

#include <Diagram.hxx>
#include <ChartType.hxx>
#include <ChartTypeHelper.hxx>
#include <ThreeDHelper.hxx>
#include <ControllerLockGuard.hxx>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <tools/helpers.hxx>
#include <utility>
#include <vcl/svapp.hxx>

namespace chart
{

using namespace ::com::sun::star;

namespace
{

void lcl_SetMetricFieldLimits(weld::MetricSpinButton& rField, sal_Int64 nLimit)
{
    rField.set_range(-1*nLimit, nLimit, FieldUnit::DEGREE);
}

}

ThreeD_SceneGeometry_TabPage::ThreeD_SceneGeometry_TabPage(weld::Container* pParent,
        rtl::Reference< ::chart::Diagram > xDiagram,
        ControllerLockHelper & rControllerLockHelper)
    : m_xDiagram(std::move( xDiagram ))
    , m_aAngleTimer("chart2 ThreeD_SceneGeometry_TabPage m_aAngleTimer")
    , m_aPerspectiveTimer("chart2 ThreeD_SceneGeometry_TabPage m_aPerspectiveTimer")
    , m_nXRotation(0)
    , m_nYRotation(0)
    , m_nZRotation(0)
    , m_bAngleChangePending( false )
    , m_bPerspectiveChangePending( false )
    , m_rControllerLockHelper( rControllerLockHelper )
    , m_xBuilder(Application::CreateBuilder(pParent, "modules/schart/ui/tp_3D_SceneGeometry.ui"))
    , m_xContainer(m_xBuilder->weld_container("tp_3DSceneGeometry"))
    , m_xCbxRightAngledAxes(m_xBuilder->weld_check_button("CBX_RIGHT_ANGLED_AXES"))
    , m_xMFXRotation(m_xBuilder->weld_metric_spin_button("MTR_FLD_X_ROTATION", FieldUnit::DEGREE))
    , m_xMFYRotation(m_xBuilder->weld_metric_spin_button("MTR_FLD_Y_ROTATION", FieldUnit::DEGREE))
    , m_xFtZRotation(m_xBuilder->weld_label("FT_Z_ROTATION"))
    , m_xMFZRotation(m_xBuilder->weld_metric_spin_button("MTR_FLD_Z_ROTATION", FieldUnit::DEGREE))
    , m_xCbxPerspective(m_xBuilder->weld_check_button("CBX_PERSPECTIVE"))
    , m_xMFPerspective(m_xBuilder->weld_metric_spin_button("MTR_FLD_PERSPECTIVE", FieldUnit::PERCENT))
{
    double fXAngle, fYAngle, fZAngle;
    m_xDiagram->getRotationAngle( fXAngle, fYAngle, fZAngle );

    fXAngle = basegfx::rad2deg(fXAngle);
    fYAngle = basegfx::rad2deg(fYAngle);
    fZAngle = basegfx::rad2deg(fZAngle);

    OSL_ENSURE( fZAngle>=-90 && fZAngle<=90, "z angle is out of valid range" );

    lcl_SetMetricFieldLimits( *m_xMFZRotation, 90 );

    m_nXRotation = NormAngle180(
        ::basegfx::fround(fXAngle * pow(10.0, m_xMFXRotation->get_digits())));
    m_nYRotation = NormAngle180(
        ::basegfx::fround(-1.0 * fYAngle * pow(10.0, m_xMFYRotation->get_digits())));
    m_nZRotation = NormAngle180(
        ::basegfx::fround(-1.0 * fZAngle * pow(10.0, m_xMFZRotation->get_digits())));

    m_xMFXRotation->set_value(m_nXRotation, FieldUnit::DEGREE);
    m_xMFYRotation->set_value(m_nYRotation, FieldUnit::DEGREE);
    m_xMFZRotation->set_value(m_nZRotation, FieldUnit::DEGREE);

    const int nTimeout = 4*EDIT_UPDATEDATA_TIMEOUT;
    m_aAngleTimer.SetTimeout(nTimeout);
    m_aAngleTimer.SetInvokeHandler( LINK( this, ThreeD_SceneGeometry_TabPage, AngleChanged ) );

    Link<weld::MetricSpinButton&,void> aAngleEditedLink( LINK( this, ThreeD_SceneGeometry_TabPage, AngleEdited ));
    m_xMFXRotation->connect_value_changed( aAngleEditedLink );
    m_xMFYRotation->connect_value_changed( aAngleEditedLink );
    m_xMFZRotation->connect_value_changed( aAngleEditedLink );

    drawing::ProjectionMode aProjectionMode = drawing::ProjectionMode_PERSPECTIVE;
    m_xDiagram->getPropertyValue( "D3DScenePerspective" ) >>= aProjectionMode;
    m_xCbxPerspective->set_active( aProjectionMode == drawing::ProjectionMode_PERSPECTIVE );
    m_xCbxPerspective->connect_toggled( LINK( this, ThreeD_SceneGeometry_TabPage, PerspectiveToggled ));

    sal_Int32 nPerspectivePercentage = 20;
    m_xDiagram->getPropertyValue( "Perspective" ) >>= nPerspectivePercentage;
    m_xMFPerspective->set_value(nPerspectivePercentage, FieldUnit::PERCENT);

    m_aPerspectiveTimer.SetTimeout(nTimeout);
    m_aPerspectiveTimer.SetInvokeHandler( LINK( this, ThreeD_SceneGeometry_TabPage, PerspectiveChanged ) );
    m_xMFPerspective->connect_value_changed( LINK( this, ThreeD_SceneGeometry_TabPage, PerspectiveEdited ) );
    m_xMFPerspective->set_sensitive( m_xCbxPerspective->get_active() );

    //RightAngledAxes
    if (ChartTypeHelper::isSupportingRightAngledAxes(m_xDiagram->getChartTypeByIndex(0)))
    {
        bool bRightAngledAxes = false;
        m_xDiagram->getPropertyValue( "RightAngledAxes" ) >>= bRightAngledAxes;
        m_xCbxRightAngledAxes->connect_toggled( LINK( this, ThreeD_SceneGeometry_TabPage, RightAngledAxesToggled ));
        m_xCbxRightAngledAxes->set_active( bRightAngledAxes );
        RightAngledAxesToggled(*m_xCbxRightAngledAxes);
    }
    else
    {
        m_xCbxRightAngledAxes->set_sensitive(false);
    }
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

    if (m_xMFZRotation->get_sensitive())
        m_nZRotation = m_xMFZRotation->get_value(FieldUnit::DEGREE);

    fXAngle = double(m_nXRotation)/pow(10.0,m_xMFXRotation->get_digits());
    fYAngle = double(-1.0*m_nYRotation)/pow(10.0,m_xMFYRotation->get_digits());
    fZAngle = double(-1.0*m_nZRotation)/pow(10.0,m_xMFZRotation->get_digits());

    fXAngle = basegfx::deg2rad(fXAngle);
    fYAngle = basegfx::deg2rad(fYAngle);
    fZAngle = basegfx::deg2rad(fZAngle);

    m_xDiagram->setRotationAngle( fXAngle, fYAngle, fZAngle );

    m_bAngleChangePending = false;
    m_aAngleTimer.Stop();
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, AngleEdited, weld::MetricSpinButton&, void)
{
    m_nXRotation = m_xMFXRotation->get_value(FieldUnit::DEGREE);
    m_nYRotation = m_xMFYRotation->get_value(FieldUnit::DEGREE);

    m_bAngleChangePending = true;

    m_aAngleTimer.Start();
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, AngleChanged, Timer *, void)
{
    applyAnglesToModel();
}

void ThreeD_SceneGeometry_TabPage::applyPerspectiveToModel()
{
    ControllerLockHelperGuard aGuard( m_rControllerLockHelper );

    drawing::ProjectionMode aMode = m_xCbxPerspective->get_active()
        ? drawing::ProjectionMode_PERSPECTIVE
        : drawing::ProjectionMode_PARALLEL;

    try
    {
        m_xDiagram->setPropertyValue( "D3DScenePerspective" , uno::Any( aMode ));
        m_xDiagram->setPropertyValue( "Perspective" , uno::Any( static_cast<sal_Int32>(m_xMFPerspective->get_value(FieldUnit::PERCENT)) ));
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    m_bPerspectiveChangePending = false;
    m_aPerspectiveTimer.Stop();
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, PerspectiveEdited, weld::MetricSpinButton&, void)
{
    m_bPerspectiveChangePending = true;
    m_aPerspectiveTimer.Start();
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, PerspectiveChanged, Timer *, void)
{
    applyPerspectiveToModel();
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, PerspectiveToggled, weld::Toggleable&, void)
{
    m_xMFPerspective->set_sensitive(m_xCbxPerspective->get_active());
    applyPerspectiveToModel();
}

IMPL_LINK_NOARG(ThreeD_SceneGeometry_TabPage, RightAngledAxesToggled, weld::Toggleable&, void)
{
    ControllerLockHelperGuard aGuard( m_rControllerLockHelper );

    bool bEnableZ = !m_xCbxRightAngledAxes->get_active();
    m_xFtZRotation->set_sensitive( bEnableZ );
    m_xMFZRotation->set_sensitive( bEnableZ );
    if (!bEnableZ)
    {
        m_nXRotation = m_xMFXRotation->get_value(FieldUnit::DEGREE);
        m_nYRotation = m_xMFYRotation->get_value(FieldUnit::DEGREE);
        m_nZRotation = m_xMFZRotation->get_value(FieldUnit::DEGREE);

        m_xMFXRotation->set_value(static_cast<sal_Int64>(ThreeDHelper::getValueClippedToRange(static_cast<double>(m_nXRotation), ThreeDHelper::getXDegreeAngleLimitForRightAngledAxes())), FieldUnit::DEGREE);
        m_xMFYRotation->set_value(static_cast<sal_Int64>(ThreeDHelper::getValueClippedToRange(static_cast<double>(m_nYRotation), ThreeDHelper::getYDegreeAngleLimitForRightAngledAxes())), FieldUnit::DEGREE);
        m_xMFZRotation->set_text("");

        lcl_SetMetricFieldLimits( *m_xMFXRotation, static_cast<sal_Int64>(ThreeDHelper::getXDegreeAngleLimitForRightAngledAxes()));
        lcl_SetMetricFieldLimits( *m_xMFYRotation, static_cast<sal_Int64>(ThreeDHelper::getYDegreeAngleLimitForRightAngledAxes()));
    }
    else
    {
        lcl_SetMetricFieldLimits( *m_xMFXRotation, 180 );
        lcl_SetMetricFieldLimits( *m_xMFYRotation, 180 );

        m_xMFXRotation->set_value(m_nXRotation, FieldUnit::DEGREE);
        m_xMFYRotation->set_value(m_nYRotation, FieldUnit::DEGREE);
        m_xMFZRotation->set_value(m_nZRotation, FieldUnit::DEGREE);
    }

    if (m_xDiagram)
        m_xDiagram->switchRightAngledAxes( m_xCbxRightAngledAxes->get_active() );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
