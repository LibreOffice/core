/*************************************************************************
 *
 *  $RCSfile: moduldl2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tbe $ $Date: 2000-11-20 08:32:42 $
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


#define GLOBALOVERFLOW

#include <ide_pch.hxx>

#ifdef VCL
#include <svtools/filedlg.hxx>
#endif

#pragma hdrstop

#include <sot/storinfo.hxx>

#include <moduldlg.hrc>
#include <moduldlg.hxx>
#include <basidesh.hrc>
#include <bastypes.hxx>
#include <basobj.hxx>
#include <baside2.hrc>
#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <svx/passwd.hxx>
#include <sbxitem.hxx>

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif

#ifndef _IODLG_HXX //autogen
#include <sfx2/iodlg.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>

using namespace comphelper;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;


LibPage::LibPage( Window * pParent ) :
        TabPage(        pParent,IDEResId( RID_TP_LIBS ) ),
        aLibText(       this,   IDEResId( RID_STR_LIB ) ),
        aLibBox(        this,   IDEResId( RID_TRLBOX ) ),
        aBasicsBox(     this,   IDEResId( RID_LB_BASICS ) ),
        aEditButton(    this,   IDEResId( RID_PB_EDIT ) ),
        aCloseButton(   this,   IDEResId( RID_PB_CLOSE ) ),
        aPasswordButton(this,   IDEResId( RID_PB_PASSWORD ) ),
        aNewLibButton(  this,   IDEResId( RID_PB_NEWLIB ) ),
        aInsertLibButton( this, IDEResId( RID_PB_APPEND ) ),
        aDelButton(     this,   IDEResId( RID_PB_DELETE ) )
{
    FreeResource();
    pTabDlg = 0;

    aEditButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aNewLibButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aPasswordButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aInsertLibButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aDelButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aCloseButton.SetClickHdl( LINK( this, LibPage, ButtonHdl ) );
    aLibBox.SetSelectHdl( LINK( this, LibPage, TreeListHighlightHdl ) );

    aBasicsBox.SetSelectHdl( LINK( this, LibPage, BasicSelectHdl ) );

    aLibBox.SetMode( LIBMODE_MANAGER );
    aLibBox.EnableInplaceEditing( TRUE );
    aLibBox.SetCheckButtonHdl( LINK( this, LibPage, CheckBoxHdl ) );
    aCloseButton.GrabFocus();

    long aTabs[] = { 2, 30, 120 };
    aLibBox.SetTabs( aTabs, MAP_PIXEL );

    FillListBox();
    aBasicsBox.SelectEntryPos( 0 );
    SetCurLib();

    CheckButtons();
}

void LibPage::CheckButtons()
{
    SvLBoxEntry* pCur = aLibBox.GetCurEntry();
    BOOL bCurChecked = pCur ? aLibBox.IsChecked( aLibBox.GetModel()->GetAbsPos( pCur ) ) : FALSE;
    if ( bCurChecked )
    {
        aEditButton.Enable();
        if ( aLibBox.GetModel()->GetAbsPos( pCur ) != 0 )
            aPasswordButton.Enable();
        else
            aPasswordButton.Disable();
    }
    else
    {
        aEditButton.Disable();
        aPasswordButton.Disable();
    }

    if ( pCur && aLibBox.GetModel()->GetAbsPos( pCur ) )
        aDelButton.Enable();
    else
        aDelButton.Disable();
}



IMPL_LINK_INLINE_START( LibPage, CheckBoxHdl, SvTreeListBox *, EMPTYARG )
{
    CheckButtons();
    return 0;
}
IMPL_LINK_INLINE_END( LibPage, CheckBoxHdl, SvTreeListBox *, EMPTYARG )



void __EXPORT LibPage::ActivatePage()
{
    SetCurLib();
}



void __EXPORT LibPage::DeactivatePage()
{
    ActivateCurrentLibSettings();
}



IMPL_LINK_INLINE_START( LibPage, TreeListHighlightHdl, SvTreeListBox *, pBox )
{
    if ( pBox->IsSelected( pBox->GetHdlEntry() ) )
        CheckButtons();
    return 0;
}
IMPL_LINK_INLINE_END( LibPage, TreeListHighlightHdl, SvTreeListBox *, pBox )



IMPL_LINK_INLINE_START( LibPage, BasicSelectHdl, ListBox *, pBox )
{
    SetCurLib();
    CheckButtons();
    return 0;
}
IMPL_LINK_INLINE_END( LibPage, BasicSelectHdl, ListBox *, pBox )



IMPL_LINK( LibPage, ButtonHdl, Button *, pButton )
{
    if ( pButton == &aEditButton )
    {
        ActivateCurrentLibSettings();
        SfxViewFrame* pCurFrame = SfxViewFrame::Current();
        DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
        SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
        if( pDispatcher )
        {
            pDispatcher->Execute( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON );
        }
        SvLBoxEntry* pCurEntry = aLibBox.GetCurEntry();
        DBG_ASSERT( pCurEntry, "Entry?!" );
        String aLib( CreateMgrAndLibStr( aCurBasMgr, aLibBox.GetEntryText( pCurEntry, 0 ) ) );
        SfxStringItem aLibName( SID_BASICIDE_ARG_LIBNAME, aLib );
        if( pDispatcher )
        {
            pDispatcher->Execute( SID_BASICIDE_LIBSELECTED,
                                    SFX_CALLMODE_ASYNCHRON, &aLibName, 0L );
        }
        EndTabDialog( 1 );
    }
    else if ( pButton == &aNewLibButton )
        NewLib();
    else if ( pButton == &aInsertLibButton )
        InsertLib();
    else if ( pButton == &aDelButton )
        DeleteCurrent();
    else if ( pButton == &aCloseButton )
    {
        ActivateCurrentLibSettings();
        EndTabDialog( 0 );
    }
    else if ( pButton == &aPasswordButton )
    {
        String aBasicManager( aBasicsBox.GetSelectEntry() );
        BasicManager* pBasMgr = BasicIDE::FindBasicManager( aBasicManager );
        DBG_ASSERT( pBasMgr, "BasicManager?!" );

        SvLBoxEntry* pCurEntry = aLibBox.GetCurEntry();
        USHORT nLib = (USHORT)aLibBox.GetModel()->GetAbsPos( pCurEntry );

        BOOL bHadPassword = aLibBox.GetBasicManager()->HasPassword( nLib );
        // Noch nicht geladen, falls gerade erst aktiviert.
        // Wuerde sonst erst beim Beenden des Dlg's geschehen.
        if ( !aLibBox.GetBasicManager()->IsLibLoaded( nLib ) )
        {
            Application::EnterWait();
            aLibBox.GetBasicManager()->LoadLib( nLib );
            Application::LeaveWait();
        }

        SvxPasswordDialog* pDlg = new SvxPasswordDialog( this, TRUE );
        String aPassword = pBasMgr->GetPassword( nLib );
#ifdef DEBUG
        InfoBox( 0, aPassword ).Execute();
#endif
        pDlg->SetOldPassword( aPassword );

        if ( pDlg->Execute() == RET_OK )
        {
            pBasMgr->SetPassword( nLib, pDlg->GetNewPassword() );
            pBasMgr->SetPasswordVerified( nLib );
            if ( bHadPassword != aLibBox.GetBasicManager()->HasPassword( nLib ) )
            {
                aLibBox.GetModel()->Remove( pCurEntry );
                ImpInsertLibEntry( nLib );
                aLibBox.GetBasicManager()->SetPasswordVerified( nLib );
            }
        }
        delete pDlg;
    }
    CheckButtons();
    return 0;
}

void LibPage::NewLib()
{
    BasicManager* pBasMgr = BasicIDE::FindBasicManager( aCurBasMgr );
    DBG_ASSERT( pBasMgr, "BasMgr?!" );
    String aLibName;
    String aLibStdName( IDEResId( RID_STR_STDLIBNAME ) );
    BOOL bValid = FALSE;
    USHORT i = 1;
    while ( !bValid )
    {
        aLibName = aLibStdName;
        aLibName += String::CreateFromInt32( i );
        if ( !pBasMgr->HasLib( aLibName ) )
            bValid = TRUE;
        i++;
    }

    NewObjectDialog* pNewDlg = new NewObjectDialog( this, NEWOBJECTMODE_LIB );
    pNewDlg->SetObjectName( aLibName );
    BOOL bEnableSepFile = ( pBasMgr->GetStorageName().Len() && ( pBasMgr == SFX_APP()->GetBasicManager() ) );
    pNewDlg->EnableSeparateFile( bEnableSepFile );
    if ( pNewDlg->Execute() )
    {
        if ( pNewDlg->GetObjectName().Len() )
            aLibName = pNewDlg->GetObjectName();

        if ( aLibName.Len() > 30 )
        {
            ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_LIBNAMETOLONG ) ) ).Execute();
        }
        else if ( !BasicIDE::IsValidSbxName( aLibName ) )
        {
            ErrorBox( this, WB_OK | WB_DEF_OK,
                        String( IDEResId( RID_STR_BADSBXNAME ) ) ).Execute();
        }
        else if ( pBasMgr->HasLib( aLibName ) )
        {
            ErrorBox( this, WB_OK | WB_DEF_OK,
                        String( IDEResId( RID_STR_SBXNAMEALLREADYUSED2 ) ) ).Execute();
        }
        else
        {
            BOOL bCreateLib = TRUE;
            String aLibStorageName;
            if ( pNewDlg->IsSeparateFile() )
            {
                INetURLObject aFileURL( pBasMgr->GetStorageName() , INetURLObject::FSYS_DETECT );
                String aExt = aFileURL.getExtension();
                aFileURL.setName( aLibName );
                aFileURL.setExtension( aExt );
                aLibStorageName = aFileURL.getFSysPath( INetURLObject::FSYS_DETECT );
                String aFileURLStr = aFileURL.GetMainURL();

                Reference< XSimpleFileAccess > xSFI;
                Reference< XMultiServiceFactory > xSMgr = getProcessServiceFactory();
                if( xSMgr.is() )
                {
                    xSFI = Reference< XSimpleFileAccess >( xSMgr->createInstance
                        ( OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ), UNO_QUERY );
                }
                if( xSFI.is() )
                {
                    sal_Bool bExists = sal_False;
                    try { bExists = xSFI->exists( aFileURLStr ); }
                    catch( Exception & ) {}
                    if( bExists )
                    {
                        String aText( IDEResId( RID_STR_FILEEXISTS ) );
                        aText.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "XX" ) ), aLibStorageName );
                        ErrorBox( this, WB_OK | WB_DEF_OK, aText ).Execute();
                        bCreateLib = FALSE;
                    }
                }
            }

            if ( bCreateLib )
            {
                StarBASIC* pLib = pBasMgr->CreateLib( aLibName );

                SvLBoxEntry* pEntry = aLibBox.InsertEntry( aLibName );
                USHORT nPos = (USHORT)aLibBox.GetModel()->GetAbsPos( pEntry );
                USHORT nLib = pBasMgr->GetLibId( aLibName );
                aLibBox.CheckEntryPos( nPos, pBasMgr->IsLibLoaded( nLib ) );

                if ( aLibStorageName.Len() )
                {
                    INetURLObject aFileURL( aLibStorageName , INetURLObject::FSYS_DETECT );
                    String aShortName = aFileURL.getName();

                    // conversion to 8.3 filename for FAT filesystem not implemented, because
                    // INetURLObject doesn't have a method corresponding to old DirEntry::MakeShortName
                    //
                    // old code for comparison:
                    /*
                    DirEntry aDirEntry( aLibStorageName );
                    String aShortName = aDirEntry.GetName();

                    // Immer 8.3
                    DirEntry aTempEntry( aDirEntry.GetPath() );
                    if( aTempEntry.MakeShortName( aDirEntry.GetName(), FSYS_KIND_NONE, TRUE, FSYS_STYLE_FAT ) )
                    {
                        aLibStorageName = aTempEntry.GetFull();
                        aShortName = aTempEntry.GetName();
                    }
                    */

                    pBasMgr->SetLibStorageName( nLib, aLibStorageName );
                    aLibBox.SetEntryText( aShortName, pEntry, 1 );

                    // Sofort speichern, sonst erhaelt man ggf. spaeter
                    // nochmal den gleichen ShortName
                    pBasMgr->StoreLib( nLib );
                }

                // Ein Modul anlegen:
                SbModule* pModule = BasicIDE::CreateModule( pLib, String(), TRUE );
                DBG_ASSERT( pModule , "Modul wurde nicht erzeugt!" );
                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, pModule );

                SfxViewFrame* pCurFrame = SfxViewFrame::Current();
                DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
                SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
                if( pDispatcher )
                {
                    pDispatcher->Execute( SID_BASICIDE_SBXINSERTED,
                                          SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
                }
            }
        }
    }
    delete pNewDlg;
}

