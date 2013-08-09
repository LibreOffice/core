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

#include <svl/whiter.hxx>
#include <svl/style.hxx>
#include <vcl/msgbox.hxx>

#include <sfx2/styledlg.hxx>
#include <sfx2/mgetempl.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/sfxuno.hxx>

#include "dialog.hrc"

// class SfxStyleDialog --------------------------------------------------

SfxStyleDialog::SfxStyleDialog
(
    Window* pParent,           // Parent
    const OString& rID, const OUString& rUIXMLDescription,
    SfxStyleSheetBase& rStyle  // stylesheet to be processed
)

/*  [Description]

    Constructor: Add Manage TabPage, set ExampleSet from style.
*/

    : SfxTabDialog( pParent, rID, rUIXMLDescription,
                  rStyle.GetItemSet().Clone(),
                  // return TRUE also without ParentSupport , but extended
                  // to suppress the standardButton
                  rStyle.HasParentSupport() ? sal_True : 2 )

    , pStyle( &rStyle )

{
    m_nOrganizerId = AddTabPage("organizer", SfxManageStyleSheetPage::Create, 0);

    // With new template always set the management page as the current page

    if( rStyle.GetName().isEmpty() )
        SetCurPageId(m_nOrganizerId);
    else
    {
        OUString sTxt = OUString(GetText()) + ": " + rStyle.GetName();
        SetText( sTxt );
    }
    delete pExampleSet; // in SfxTabDialog::Ctor() already created
    pExampleSet = &pStyle->GetItemSet();

    GetCancelButton().SetClickHdl( LINK(this, SfxStyleDialog, CancelHdl) );
}

// -----------------------------------------------------------------------

SfxStyleDialog::~SfxStyleDialog()

/*  [Description]

    Destructor: set ExampleSet to NULL, so that SfxTabDialog does not delete
    the Set from Style.
*/

{
    pExampleSet = 0;
    pStyle = 0;
    delete GetInputSetImpl();
}

// -----------------------------------------------------------------------

const SfxItemSet* SfxStyleDialog::GetRefreshedSet()

/*  [Description]

    This is called when <SfxTabPage::DeactivatePage(SfxItemSet *)>
    returns <SfxTabPage::REFRESH_SET>.
*/

{
    return GetInputSetImpl();
}

// -----------------------------------------------------------------------

short SfxStyleDialog::Ok()

/*  [Description]

    Overloaded, so that always RET_OK is returned.
*/

{
    SfxTabDialog::Ok();
    return RET_OK;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxStyleDialog, CancelHdl, Button *, pButton )

/*  [Description]

    If the dialogue was canceled, then all selected attributes must be reset
    again.
*/

{
    (void)pButton; //unused
    SfxTabPage* pPage = GetTabPage(m_nOrganizerId);

    const SfxItemSet* pInSet = GetInputSetImpl();
    SfxWhichIter aIter( *pInSet );
    sal_uInt16 nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        SfxItemState eState = pInSet->GetItemState( nWhich, sal_False );

        if ( SFX_ITEM_DEFAULT == eState )
            pExampleSet->ClearItem( nWhich );
        else
            pExampleSet->Put( pInSet->Get( nWhich ) );
        nWhich = aIter.NextWhich();
    }

    if ( pPage )
        pPage->Reset( *GetInputSetImpl() );
    EndDialog( RET_CANCEL );
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
