/*************************************************************************
 *
 *  $RCSfile: versdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2000-11-08 14:25:46 $
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

#include <svtools/eitem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/stritem.hxx>
#include <svtools/itemset.hxx>
#include <svtools/useroptions.hxx>
#include <vcl/msgbox.hxx>

#include "versdlg.hrc"
#include "versdlg.hxx"
#include "viewfrm.hxx"
#include "sfxresid.hxx"
#include "docfile.hxx"
#include "objsh.hxx"
#include "sfxsids.hrc"
#include "dispatch.hxx"
#if SUPD<613//MUSTINI
#include "inimgr.hxx"
#endif
#include "request.hxx"

// **************************************************************************

static String ConvertDateTime_Impl(const SfxStamp &rTime)
{
     const String pDelim ( DEFINE_CONST_UNICODE( ", "));
     const International& rInter = Application::GetAppInternational();
     String aStr(rInter.GetDate(rTime.GetTime()));
     aStr += pDelim;
     aStr += rInter.GetTime(rTime.GetTime(), TRUE, FALSE);
     return aStr;
}

// Achtung im Code wird dieses Array direkt (0, 1, ...) indiziert
static long nTabs_Impl[] =
{
    3, // Number of Tabs
    0, 62, 124
};

void SfxVersionsTabListBox_Impl::KeyInput( const KeyEvent& rKeyEvent )
{
    const KeyCode& rCode = rKeyEvent.GetKeyCode();
    switch ( rCode.GetCode() )
    {
        case KEY_RETURN :
        case KEY_ESCAPE :
        case KEY_TAB :
            Window::GetParent()->KeyInput( rKeyEvent );
            break;
        default:
            SvTabListBox::KeyInput( rKeyEvent );
            break;
    }
}

SfxVersionsTabListBox_Impl::SfxVersionsTabListBox_Impl( Window* pParent, const ResId& rResId )
    : SvTabListBox( pParent, rResId )
{
}

SfxVersionDialog::SfxVersionDialog ( SfxViewFrame* pFrame, Window *pParent )
    : SfxModalDialog( pFrame, pParent, SfxResId( DLG_VERSIONS ) )
    , aNewGroup( this, ResId( GB_NEWVERSIONS ) )
    , aSaveButton( this, ResId( PB_SAVE ) )
    , aSaveCheckBox( this, ResId( CB_SAVEONCLOSE ) )
    , aExistingGroup( this, ResId( GB_OLDVERSIONS ) )
    , aDateTimeText( this, ResId( FT_DATETIME ) )
    , aSavedByText( this, ResId( FT_SAVEDBY ) )
    , aCommentText( this, ResId( FT_COMMENTS ) )
    , aVersionBox( this, ResId( TLB_VERSIONS ) )
    , aViewButton( this, ResId( PB_VIEW ) )
    , aOpenButton( this, ResId( PB_OPEN ) )
    , aDeleteButton( this, ResId( PB_DELETE ) )
    , aCompareButton( this, ResId( PB_COMPARE ) )
    , aCloseButton( this, ResId( PB_CLOSE ) )
    , aHelpButton( this, ResId ( PB_HELP ) )
    , pViewFrame( pFrame )
{
    FreeResource();

    aViewButton.SetClickHdl ( LINK( this, SfxVersionDialog, ButtonHdl_Impl ) );
    aSaveButton.SetClickHdl ( LINK( this, SfxVersionDialog, ButtonHdl_Impl ) );
    aDeleteButton.SetClickHdl ( LINK( this, SfxVersionDialog, ButtonHdl_Impl ) );
    aCompareButton.SetClickHdl ( LINK( this, SfxVersionDialog, ButtonHdl_Impl ) );
    aCloseButton.SetClickHdl ( LINK( this, SfxVersionDialog, ButtonHdl_Impl ) );
    aOpenButton.SetClickHdl ( LINK( this, SfxVersionDialog, ButtonHdl_Impl ) );
    aVersionBox.SetSelectHdl( LINK( this, SfxVersionDialog, SelectHdl_Impl ) );
    aVersionBox.SetDoubleClickHdl( LINK( this, SfxVersionDialog, DClickHdl_Impl ) );
    aSaveCheckBox.SetClickHdl ( LINK( this, SfxVersionDialog, ButtonHdl_Impl ) );

    aVersionBox.GrabFocus();
    aVersionBox.SetWindowBits( WB_HSCROLL | WB_CLIPCHILDREN );
    aVersionBox.SetSelectionMode( SINGLE_SELECTION );
    aVersionBox.SetTabs( &nTabs_Impl[0], MAP_APPFONT );
    aVersionBox.Resize();   // OS: Hack fuer richtige Selektion
    Init_Impl();
}

void SfxVersionDialog::Init_Impl()
{
    SfxObjectShell *pObjShell = pViewFrame->GetObjectShell();
    SfxMedium* pMedium = pObjShell->GetMedium();
    const SfxVersionTableDtor* pTable = pMedium->GetVersionList();
    if ( pTable )
    {
        for ( USHORT n=0; n<pTable->Count(); n++ )
        {
            SfxVersionInfo *pInfo = pTable->GetObject(n);
            String aEntry = ConvertDateTime_Impl( pInfo->aCreateStamp );
            aEntry += '\t';
            aEntry += pInfo->aCreateStamp.GetName();
            aEntry += '\t';
            aEntry += pInfo->aComment;
            SvLBoxEntry *pEntry = aVersionBox.InsertEntry( aEntry );
            pEntry->SetUserData( pInfo );
        }
    }

    aSaveCheckBox.Check( pObjShell->GetDocInfo().IsSaveVersionOnClose() );

    aOpenButton.Enable( FALSE );
    aSaveButton.Enable( !pObjShell->IsReadOnly() );
    aSaveCheckBox.Enable( !pObjShell->IsReadOnly() );
    aDeleteButton.Enable( FALSE );

    const SfxPoolItem *pDummy=NULL;
    SfxItemState eState = pViewFrame->GetDispatcher()->QueryState( SID_DOCUMENT_MERGE, pDummy );
    eState = pViewFrame->GetDispatcher()->QueryState( SID_DOCUMENT_COMPARE, pDummy );
    aCompareButton.Enable( eState >= SFX_ITEM_AVAILABLE );

    // set dialog title (filename or docinfo title)
    String sText = GetText();
    ( sText += ' ' ) += pObjShell->GetTitle();
    SetText( sText );
}

SfxVersionDialog::~SfxVersionDialog ()
{
}

void SfxVersionDialog::Open_Impl()
{
    SfxObjectShell *pObjShell = pViewFrame->GetObjectShell();

    SvLBoxEntry *pEntry = aVersionBox.FirstSelected();
    ULONG nPos = aVersionBox.GetModel()->GetRelPos( pEntry );
    SfxInt16Item aItem( SID_VERSION, nPos+1 );
    SfxStringItem aTarget( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") );
    SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE("private:user") );
    SfxStringItem aFile( SID_FILE_NAME, pObjShell->GetMedium()->GetName() );
    pViewFrame->GetDispatcher()->Execute(
        SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, &aFile, &aItem, &aTarget, &aReferer, 0L );
    Close();
}

IMPL_LINK( SfxVersionDialog, DClickHdl_Impl, Control*, pControl )
{
    Open_Impl();
    return 0L;
}

IMPL_LINK( SfxVersionDialog, SelectHdl_Impl, Control*, pControl )
{
    SfxObjectShell *pObjShell = pViewFrame->GetObjectShell();
    SvLBoxEntry *pEntry = aVersionBox.FirstSelected();
    aDeleteButton.Enable( !pObjShell->IsReadOnly() );
    aOpenButton.Enable( TRUE );
    return 0L;
}

IMPL_LINK( SfxVersionDialog, ButtonHdl_Impl, Button*, pButton )
{
    SfxObjectShell *pObjShell = pViewFrame->GetObjectShell();
    SvLBoxEntry *pEntry = aVersionBox.FirstSelected();

    if ( pButton == &aCloseButton )
    {
        Close();
    }
    else if ( pButton == &aSaveCheckBox )
    {
        pObjShell->GetDocInfo().SetSaveVersionOnClose( aSaveCheckBox.IsChecked() );
        pObjShell->SetModified( TRUE );
    }
    else if ( pButton == &aSaveButton )
    {
        SfxVersionInfo aInfo;
#if SUPD<613//MUSTINI
        aInfo.aCreateStamp = SfxStamp( SFX_INIMANAGER()->GetUserFullName() );
#else
        aInfo.aCreateStamp = SfxStamp( SvtUserOptions().GetFullName() );
#endif
        SfxViewVersionDialog_Impl* pDlg = new SfxViewVersionDialog_Impl( this, aInfo, TRUE );
        short nRet = pDlg->Execute();
        if ( nRet == RET_OK )
        {
            SfxStringItem aComment( SID_VERSION, aInfo.aComment );
            pObjShell->SetModified( TRUE );
            pViewFrame->GetDispatcher()->Execute(
                SID_SAVEDOC, SFX_CALLMODE_SYNCHRON, &aComment, 0L );
            aVersionBox.SetUpdateMode( FALSE );
            aVersionBox.Clear();
            Init_Impl();
            aVersionBox.SetUpdateMode( TRUE );
        }

        delete pDlg;
    }
    if ( pButton == &aDeleteButton && pEntry )
    {
        pObjShell->GetMedium()->RemoveVersion_Impl( *(SfxVersionInfo*) pEntry->GetUserData() );
        pObjShell->SetModified( TRUE );
            aVersionBox.SetUpdateMode( FALSE );
        aVersionBox.Clear();
        Init_Impl();
            aVersionBox.SetUpdateMode( TRUE );
    }
    else if ( pButton == &aOpenButton && pEntry )
    {
        Open_Impl();
    }
    else if ( pButton == &aViewButton && pEntry )
    {
        SfxVersionInfo* pInfo = (SfxVersionInfo*) pEntry->GetUserData();
        SfxViewVersionDialog_Impl* pDlg = new SfxViewVersionDialog_Impl( this, *pInfo, FALSE );
        pDlg->Execute();
        delete pDlg;
    }
    else if ( pEntry && pButton == &aCompareButton )
    {
        SfxAllItemSet aSet( pObjShell->GetPool() );
        ULONG nPos = aVersionBox.GetModel()->GetRelPos( pEntry );
        aSet.Put( SfxInt16Item( SID_VERSION, nPos+1 ) );
        aSet.Put( SfxStringItem( SID_FILE_NAME, pObjShell->GetMedium()->GetName() ) );

        SfxItemSet* pSet = pObjShell->GetMedium()->GetItemSet();
        SFX_ITEMSET_ARG( pSet, pFilterItem, SfxStringItem, SID_FILTER_NAME, FALSE );
        SFX_ITEMSET_ARG( pSet, pFilterOptItem, SfxStringItem, SID_FILE_FILTEROPTIONS, FALSE );
        if ( pFilterItem )
            aSet.Put( *pFilterItem );
        if ( pFilterOptItem )
            aSet.Put( *pFilterOptItem );

        pViewFrame->GetDispatcher()->Execute( SID_DOCUMENT_COMPARE, SFX_CALLMODE_ASYNCHRON, aSet );
        Close();
    }

    return 0L;
}

SfxViewVersionDialog_Impl::SfxViewVersionDialog_Impl ( Window *pParent, SfxVersionInfo& rInfo, BOOL bEdit )
    : SfxModalDialog( pParent, SfxResId( DLG_COMMENTS ) )
    , aDateTimeText( this, ResId( FT_DATETIME ) )
    , aSavedByText( this, ResId( FT_SAVEDBY ) )
    , aEdit( this, ResId ( ME_VERSIONS ) )
    , aOKButton( this, ResId( PB_OK ) )
    , aCancelButton( this, ResId( PB_CANCEL ) )
    , aCloseButton( this, ResId( PB_CLOSE ) )
    , aHelpButton( this, ResId ( PB_HELP ) )
    , pInfo( &rInfo )
{
    FreeResource();

    aDateTimeText.SetText( aDateTimeText.GetText().Append(ConvertDateTime_Impl( pInfo->aCreateStamp )) );
    aSavedByText.SetText( aSavedByText.GetText().Append(pInfo->aCreateStamp.GetName()) );
    aEdit.SetText( rInfo.aComment );

    aCloseButton.SetClickHdl ( LINK( this, SfxViewVersionDialog_Impl, ButtonHdl ) );
    aOKButton.SetClickHdl ( LINK( this, SfxViewVersionDialog_Impl, ButtonHdl ) );

    aEdit.GrabFocus();
    if ( !bEdit )
    {
        aOKButton.Hide();
        aCancelButton.Hide();
        aEdit.SetReadOnly( TRUE );
    }
    else
        aCloseButton.Hide();
}

IMPL_LINK( SfxViewVersionDialog_Impl, ButtonHdl, Button*, pButton )
{
    if ( pButton == &aCloseButton )
    {
        EndDialog( RET_CANCEL );
    }
    else if ( pButton == &aOKButton )
    {
        pInfo->aComment = aEdit.GetText();
        EndDialog( RET_OK );
    }

    return 0L;
}


