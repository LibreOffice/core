/*************************************************************************
 *
 *  $RCSfile: bastype3.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mh $ $Date: 2000-09-29 11:02:36 $
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
#define _SI_NOSBXCONTROLS
#include <vcsbx.hxx>
#include <svtools/sbx.hxx>
#include <sidll.hxx>
#include <bastype2.hxx>
#include <baside2.hrc>
#include <iderid.hxx>
#include <bastypes.hxx>

SV_DECL_VARARR( EntryArray, SvLBoxEntry*, 4, 4 );

SV_IMPL_VARARR( EntryArray, SvLBoxEntry*);

USHORT GetDialogSbxId()
{
    // damit nicht ueberall si.hxx includet werden muss!
    return SBXID_DIALOG;
}



void __EXPORT BasicTreeListBox::RequestingChilds( SvLBoxEntry* pEntry )
{
    BasicEntry* pUser = (BasicEntry*)pEntry->GetUserData();
    if ( ( pUser->GetType() == OBJTYPE_SUBOBJ ) ||
         ( pUser->GetType() == OBJTYPE_OBJECT ) )
    {
        SbxObject* pObj = FindObject( pEntry );
        DBG_ASSERT( pObj, "RequestingChilds: Kein gueltiges Objekt");
        if ( pObj )
            ScanSbxObject( pObj, pEntry );
    }
    else if ( pUser->GetType() == OBJTYPE_LIB )
    {
        String aLibName = GetEntryText( pEntry );
        SvLBoxEntry* pParent = GetParent( pEntry );
        pUser = (BasicEntry*)pParent->GetUserData();
        DBG_ASSERT( pUser->GetType() == OBJTYPE_BASICMANAGER, "BasicManager?" );
        BasicManager* pBasMgr = ((BasicManagerEntry*)pUser)->GetBasicManager();
        USHORT nLib = pBasMgr->GetLibId( aLibName );

        BOOL bOK = TRUE;
        if ( pBasMgr->HasPassword( nLib ) &&
                !pBasMgr->IsPasswordVerified( nLib ) )
        {
            bOK = QueryPassword( pBasMgr, nLib );
        }
        if ( bOK )
        {
            if ( !pBasMgr->IsLibLoaded( nLib ) )
                pBasMgr->LoadLib( nLib );

            StarBASIC* pLib = pBasMgr->GetLib( nLib );
            if ( pLib )
            {
                ImpCreateLibSubEntries( pEntry, pLib );

                // Das Image austauschen...
                Image aImage( aImages.GetImage( IMGID_LIB ) );
                SetExpandedEntryBmp( pEntry, aImage );
                SetCollapsedEntryBmp( pEntry, aImage );
            }
            else
            {
                // Lib konnte nicht geladen werden...
                ErrorBox( this, WB_OK|WB_DEF_OK, String( IDEResId( RID_STR_ERROROPENLIB ) ) ).Execute();
            }
        }
    }
    else
        DBG_ERROR( "RequestingChilds: Was?" );
}



void __EXPORT BasicTreeListBox::ExpandedHdl()
{
    SvLBoxEntry* pEntry = GetHdlEntry();
    DBG_ASSERT( pEntry, "Was wurde zugeklappt?" );
    // Die OnDemand erzeugten Childs loeschen,
    // SubChilds werden automatisch geloescht.
    if ( !IsExpanded( pEntry ) && pEntry->HasChildsOnDemand() )
    {
        SvLBoxEntry* pChild = FirstChild( pEntry );
        while ( pChild )
        {
            GetModel()->Remove( pChild );   // Ruft auch den DTOR
            pChild = FirstChild( pEntry );
        }
    }
}



void BasicTreeListBox::ScanAllBasics()
{
    ScanBasic( SFX_APP()->GetBasicManager(), Application::GetAppName() );
    SfxObjectShell* pDocShell = SfxObjectShell::GetFirst();
    while ( pDocShell )
    {
        // Nur, wenn es ein dazugehoeriges Fenster gibt, damit nicht die
        // Gecachten Docs, die nicht sichtbar sind ( Remot-Dokumente )
        BasicManager* pBasMgr = pDocShell->GetBasicManager();
        if ( ( pBasMgr != SFX_APP()->GetBasicManager() ) && ( SfxViewFrame::GetFirst( pDocShell ) ) )
            ScanBasic( pBasMgr, pDocShell->GetTitle( SFX_TITLE_FILENAME ) );
        pDocShell = SfxObjectShell::GetNext( *pDocShell );
    }
}

/*
void BasicTreeListBox::ExpandTree( BasicManager* pBasMgr )
{
    ULONG nRootPos = 0;
    SvLBoxEntry* pRootEntry = GetEntry( nRootPos );
    // Falsch:
    while ( pRootEntry && (((BasicEntry*)pRootEntry->GetUserData())->GetType() != OBJTYPE_BASICMANAGER ) )
        pRootEntry = GetEntry( ++nRootPos );

    if ( pRootEntry )
        ExpandTree( pRootEntry );
}
*/



