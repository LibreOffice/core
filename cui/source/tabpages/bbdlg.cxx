/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------
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
    // Umschalten zwischen Farbe/Grafik ermoeglichen:

    if ( bEnableBackgroundSelector && (RID_SVXPAGE_BACKGROUND == nPageId) )
        ((SvxBackgroundTabPage&)rTabPage).ShowSelector( );
}


