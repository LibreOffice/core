/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif

#ifndef _SD_SDRESID_HXX
#include "sdresid.hxx"
#endif

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
        DBG_ERROR( "MasterLayoutDialog::MasterLayoutDialog() - no current page?" );
    }

    switch( mpCurrentPage->GetPageKind() )
    {
    case PK_STANDARD:
    {
        //      aTitle = String( SdResId( STR_MASTER_LAYOUT_TITLE ) );
        maCBHeader.Enable( false );
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

    if( (mpCurrentPage->GetPageKind() != PK_STANDARD) && (mbOldHeader != (bool)maCBHeader.IsChecked() ) )
    {
        if( mbOldHeader )
            remove( PRESOBJ_HEADER );
        else
            create( PRESOBJ_HEADER );
    }

    if( mbOldFooter != (bool)maCBFooter.IsChecked() )
    {
        if( mbOldFooter )
            remove( PRESOBJ_FOOTER );
        else
            create( PRESOBJ_FOOTER );
    }

    if( mbOldDate != (bool)maCBDate.IsChecked() )
    {
        if( mbOldDate )
            remove( PRESOBJ_DATETIME );
        else
            create( PRESOBJ_DATETIME );
    }

    if( mbOldPageNumber != (bool)maCBPageNumber.IsChecked() )
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

    if( pObject && pObject->getParentOfSdrObject() )
    {
        const bool bUndo = mpDoc->IsUndoEnabled();
        if( bUndo )
            mpDoc->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoDeleteObject(*pObject));

        pObject->getParentOfSdrObject()->RemoveObjectFromSdrObjList(pObject->GetNavigationPosition());

        if( !bUndo )
        {
            deleteSdrObjectSafeAndClearPointer(pObject);
        }
    }
}