void LibPage::InsertLib()
{
//  BasicManager* pBasMgr = BasicIDE::FindBasicManager( aCurBasMgr );
    BasicManager* pBasMgr = aLibBox.GetBasicManager();
    DBG_ASSERT( pBasMgr, "BasMgr?!" );

    SfxFileDialog aFileDialogBox( this, WinBits( WB_OPEN | WB_3DLOOK ) );
    aFileDialogBox.SetText( String( IDEResId( RID_STR_APPENDLIBS ) ) );
    aFileDialogBox.AddFilter( String( IDEResId( RID_STR_BASIC ) ), String( RTL_CONSTASCII_USTRINGPARAM( "*.sbl" ) ) );
    aFileDialogBox.AddFilter( String( IDEResId( RID_STR_DOC ) ), String( RTL_CONSTASCII_USTRINGPARAM( "*.sdw;*.sdc;*.sdd" ) ) );

    String aPath( IDE_DLL()->GetExtraData()->GetAddLibPath() );
    if ( aPath.Len() )
    {
        aFileDialogBox.SetPath( aPath );
        aFileDialogBox.SetCurFilter( IDE_DLL()->GetExtraData()->GetAddLibFilter() );
    }
    else
    {
        // macro path from configuration management
        aPath = SvtPathOptions().GetWorkPath();
        INetURLObject aFileURL( aPath , INetURLObject::FSYS_DETECT );
        aFileURL.setFinalSlash();
        aPath = aFileURL.getFSysPath( INetURLObject::FSYS_DETECT );
        aFileDialogBox.SetPath( aPath );
        aFileDialogBox.SetCurFilter( String( IDEResId( RID_STR_DOC ) ) );
    }

    if ( aFileDialogBox.Execute() )
    {
        aPath = aFileDialogBox.GetPath();
        IDE_DLL()->GetExtraData()->SetAddLibPath( aPath );
        IDE_DLL()->GetExtraData()->SetAddLibFilter( aFileDialogBox.GetCurFilter() );

        INetURLObject aFileURL( aPath );
        String aFullName( aFileURL.getFSysPath( INetURLObject::FSYS_DETECT ) );

        if ( SvStorage::IsStorageFile( aFullName ) )
        {
            SvStorageRef xStorage = new SvStorage( aFullName, STREAM_READ | STREAM_SHARE_DENYWRITE );
            if ( xStorage->GetError() )
                ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_ERROROPENSTORAGE ) ) ).Execute();
            else
            {
                // Die einzelnen Libs aus dem BasicStorage...
                SvStorageRef xBasicStorage = xStorage->OpenStorage( String( RTL_CONSTASCII_USTRINGPARAM( "StarBASIC" ) ), STREAM_READ | STREAM_SHARE_DENYWRITE, 0 );
                if ( xBasicStorage->GetError() )
                    InfoBox( this, String( IDEResId( RID_STR_NOLIBINSTORAGE ) ) ).Execute();
                else
                {
                    LibDialog* pLibDlg = 0;
                    SvStorageInfoList aInfoList( 4, 4 );
                    xBasicStorage->FillInfoList( &aInfoList );
                    for ( USHORT nStream = 0; nStream < aInfoList.Count(); nStream++ )
                    {
                        SvStorageInfo& rInf = aInfoList[nStream];
                        if ( rInf.IsStream() )
                        {
                            if ( !pLibDlg )
                            {
                                pLibDlg = new LibDialog( this );
                                pLibDlg->SetStorageName( aFileURL.getName() );
                                pLibDlg->GetLibBox().SetMode( LIBMODE_CHOOSER );
                                if ( pBasMgr == SFX_APP()->GetBasicManager() )
                                    pLibDlg->SetSeparateFileEnabled( TRUE );
                            }
                            SvLBoxEntry* pEntry = pLibDlg->GetLibBox().InsertEntry( rInf.GetName() );
                            USHORT nPos = (USHORT) pLibDlg->GetLibBox().GetModel()->GetAbsPos( pEntry );
                            pLibDlg->GetLibBox().CheckEntryPos( nPos, TRUE);

                        }
                    }
                    xBasicStorage.Clear();
                    if ( !pLibDlg )
                        InfoBox( this, String( IDEResId( RID_STR_NOLIBINSTORAGE ) ) ).Execute();
                    else
                    {
                        BOOL bChanges = FALSE;
                        if ( pLibDlg->Execute() )
                        {
                            ULONG nNewPos = aLibBox.GetEntryCount();
//                          ULONG nCurPos = aLibBox.GetAbsPos( aLibBox.GetCurEntry() );
                            BOOL bReplace = pLibDlg->IsReplace();
                            BOOL bReference = pLibDlg->IsReference();
                            for ( USHORT nLib = 0; nLib < pLibDlg->GetLibBox().GetEntryCount(); nLib++ )
                            {
                                if ( pLibDlg->GetLibBox().IsChecked( nLib ) )
                                {
                                    SvLBoxEntry* pEntry = pLibDlg->GetLibBox().GetEntry( nLib );
                                    DBG_ASSERT( pEntry, "Entry?!" );
                                    String aName( pLibDlg->GetLibBox().GetEntryText( pEntry, 0 ) );
                                    // Optionen auswerten...
                                    if ( pBasMgr->HasLib( aName ) )
                                    {
                                        // Die Standard-Lib kann nicht ersetzt werden,
                                        // weil Basics verkettet.
                                        if ( bReplace && ( pBasMgr->GetLib( aName ) == pBasMgr->GetStdLib() ) )
                                        {
                                            ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_REPLACESTDLIB ) ) ).Execute();
                                            continue;
                                        }
                                        if ( bReplace )
                                        {
                                            SvLBoxEntry* pEntry = aLibBox.FindEntry( aName );
                                            if ( pEntry )
                                                aLibBox.SvTreeListBox::GetModel()->Remove( pEntry );
                                            pBasMgr->RemoveLib( pBasMgr->GetLibId( aName ) );
                                        }
                                        else if ( bReference )
                                        {
                                            // Referenz nicht moeglich, wenn Lib mit
                                            // Namen schon existiert, ausser bei Replace.
                                            String aErrStr( IDEResId( RID_STR_REFNOTPOSSIBLE ) );
                                            aErrStr.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "XX" ) ), aName );
                                            aErrStr += '\n';
                                            aErrStr += String( IDEResId( RID_STR_SBXNAMEALLREADYUSED ) );
                                            ErrorBox( this, WB_OK | WB_DEF_OK, aErrStr ).Execute();
                                            continue;
                                        }
                                    }
                                    StarBASIC* pNew = pBasMgr->AddLib( *xStorage, aName, bReference );
                                    if ( !pNew )
                                    {
                                        String aErrStr( IDEResId( RID_STR_ERROROPENLIB ) );
                                        ErrorBox( this, WB_OK | WB_DEF_OK, aErrStr ).Execute();
                                        continue;
                                    }
                                    USHORT nLib = pBasMgr->GetLibId( pNew->GetName() );
                                    if ( pLibDlg->IsSeparateFile() )
                                    {
                                        INetURLObject aFileURL( pBasMgr->GetStorageName() , INetURLObject::FSYS_DETECT );
                                        String aExt = aFileURL.getExtension();
                                        aFileURL.setName( pBasMgr->GetLibName( nLib ) );
                                        aFileURL.setExtension( aExt );
                                        pBasMgr->SetLibStorageName( nLib, aFileURL.getFSysPath( INetURLObject::FSYS_DETECT ) );
                                    }
                                    DBG_ASSERT( nLib != LIB_NOTFOUND, "Lib nicht eingefuegt?!" );
                                    ImpInsertLibEntry( nLib );
                                    bChanges = TRUE;
                                }
                            }
                            SvLBoxEntry* pFirstNew = aLibBox.GetEntry( nNewPos );
                            if ( pFirstNew )
                                aLibBox.SetCurEntry( pFirstNew );
                        }
                        delete pLibDlg;
                        if ( bChanges )
                            BasicIDE::MarkDocShellModified( pBasMgr->GetStdLib() );
                    }
                }
            }
        }
        else
        {
            // Erstmal nur in Storages moeglich, wie soll ein Basic in eine
            // Datei kommen?
            InfoBox( this, String( IDEResId( RID_STR_NOLIBINSTORAGE ) ) ).Execute();
        }
    }
}

