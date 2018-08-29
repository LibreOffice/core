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

#include <sfx2/styledlg.hxx>
#include <sfx2/mgetempl.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/sfxuno.hxx>

#include <sfx2/strings.hrc>

// class SfxStyleDialog --------------------------------------------------

SfxStyleDialog::SfxStyleDialog
(
    vcl::Window* pParent,           // Parent
    const OUString& rID, const OUString& rUIXMLDescription,
    SfxStyleSheetBase& rStyle  // stylesheet to be processed
)

/*  [Description]

    Constructor: Add Manage TabPage, set ExampleSet from style.
*/

    : SfxTabDialog(pParent, rID, rUIXMLDescription,
                  &rStyle.GetItemSet(), true)
    , pStyle( &rStyle )

{
    // without ParentSupport suppress the standardButton
    if (!rStyle.HasParentSupport())
        RemoveStandardButton();

    m_nOrganizerId = AddTabPage("organizer", SfxManageStyleSheetPage::Create, nullptr);

    // With new template always set the management page as the current page

    if( rStyle.GetName().isEmpty() )
        SetCurPageId(m_nOrganizerId);
    else
    {
        OUString sTxt = GetText() + ": " + rStyle.GetName();
        SetText( sTxt );
    }
    delete m_pExampleSet; // in SfxTabDialog::Ctor() already created
    m_pExampleSet = &pStyle->GetItemSet();

    GetCancelButton().SetClickHdl( LINK(this, SfxStyleDialog, CancelHdl) );
}


SfxStyleDialog::~SfxStyleDialog()
{
    disposeOnce();
}

/*  [Description]

    Destructor: set ExampleSet to NULL, so that SfxTabDialog does not delete
    the Set from Style.
*/

void SfxStyleDialog::dispose()
{
    m_pExampleSet = nullptr;
    pStyle = nullptr;
    SfxTabDialog::dispose();
}


short SfxStyleDialog::Ok()

/*  [Description]

    Override so that always RET_OK is returned.
*/

{
    SfxTabDialog::Ok();
    return RET_OK;
}


IMPL_LINK_NOARG( SfxStyleDialog, CancelHdl, Button *, void )

/*  [Description]

    If the dialogue was canceled, then all selected attributes must be reset
    again.
*/

{
    SfxTabPage* pPage = GetTabPage(m_nOrganizerId);

    const SfxItemSet* pInSet = GetInputSetImpl();
    SfxWhichIter aIter( *pInSet );
    sal_uInt16 nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        SfxItemState eState = pInSet->GetItemState( nWhich, false );

        if ( SfxItemState::DEFAULT == eState )
            m_pExampleSet->ClearItem( nWhich );
        else
            m_pExampleSet->Put( pInSet->Get( nWhich ) );
        nWhich = aIter.NextWhich();
    }

    if ( pPage )
        pPage->Reset( GetInputSetImpl() );
    EndDialog();
}

/*  [Description]

    Constructor: Add Manage TabPage, set ExampleSet from style.
*/
SfxStyleDialogController::SfxStyleDialogController
(
    weld::Window* pParent,           // Parent
    const OUString& rUIXMLDescription, const OString& rID,
    SfxStyleSheetBase& rStyle  // stylesheet to be processed
)
    : SfxTabDialogController(pParent, rUIXMLDescription, rID, &rStyle.GetItemSet(), true)
    , m_rStyle(rStyle)
{
    // without ParentSupport suppress the standardButton
    if (!rStyle.HasParentSupport())
        RemoveStandardButton();

    AddTabPage("organizer", SfxManageStyleSheetPage::Create, nullptr);

    // With new template always set the management page as the current page
    if (rStyle.GetName().isEmpty())
        SetCurPageId("organizer");
    else
    {
        OUString sTxt = m_xDialog->get_title() + ": " + rStyle.GetName();
        m_xDialog->set_title(sTxt);
    }
    m_xExampleSet.reset(&m_rStyle.GetItemSet()); // in SfxTabDialog::Ctor() already created, reset will delete it

    GetCancelButton().connect_clicked(LINK(this, SfxStyleDialogController, CancelHdl));
}

/*  [Description]

    Destructor: set ExampleSet to NULL, so that SfxTabDialog does not delete
    the Set from Style.
*/
SfxStyleDialogController::~SfxStyleDialogController()
{
    m_xExampleSet.release();
}

/*  [Description]

    Override so that always RET_OK is returned.
*/
short SfxStyleDialogController::Ok()
{
    SfxTabDialogController::Ok();
    return RET_OK;
}

/*  [Description]

    If the dialogue was canceled, then all selected attributes must be reset
    again.
*/
IMPL_LINK_NOARG(SfxStyleDialogController, CancelHdl, weld::Button&, void)
{
    SfxTabPage* pPage = GetTabPage("organizer");

    const SfxItemSet* pInSet = GetInputSetImpl();
    SfxWhichIter aIter(*pInSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while (nWhich)
    {
        SfxItemState eState = pInSet->GetItemState(nWhich, false);

        if (SfxItemState::DEFAULT == eState)
            m_xExampleSet->ClearItem(nWhich);
        else
            m_xExampleSet->Put(pInSet->Get(nWhich));
        nWhich = aIter.NextWhich();
    }

    if (pPage)
        pPage->Reset(GetInputSetImpl());

    m_xDialog->response(RET_CANCEL);
}

OUString SfxStyleDialogController::GenerateUnusedName(SfxStyleSheetBasePool &rPool)
{
    OUString aNo(SfxResId(STR_NONAME));
    sal_uInt16 i = 1;
    OUString aNoName = aNo + OUString::number(i);
    while (rPool.Find(aNoName))
    {
        ++i;
        aNoName = aNo + OUString::number(i);
    }
    return aNoName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
