/*************************************************************************
 *
 *  $RCSfile: basobj3.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mt $ $Date: 2000-10-19 09:19:37 $
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


#include <ide_pch.hxx>

#pragma hdrstop

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>
#include <iderdll.hxx>
#include <iderid.hxx>
#include <basobj.hxx>
#include <basidesh.hxx>
#include <basidesh.hrc>
#include <objdlg.hxx>
#include <bastypes.hxx>

#include <baside2.hxx>


// In bastype3.cxx:
USHORT GetDialogSbxId();

#define LINE_SEP    0x0A

SbMethod* BasicIDE::CreateMacro( SbModule* pModule, const String& rMacroName )
{
    SfxViewFrame* pCurFrame = SfxViewFrame::Current();
    DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
    SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
    if( pDispatcher )
    {
        pDispatcher->Execute( SID_BASICIDE_STOREALLMODULESOURCES );
    }

    if ( pModule->GetMethods()->Find( rMacroName, SbxCLASS_METHOD ) )
        return 0;

    String aMacroName( rMacroName );
    if ( aMacroName.Len() == 0 )
    {
        if ( !pModule->GetMethods()->Count() )
            aMacroName = String( RTL_CONSTASCII_USTRINGPARAM( "Main" ) );
        else
        {
            BOOL bValid = FALSE;
            String aStdMacroText( IDEResId( RID_STR_STDMACRONAME ) );
            USHORT nMacro = 1;
            while ( !bValid )
            {
                aMacroName = aStdMacroText;
                aMacroName += String::CreateFromInt32( nMacro );
                // Pruefen, ob vorhanden...
                bValid = pModule->GetMethods()->Find( aMacroName, SbxCLASS_METHOD ) ? FALSE : TRUE;
                nMacro++;
            }
        }
    }

    String aSource( pModule->GetSource() );
    aSource.ConvertLineEnd( LINEEND_LF );

    // Nicht zu viele Leerzeilen erzeugen...
    if ( aSource.Len() > 2 )
    {
        if ( aSource.GetChar( aSource.Len() - 1 )  != LINE_SEP )
            aSource += String( RTL_CONSTASCII_USTRINGPARAM( "\n\n" ) );
        else if ( aSource.GetChar( aSource.Len() - 2 ) != LINE_SEP )
            aSource += String( RTL_CONSTASCII_USTRINGPARAM( "\n" ) );
        else if ( aSource.GetChar( aSource.Len() - 3 ) == LINE_SEP )
            aSource.Erase( (USHORT)(aSource.Len()-1), 1 );
    }

    String aSubStr;
    aSubStr = String( RTL_CONSTASCII_USTRINGPARAM( "Sub " ) );
    aSubStr += aMacroName;
    aSubStr += String( RTL_CONSTASCII_USTRINGPARAM( "\n\nEnd Sub" ) );

    aSource += aSubStr;
    pModule->SetSource( aSource );
//  SbxObject* pObject = pModule->GetParent();
//  DBG_ASSERT( pObject->ISA( StarBASIC ), "Kein Basic! (GetParent)" );
//  ((StarBASIC*)pObject)->Compile( pModule );
    SbMethod* pMethod = (SbMethod*)pModule->GetMethods()->Find( aMacroName, SbxCLASS_METHOD );
    if( pDispatcher )
    {
        pDispatcher->Execute( SID_BASICIDE_UPDATEALLMODULESOURCES );
    }
    SbxObject* pParent = pModule->GetParent();
    if ( pParent && pParent->ISA( StarBASIC ) )
        BasicIDE::MarkDocShellModified( (StarBASIC*)pParent );
    return pMethod;
}

String BasicIDE::CreateDialogName( StarBASIC* pBasic, const String& rDlgName )
{
    String aDlgName( rDlgName );
    if ( aDlgName.Len() == 0 )
    {
        String aDlgStdName( IDEResId( RID_STR_STDDIALOGNAME ) );
        BOOL bValid = FALSE;
        USHORT i = 1;
        while ( !bValid )
        {
            aDlgName = aDlgStdName;
            aDlgName += String::CreateFromInt32( i );
            if ( !BasicIDE::FindDialog( pBasic, aDlgName ) )
                bValid = TRUE;

            i++;
        }
    }
    return aDlgName;
}

SbxObject* BasicIDE::CreateDialog( StarBASIC* pBasic, const String& rDlgName )
{
    String aDlgName = CreateDialogName( pBasic, rDlgName );
    SbxObject* pDlg = pBasic->MakeObject( aDlgName, String( RTL_CONSTASCII_USTRINGPARAM( "Dialog" ) ) );
    BasicIDE::MarkDocShellModified( pBasic );

    return pDlg;
}

String BasicIDE::CreateModuleName( StarBASIC* pBasic, const String& rModName )
{
    String aModName( rModName );
    if ( aModName.Len() == 0 )
    {
        // Namen generieren...
        String aModStdName( IDEResId( RID_STR_STDMODULENAME ) );
        BOOL bValid = FALSE;
        USHORT i = 1;
        while ( !bValid )
        {
            aModName = aModStdName;
            aModName += String::CreateFromInt32( i );
            if ( !pBasic->FindModule( aModName ) )
                bValid = TRUE;

            i++;
        }
    }
    return aModName;
}

SbModule* BasicIDE::CreateModule( StarBASIC* pBasic, const String& rModName, BOOL bCreateMain )
{
    if ( pBasic->FindModule( rModName ) )
        return 0;

    String aModName = CreateModuleName( pBasic, rModName );

    String aSource( String( RTL_CONSTASCII_USTRINGPARAM( "REM  *****  BASIC  *****\n\n" ) ) );
    if ( bCreateMain )
        aSource += String( RTL_CONSTASCII_USTRINGPARAM( "Sub Main\n\nEnd Sub" ) );

    SbModule* pModule;
    pModule = pBasic->MakeModule( aModName, aSource );

    DBG_ASSERT( pModule, "Modul?!" );
    BasicIDE::MarkDocShellModified( pBasic );
    return pModule;
}



SbxObject* BasicIDE::FindDialog( StarBASIC* pBasic, const String& rDlgName )
{
    pBasic->GetAll( SbxCLASS_OBJECT );

    USHORT nObjs = pBasic->GetObjects()->Count();
    for ( USHORT nObject = 0; nObject < nObjs; nObject++ )
    {
        SbxVariable* pVar = pBasic->GetObjects()->Get( nObject );
        if ( ( pVar->GetSbxId() == GetDialogSbxId() ) && ( pVar->GetName() == rDlgName ) )
            return (SbxObject*)pVar;
    }

    return 0;
}



StarBASIC* BasicIDE::FindBasic( const SbxVariable* pVar )
{
    const SbxVariable* pSbx = pVar;
    while ( pSbx && !pSbx->ISA( StarBASIC ) )
        pSbx = pSbx->GetParent();

    DBG_ASSERT( !pSbx || pSbx->ISA( StarBASIC ), "Find Basic: Kein Basic!" );
    return (StarBASIC*)pSbx;
}



BasicManager* BasicIDE::FindBasicManager( StarBASIC* pLib )
{
    BasicManager* pBasicMgr = SFX_APP()->GetBasicManager();
    SfxObjectShell* pDocShell = 0;
    while ( pBasicMgr )
    {
        USHORT nLibs = pBasicMgr->GetLibCount();
        for ( USHORT nLib = 0; nLib < nLibs; nLib++ )
        {
            StarBASIC* pL = pBasicMgr->GetLib( nLib );
            if ( pL == pLib )
                return pBasicMgr;
        }

        if ( pDocShell  )
            pDocShell = SfxObjectShell::GetNext( *pDocShell );
        else
            pDocShell = SfxObjectShell::GetFirst();

        pBasicMgr = ( pDocShell ? pDocShell->GetBasicManager() : 0 );
    }
    return 0;
}

BasicManager* BasicIDE::FindBasicManager( const String& aBasMgr )
{
    BasicManager* pBasicMgr = SFX_APP()->GetBasicManager();
    SfxObjectShell* pDocShell = 0;
    while ( pBasicMgr )
    {
        String aMgr;
        if (pDocShell)
            aMgr = pDocShell->GetTitle( SFX_TITLE_FILENAME );
        else
            aMgr = Application::GetAppName();

        if ( aMgr == aBasMgr )
            return pBasicMgr;

        if ( pDocShell  )
            pDocShell = SfxObjectShell::GetNext( *pDocShell );
        else
            pDocShell = SfxObjectShell::GetFirst();

        pBasicMgr = ( pDocShell ? pDocShell->GetBasicManager() : 0 );
    }
    return 0;
}

String BasicIDE::FindTitle( BasicManager* pBasicManager, USHORT nSFXTitleType )
{
    BasicManager* pBasicMgr = SFX_APP()->GetBasicManager();
    SfxObjectShell* pDocShell = 0;
    String aTitle;
    while ( pBasicMgr )
    {
        if ( pBasicMgr == pBasicManager )
        {
            if (pDocShell)
                aTitle = pDocShell->GetTitle( nSFXTitleType );
            else
                aTitle = Application::GetAppName();
            return aTitle;
        }

        if ( pDocShell  )
            pDocShell = SfxObjectShell::GetNext( *pDocShell );
        else
            pDocShell = SfxObjectShell::GetFirst();

        pBasicMgr = ( pDocShell ? pDocShell->GetBasicManager() : 0 );
    }
    return aTitle;
}

SfxObjectShell* BasicIDE::FindDocShell( BasicManager* pBasMgr )
{
    SfxObjectShell* pDocShell = SfxObjectShell::GetFirst();
    while ( pDocShell )
    {
        if ( ( pDocShell->GetBasicManager() != SFX_APP()->GetBasicManager() ) &&
             ( pDocShell->GetBasicManager() == pBasMgr ) )
        {
            return pDocShell;
        }
        pDocShell = SfxObjectShell::GetNext( *pDocShell );
    }
    return 0;
}

void BasicIDE::MarkDocShellModified( StarBASIC* pBasic )
{
        // BasicManager suchen, der zur Lib passt...
        BasicManager* pBasMgr = BasicIDE::FindBasicManager( pBasic );
        // Koennte z.B. nach CloseDoc schon weg sein...
        if ( pBasMgr )
        {
            SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );
            // Muss ja nicht aus einem Document kommen...
            if ( pShell )
            {
                pShell->SetModified();
                SfxBindings& rBindings = BasicIDE::GetBindings();
                rBindings.Invalidate( SID_SAVEDOC );
                rBindings.Update( SID_SAVEDOC );

            }
        }

        // Objectcatalog updaten...
        BasicIDEShell* pShell = IDE_DLL()->GetShell();
        ObjectCatalog* pObjCatalog = pShell ? pShell->GetObjectCatalog() : 0;
        if ( pObjCatalog )
            pObjCatalog->UpdateEntries();
}

void BasicIDE::RunMethod( SbMethod* pMethod )
{
    SbxValues aRes;
    aRes.eType = SbxVOID;
    pMethod->Get( aRes );
}

void BasicIDE::StopBasic()
{
    StarBASIC::Stop();
    BasicIDEShell* pShell = IDE_DLL()->GetShell();
    if ( pShell )
    {
        IDEWindowTable& rWindows = pShell->GetIDEWindowTable();
        IDEBaseWindow* pWin = rWindows.First();
        while ( pWin )
        {
            // BasicStopped von Hand rufen, da das Stop-Notify ggf. sonst nicht
            // durchkommen kann.
            pWin->BasicStopped();
            pWin = rWindows.Next();
        }
    }
    BasicIDE::BasicStopped();
}

void BasicIDE::BasicStopped( BOOL* pbAppWindowDisabled,
        BOOL* pbDispatcherLocked, USHORT* pnWaitCount,
        SfxUInt16Item** ppSWActionCount, SfxUInt16Item** ppSWLockViewCount )
{
    // Nach einem Error oder dem expliziten abbrechen des Basics muessen
    // ggf. einige Locks entfernt werden...

    if ( pbAppWindowDisabled )
        *pbAppWindowDisabled = FALSE;
    if ( pbDispatcherLocked )
        *pbDispatcherLocked = FALSE;
    if ( pnWaitCount )
        *pnWaitCount = 0;
    if ( ppSWActionCount )
        *ppSWActionCount = 0;
    if ( ppSWLockViewCount )
        *ppSWLockViewCount = 0;

    // AppWait ?
    USHORT nWait = 0;
    while ( Application::IsWait() )
    {
        Application::LeaveWait();
        nWait++;
    }
    if ( pnWaitCount )
        *pnWaitCount = nWait;

    // Interactive = FALSE ?
    if ( SFX_APP()->IsDispatcherLocked() )
    {
        SFX_APP()->LockDispatcher( FALSE );
        if ( pbDispatcherLocked )
            *pbDispatcherLocked = TRUE;
    }
    Window* pDefParent = Application::GetDefDialogParent();
    if ( pDefParent && !pDefParent->IsEnabled() )
    {
        // Aber nicht wenn sich noch ein Dialog im Testmodus befindet!
//      if ( pDefParent == Application::GetAppWindow() )
        {
//          Application::GetAppWindow()->Enable( TRUE );
            pDefParent->Enable( TRUE );
            if ( pbAppWindowDisabled )
                *pbAppWindowDisabled = TRUE;
        }
    }

}

void BasicIDE::InvalidateDebuggerSlots()
{
    SfxBindings& rBindings = BasicIDE::GetBindings();
    rBindings.Invalidate( SID_BASICSTOP );
    rBindings.Update( SID_BASICSTOP );
    rBindings.Invalidate( SID_BASICRUN );
    rBindings.Update( SID_BASICRUN );
    rBindings.Invalidate( SID_BASICCOMPILE );
    rBindings.Update( SID_BASICCOMPILE );
    rBindings.Invalidate( SID_BASICSTEPOVER );
    rBindings.Update( SID_BASICSTEPOVER );
    rBindings.Invalidate( SID_BASICSTEPINTO );
    rBindings.Update( SID_BASICSTEPINTO );
    rBindings.Invalidate( SID_BASICSTEPOUT );
    rBindings.Update( SID_BASICSTEPOUT );
    rBindings.Invalidate( SID_BASICIDE_TOGGLEBRKPNT );
    rBindings.Update( SID_BASICIDE_TOGGLEBRKPNT );
    rBindings.Invalidate( SID_BASICIDE_STAT_POS );
    rBindings.Update( SID_BASICIDE_STAT_POS );
}

void BasicIDE::HandleBasicError()
{
    ErrorHandler::HandleError( StarBASIC::GetErrorCode() );
}

SvStrings* BasicIDE::CreateBasicLibBoxEntries()
{
    SvStrings* pStrings = new SvStrings;
    pStrings->Insert( new String( IDEResId( RID_STR_ALL ) ), 0 );

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

            USHORT nLibs = pBasicMgr->GetLibCount();
            for ( USHORT nLib = 0; nLib < nLibs; nLib++ )
            {
                // Auch nicht geladene Libs anbieten...
                String aLibName = pBasicMgr->GetLibName( nLib );
                String* pEntry = new String( CreateMgrAndLibStr( aBasMgr, aLibName ) );
                pStrings->Insert( pEntry, pStrings->Count() );
            }
        }

        if ( pDocShell  )
            pDocShell = SfxObjectShell::GetNext( *pDocShell );
        else
            pDocShell = SfxObjectShell::GetFirst();

        pBasicMgr = ( pDocShell ? pDocShell->GetBasicManager() : 0 );
    }
    return pStrings;
}

SfxBindings& BasicIDE::GetBindings()
{
    BasicIDEDLL* pIDEDLL = IDE_DLL();
    if ( pIDEDLL && pIDEDLL->GetShell() )
    {
        // #63960# fuer BasicIDE die Bindings der APP
        // 07/00 Now the APP Dispatcher is 'dead', SFX changes...
//      return SFX_APP()->GetAppBindings();
        return pIDEDLL->GetShell()->GetFrame()->GetBindings();
    }
    SfxViewFrame* pCurFrame = SfxViewFrame::Current();
    DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
    return pCurFrame->GetBindings();
}