void LibPage::DeleteCurrent()
{
    BasicManager* pBasMgr = BasicIDE::FindBasicManager( aCurBasMgr );
    DBG_ASSERT( pBasMgr, "BasMgr?!" );
    SvLBoxEntry* pCurEntry = aLibBox.GetCurEntry();
    DBG_ASSERT( pCurEntry && aLibBox.GetModel()->GetAbsPos( pCurEntry ), "Kann nicht loeschen!" );
    String aLibName( aLibBox.GetEntryText( pCurEntry, 0 ) );
    USHORT nLib = pBasMgr->GetLibId( aLibName );
    if ( QueryDelLib( aLibName, pBasMgr->IsReference( nLib ), this ) )
    {
        // BasicIDE informieren, falls oben.
        String aLib( CreateMgrAndLibStr( aCurBasMgr, aLibName ) );
        SfxStringItem aLibItem( SID_BASICIDE_ARG_LIBNAME, aLib );
        SfxViewFrame* pCurFrame = SfxViewFrame::Current();
        DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
        SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
        if( pDispatcher )
        {
            pDispatcher->Execute( SID_BASICIDE_LIBREMOVED,
                                  SFX_CALLMODE_SYNCHRON, &aLibItem, 0L );
        }
        pBasMgr->RemoveLib( nLib, TRUE );
        ((SvLBox&)aLibBox).GetModel()->Remove( pCurEntry );
        BasicIDE::MarkDocShellModified( pBasMgr->GetStdLib() );
    }
}



