/*************************************************************************
 *
 *  $RCSfile: bastype2.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: sb $ $Date: 2002-07-03 15:48:15 $
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

#include <memory>

#include "vcl/bitmap.hxx"

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

#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINER_HPP_
#include <com/sun/star/script/XLibraryContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINERPASSWORD_HPP_
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;


BasicTreeListBox::BasicTreeListBox( Window* pParent, const ResId& rRes ) :
    SvTreeListBox( pParent, IDEResId( rRes.GetId() ) ),
    m_aImagesNormal(IDEResId(RID_IMGLST_OBJECTS)),
    m_aImagesHighContrast(IDEResId(RID_IMGLST_OBJECTS_HC))
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
    // can be called multiple times for updating!

    DBG_ASSERT( pBasMgr, "BasicTreeListBox::ScanBasic: No BasicManager!" );
    SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );

    // eigentlich prueffen, ob Basic bereits im Baum ?!
    SetUpdateMode( FALSE );

    // level 1: BasicManager (application, document, ...)

    // create tree list box entry
    SvLBoxEntry* pBasicManagerRootEntry = FindEntry( 0, rName, OBJTYPE_BASICMANAGER );
    if ( !pBasicManagerRootEntry )
        pBasicManagerRootEntry = insertEntry(
            rName,
            pBasMgr == SFX_APP()->GetBasicManager()
            ? IMGID_APPICON : IMGID_DOCUMENT,
            0, false,
            std::auto_ptr< BasicEntry >(new BasicManagerEntry(pBasMgr)));

    // level 2: libraries (Standard, ...)

    // get a sorted list of library names
    Sequence< ::rtl::OUString > aLibNames = BasicIDE::GetLibraryNames( pShell );
    sal_Int32 nLibCount = aLibNames.getLength();
    const ::rtl::OUString* pLibNames = aLibNames.getConstArray();

    for ( sal_Int32 i = 0 ; i < nLibCount ; i++ )
    {
        String aLibName = pLibNames[ i ];
        ::rtl::OUString aOULibName( aLibName );

        // check, if the module library is loaded
        BOOL bModLibLoaded = FALSE;
        Reference< script::XLibraryContainer > xModLibContainer = BasicIDE::GetModuleLibraryContainer( pShell );
        if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryLoaded( aOULibName ) )
            bModLibLoaded = TRUE;

        // check, if the dialog library is loaded
        BOOL bDlgLibLoaded = FALSE;
        Reference< script::XLibraryContainer > xDlgLibContainer = BasicIDE::GetDialogLibraryContainer( pShell );
        if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryLoaded( aOULibName ) )
            bDlgLibLoaded = TRUE;

        BOOL bLoaded = bModLibLoaded || bDlgLibLoaded;

        // if only one of the libraries is loaded, load also the other
        if ( bLoaded )
        {
            if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && !xModLibContainer->isLibraryLoaded( aOULibName ) )
                xModLibContainer->loadLibrary( aOULibName );

            if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && !xDlgLibContainer->isLibraryLoaded( aOULibName ) )
                xDlgLibContainer->loadLibrary( aOULibName );
        }

        // create a tree list box entry
        SvLBoxEntry* pLibRootEntry = FindEntry( pBasicManagerRootEntry, aLibName, OBJTYPE_LIB );
        if ( !pLibRootEntry )
            pLibRootEntry = insertEntry(
                aLibName, bLoaded ? IMGID_LIB : IMGID_LIBNOTLOADED,
                pBasicManagerRootEntry, !bLoaded,
                std::auto_ptr< BasicEntry >(new BasicEntry(OBJTYPE_LIB)));

        // create the sub entries
        if ( bLoaded )
            ImpCreateLibSubEntries( pLibRootEntry, pShell, aLibName );
    }

    SetUpdateMode( TRUE );
}

void BasicTreeListBox::ImpCreateLibSubEntries( SvLBoxEntry* pLibRootEntry, SfxObjectShell* pShell, const String& rLibName )
{
    ::rtl::OUString aOULibName( rLibName );

    // modules
    if ( nMode & BROWSEMODE_MODULES )
    {
        Reference< script::XLibraryContainer > xModLibContainer = BasicIDE::GetModuleLibraryContainer( pShell );

        if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryLoaded( aOULibName ) )
        {
            try
            {
                // get a sorted list of module names
                Sequence< ::rtl::OUString > aModNames = BasicIDE::GetModuleNames( pShell, rLibName );
                sal_Int32 nModCount = aModNames.getLength();
                const ::rtl::OUString* pModNames = aModNames.getConstArray();

                for ( sal_Int32 i = 0 ; i < nModCount ; i++ )
                {
                    String aModName = pModNames[ i ];
                    SvLBoxEntry* pModuleEntry = FindEntry( pLibRootEntry, aModName, OBJTYPE_MODULE );
                    if ( !pModuleEntry )
                        pModuleEntry = insertEntry(
                            aModName, IMGID_MODULE, pLibRootEntry, false,
                            std::auto_ptr< BasicEntry >(
                                new BasicEntry(OBJTYPE_MODULE)));

                    // methods
                    if ( nMode & BROWSEMODE_SUBS )
                    {
                        Sequence< ::rtl::OUString > aNames = BasicIDE::GetMethodNames( pShell, rLibName, aModName );
                        sal_Int32 nCount = aNames.getLength();
                        const ::rtl::OUString* pNames = aNames.getConstArray();

                        for ( sal_Int32 j = 0 ; j < nCount ; j++ )
                        {
                            String aName = pNames[ j ];
                            SvLBoxEntry* pEntry = FindEntry( pModuleEntry, aName, OBJTYPE_METHOD );
                            if ( !pEntry )
                                pEntry = insertEntry(
                                    aName, IMGID_MACRO, pModuleEntry,
                                    false,
                                    std::auto_ptr< BasicEntry >(
                                        new BasicEntry(OBJTYPE_METHOD)));
                        }
                    }
                }
            }
            catch ( container::NoSuchElementException& e )
            {
                ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
                DBG_ERROR( aBStr.GetBuffer() );
            }
        }
    }

    // dialogs
    if ( nMode & BROWSEMODE_OBJS )
    {
         Reference< script::XLibraryContainer > xDlgLibContainer = BasicIDE::GetDialogLibraryContainer( pShell );

         if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryLoaded( aOULibName ) )
         {
            try
            {
                // get a sorted list of dialog names
                Sequence< ::rtl::OUString > aDlgNames = BasicIDE::GetDialogNames( pShell, rLibName );
                sal_Int32 nDlgCount = aDlgNames.getLength();
                const ::rtl::OUString* pDlgNames = aDlgNames.getConstArray();

                for ( sal_Int32 i = 0 ; i < nDlgCount ; i++ )
                {
                    String aDlgName = pDlgNames[ i ];
                    SvLBoxEntry* pDialogEntry = FindEntry( pLibRootEntry, aDlgName, OBJTYPE_OBJECT );
                    if ( !pDialogEntry )
                        pDialogEntry = insertEntry(
                            aDlgName, IMGID_OBJECT, pLibRootEntry, true,
                            std::auto_ptr< BasicEntry >(
                                new BasicEntry(OBJTYPE_OBJECT)));
                }
            }
            catch ( container::NoSuchElementException& e )
            {
                ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
                DBG_ERROR( aBStr.GetBuffer() );
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

        // probably it's a BasicManager
        if ( !bFound && !GetModel()->GetDepth( pEntry ) )
        {
            bFound = BasicIDE::FindBasicManager( GetEntryText( pEntry ) ) ? TRUE : FALSE;
        }

        // probably it's an unloaded library
        // (FindVariable fails for unloaded libraries!)
        if ( !bFound && GetModel()->GetDepth( pEntry ) == 1 )
        {
            BasicManager* pBasMgr = BasicIDE::FindBasicManager( GetEntryText( GetParent( pEntry ) ) );
            if ( pBasMgr )
                bFound = pBasMgr->HasLib( GetEntryText( pEntry ) );
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

void BasicTreeListBox::ScanSbxObject( SbxObject* pObj, SvLBoxEntry* pObjEntry )     // not called anymore (sbx dialogs removed)
{
    // die Methoden des Objects...
    if ( nMode & BROWSEMODE_SUBS )
    {
        for ( USHORT nMethod = 0; nMethod < pObj->GetMethods()->Count(); nMethod++ )
        {
            SbMethod* pMethod= (SbMethod*) pObj->GetMethods()->Get( nMethod );
            DBG_ASSERT( pMethod , "Methode nicht gefunden! (NULL)" );
            insertEntry(pMethod->GetName(), IMGID_MACRO, pObjEntry, false,
                        std::auto_ptr< BasicEntry >(
                            new BasicEntry(OBJTYPE_METHODINOBJ)));
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
            insertEntry(pVar->GetName(), IMGID_PROP, pObjEntry, false,
                        std::auto_ptr< BasicEntry >(
                            new BasicEntry(OBJTYPE_PROPERTY)));
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
                // SubObjecte erhalten ChildsOnDemand und koennen
                // jederzeit weiter aufgeklappt werden...
                insertEntry(pVar->GetName(), IMGID_SUBOBJ, pObjEntry, true,
                            std::auto_ptr< BasicEntry >(
                                new BasicEntry(OBJTYPE_SUBOBJ)));
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
        String aLibName, aDummy1, aDummy2;
        BasicManager* pBasMgr = GetSelectedSbx( aLibName, aDummy1, aDummy2 );
        SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );

        if ( aLibName.Len() && !aDummy1.Len() && !aDummy2.Len() )
        {
            // check password, if library is password protected and not verified
            ::rtl::OUString aOULibName( aLibName );
            Reference< script::XLibraryContainer > xModLibContainer( BasicIDE::GetModuleLibraryContainer( pShell ), UNO_QUERY );
            if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
            {
                Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
                {
                    String aPassword;
                    bOK = QueryPassword( xModLibContainer, aLibName, aPassword );
                }
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
        String aLibName, aDummy1, aDummy2, aDummy3;
        BasicManager* pBasMgr = GetSbx( pEntry, aLibName, aDummy1, aDummy2, aDummy3 );
        SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );
        ::rtl::OUString aOULibName( aLibName );
        Reference< script::XLibraryContainer > xModLibContainer( BasicIDE::GetModuleLibraryContainer( pShell ), UNO_QUERY );
        if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
        {
            Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
            if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
            {
                bProtected = TRUE;
            }
        }
    }
    return bProtected;
}

SvLBoxEntry * BasicTreeListBox::insertEntry(
    String const & rText, USHORT nBitmap, SvLBoxEntry * pParent,
    bool bChildrenOnDemand, std::auto_ptr< BasicEntry > aUserData)
{
    Image aImage(m_aImagesNormal.GetImage(nBitmap));
    SvLBoxEntry * p = InsertEntry(
        rText, aImage, aImage, pParent, bChildrenOnDemand, LIST_APPEND,
        aUserData.release()); // XXX possible leak
    aImage = m_aImagesHighContrast.GetImage(nBitmap);
    SetExpandedEntryBmp(p, aImage, BMP_COLOR_HIGHCONTRAST);
    SetCollapsedEntryBmp(p, aImage, BMP_COLOR_HIGHCONTRAST);
    return p;
}

void BasicTreeListBox::setEntryBitmap(SvLBoxEntry * pEntry, USHORT nBitmap)
{
    Image aImage(m_aImagesNormal.GetImage(nBitmap));
    SetExpandedEntryBmp(pEntry, aImage, BMP_COLOR_NORMAL);
    SetCollapsedEntryBmp(pEntry, aImage, BMP_COLOR_NORMAL);
    aImage = m_aImagesHighContrast.GetImage(nBitmap);
    SetExpandedEntryBmp(pEntry, aImage, BMP_COLOR_HIGHCONTRAST);
    SetCollapsedEntryBmp(pEntry, aImage, BMP_COLOR_HIGHCONTRAST);
}
