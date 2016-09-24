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


#include <sal/macros.h>
#include <sfx2/sfxresid.hxx>
#include <sfx2/sfxuno.hxx>
#include <unotools/saveopt.hxx>
#include <vcl/msgbox.hxx>
#include "alienwarn.hxx"

SfxAlienWarningDialog::SfxAlienWarningDialog(vcl::Window* pParent, const OUString& _rFormatName,
                                             const OUString& _rDefaultExtension, bool rDefaultIsAlien)
    : MessageDialog(pParent, "AlienWarnDialog", "sfx/ui/alienwarndialog.ui")
{
    get(m_pWarningOnBox, "ask");
    //fdo#75121, a bit tricky because the widgets we want to align with
    //don't actually exist in the ui description, they're implied
    m_pWarningOnBox->set_margin_left(QueryBox::GetStandardImage().GetSizePixel().Width() + 12);

    get(m_pKeepCurrentBtn, "save");
    get(m_pUseDefaultFormatBtn, "cancel");

    OUString aExtension = "ODF";

    // replace formatname (text)
    OUString sInfoText = get_primary_text();
    sInfoText = sInfoText.replaceAll( "%FORMATNAME", _rFormatName );
    set_primary_text(sInfoText);

    // replace formatname (button)
    sInfoText = m_pKeepCurrentBtn->GetText();
    sInfoText = sInfoText.replaceAll( "%FORMATNAME", _rFormatName );
    m_pKeepCurrentBtn->SetText( sInfoText );

    // hide ODF explanation if default format is alien
    // and set the proper extension in the button
    if( rDefaultIsAlien )
    {
        set_secondary_text(OUString());
        aExtension = _rDefaultExtension.toAsciiUpperCase();
    }

    // replace defaultextension (button)
    sInfoText = m_pUseDefaultFormatBtn->GetText();
    sInfoText = sInfoText.replaceAll( "%DEFAULTEXTENSION", aExtension );
    m_pUseDefaultFormatBtn->SetText( sInfoText );

    // load value of "warning on" checkbox from save options
    m_pWarningOnBox->Check( SvtSaveOptions().IsWarnAlienFormat() );
}

SfxAlienWarningDialog::~SfxAlienWarningDialog()
{
    disposeOnce();
}

void SfxAlienWarningDialog::dispose()
{
    // save value of "warning off" checkbox, if necessary
    SvtSaveOptions aSaveOpt;
    bool bChecked = m_pWarningOnBox->IsChecked();
    if ( aSaveOpt.IsWarnAlienFormat() != bChecked )
        aSaveOpt.SetWarnAlienFormat( bChecked );
    m_pKeepCurrentBtn.clear();
    m_pUseDefaultFormatBtn.clear();
    m_pWarningOnBox.clear();
    MessageDialog::dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
