/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_3D_SceneGeometry.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:42:12 $
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

#include "tp_3D_SceneGeometry.hxx"
#include "tp_3D_SceneGeometry.hrc"
#include "ResId.hxx"
#include "NoWarningThisInCTOR.hxx"
#include "BaseGFXHelper.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _SVX_UNOPRNMS_HXX
#include <svx/unoprnms.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_PROJECTIONMODE_HPP_
#include <com/sun/star/drawing/ProjectionMode.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
//using namespace ::com::sun::star::chart2;

ThreeD_SceneGeometry_TabPage::ThreeD_SceneGeometry_TabPage( Window* pWindow
                , const uno::Reference< beans::XPropertySet > & xSceneProperties
                , ControllerLockHelper & rControllerLockHelper )
                : TabPage       ( pWindow, SchResId( TP_3D_SCENEGEOMETRY ) )
                , m_xSceneProperties( xSceneProperties )
                , m_aFtXRotation    ( this, SchResId( FT_X_ROTATION ) )
                , m_aMFXRotation    ( this, SchResId( MTR_FLD_X_ROTATION ) )
                , m_aFtYRotation    ( this, SchResId( FT_Y_ROTATION ) )
                , m_aMFYRotation    ( this, SchResId( MTR_FLD_Y_ROTATION ) )
                , m_aFtZRotation    ( this, SchResId( FT_Z_ROTATION ) )
                , m_aMFZRotation    ( this, SchResId( MTR_FLD_Z_ROTATION ) )
                , m_aCbxPerspective ( this, SchResId( CBX_PERSPECTIVE ) )
                , m_aMFPerspective  ( this, SchResId( MTR_FLD_PERSPECTIVE ) )
                , m_bAngleChangePending( false )
                , m_bPerspectiveChangePending( false )
                , m_rControllerLockHelper( rControllerLockHelper )
{
    FreeResource();

    double fXAngle, fYAngle, fZAngle;
    DiagramHelper::getRotationAngleFromDiagram( m_xSceneProperties, fXAngle, fYAngle, fZAngle );

    fXAngle = BaseGFXHelper::Rad2Deg( fXAngle );
    fYAngle = BaseGFXHelper::Rad2Deg( fYAngle );
    fZAngle = BaseGFXHelper::Rad2Deg( fZAngle );

    shiftAngleToValidRange( fXAngle );
    shiftAngleToValidRange( fYAngle );
    shiftAngleToValidRange( fZAngle );

    m_aMFXRotation.SetValue(::basegfx::fround(fXAngle*pow(10.0,m_aMFXRotation.GetDecimalDigits())));
    m_aMFYRotation.SetValue(::basegfx::fround(-1.0*fYAngle*pow(10.0,m_aMFYRotation.GetDecimalDigits())));
    m_aMFZRotation.SetValue(::basegfx::fround(-1.0*fZAngle*pow(10.0,m_aMFZRotation.GetDecimalDigits())));

    const ULONG nTimeout = 4*EDIT_UPDATEDATA_TIMEOUT;
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
    m_xSceneProperties->getPropertyValue( C2U("D3DScenePerspective")) >>= aProjectionMode;
    m_aCbxPerspective.Check( aProjectionMode == drawing::ProjectionMode_PERSPECTIVE );
    m_aCbxPerspective.SetToggleHdl( LINK( this, ThreeD_SceneGeometry_TabPage, PerspectiveToggled ));

    m_aMFPerspective.SetValue( ::basegfx::fround( CameraDistanceToPerspective(
        DiagramHelper::getCameraDistance( m_xSceneProperties ) ) ) );

    m_aMFPerspective.EnableUpdateData( nTimeout );
    m_aMFPerspective.SetUpdateDataHdl( LINK( this, ThreeD_SceneGeometry_TabPage, PerspectiveChanged ) );
    m_aMFPerspective.SetModifyHdl( LINK( this, ThreeD_SceneGeometry_TabPage, PerspectiveEdited ) );
    m_aMFPerspective.Enable( m_aCbxPerspective.IsChecked() );
}

ThreeD_SceneGeometry_TabPage::~ThreeD_SceneGeometry_TabPage()
{
}