void LibPage::EndTabDialog( USHORT nRet )
{
    DBG_ASSERT( pTabDlg, "TabDlg nicht gesetzt!" );
    if ( pTabDlg )
        pTabDlg->EndDialog( nRet );
}



void LibPage::FillListBox()
{
    BasicManager* pBasicMgr = SFX_APP()->GetBasicManager();
    SfxObjectShell* pDocShell = 0;
    while ( pBasicMgr )
    {
        // Nur, wenn es ein dazugehoeriges Fenster gibt, damit nicht die
        // Gecachten Docs, die nicht sichtbar sind ( Remot-Dokumente )
        if ( !pDocShell || ( ( pBasicMgr != SFX_APP()->GetBasicManager() )
                                && ( SfxViewFrame::GetFirst( pDocShell ) ) ) )
        {
            String aBasMgr;
            if (pDocShell)
                aBasMgr = pDocShell->GetTitle( SFX_TITLE_FILENAME );
            else
                aBasMgr = Application::GetAppName();

            aBasicsBox.InsertEntry( aBasMgr, LISTBOX_APPEND );
        }

        if ( pDocShell  )
            pDocShell = SfxObjectShell::GetNext( *pDocShell );
        else
            pDocShell = SfxObjectShell::GetFirst();

        pBasicMgr = ( pDocShell ? pDocShell->GetBasicManager() : 0 );
    }
}



