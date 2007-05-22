/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_3D_SceneGeometry.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:42:31 $
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
#ifndef _CHART2_TP_3D_SCENE_GEOMETRY_HXX
#define _CHART2_TP_3D_SCENE_GEOMETRY_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

// header for class TabPage
#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
// header for class FixedLine
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for class MetricField
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
// header for class OKButton
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

#include "ControllerLockGuard.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

class ThreeD_SceneGeometry_TabPage : public TabPage
{
public:
    ThreeD_SceneGeometry_TabPage( Window* pWindow,
                                  const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::beans::XPropertySet > & xSceneProperties,
                                  ControllerLockHelper & rControllerLockHelper );
    virtual ~ThreeD_SceneGeometry_TabPage();

    // has to be called in case the dialog was closed with OK
    void commitPendingChanges();

    // is called by timer to apply changes to model
    DECL_LINK( AngleChanged, void* );
    // is called immediately when a field changes
    DECL_LINK( AngleEdited, void* );

    // is called by timer to apply changes to model
    DECL_LINK( PerspectiveChanged, void* );
    // is called immediately when a field changes
    DECL_LINK( PerspectiveEdited, void* );
    DECL_LINK( PerspectiveToggled, void* );

private:
    void fillDialogAnglesFromModel() const;
    void applyAnglesToModel();
    void applyPerspectiveToModel();

    double CameraDistanceToPerspective( double fCameraDistance );
    double PerspectiveToCameraDistance( double fPerspective );

    void shiftAngleToValidRange( double& rfAngleDegree );

    ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > m_xSceneProperties;

    FixedText       m_aFtXRotation;
    MetricField     m_aMFXRotation;

    FixedText       m_aFtYRotation;
    MetricField     m_aMFYRotation;

    FixedText       m_aFtZRotation;
    MetricField     m_aMFZRotation;

    CheckBox        m_aCbxPerspective;
    MetricField     m_aMFPerspective;

    bool            m_bAngleChangePending;
    bool            m_bPerspectiveChangePending;

    ControllerLockHelper & m_rControllerLockHelper;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
