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

#include <svx/dialogs.hrc>

#include "sdresid.hxx"

#include "strings.hrc"
#include "dialogs.hrc"
#include "masterlayoutdlg.hxx"
#include "drawdoc.hxx"

using namespace ::sd;

MasterLayoutDialog::MasterLayoutDialog( vcl::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage )
    : ModalDialog(pParent, "MasterLayoutDialog", "modules/simpress/ui/masterlayoutdlg.ui")
    , mpDoc(pDoc)
    , mpCurrentPage(pCurrentPage)
{
    get(mpCBDate, "datetime");
    get(mpCBPageNumber, "pagenumber");
    get(mpCBSlideNumber, "slidenumber");
    get(mpCBHeader, "header");
    get(mpCBFooter, "footer");

    if( mpCurrentPage && !mpCurrentPage->IsMasterPage() )
    {
        mpCurrentPage = static_cast<SdPage*>(&(mpCurrentPage->TRG_GetMasterPage()));
    }

    if( mpCurrentPage == nullptr )
    {
        mpCurrentPage = pDoc->GetMasterSdPage( 0, PK_STANDARD );
        OSL_FAIL( "MasterLayoutDialog::MasterLayoutDialog() - no current page?" );
    }

    switch( mpCurrentPage->GetPageKind() )
    {
        case PK_STANDARD:
        {
            mpCBHeader->Enable(false);
            mpCBPageNumber->SetText(mpCBSlideNumber->GetText());
            break;
        }
        case PK_NOTES:
            break;
        case PK_HANDOUT:
            break;
    }

    mbOldHeader = mpCurrentPage->GetPresObj( PRESOBJ_HEADER ) != nullptr;
    mbOldDate = mpCurrentPage->GetPresObj( PRESOBJ_DATETIME ) != nullptr;
    mbOldFooter = mpCurrentPage->GetPresObj( PRESOBJ_FOOTER ) != nullptr;
    mbOldPageNumber = mpCurrentPage->GetPresObj( PRESOBJ_SLIDENUMBER ) != nullptr;

    mpCBHeader->Check( mbOldHeader );
    mpCBDate->Check( mbOldDate );
    mpCBFooter->Check( mbOldFooter );
    mpCBPageNumber->Check( mbOldPageNumber );
}

MasterLayoutDialog::~MasterLayoutDialog()
{
    disposeOnce();
}

void MasterLayoutDialog::dispose()
{
    mpCBDate.clear();
    mpCBPageNumber.clear();
    mpCBSlideNumber.clear();
    mpCBHeader.clear();
    mpCBFooter.clear();
    ModalDialog::dispose();
}

short MasterLayoutDialog::Execute()
{
    if ( ModalDialog::Execute() )
        applyChanges();
    return 1;
}

void MasterLayoutDialog::applyChanges()
{
    mpDoc->BegUndo(GetText());

    if( (mpCurrentPage->GetPageKind() != PK_STANDARD) && (mbOldHeader != (bool) mpCBHeader->IsChecked() ) )
    {
        if( mbOldHeader )
            remove( PRESOBJ_HEADER );
        else
            create( PRESOBJ_HEADER );
    }

    if( mbOldFooter != (bool) mpCBFooter->IsChecked() )
    {
        if( mbOldFooter )
            remove( PRESOBJ_FOOTER );
        else
            create( PRESOBJ_FOOTER );
    }

    if( mbOldDate != (bool) mpCBDate->IsChecked() )
    {
        if( mbOldDate )
            remove( PRESOBJ_DATETIME );
        else
            create( PRESOBJ_DATETIME );
    }

    if( mbOldPageNumber != (bool) mpCBPageNumber->IsChecked() )
    {
        if( mbOldPageNumber )
            remove( PRESOBJ_SLIDENUMBER );
        else
            create( PRESOBJ_SLIDENUMBER );
    }

    mpDoc->EndUndo();
}

void MasterLayoutDialog::create(PresObjKind eKind)
{
    mpCurrentPage->CreateDefaultPresObj(eKind, true);
}

void MasterLayoutDialog::remove( PresObjKind eKind )
{
    mpCurrentPage->DestroyDefaultPresObj(eKind);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