void LibPage::SetCurLib()
{
    String aSelected( aBasicsBox.GetSelectEntry() );
    if ( aSelected != aCurBasMgr )
    {
        ActivateCurrentLibSettings();
        aCurBasMgr = aSelected;
        BasicManager* pBasMgr = BasicIDE::FindBasicManager( aCurBasMgr );
        DBG_ASSERT( pBasMgr, "BasMgr?!" );
        aLibBox.SetBasicManager( pBasMgr );
        USHORT nLibs = pBasMgr->GetLibCount();
        aLibBox.Clear();
        Image aLockedImg( IDEResId( RID_IMG_LOCKED ) );
        for ( USHORT nLib = 0; nLib < nLibs; nLib++ )
            ImpInsertLibEntry( nLib );
        aLibBox.SetCurEntry( aLibBox.GetEntry( 0 ) );
    }
}



void LibPage::ActivateCurrentLibSettings()
{
    BasicManager* pBasMgr = BasicIDE::FindBasicManager( aCurBasMgr );
    if ( pBasMgr )  // Beim ersten mal 0
    {
        USHORT nLibs = (USHORT) aLibBox.GetEntryCount();
        // StandardLib kann nicht geaendert werden...
        for ( USHORT nLib = 1; nLib < nLibs; nLib++ )
        {
            BOOL bChecked = aLibBox.IsChecked( nLib );
            if ( bChecked != pBasMgr->IsLibLoaded( nLib ) )
            {
                String aLibName( pBasMgr->GetLibName( nLib ) );
                String aLib( CreateMgrAndLibStr( aCurBasMgr, aLibName ) );
                SfxStringItem aLibItem( SID_BASICIDE_ARG_LIBNAME, aLib );

                SfxViewFrame* pCurFrame = SfxViewFrame::Current();
                DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
                SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
                if ( bChecked )
                {
                    pBasMgr->LoadLib( nLib );
                    if( pDispatcher )
                    {
                        pDispatcher->Execute( SID_BASICIDE_LIBLOADED,
                            SFX_CALLMODE_ASYNCHRON, &aLibItem, 0L );
                    }
                }
                else
                {
                    // Vorher aufrufen, damit noch im BasMgr
                    if( pDispatcher )
                    {
                        pDispatcher->Execute( SID_BASICIDE_LIBREMOVED,
                            SFX_CALLMODE_SYNCHRON, &aLibItem, 0L );
                    }
                    pBasMgr->StoreLib( nLib );
                    pBasMgr->UnloadLib( nLib );
                }
            }
        }
    }
}

