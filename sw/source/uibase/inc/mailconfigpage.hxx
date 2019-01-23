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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_MAILCONFIGPAGE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_MAILCONFIGPAGE_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/basedlgs.hxx>

class SwTestAccountSettingsDialog;
class SwMailMergeConfigItem;

class SwMailConfigPage : public SfxTabPage
{
    friend class SwTestAccountSettingsDialog;

    std::unique_ptr<SwMailMergeConfigItem>  m_pConfigItem;

    std::unique_ptr<weld::Entry> m_xDisplayNameED;
    std::unique_ptr<weld::Entry> m_xAddressED;
    std::unique_ptr<weld::CheckButton> m_xReplyToCB;
    std::unique_ptr<weld::Label> m_xReplyToFT;
    std::unique_ptr<weld::Entry> m_xReplyToED;
    std::unique_ptr<weld::Entry> m_xServerED;
    std::unique_ptr<weld::SpinButton> m_xPortNF;
    std::unique_ptr<weld::CheckButton> m_xSecureCB;
    std::unique_ptr<weld::Button> m_xServerAuthenticationPB;
    std::unique_ptr<weld::Button> m_xTestPB;

    DECL_LINK(ReplyToHdl, weld::ToggleButton&, void);
    DECL_LINK(AuthenticationHdl, weld::Button&, void);
    DECL_LINK(TestHdl, weld::Button&, void);
    DECL_LINK(SecureHdl, weld::ToggleButton&, void);

public:
    SwMailConfigPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SwMailConfigPage() override;
    virtual void        dispose() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent,
                                     const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

class SwMailConfigDlg : public SfxSingleTabDialogController
{
public:
    SwMailConfigDlg(weld::Window* pParent, SfxItemSet& rSet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
