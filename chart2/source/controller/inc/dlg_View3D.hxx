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

#include <vcl/weld.hxx>
#include <ControllerLockGuard.hxx>

namespace com::sun::star::frame { class XModel; }

namespace chart
{
class ThreeD_SceneGeometry_TabPage;
class ThreeD_SceneAppearance_TabPage;
class ThreeD_SceneIllumination_TabPage;

class View3DDialog : public weld::GenericDialogController
{
public:
    View3DDialog(weld::Window* pWindow, const css::uno::Reference<css::frame::XModel> & xChartModel);
    virtual ~View3DDialog() override;

    virtual short run() override;

private:
    DECL_LINK(ActivatePageHdl, const OString&, void);

    ControllerLockHelper                m_aControllerLocker;

    static sal_uInt16 m_nLastPageId;

    std::unique_ptr<weld::Notebook> m_xTabControl;
    std::unique_ptr<ThreeD_SceneGeometry_TabPage> m_xGeometry;
    std::unique_ptr<ThreeD_SceneAppearance_TabPage> m_xAppearance;
    std::unique_ptr<ThreeD_SceneIllumination_TabPage> m_xIllumination;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
