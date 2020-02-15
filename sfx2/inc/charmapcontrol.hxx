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
#include <sfx2/dllapi.h>
#include <sfx2/charwin.hxx>
#include <svtools/toolbarmenu.hxx>
#include <deque>

class CharmapPopup;

namespace com::sun::star::frame { class XFrame; }

class SfxCharmapCtrl final : public WeldToolbarPopup
{
public:
    explicit SfxCharmapCtrl(CharmapPopup* pControl, weld::Widget* pParent);
    virtual ~SfxCharmapCtrl() override;

    virtual void GrabFocus() override;

private:
    rtl::Reference<CharmapPopup> m_xControl;

    ScopedVclPtr<VirtualDevice> m_xVirDev;

    std::deque<OUString>   m_aRecentCharList;
    std::deque<OUString>   m_aRecentCharFontList;
    std::deque<OUString>   m_aFavCharList;
    std::deque<OUString>   m_aFavCharFontList;

    SvxCharView m_aRecentCharView[16];
    SvxCharView m_aFavCharView[16];
    std::unique_ptr<weld::Button> m_xDlgBtn;
    std::unique_ptr<weld::CustomWeld> m_xRecentCharView[16];
    std::unique_ptr<weld::CustomWeld> m_xFavCharView[16];

    DECL_LINK(CharClickHdl, SvxCharView*, void);
    DECL_LINK(OpenDlgHdl, weld::Button&, void);

    void            getFavCharacterList();
    void            updateFavCharControl();

    void            getRecentCharacterList(); //gets both recent char and recent char font list
    void            updateRecentCharControl();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