SvLBoxEntry* LibPage::ImpInsertLibEntry( USHORT nLib )
{
    BasicManager* pBasicManager = aLibBox.GetBasicManager();
    DBG_ASSERT( pBasicManager, "ImpInsertLibEntry: Kein BasicManager!" );
    BOOL bPassword = pBasicManager->HasPassword( nLib );
//  if ( !pBasicManager->IsLibLoaded( nLib ) )
//  {
//      // Lib muss geladen sein, wenn Passwortabfrage...
//      pBasicManager->LoadLib( nLib );
//      bPassword = pBasicManager->HasPassword( nLib );
//      pBasicManager->UnloadLib( nLib );
//  }
    if ( bPassword )
    {
        Image aImg = Image( IDEResId( RID_IMG_LOCKED ) );
        Size aSz = aImg.GetSizePixel();
        aLibBox.SetDefaultExpandedEntryBmp( aImg );
        aLibBox.SetDefaultCollapsedEntryBmp( aImg );
    }

    SvLBoxEntry* pNewEntry = aLibBox.InsertEntry( pBasicManager->GetLibName( nLib ), nLib );

    if ( pBasicManager->IsReference( nLib ) || pBasicManager->IsExtern( nLib ) )
    {
        String aLibStorage = pBasicManager->GetLibStorageName( nLib );
        if ( pBasicManager->GetStorageName().Len() )
        {
            INetURLObject aFileURL( pBasicManager->GetStorageName() , INetURLObject::FSYS_DETECT );
            aFileURL.removeSegment();
            aFileURL.removeFinalSlash();
            String aPath = aFileURL.getFSysPath( INetURLObject::FSYS_DETECT );

            if ( aLibStorage.CompareIgnoreCaseToAscii( aPath, aPath.Len() ) == COMPARE_EQUAL )
                aLibStorage.Erase( 0, aPath.Len()+1 );  // Dann ohne Pfadangabe...
            else if ( pBasicManager->GetRelLibStorageName( nLib ).Len() )
                aLibStorage = pBasicManager->GetRelLibStorageName( nLib ); // Kuerzer...
        }

        aLibBox.SetEntryText( aLibStorage, pNewEntry, 1 );
    }

    aLibBox.CheckEntryPos( nLib, pBasicManager->IsLibLoaded( nLib ) );

    if ( bPassword )
    {
        Image aImg; // Default zuruecksetzen
        aLibBox.SetDefaultExpandedEntryBmp( aImg );
        aLibBox.SetDefaultCollapsedEntryBmp( aImg );
    }

    return pNewEntry;
}