void BasicTreeListBox::ExpandTree( SvLBoxEntry* pRootEntry )
{
    DBG_ASSERT( pRootEntry, "Keine Wurzel ?" );

    Expand( pRootEntry );

    SvLBoxEntry* pLibEntry = FirstChild( pRootEntry );
    while ( pLibEntry )
    {
        // Nur die mit Childs, sonst waere ChildsOnDemand ueberfluessig
        if ( !IsEntryProtected( pLibEntry ) && GetChildCount( pLibEntry ) )
        {
            Expand( pLibEntry );

            // Im ObjectDlg nicht alles expandieren...
            if ( !( nMode & BROWSEMODE_PROPS )  )
            {
                SvLBoxEntry* pModOrObjEntry = FirstChild( pLibEntry );
                while ( pModOrObjEntry )
                {
                    Expand( pModOrObjEntry );
                    pModOrObjEntry = NextSibling( pModOrObjEntry );
                }
            }
        }
        pLibEntry = NextSibling( pLibEntry );
    }
}



void BasicTreeListBox::ExpandAllTrees()
{
    ULONG nRootPos = 0;
    SvLBoxEntry* pRootEntry = GetEntry( nRootPos );
    while ( pRootEntry )
    {
        ExpandTree( pRootEntry );
        pRootEntry = GetEntry( ++nRootPos );
    }
}



BYTE BasicTreeListBox::GetSelectedType()
{
    SvLBoxEntry* pEntry = GetCurEntry();

    if ( !pEntry )
        return 0;

    USHORT nDepth = GetModel()->GetDepth( pEntry );
    if ( nDepth == 0 )
        return OBJTYPE_BASICMANAGER;
    else if ( nDepth == 1 )
        return OBJTYPE_LIB;

    return ((BasicEntry*)pEntry->GetUserData())->GetType();
}



BasicManager* BasicTreeListBox::GetSelectedSbx( String& rLib, String& rModOrObj, String& rSubOrPropOrSObj )
{
    // Methode kann eigentlich weg, wenn nicht mehr in den Apps !!!
    // Die sollten dann auch direkt mit FindMethod() arbeiten!
    String aTmpStr;
    return GetSelectedSbx( rLib, rModOrObj, rSubOrPropOrSObj, aTmpStr );
}




BasicManager* BasicTreeListBox::GetSelectedSbx( String& rLib, String& rModOrObj, String& rSubOrPropOrSObj, String& rPropOrSubInSObj )
{
    SvLBoxEntry* pCurEntry = GetCurEntry();
    return GetSbx( pCurEntry, rLib, rModOrObj, rSubOrPropOrSObj, rPropOrSubInSObj );
}



BasicManager* BasicTreeListBox::GetSbx( SvLBoxEntry* pEntry, String& rLib, String& rModOrObj, String& rSubOrPropOrSObj, String& rPropOrSubInSObj )
{
    rLib.Erase();
    rModOrObj.Erase();
    rSubOrPropOrSObj.Erase();
    rPropOrSubInSObj.Erase();
    BasicManager* pBasMgr = 0;

    while ( pEntry )
    {
        USHORT nDepth = GetModel()->GetDepth( pEntry );
        switch ( nDepth )
        {
            case 4: rPropOrSubInSObj = GetEntryText( pEntry );
                    break;
            case 3: rSubOrPropOrSObj = GetEntryText( pEntry );
                    break;
            case 2: rModOrObj = GetEntryText( pEntry );
                    break;
            case 1: rLib = GetEntryText( pEntry );
                    break;
            case 0: pBasMgr = ((BasicManagerEntry*)pEntry->GetUserData())->GetBasicManager();
                    break;
        }
        pEntry = GetParent( pEntry );
    }
    return pBasMgr;
}




