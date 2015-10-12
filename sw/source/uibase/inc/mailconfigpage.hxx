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
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/basedlgs.hxx>

class SwTestAccountSettingsDialog;
class SwMailMergeConfigItem;

class SwMailConfigPage : public SfxTabPage
{
    friend class SwTestAccountSettingsDialog;

    VclPtr<Edit>            m_pDisplayNameED;
    VclPtr<Edit>            m_pAddressED;

    VclPtr<CheckBox>        m_pReplyToCB;
    VclPtr<FixedText>        m_pReplyToFT;
    VclPtr<Edit>            m_pReplyToED;

    VclPtr<Edit>            m_pServerED;
    VclPtr<NumericField>    m_pPortNF;

    VclPtr<CheckBox>        m_pSecureCB;

    VclPtr<PushButton>      m_pServerAuthenticationPB;
    VclPtr<PushButton>      m_pTestPB;

    SwMailMergeConfigItem*  m_pConfigItem;

    DECL_LINK_TYPED(ReplyToHdl, Button*, void);
    DECL_LINK_TYPED(AuthenticationHdl, Button*, void);
    DECL_LINK_TYPED(TestHdl, Button*, void);

public:
    SwMailConfigPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SwMailConfigPage();
    virtual void        dispose() override;

    static VclPtr<SfxTabPage> Create( vcl::Window* pParent,
                                      const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

};

class SwMailConfigDlg : public SfxSingleTabDialog
{
public:
    SwMailConfigDlg(vcl::Window* pParent, SfxItemSet& rSet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
