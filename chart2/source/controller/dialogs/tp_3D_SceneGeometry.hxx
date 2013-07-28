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
#ifndef _CHART2_TP_3D_SCENE_GEOMETRY_HXX
#define _CHART2_TP_3D_SCENE_GEOMETRY_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <basegfx/matrix/b3dhommatrix.hxx>

// header for class TabPage
#include <vcl/tabpage.hxx>
// header for class FixedLine
#include <vcl/fixed.hxx>
// header for class MetricField
#include <vcl/field.hxx>
// header for class OKButton
#include <vcl/button.hxx>

#include "ControllerLockGuard.hxx"

namespace chart
{

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
    DECL_LINK( RightAngledAxesToggled, void* );

private:
    void fillDialogAnglesFromModel() const;
    void applyAnglesToModel();
    void applyPerspectiveToModel();

    ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > m_xSceneProperties;

    CheckBox        m_aCbxRightAngledAxes;

    FixedText       m_aFtXRotation;
    MetricField     m_aMFXRotation;

    FixedText       m_aFtYRotation;
    MetricField     m_aMFYRotation;

    FixedText       m_aFtZRotation;
    MetricField     m_aMFZRotation;

    CheckBox        m_aCbxPerspective;
    MetricField     m_aMFPerspective;

    //to keep old values when switching to right angled axes
    sal_Int64       m_nXRotation;
    sal_Int64       m_nYRotation;
    sal_Int64       m_nZRotation;

    bool            m_bAngleChangePending;
    bool            m_bPerspectiveChangePending;

    ControllerLockHelper & m_rControllerLockHelper;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
