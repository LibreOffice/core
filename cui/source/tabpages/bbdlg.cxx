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

#include <tools/shl.hxx>
#include <svx/dialogs.hrc>
#include <cuires.hrc>
#include "page.hrc"

#include "bbdlg.hxx"
#include "border.hxx"
#include "backgrnd.hxx"
#include <dialmgr.hxx>

// class SvxBorderBackgroundDlg ------------------------------------------

SvxBorderBackgroundDlg::SvxBorderBackgroundDlg( Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                sal_Bool bEnableSelector ) :

    SfxTabDialog( pParent, CUI_RES( RID_SVXDLG_BBDLG ), &rCoreSet ),
    bEnableBackgroundSelector( bEnableSelector )
{
    FreeResource();
    AddTabPage( RID_SVXPAGE_BORDER, SvxBorderTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_BACKGROUND, SvxBackgroundTabPage::Create, 0 );
}

// -----------------------------------------------------------------------

SvxBorderBackgroundDlg::~SvxBorderBackgroundDlg()
{
}

// -----------------------------------------------------------------------

void SvxBorderBackgroundDlg::PageCreated( sal_uInt16 nPageId, SfxTabPage& rTabPage )
{
    // Make it possible to switch between color/graphic:
    if ( bEnableBackgroundSelector && (RID_SVXPAGE_BACKGROUND == nPageId) )
        ((SvxBackgroundTabPage&)rTabPage).ShowSelector( );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
