/*************************************************************************
 *
 *  $RCSfile: bastype2.cxx,v $
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

#include <basidesh.hrc>
#include <bastypes.hxx>
#include <bastype2.hxx>
#include <basobj.hxx>
#include <baside2.hrc>
#include <iderid.hxx>

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif



BasicTreeListBox::BasicTreeListBox( Window* pParent, const ResId& rRes ) :
    SvTreeListBox( pParent, IDEResId( rRes.GetId() ) ),
    aImages( IDEResId( RID_IMGLST_OBJECTS ) )
{
    SetSelectionMode( SINGLE_SELECTION );
    nMode = 0xFF;   // Alles
}



BasicTreeListBox::~BasicTreeListBox()
{
    // UserDaten zerstoeren
    SvLBoxEntry* pEntry = First();
    while ( pEntry )
    {
        delete (BasicEntry*)pEntry->GetUserData();
        pEntry = Next( pEntry );
    }
}

void BasicTreeListBox::ScanBasic( BasicManager* pBasMgr, const String& rName )
{
    DBG_ASSERT( pBasMgr, "Wir scannen einen NULL-Pointer... (warum kein BasicManager?!)" );

    // eigentlich prueffen, ob Basic bereits im Baum ?!
    SetUpdateMode( FALSE );

    // Erste Ebene: Welcher BasicManager ( App, Doc, ... )
    // 30.1.96: ScanBasic kann mehrfach zum Updateten aufgerufen werden
    // => pruefen, ob Eintrag schon vorhanden.
    SvLBoxEntry* pBasicManagerRootEntry = FindEntry( 0, rName, OBJTYPE_BASICMANAGER );
    if ( !pBasicManagerRootEntry )
    {
        Image aImage( aImages.GetImage( ( pBasMgr == SFX_APP()->GetBasicManager() ) ? IMGID_APPICON : IMGID_DOCUMENT ) );
        pBasicManagerRootEntry = InsertEntry( rName, aImage, aImage, 0, FALSE, LIST_APPEND );
        pBasicManagerRootEntry->SetUserData( new BasicManagerEntry( pBasMgr ) );
    }

    // Zweite Ebene: Libs ( Standard, ... )
    USHORT nLibs = pBasMgr->GetLibCount();
    for ( USHORT nLib = 0; nLib < nLibs; nLib++ )
    {
        StarBASIC* pLib = pBasMgr->GetLib( nLib );
        String aLibName = pBasMgr->GetLibName( nLib );

        // Jetzt LoadOnDemand...
//      if ( !pLib )    // Nicht geladen...
//          continue;

        SvLBoxEntry* pLibRootEntry = FindEntry( pBasicManagerRootEntry, aLibName, OBJTYPE_LIB );
        if ( !pLibRootEntry )
        {
            Image aImage( aImages.GetImage( pLib ? IMGID_LIB : IMGID_LIBNOTLOADED ) );
            pLibRootEntry = InsertEntry( aLibName, aImage, aImage,
                pBasicManagerRootEntry, pLib ? FALSE : TRUE , LIST_APPEND );
            pLibRootEntry->SetUserData( new BasicEntry( OBJTYPE_LIB ) );
        }

        if ( pLib )
            ImpCreateLibSubEntries( pLibRootEntry, pLib );
    }

    SetUpdateMode( TRUE );
}

void BasicTreeListBox::ImpCreateLibSubEntries( SvLBoxEntry* pLibRootEntry, StarBASIC* pLib )
{
    // Erst die Module...
    if ( pLib && ( nMode & BROWSEMODE_MODULES ) )
    {
        for ( USHORT nMod = 0; nMod < pLib->GetModules()->Count(); nMod++ )
        {
            SbModule* pModule = (SbModule*)pLib->GetModules()->Get( nMod );
            DBG_ASSERT( pModule, "Modul nicht erhalten!" );
            String aName( pModule->GetName() );
            SvLBoxEntry* pModuleEntry = FindEntry( pLibRootEntry, aName, OBJTYPE_MODULE );
            if ( !pModuleEntry )
            {
                USHORT nImgId = IMGID_MODULE;
                Image aImage( aImages.GetImage( nImgId ) );
                pModuleEntry = InsertEntry( aName, aImage, aImage, pLibRootEntry, FALSE, LIST_APPEND );
                pModuleEntry->SetUserData( new BasicEntry( OBJTYPE_MODULE ) );
            }

            if ( nMode & BROWSEMODE_SUBS )
            {
                Image aImage( aImages.GetImage( IMGID_MACRO ) );
                for ( USHORT nMethod = 0; nMethod < pModule->GetMethods()->Count(); nMethod++ )
                {
                    SbMethod* pMethod = (SbMethod*)pModule->GetMethods()->Get( nMethod );
                    DBG_ASSERT( pMethod, "Methode nicht gefunden! (NULL)" );
                    SvLBoxEntry* pEntry = FindEntry( pModuleEntry, pMethod->GetName(), OBJTYPE_METHOD );
                    if ( !pEntry )
                    {
                        pEntry = InsertEntry( pMethod->GetName(), aImage, aImage, pModuleEntry, FALSE, LIST_APPEND );
                        pEntry->SetUserData( new BasicEntry( OBJTYPE_METHOD ) );
                    }
                }
            }
        }
    }

    // ...und dann die lebenden Objecte.
    if ( pLib && ( nMode & BROWSEMODE_OBJS ) )
    {
        pLib->GetAll( SbxCLASS_OBJECT );
        for ( USHORT nObject = 0; nObject < pLib->GetObjects()->Count(); nObject++ )
        {
            SbxVariable* pVar = pLib->GetObjects()->Get( nObject );
#ifndef BASICDEBUG
            if ( pVar->GetSbxId() == GetDialogSbxId() )
#endif
            {
                // Dialoge erhalten ChildsOnDemand und koennen
                // jederzeit weiter aufgeklappt werden...
                SvLBoxEntry* pEntry = FindEntry( pLibRootEntry, pVar->GetName(), OBJTYPE_OBJECT );
                if ( !pEntry )
                {
                    pEntry = InsertEntry( pVar->GetName(), aImages.GetImage( IMGID_OBJECT ), aImages.GetImage( IMGID_OBJECT ), pLibRootEntry, TRUE, LIST_APPEND );
                    pEntry->SetUserData( new BasicEntry( OBJTYPE_OBJECT ) );
                }
            }
        }
    }

}

void BasicTreeListBox::UpdateEntries()
{
    String aCurEntry = CreateEntryDescription( *this, FirstSelected() );

    // Erstmal die vorhandenen Eintraege auf existens pruefen:
    SvLBoxEntry* pLastValid = 0;
    SvLBoxEntry* pEntry = First();
    while ( pEntry )
    {
        BOOL bFound = FindVariable( pEntry ) ? TRUE : FALSE;
        if ( !bFound && !GetModel()->GetDepth( pEntry ) )
        {
            // Vielleicht ist es ein BasicManager?
            bFound = BasicIDE::FindBasicManager( GetEntryText( pEntry ) ) ? TRUE : FALSE;
        }

        if ( bFound )
            pLastValid = pEntry;
        else
        {
            delete (BasicEntry*)pEntry->GetUserData();
            GetModel()->Remove( pEntry );
        }
        pEntry = pLastValid ? Next( pLastValid ) : First();
    }

    // Jetzt ueber die Basics rennen und in die Zweige eintragen
    ScanAllBasics();

    SvLBoxEntry* pNewCurrent = FindMostMatchingEntry( *this, aCurEntry );
    if ( pNewCurrent )
        Select( pNewCurrent, TRUE );
}

void BasicTreeListBox::ScanSbxObject( SbxObject* pObj, SvLBoxEntry* pObjEntry )
{
    // die Methoden des Objects...
    if ( nMode & BROWSEMODE_SUBS )
    {
        for ( USHORT nMethod = 0; nMethod < pObj->GetMethods()->Count(); nMethod++ )
        {
            SbMethod* pMethod= (SbMethod*) pObj->GetMethods()->Get( nMethod );
            DBG_ASSERT( pMethod , "Methode nicht gefunden! (NULL)" );
            SvLBoxEntry* pEntry = InsertEntry( pMethod->GetName(), aImages.GetImage( IMGID_MACRO ), aImages.GetImage( IMGID_MACRO ), pObjEntry, FALSE, LIST_APPEND );
            pEntry->SetUserData( new BasicEntry( OBJTYPE_METHODINOBJ ) );
        }
    }

    // die Properties des Objects...
    if ( nMode & BROWSEMODE_PROPS )
    {
        pObj->GetAll( SbxCLASS_PROPERTY );
        for ( USHORT nProp = 0; nProp < pObj->GetProperties()->Count(); nProp++ )
        {
            SbxVariable* pVar = pObj->GetProperties()->Get( nProp );
            DBG_ASSERT( pVar, "Property nicht gefunden! (NULL)" );
            SvLBoxEntry* pEntry = InsertEntry( pVar->GetName(), aImages.GetImage( IMGID_PROP ), aImages.GetImage( IMGID_PROP ), pObjEntry, FALSE, LIST_APPEND );
            pEntry->SetUserData( new BasicEntry( OBJTYPE_PROPERTY ) );
        }
    }

    // die Subobjecte
    if ( nMode & BROWSEMODE_SUBOBJS )
    {
        pObj->GetAll( SbxCLASS_OBJECT );
        for ( USHORT nObject = 0; nObject < pObj->GetObjects()->Count(); nObject++ )
        {
            SbxVariable* pVar = pObj->GetObjects()->Get( nObject );
            if ( pVar->GetClass() == SbxCLASS_OBJECT )
            {
                // SubObjecte erhalten ChildsOnDemand und koennen
                // jederzeit weiter aufgeklappt werden...
                SvLBoxEntry* pEntry = InsertEntry( pVar->GetName(), aImages.GetImage( IMGID_SUBOBJ ), aImages.GetImage( IMGID_SUBOBJ ), pObjEntry, TRUE, LIST_APPEND );
                pEntry->SetUserData( new BasicEntry( OBJTYPE_SUBOBJ ) );
            }
        }
    }
}


SvLBoxEntry* __EXPORT BasicTreeListBox::CloneEntry( SvLBoxEntry* pSource )
{
    SvLBoxEntry* pNew = SvTreeListBox::CloneEntry( pSource );
    BasicEntry* pUser = (BasicEntry*)pSource->GetUserData();

    DBG_ASSERT( pUser, "User-Daten?!" );
    DBG_ASSERT( pUser->GetType() != OBJTYPE_BASICMANAGER, "BasMgr-Daten?!" );

    BasicEntry* pNewUser = new BasicEntry( *pUser );
    pNew->SetUserData( pNewUser );
    return pNew;
}

SvLBoxEntry* BasicTreeListBox::FindEntry( SvLBoxEntry* pParent, const String& rText, BYTE nType )
{
    ULONG nRootPos = 0;
    SvLBoxEntry* pEntry = pParent ? FirstChild( pParent ) : GetEntry( nRootPos );
    while ( pEntry )
    {
        BasicEntry* pBasicEntry = (BasicEntry*)pEntry->GetUserData();
        DBG_ASSERT( pBasicEntry, "FindEntry: Kein BasicEntry ?!" );
        if ( ( pBasicEntry->GetType() == nType  ) && ( GetEntryText( pEntry ) == rText ) )
            return pEntry;

        pEntry = pParent ? NextSibling( pEntry ) : GetEntry( ++nRootPos );
    }
    return 0;
}

long BasicTreeListBox::ExpandingHdl()
{
    // Expanding oder Collaps?
    BOOL bOK = TRUE;
    if ( GetModel()->GetDepth( GetHdlEntry() ) == 1 )
    {
        String aLib, aDummy1, aDummy2;
        BasicManager* pBasicManager = GetSelectedSbx( aLib, aDummy1, aDummy2 );
        if ( aLib.Len() && !aDummy1.Len() && !aDummy2.Len() )
        {
            // Beim expandieren einer Lib pruefen, ob Passwortschutz!
            USHORT nLib = pBasicManager->GetLibId( aLib );
            if ( pBasicManager->HasPassword( nLib ) &&
                    !pBasicManager->IsPasswordVerified( nLib ) )
            {
                bOK = QueryPassword( pBasicManager, nLib );
            }
        }
    }
    return bOK;
}

BOOL BasicTreeListBox::IsEntryProtected( SvLBoxEntry* pEntry )
{
    BOOL bProtected = FALSE;
    if ( pEntry && ( GetModel()->GetDepth( pEntry ) == 1 ) )
    {
        String aLib, aDummy1, aDummy2, aDummy3;
        BasicManager* pBasicManager = GetSbx( pEntry, aLib, aDummy1, aDummy2, aDummy3 );
        USHORT nLib = pBasicManager->GetLibId( aLib );
        if ( pBasicManager->HasPassword( nLib ) &&
                !pBasicManager->IsPasswordVerified( nLib ) )
        {
            bProtected = TRUE;
        }
    }
    return bProtected;
}