SbxVariable* BasicTreeListBox::FindVariable( SvLBoxEntry* pEntry )
{
    if ( !pEntry )
        return 0;

    String aLib, aModOrObj, aSubOrPropOrSObj, aPropOrSubInSObj;
    BasicManager* pBasMgr = 0;
    EntryArray aEntries;

    while ( pEntry )
    {
        USHORT nDepth = GetModel()->GetDepth( pEntry );
        switch ( nDepth )
        {
            case 4:
            case 3:
            case 2:
            case 1: {
                        aEntries.C40_INSERT( SvLBoxEntry, pEntry, 0 );
                        break;
                    }
            case 0: pBasMgr = ((BasicManagerEntry*)pEntry->GetUserData())->GetBasicManager();
                    break;
        }
        pEntry = GetParent( pEntry );
    }

    DBG_ASSERT( pBasMgr, "Fuer den Eintrag keinen BasicManager gefunden!" );
    SbxVariable* pVar = 0;
    if ( pBasMgr && aEntries.Count() )
    {
        for ( USHORT n = 0; n < aEntries.Count(); n++ )
        {
            SvLBoxEntry* pLE = aEntries[n];
            DBG_ASSERT( pLE, "Entrie im Array nicht gefunden" );
            BasicEntry* pBE = (BasicEntry*)pLE->GetUserData();
            DBG_ASSERT( pBE, "Keine Daten im Eintrag gefunden!" );
            String aName( GetEntryText( pLE ) );

            switch ( pBE->GetType() )
            {
                case OBJTYPE_LIB:
                {
                    pVar = pBasMgr->GetLib( aName );
                }
                break;
                case OBJTYPE_MODULE:
                {
                    DBG_ASSERT( pVar && pVar->IsA( TYPE(StarBASIC) ), "FindVariable: Ungueltiges Basic" );
                    pVar = ((StarBASIC*)pVar)->FindModule( aName );
                }
                break;
                case OBJTYPE_METHOD:
                case OBJTYPE_METHODINOBJ:
                {
                    DBG_ASSERT( pVar && ( (pVar->IsA( TYPE(SbModule) )) || (pVar->IsA( TYPE(SbxObject) )) ), "FindVariable: Ungueltiges Modul/Objekt" );
                    pVar = ((SbxObject*)pVar)->GetMethods()->Find( aName, SbxCLASS_METHOD );
                }
                break;
                case OBJTYPE_OBJECT:
                case OBJTYPE_SUBOBJ:
                {
                    DBG_ASSERT( pVar && pVar->IsA( TYPE(SbxObject) ), "FindVariable: Ungueltiges Objekt" );
                    pVar = ((SbxObject*)pVar)->GetObjects()->Find( aName, SbxCLASS_OBJECT );
                }
                break;
                case OBJTYPE_PROPERTY:
                {
                    DBG_ASSERT( pVar && pVar->IsA( TYPE(SbxObject) ), "FindVariable: Ungueltiges Objekt(Property)" );
                    pVar = ((SbxObject*)pVar)->GetProperties()->Find( aName, SbxCLASS_PROPERTY );
                }
                break;
                default:    DBG_ERROR( "FindVariable: Unbekannter Typ!" );
                            pVar = 0;
            }
            if ( !pVar )
                break;
        }
    }

    return pVar;
}



SbxObject* BasicTreeListBox::FindObject( SvLBoxEntry* pEntry )
{
    SbxVariable* pVar = FindVariable( pEntry );
    if ( pVar && pVar->IsA( TYPE(SbxObject) ) )
        return (SbxObject*)pVar;
    return 0;
}



SbMethod* BasicTreeListBox::FindMethod( SvLBoxEntry* pEntry )
{
    SbxVariable* pVar = FindVariable( pEntry );
    if ( pVar && pVar->IsA( TYPE(SbMethod) ) )
        return (SbMethod*)pVar;
    return 0;
}



