/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include <svx/dialogs.hrc>

#include "sdresid.hxx"

#include "strings.hrc"
#include "dialogs.hrc"
#include "masterlayoutdlg.hxx"
#include "masterlayoutdlg.hrc"
#include "drawdoc.hxx"

using namespace ::sd;

MasterLayoutDialog::MasterLayoutDialog( Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage )
:   ModalDialog( pParent, SdResId( RID_SD_DLG_MASTER_LAYOUT ) ),
    mpDoc( pDoc ),
    mpCurrentPage( pCurrentPage ),
    maFLPlaceholders( this, SdResId( FL_PLACEHOLDERS ) ),
    maCBDate( this, SdResId( CB_DATE ) ),
    maCBPageNumber( this, SdResId( CB_PAGE_NUMBER ) ),
    maCBHeader( this, SdResId( CB_HEADER ) ),
    maCBFooter( this, SdResId( CB_FOOTER ) ),
    maPBOK( this, SdResId( BT_OK ) ),
    maPBCancel( this, SdResId( BT_CANCEL ) )
{
    if( mpCurrentPage && !mpCurrentPage->IsMasterPage() )
    {
        mpCurrentPage = (SdPage*)(&(mpCurrentPage->TRG_GetMasterPage()));
    }

    if( mpCurrentPage == 0 )
    {
        mpCurrentPage = pDoc->GetMasterSdPage( 0, PK_STANDARD );
        OSL_FAIL( "MasterLayoutDialog::MasterLayoutDialog() - no current page?" );
    }

    switch( mpCurrentPage->GetPageKind() )
    {
    case PK_STANDARD:
    {
        //      aTitle = String( SdResId( STR_MASTER_LAYOUT_TITLE ) );
        maCBHeader.Enable( sal_False );
    String aSlideNumberStr( SdResId( STR_SLIDE_NUMBER ) );
        maCBPageNumber.SetText( aSlideNumberStr );
        break;
    }
    case PK_NOTES:
        //      aTitle = String( SdResId( STR_NOTES_MASTER_LAYOUT_TITLE ) );
        break;
    case PK_HANDOUT:
        //      aTitle = String( SdResId( STR_HANDOUT_TEMPLATE_LAYOUT_TITLE ) );
        break;
    }
    String aTitle (SdResId( STR_MASTER_LAYOUT_TITLE ) );

    SetText( aTitle );

    FreeResource();

    mbOldHeader = mpCurrentPage->GetPresObj( PRESOBJ_HEADER ) != NULL;
    mbOldDate = mpCurrentPage->GetPresObj( PRESOBJ_DATETIME ) != NULL;
    mbOldFooter = mpCurrentPage->GetPresObj( PRESOBJ_FOOTER ) != NULL;
    mbOldPageNumber = mpCurrentPage->GetPresObj( PRESOBJ_SLIDENUMBER ) != NULL;

    maCBHeader.Check( mbOldHeader );
    maCBDate.Check( mbOldDate );
    maCBFooter.Check( mbOldFooter );
    maCBPageNumber.Check( mbOldPageNumber );
}

MasterLayoutDialog::~MasterLayoutDialog()
{
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

    if( (mpCurrentPage->GetPageKind() != PK_STANDARD) && (mbOldHeader != maCBHeader.IsChecked() ) )
    {
        if( mbOldHeader )
            remove( PRESOBJ_HEADER );
        else
            create( PRESOBJ_HEADER );
    }

    if( mbOldFooter != maCBFooter.IsChecked() )
    {
        if( mbOldFooter )
            remove( PRESOBJ_FOOTER );
        else
            create( PRESOBJ_FOOTER );
    }

    if( mbOldDate != maCBDate.IsChecked() )
    {
        if( mbOldDate )
            remove( PRESOBJ_DATETIME );
        else
            create( PRESOBJ_DATETIME );
    }

    if( mbOldPageNumber != maCBPageNumber.IsChecked() )
    {
        if( mbOldPageNumber )
            remove( PRESOBJ_SLIDENUMBER );
        else
            create( PRESOBJ_SLIDENUMBER );
    }

    mpDoc->EndUndo();
}

void MasterLayoutDialog::create( PresObjKind eKind )
{
    mpCurrentPage->CreateDefaultPresObj( eKind, true );
}

void MasterLayoutDialog::remove( PresObjKind eKind )
{
    SdrObject* pObject = mpCurrentPage->GetPresObj( eKind );

    if( pObject )
    {
        const bool bUndo = mpDoc->IsUndoEnabled();
        if( bUndo )
            mpDoc->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoDeleteObject(*pObject));
        SdrObjList* pOL =pObject->GetObjList();
        sal_uInt32 nOrdNum=pObject->GetOrdNumDirect();
        pOL->RemoveObject(nOrdNum);

        if( !bUndo )
            SdrObject::Free(pObject);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