void ThreeD_SceneGeometry_TabPage::shiftAngleToValidRange( double& rfAngleDegree )
{
    //valid range:  ]-180,180]
    while( rfAngleDegree<=-180.0 )
        rfAngleDegree+=360.0;
    while( rfAngleDegree>180.0 )
        rfAngleDegree-=360.0;
}

void ThreeD_SceneGeometry_TabPage::commitPendingChanges()
{
    if( m_bAngleChangePending )
        applyAnglesToModel();
    if( m_bPerspectiveChangePending )
        applyPerspectiveToModel();
}

void ThreeD_SceneGeometry_TabPage::applyAnglesToModel()
{
    double fXAngle = 0.0, fYAngle = 0.0, fZAngle = 0.0;

    fXAngle = double(m_aMFXRotation.GetValue())/double(pow(10.0,m_aMFXRotation.GetDecimalDigits()));
    fYAngle = double(-1.0*m_aMFYRotation.GetValue())/double(pow(10.0,m_aMFYRotation.GetDecimalDigits()));
    fZAngle = double(-1.0*m_aMFZRotation.GetValue())/double(pow(10.0,m_aMFZRotation.GetDecimalDigits()));

    fXAngle = BaseGFXHelper::Deg2Rad( fXAngle );
    fYAngle = BaseGFXHelper::Deg2Rad( fYAngle );
    fZAngle = BaseGFXHelper::Deg2Rad( fZAngle );

    DiagramHelper::setRotationAngleToDiagram( m_xSceneProperties, fXAngle, fYAngle, fZAngle );

    m_bAngleChangePending = false;
}

IMPL_LINK( ThreeD_SceneGeometry_TabPage, AngleEdited, void*, EMPTYARG )
{
    m_bAngleChangePending = true;
    return 0;
}

IMPL_LINK( ThreeD_SceneGeometry_TabPage, AngleChanged, void*, EMPTYARG )
{
    applyAnglesToModel();
    return 0;
}

double ThreeD_SceneGeometry_TabPage::CameraDistanceToPerspective( double fCameraDistance )
{
    double fRet = fCameraDistance;
    double fMin, fMax;
    DiagramHelper::getCameraDistanceRange( fMin, fMax );
    //fMax <-> 0; fMin <->100
    //a/x + b = y
    double a = 100.0*fMax*fMin/(fMax-fMin);
    double b = -a/fMax;

    fRet = a/fCameraDistance + b;

    return fRet;
}
double ThreeD_SceneGeometry_TabPage::PerspectiveToCameraDistance( double fPerspective )
{
    double fRet = fPerspective;
    double fMin, fMax;
    DiagramHelper::getCameraDistanceRange( fMin, fMax );
    //fMax <-> 0; fMin <->100
    //a/x + b = y
    double a = 100.0*fMax*fMin/(fMax-fMin);
    double b = -a/fMax;

    fRet = a/(fPerspective - b);

    return fRet;
}

void ThreeD_SceneGeometry_TabPage::applyPerspectiveToModel()
{
    drawing::ProjectionMode aMode = m_aCbxPerspective.IsChecked()
        ? drawing::ProjectionMode_PERSPECTIVE
        : drawing::ProjectionMode_PARALLEL;

    try
    {
        m_xSceneProperties->setPropertyValue( C2U("D3DScenePerspective"), uno::makeAny( aMode ));

        DiagramHelper::setCameraDistance( m_xSceneProperties
            , PerspectiveToCameraDistance( m_aMFPerspective.GetValue() ) );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    m_bPerspectiveChangePending = false;
}

IMPL_LINK( ThreeD_SceneGeometry_TabPage, PerspectiveEdited, void*, EMPTYARG )
{
    m_bPerspectiveChangePending = true;
    return 0;
}

IMPL_LINK( ThreeD_SceneGeometry_TabPage, PerspectiveChanged, void*, EMPTYARG )
{
    applyPerspectiveToModel();
    return 0;
}

IMPL_LINK( ThreeD_SceneGeometry_TabPage, PerspectiveToggled, void*, EMPTYARG )
{
    m_aMFPerspective.Enable( m_aCbxPerspective.IsChecked() );
    applyPerspectiveToModel();
    return 0;
}

//.............................................................................
} //namespace chart
//.............................................................................