SbModule* BasicTreeListBox::FindModule( SvLBoxEntry* pEntry )
{
    SbxVariable* pVar = FindVariable( pEntry );
    if ( pVar && pVar->IsA( TYPE(SbModule ) ) )
        return (SbModule*)pVar;
    return 0;
}



SvLBoxEntry* BasicTreeListBox::FindLibEntry( StarBASIC* pLib )
{
    if ( !pLib )
        return 0;

    ULONG nRootPos = 0;
    SvLBoxEntry* pRootEntry = GetEntry( nRootPos );
    while ( pRootEntry )
    {
        BasicManager* pBasMgr = ((BasicManagerEntry*)pRootEntry->GetUserData())->GetBasicManager();
        DBG_ASSERT( pBasMgr, "Kein BasicManager?" );
        SvLBoxEntry* pLibEntry = FirstChild( pRootEntry );
        while ( pLibEntry )
        {
            DBG_ASSERT( (((BasicEntry*)pLibEntry->GetUserData())->GetType() == OBJTYPE_LIB ), "Kein Libeintrag?" );
            StarBASIC* pL = pBasMgr->GetLib( GetEntryText( pLibEntry ) );
            if ( pL == pLib )
                return pLibEntry;

            pLibEntry = NextSibling( pLibEntry );
        }
        pRootEntry = GetEntry( ++nRootPos );
    }
    return 0;
}



String CreateMgrAndLibStr( const String& rMgrName, const String& rLibName )
{
    String aName( '[' );
    aName += rMgrName;
    aName += String( RTL_CONSTASCII_USTRINGPARAM( "]." ) );
    aName += rLibName;
    return aName;
}



String GetMgrFromMgrAndLib( const String& rMgrAndLib )
{
    // Format: [XXXXXX].Lib
    DBG_ASSERT( rMgrAndLib.GetTokenCount( '.' ) >= 2, "BasMgrAndLib ungueltig! (.)" );
    DBG_ASSERT( rMgrAndLib.GetTokenCount( ']' ) >= 2, "BasMgrAndLib ungueltig! (])" );
    String aLib( rMgrAndLib.GetToken(
            rMgrAndLib.GetTokenCount( '.' ) - 1, '.' ) );
    String aBasMgrAndLib( rMgrAndLib );
    aBasMgrAndLib.Erase( aBasMgrAndLib.Len() - ( aLib.Len() + 1 ) );
    DBG_ASSERT( ( aBasMgrAndLib.Len() > 2 ) && ( aBasMgrAndLib.GetChar( 0 ) == '[' ) && ( aBasMgrAndLib.GetChar( aBasMgrAndLib.Len() - 1 ) == ']' ), "BasMgrAndLib ungueltig! ([...])" );
    String aBasMgr( aBasMgrAndLib, 1, (USHORT)(aBasMgrAndLib.Len()-2) );
    return aBasMgr;
}



String GetLibFromMgrAndLib( const String& rMgrAndLib )
{
    // Format: [XXXXXX].Lib
    DBG_ASSERT( rMgrAndLib.GetTokenCount( '.' ) >= 2, "BasMgrAndLib ungueltig! (.)" );
    DBG_ASSERT( rMgrAndLib.GetTokenCount( ']' ) >= 2, "BasMgrAndLib ungueltig! (])" );
    String aLib( rMgrAndLib.GetToken(
            rMgrAndLib.GetTokenCount( '.' ) - 1, '.' ) );
    return aLib;
}


void EnableBasicDialogs( BOOL bEnable )
{
    SiDLL* pSiDLL = SI_DLL();
    USHORT nDlgs = pSiDLL->GetSbxDlgCount();
    for ( USHORT nDlg = 0; nDlg < nDlgs; nDlg++ )
    {
        VCSbxDialog* pVCDlg = pSiDLL->GetSbxDlg( nDlg );
        Dialog* pRealDlg = pVCDlg->GetDialog();
        if ( pRealDlg )
        {
            if ( bEnable )
                pRealDlg->Enable();
            else
                pRealDlg->Disable();
        }
    }
}

