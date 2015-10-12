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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_VIEW3D_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_VIEW3D_HXX

#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <vcl/tabdlg.hxx>
#include <vcl/tabctrl.hxx>
#include <svx/xtable.hxx>

#include "ControllerLockGuard.hxx"

namespace chart
{
class ThreeD_SceneGeometry_TabPage;
class ThreeD_SceneAppearance_TabPage;
class ThreeD_SceneIllumination_TabPage;

class View3DDialog : public TabDialog
{
public:
    View3DDialog( vcl::Window* pWindow,
                  const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xChartModel,
                  const XColorListRef &pColorTable );
    virtual ~View3DDialog();
    virtual void dispose() override;

    // from Dialog (base of TabDialog)
    virtual short Execute() override;

private:
    VclPtr<TabControl>     m_pTabControl;

    VclPtr<ThreeD_SceneGeometry_TabPage>       m_pGeometry;
    VclPtr<ThreeD_SceneAppearance_TabPage>     m_pAppearance;
    VclPtr<ThreeD_SceneIllumination_TabPage>   m_pIllumination;

    ControllerLockHelper                m_aControllerLocker;

    static sal_uInt16 m_nLastPageId;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
