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

#include "res_Titles.hxx"
#include <vcl/weld.hxx>
#include <memory>
#include "ChartController.hxx"

namespace chart
{
class SchThemeDlg final : public weld::GenericDialogController
{
private:
    rtl::Reference<ChartModel> mxModel;
    ChartController* mpController;

    std::unique_ptr<weld::IconView> mxThemeIconView;
    std::unique_ptr<weld::Button> mxSaveTheme;
    std::unique_ptr<weld::Button> mxLoadTheme;
    std::unique_ptr<weld::Button> mxDeleteTheme;
    std::unique_ptr<weld::Button> mxSaveToNewTheme;

    DECL_LINK(ClickSaveHdl, weld::Button&, void);
    DECL_LINK(ClickLoadHdl, weld::Button&, void);
    DECL_LINK(ClickDeleteHdl, weld::Button&, void);
    DECL_LINK(ClickSaveToNewHdl, weld::Button&, void);
    DECL_LINK(ThemeSelectedHdl, weld::IconView&, void);

    VclPtr<VirtualDevice> makeImage(int nIndex);

public:
    SchThemeDlg(weld::Window* pParent, ChartController* pController);
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
