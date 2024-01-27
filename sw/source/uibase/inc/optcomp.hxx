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

#include <map>
#include <memory>

#include <sfx2/tabdlg.hxx>
#include <unotools/compatibility.hxx>
#include <rtl/ustring.hxx>

class SwWrtShell;

class SwCompatibilityOptPage final : public SfxTabPage
{
private:
    // shell of the current document
    SwWrtShell*             m_pWrtShell;
    // saved options after "Reset"; used in "FillItemSet" for comparison
    std::map<OUString, TriState> m_aSavedOptions;

    // controls
    std::unique_ptr<weld::Frame> m_xMain;
    std::unique_ptr<weld::TreeView> m_xOptionsLB;
    std::unique_ptr<weld::Button> m_xDefaultPB;

    // handler
    DECL_LINK(UseAsDefaultHdl, weld::Button&, void);

    // private methods
    void                    InitControls( const SfxItemSet& rSet );
    void                    SetCurrentOptions();

public:
    SwCompatibilityOptPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~SwCompatibilityOptPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );

    virtual OUString GetAllStrings() override;

    virtual bool            FillItemSet( SfxItemSet* rSet ) override;
    virtual void            Reset( const SfxItemSet* rSet ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
