/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <sal/macros.h>
#include <sfx2/sfxresid.hxx>
#include <sfx2/sfxuno.hxx>
#include <unotools/saveopt.hxx>
#include <vcl/msgbox.hxx>
#include "alienwarn.hxx"

SfxAlienWarningDialog::SfxAlienWarningDialog(Window* pParent, const OUString& _rFormatName)
    : MessageDialog(pParent, "AlienWarnDialog", "sfx/ui/alienwarndialog.ui")
{
    get(m_pWarningOnBox, "ask");
    
    
    m_pWarningOnBox->set_margin_left(QueryBox::GetStandardImage().GetSizePixel().Width() + 12);

    get(m_pKeepCurrentBtn, "save");

    
    OUString sInfoText = get_primary_text();
    sInfoText = sInfoText.replaceAll( "%FORMATNAME", _rFormatName );
    set_primary_text(sInfoText);

    
    sInfoText = m_pKeepCurrentBtn->GetText();
    sInfoText = sInfoText.replaceAll( "%FORMATNAME", _rFormatName );
    m_pKeepCurrentBtn->SetText( sInfoText );

    
    m_pWarningOnBox->Check( SvtSaveOptions().IsWarnAlienFormat() );
}

SfxAlienWarningDialog::~SfxAlienWarningDialog()
{
    
    SvtSaveOptions aSaveOpt;
    bool bChecked = m_pWarningOnBox->IsChecked();
    if ( aSaveOpt.IsWarnAlienFormat() != bChecked )
        aSaveOpt.SetWarnAlienFormat( bChecked );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
