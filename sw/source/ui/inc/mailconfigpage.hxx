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

    Edit*            m_pDisplayNameED;
    Edit*            m_pAddressED;

    CheckBox*        m_pReplyToCB;
    FixedText*        m_pReplyToFT;
    Edit*            m_pReplyToED;

    Edit*            m_pServerED;
    NumericField*    m_pPortNF;

    CheckBox*        m_pSecureCB;

    PushButton*      m_pServerAuthenticationPB;
    PushButton*      m_pTestPB;

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

class SwMailConfigDlg : public SfxSingleTabDialog
{
public:
    SwMailConfigDlg(Window* pParent, SfxItemSet& rSet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