BasicCheckBox::BasicCheckBox( Window* pParent, const ResId& rResId ) :
        SvTabListBox( pParent, rResId )
{
    nMode = 0;
    pBasMgr = 0;
    long aTabs[] = { 1, 12 };   // Mindestens einen braucht die TabPos...
                                // 12 wegen der Checkbox
    SetTabs( aTabs );
    Init();
}



__EXPORT BasicCheckBox::~BasicCheckBox()
{
    delete pCheckButton;
}

void __EXPORT BasicCheckBox::CheckButtonHdl()
{
    SvLBoxEntry* pEntry = GetHdlEntry();
    DBG_ASSERT( pEntry, "HdlEntry?!" );
    ULONG nPos = GetModel()->GetAbsPos( pEntry );
    if ( !nPos && ( GetMode() == LIBMODE_MANAGER ) )
    {
        CheckEntryPos( 0, TRUE );
        ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_CANNOTUNLOADSTDLIB ) ) ).Execute();
    }
    else
    {
        // SelectHdl ausloesen, damit Buttons geprueft werden.
        SelectEntryPos( nPos, FALSE );
        SelectEntryPos( nPos, TRUE );
    }
}

void BasicCheckBox::Init()
{
    BasicCheckBoxBitmaps theBmps;

    pCheckButton = new SvLBoxButtonData;
    pCheckButton->aBmps[SV_BMP_UNCHECKED]   = theBmps.GetUncheckedBmp();
    pCheckButton->aBmps[SV_BMP_CHECKED]     = theBmps.GetCheckedBmp();
    pCheckButton->aBmps[SV_BMP_HICHECKED]   = theBmps.GetHiCheckedBmp();
    pCheckButton->aBmps[SV_BMP_HIUNCHECKED] = theBmps.GetHiUncheckedBmp();
    pCheckButton->aBmps[SV_BMP_TRISTATE]    = theBmps.GetTriStateBmp();
    pCheckButton->aBmps[SV_BMP_HITRISTATE]  = theBmps.GetHiTriStateBmp();
    EnableCheckButton( pCheckButton );
    SetHighlightRange();
}

SvLBoxEntry* BasicCheckBox::InsertEntry( const String& rStr, ULONG nPos )
{
//  return SvTreeListBox::InsertEntry( rStr, NULL, FALSE, nPos );
    return SvTabListBox::InsertEntry( rStr, nPos, 0 );
}


void BasicCheckBox::RemoveEntry( ULONG nPos )
{
    if ( nPos < GetEntryCount() )
        SvTreeListBox::GetModel()->Remove( GetEntry( nPos ) );
}

SvLBoxEntry* BasicCheckBox::FindEntry( const String& rName )
{
    ULONG nCount = GetEntryCount();
    for ( ULONG i = 0; i < nCount; i++ )
    {
        SvLBoxEntry* pEntry = GetEntry( i );
        DBG_ASSERT( pEntry, "pEntry?!" );
        if ( rName.CompareIgnoreCaseToAscii( GetEntryText( pEntry, 0 ) ) == COMPARE_EQUAL )
            return pEntry;
    }
    return 0;
}


void BasicCheckBox::SelectEntryPos( ULONG nPos, BOOL bSelect )
{
    if ( nPos < GetEntryCount() )
        Select( GetEntry( nPos ), bSelect );
}



ULONG BasicCheckBox::GetSelectEntryPos() const
{
    return GetModel()->GetAbsPos( FirstSelected() );
}



ULONG BasicCheckBox::GetCheckedEntryCount() const
{
    ULONG   nCheckCount = 0;
    ULONG   nCount      = GetEntryCount();

    for (ULONG i=0; i<nCount; i++ )
    {
        if ( IsChecked( i ) )
            nCheckCount++;
    }

    return nCheckCount;
}



