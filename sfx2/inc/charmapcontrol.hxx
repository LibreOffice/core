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

#include <sal/config.h>
#include <sfx2/charwin.hxx>
#include <svtools/toolbarmenu.hxx>
#include <deque>

class CharmapPopup;

class SfxCharmapCtrl final : public WeldToolbarPopup
{
public:
    explicit SfxCharmapCtrl(CharmapPopup* pControl, weld::Widget* pParent);
    virtual ~SfxCharmapCtrl() override;

    virtual void GrabFocus() override;

private:
    rtl::Reference<CharmapPopup> m_xControl;

    ScopedVclPtr<VirtualDevice> m_xVirDev;

    SfxCharmapContainer m_aCharmapContents;

    std::unique_ptr<weld::Label> m_xRecentLabel;
    std::unique_ptr<weld::Button> m_xDlgBtn;
    std::unique_ptr<weld::Label> m_xCharInfoLabel;

    DECL_LINK(CharFocusInHdl, SvxCharView*, void);
    DECL_LINK(CharClickHdl, SvxCharView*, void);
    DECL_LINK(OpenDlgHdl, weld::Button&, void);
    DECL_LINK(DlgBtnFocusInHdl, weld::Widget&, void);
    DECL_LINK(UpdateRecentHdl, void*, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
