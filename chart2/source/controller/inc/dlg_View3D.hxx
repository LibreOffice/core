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
#ifndef _CHART2_DLG_VIEW3D_DIAGRAM_HXX
#define _CHART2_DLG_VIEW3D_DIAGRAM_HXX

#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/frame/XModel.hpp>

// header for class TabDialog
#include <vcl/tabdlg.hxx>
// header for class TabControl
#include <vcl/tabctrl.hxx>
// header for class OKButton
#include <vcl/button.hxx>
// header for class XColorList
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
    View3DDialog( Window* pWindow,
                  const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xChartModel,
                  const XColorListRef &pColorTable );
    ~View3DDialog();

    // from Dialog (base of TabDialog)
    virtual short Execute();

private:
    TabControl      m_aTabControl;
    OKButton        m_aBtnOK;
    CancelButton    m_aBtnCancel;
    HelpButton      m_aBtnHelp;

    ThreeD_SceneGeometry_TabPage*       m_pGeometry;
    ThreeD_SceneAppearance_TabPage*     m_pAppearance;
    ThreeD_SceneIllumination_TabPage*   m_pIllumination;

    ControllerLockHelper                m_aControllerLocker;

    static sal_uInt16 m_nLastPageId;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
