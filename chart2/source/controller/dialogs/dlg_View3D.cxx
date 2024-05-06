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

#include <dlg_View3D.hxx>
#include <strings.hrc>
#include <ResId.hxx>
#include "tp_3D_SceneGeometry.hxx"
#include "tp_3D_SceneAppearance.hxx"
#include "tp_3D_SceneIllumination.hxx"
#include <ChartModelHelper.hxx>
#include <ChartModel.hxx>
#include <Diagram.hxx>

namespace chart
{

using namespace ::com::sun::star;

sal_uInt16 View3DDialog::m_nLastPageId = 0;

View3DDialog::View3DDialog(weld::Window* pParent, const rtl::Reference<::chart::ChartModel> & xChartModel)
    : GenericDialogController(pParent, u"modules/schart/ui/3dviewdialog.ui"_ustr, u"3DViewDialog"_ustr)
    , m_aControllerLocker(xChartModel)
    , m_xTabControl(m_xBuilder->weld_notebook(u"tabcontrol"_ustr))
{
    rtl::Reference< Diagram > xSceneProperties = xChartModel->getFirstChartDiagram();

    m_xTabControl->append_page(u"geometry"_ustr, SchResId(STR_PAGE_PERSPECTIVE));
    m_xGeometry.reset(new ThreeD_SceneGeometry_TabPage(m_xTabControl->get_page(u"geometry"_ustr), xSceneProperties, m_aControllerLocker));

    m_xTabControl->append_page(u"appearance"_ustr, SchResId(STR_PAGE_APPEARANCE));
    m_xAppearance.reset(new ThreeD_SceneAppearance_TabPage(m_xTabControl->get_page(u"appearance"_ustr), xChartModel, m_aControllerLocker));

    m_xTabControl->append_page(u"illumination"_ustr, SchResId(STR_PAGE_ILLUMINATION));
    m_xIllumination.reset(new ThreeD_SceneIllumination_TabPage(m_xTabControl->get_page(u"illumination"_ustr), m_xDialog.get(),
        xSceneProperties, xChartModel));

    m_xTabControl->connect_enter_page(LINK(this, View3DDialog, ActivatePageHdl));

    m_xTabControl->set_current_page(m_nLastPageId);
}

IMPL_LINK(View3DDialog, ActivatePageHdl, const OUString&, rPage, void)
{
    if (rPage == "appearance")
        m_xAppearance->ActivatePage();
}

View3DDialog::~View3DDialog()
{
    m_nLastPageId = m_xTabControl->get_current_page();
}

short View3DDialog::run()
{
    short nResult = GenericDialogController::run();
    if (nResult == RET_OK && m_xGeometry)
        m_xGeometry->commitPendingChanges();
    return nResult;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
