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
#pragma once

#include <vcl/timer.hxx>
#include <vcl/weld.hxx>

namespace com::sun::star::beans { class XPropertySet; }
namespace chart { class ControllerLockHelper; }

namespace chart
{

class ThreeD_SceneGeometry_TabPage
{
public:
    ThreeD_SceneGeometry_TabPage(weld::Container* pWindow,
                                 const css::uno::Reference< css::beans::XPropertySet > & xSceneProperties,
                                 ControllerLockHelper & rControllerLockHelper);
    ~ThreeD_SceneGeometry_TabPage();

    // has to be called in case the dialog was closed with OK
    void commitPendingChanges();

    // is called by timer to apply changes to model
    DECL_LINK( AngleChanged, Timer *, void);
    // is called immediately when a field changes
    DECL_LINK( AngleEdited, weld::MetricSpinButton&, void );

    // is called by timer to apply changes to model
    DECL_LINK( PerspectiveChanged, Timer *, void);
    // is called immediately when a field changes
    DECL_LINK( PerspectiveEdited, weld::MetricSpinButton&, void );
    DECL_LINK( PerspectiveToggled, weld::ToggleButton&, void );
    DECL_LINK( RightAngledAxesToggled, weld::ToggleButton&, void );

private:
    void applyAnglesToModel();
    void applyPerspectiveToModel();

    css::uno::Reference< css::beans::XPropertySet > m_xSceneProperties;

    Timer           m_aAngleTimer;
    Timer           m_aPerspectiveTimer;

    //to keep old values when switching to right angled axes
    sal_Int64       m_nXRotation;
    sal_Int64       m_nYRotation;
    sal_Int64       m_nZRotation;

    bool            m_bAngleChangePending;
    bool            m_bPerspectiveChangePending;

    ControllerLockHelper & m_rControllerLockHelper;

    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::CheckButton> m_xCbxRightAngledAxes;
    std::unique_ptr<weld::MetricSpinButton> m_xMFXRotation;
    std::unique_ptr<weld::MetricSpinButton> m_xMFYRotation;
    std::unique_ptr<weld::Label> m_xFtZRotation;
    std::unique_ptr<weld::MetricSpinButton> m_xMFZRotation;
    std::unique_ptr<weld::CheckButton> m_xCbxPerspective;
    std::unique_ptr<weld::MetricSpinButton> m_xMFPerspective;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