void BasicCheckBox::CheckEntryPos( ULONG nPos, BOOL bCheck )
{
    if ( nPos < GetEntryCount() )
    {
        SvLBoxEntry* pEntry = GetEntry( nPos );

        if ( bCheck != GetCheckButtonState( pEntry ) )
            SetCheckButtonState( pEntry,
                                 bCheck
                                    ? SvButtonState(SV_BUTTON_CHECKED)
                                    : SvButtonState(SV_BUTTON_UNCHECKED) );
    }
}



BOOL BasicCheckBox::IsChecked( ULONG nPos ) const
{
    if ( nPos < GetEntryCount() )
        return (GetCheckButtonState( GetEntry( nPos ) ) == SV_BUTTON_CHECKED);
    return FALSE;
}


BOOL __EXPORT BasicCheckBox::EditingEntry( SvLBoxEntry* pEntry, Selection& )
{
    if ( nMode != LIBMODE_MANAGER )
        return FALSE;

    DBG_ASSERT( pEntry, "Kein Eintrag?" );
    ULONG nAbsPos = GetModel()->GetAbsPos( pEntry );
    if ( nAbsPos == 0 )
    {
        ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_CANNOTCHANGENAMESTDLIB ) ) ).Execute();
        return FALSE;
    }

    // Prueffen, ob Referenz...
    USHORT nLib = pBasMgr->GetLibId( GetEntryText( pEntry, 0 ) );
    DBG_ASSERT( nLib != LIB_NOTFOUND, "LibId ?!" );
    if ( pBasMgr->IsReference( nLib ) )
    {
        ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_CANNOTCHANGENAMEREFLIB ) ) ).Execute();
        return FALSE;
    }
    return TRUE;
}



BOOL __EXPORT BasicCheckBox::EditedEntry( SvLBoxEntry* pEntry, const String& rNewText )
{
    BOOL bValid = ( rNewText.Len() <= 30 ) && BasicIDE::IsValidSbxName( rNewText );
    String aCurText( GetEntryText( pEntry, 0 ) );
    if ( bValid && ( aCurText != rNewText ) )
    {
        // Prueffen, ob Lib mit dem Namen existiert!
        DBG_ASSERT( pBasMgr, "BasMgr nicht gesetzt!" );
        if ( pBasMgr->HasLib( rNewText ) )
        {
            ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_SBXNAMEALLREADYUSED ) ) ).Execute();
            return FALSE;
        }
        USHORT nLib = pBasMgr->GetLibId( aCurText );
        DBG_ASSERT( nLib != LIB_NOTFOUND, "Lib nicht gefunden!" );
        pBasMgr->SetLibName( nLib, rNewText );
        BasicIDE::MarkDocShellModified( pBasMgr->GetStdLib() );
        BasicIDE::GetBindings().Invalidate( SID_BASICIDE_LIBSELECTOR );
        BasicIDE::GetBindings().Update( SID_BASICIDE_LIBSELECTOR );
    }

    if ( !bValid )
    {
        if ( rNewText.Len() > 30 )
            ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_LIBNAMETOLONG ) ) ).Execute();
        else
            ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_BADSBXNAME ) ) ).Execute();
    }
    return bValid;
}



BasicCheckBoxBitmaps::BasicCheckBoxBitmaps() :
    Resource        ( ResId( RID_RES_CHECKBITMAPS ) ),
    aUncheckedBmp   ( ResId( CHKBTN_UNCHECKED ) ),
    aCheckedBmp     ( ResId( CHKBTN_CHECKED ) ),
    aHiCheckedBmp   ( ResId( CHKBTN_HICHECKED ) ),
    aHiUncheckedBmp ( ResId( CHKBTN_HIUNCHECKED ) ),
    aTriStateBmp    ( ResId( CHKBTN_TRISTATE ) ),
    aHiTriStateBmp  ( ResId( CHKBTN_HITRISTATE ) )
{
    FreeResource();
}


NewObjectDialog::NewObjectDialog( Window* pParent, USHORT nMode )
    : ModalDialog( pParent, IDEResId( RID_DLG_NEWLIB ) ),
        aText( this, IDEResId( RID_FT_NEWLIB ) ),
        aEdit( this, IDEResId( RID_ED_LIBNAME ) ),
        aCheckBox( this, IDEResId( RID_CHKB_EXTRAFILE ) ),
        aOKButton( this, IDEResId( RID_PB_OK ) ),
        aCancelButton( this, IDEResId( RID_PB_CANCEL ) )
{
    FreeResource();
    aEdit.GrabFocus();

    if ( nMode == NEWOBJECTMODE_LIB )
    {
        aCheckBox.Show();
        SetText( String( IDEResId( RID_STR_NEWLIB ) ) );
    }
    else if ( nMode == NEWOBJECTMODE_MOD )
    {
        SetText( String( IDEResId( RID_STR_NEWMOD ) ) );
    }
    else
    {
        SetText( String( IDEResId( RID_STR_NEWDLG ) ) );
    }
}

NewObjectDialog::~NewObjectDialog()
{
}

