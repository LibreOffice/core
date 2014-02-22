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

#include <svx/linkwarn.hxx>
#include <svtools/miscopt.hxx>

SvxLinkWarningDialog::SvxLinkWarningDialog( Window* pParent, const OUString& _rFileName )
    : MessageDialog(pParent, "LinkWarnDialog" , "svx/ui/linkwarndialog.ui")
{
    get(m_pWarningOnBox, "ask");

    
    OUString sInfoText = get_primary_text();
    sInfoText = sInfoText.replaceAll("%FILENAME", _rFileName);
    set_primary_text( sInfoText );

    
    SvtMiscOptions aMiscOpt;
    m_pWarningOnBox->Check( aMiscOpt.ShowLinkWarningDialog() == sal_True );
    if( aMiscOpt.IsShowLinkWarningDialogReadOnly() )
        m_pWarningOnBox->Disable();
}

SvxLinkWarningDialog::~SvxLinkWarningDialog()
{
    
    SvtMiscOptions aMiscOpt;
    sal_Bool bChecked = m_pWarningOnBox->IsChecked();
    if ( aMiscOpt.ShowLinkWarningDialog() != bChecked )
        aMiscOpt.SetShowLinkWarningDialog( bChecked );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
