/*************************************************************************
 *
 *  $RCSfile: masterlayoutdlg.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:59:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
    maFLPlaceholders( this, SdResId( FL_PLACEHOLDERS ) ),
    maCBHeader( this, SdResId( CB_HEADER ) ),
    maCBDate( this, SdResId( CB_DATE ) ),
    maCBFooter( this, SdResId( CB_FOOTER ) ),
    maCBPageNumber( this, SdResId( CB_PAGE_NUMBER ) ),
    maPBOK( this, SdResId( BT_OK ) ),
    maPBCancel( this, SdResId( BT_CANCEL ) ),
    mpDoc( pDoc ),
    mpCurrentPage( pCurrentPage )
{
    String aTitle;

    if( mpCurrentPage == 0 )
    {
        mpCurrentPage = pDoc->GetMasterSdPage( 0, PK_STANDARD );
        DBG_ERROR( "MasterLayoutDialog::MasterLayoutDialog() - no current page?" );
    }
    else if( !mpCurrentPage->IsMasterPage() )
    {
        mpCurrentPage = (SdPage*)(&(mpCurrentPage->TRG_GetMasterPage()));
    }


    switch( mpCurrentPage->GetPageKind() )
    {
    case PK_STANDARD:
    {
        aTitle = String( SdResId( STR_MASTER_LAYOUT_TITLE ) );
        maCBHeader.Enable( FALSE );
    String aSlideNumberStr( SdResId( STR_SLIDE_NUMBER ) );
        maCBPageNumber.SetText( aSlideNumberStr );
        break;
    }
    case PK_NOTES:
        aTitle = String( SdResId( STR_NOTES_MASTER_LAYOUT_TITLE ) );
        break;
    case PK_HANDOUT:
        aTitle = String( SdResId( STR_HANDOUT_TEMPLATE_LAYOUT_TITLE ) );
        break;
    }

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
    SdrObject* pObject = mpCurrentPage->CreateDefaultPresObj( eKind, true );
    if( pObject )
    {
        mpDoc->AddUndo(new SdrUndoUserCallObj( *pObject, NULL, mpCurrentPage ) );
        mpDoc->AddUndo(new SdrUndoNewObj(*pObject));
    }
}

void MasterLayoutDialog::remove( PresObjKind eKind )
{
    SdrObject* pObject = mpCurrentPage->GetPresObj( eKind );

    if( pObject )
    {
        mpDoc->AddUndo(new SdrUndoDelObj(*pObject));
        SdrObjList* pOL =pObject->GetObjList();
        UINT32 nOrdNum=pObject->GetOrdNumDirect();
        SdrObject* pChkObj=pOL->RemoveObject(nOrdNum);
    }
}
