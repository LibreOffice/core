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
#ifndef _MAILCONFIGPAGE_HXX
#define _MAILCONFIGPAGE_HXX

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

    FixedLine       m_aIdentityFL;

    FixedText       m_aDisplayNameFT;
    Edit            m_aDisplayNameED;
    FixedText       m_aAddressFT;
    Edit            m_aAddressED;

    CheckBox        m_aReplyToCB;
    FixedText       m_aReplyToFT;
    Edit            m_aReplyToED;

    FixedLine       m_aSMTPFL;

    FixedText       m_aServerFT;
    Edit            m_aServerED;
    FixedText       m_aPortFT;
    NumericField    m_aPortNF;

    CheckBox        m_aSecureCB;

    PushButton      m_aServerAuthenticationPB;

    FixedLine       m_aSeparatorFL;
    PushButton      m_aTestPB;

    SwMailMergeConfigItem*  m_pConfigItem;

    DECL_LINK(ReplyToHdl, CheckBox*);
    DECL_LINK(AuthenticationHdl, void *);
    DECL_LINK(TestHdl, void *);


public:
    SwMailConfigPage( Window* pParent, const SfxItemSet& rSet );
    ~SwMailConfigPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

};

class SwMailConfigDlg : public SfxNoLayoutSingleTabDialog
{
public:

    SwMailConfigDlg( Window* pParent, SfxItemSet& rSet );
    ~SwMailConfigDlg();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
