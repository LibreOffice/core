/*************************************************************************
 *
 *  $RCSfile: soldep.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: obo $ $Date: 2004-02-26 14:48:16 $
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


#include <osl/file.hxx>
#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <bootstrp/sstring.hxx>
#include <svtools/filedlg.hxx>
#include <tools/iparser.hxx>
#include <tools/geninfo.hxx>
#include <bootstrp/appdef.hxx>
#include "depper.hxx"
#include "soldep.hxx"
#include "soldlg.hxx"
#include "dtsodcmp.hrc"

IMPLEMENT_HASHTABLE_OWNER( SolIdMapper, ByteString, ULONG* );

ByteString sDelimiterLine("#==========================================================================");


//
// class SolDep
//

/*****************************************************************************/
SolDep::SolDep( Window* pBaseWindow )
/*****************************************************************************/
                : Depper( pBaseWindow ),
                mpPrjDep( NULL )
{
    mpSolIdMapper = new SolIdMapper( 63997 );
    mpStarWriter = new StarWriter( msSourceName, TRUE );

    mpBaseWin->mpPopup->InsertSeparator();
    mpBaseWin->mpPopup->InsertItem( DEPPOPUP_READ_SOURCE, String::CreateFromAscii("Revert all changes") );
    mpBaseWin->mpPopup->InsertSeparator();
    mpBaseWin->mpPopup->InsertItem( DEPPOPUP_OPEN_SOURCE, String::CreateFromAscii("Open") );
    mpBaseWin->mpPopup->InsertItem( DEPPOPUP_WRITE_SOURCE, String::CreateFromAscii("Save") );
//  mpBaseWin->mpPopup->InsertItem( DEPPOPUP_CLOSE, String::CreateFromAscii("Close") );
//  mpBaseWin->mpPopup->InsertItem( DEPPOPUP_HELP, String::CreateFromAscii("Help") );
}

/*****************************************************************************/
SolDep::~SolDep()
/*****************************************************************************/
{
    delete mpSolIdMapper;
    delete mpStarWriter;
    delete mpPrjDep;
    delete pStandLst;
}

/*****************************************************************************/
void SolDep::Init()
/*****************************************************************************/
{
    InformationParser aParser;
    String sStandLst( GetDefStandList(), RTL_TEXTENCODING_ASCII_US );
    pStandLst = aParser.Execute( sStandLst );

    if ( pStandLst ) {
        if ( GetVersion())
            ReadSource();
    }
}

/*****************************************************************************/
void SolDep::Init( ByteString &rVersion, GenericInformationList *pVersionList )
/*****************************************************************************/
{
    if ( pVersionList )
        pStandLst = new GenericInformationList( *pVersionList );
    else {
        InformationParser aParser;
        String sStandLst( GetDefStandList(), RTL_TEXTENCODING_ASCII_US );
        pStandLst = aParser.Execute( sStandLst );
    }
    if ( pStandLst ) {
        msVersion = rVersion;
        ReadSource();
    }
}

/*****************************************************************************/
BOOL SolDep::GetVersion()
/*****************************************************************************/
{
    SolSelectVersionDlg aVersionDlg( mpBaseWin, pStandLst );
    if ( aVersionDlg.Execute() == RET_OK ) {
        msVersion = aVersionDlg.GetVersion();
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************/
ObjectWin *SolDep::RemoveObject( USHORT nId, BOOL bDelete )
/*****************************************************************************/
{
    Prj* pPrj;

//hshtable auf stand halten
    ObjectWin* pWin = Depper::RemoveObject( nId, FALSE );
    if ( pWin )
    {
        ByteString aBodyText( pWin->GetBodyText() );
        if( pPrj = mpStarWriter->GetPrj( aBodyText ))
        {
            mpStarWriter->Remove( pPrj );
//cleanup ist teuer...
            mpStarWriter->CleanUp();
            delete pPrj;
        }
        else
            DBG_ASSERT( FALSE, "project not found - write" );

        mpSolIdMapper->Delete( aBodyText );
        if ( bDelete )
            delete pWin;
        return pWin;
    }
    else
        return NULL;
}

/*****************************************************************************/
ULONG SolDep::AddObject( ByteString& rBodyText, BOOL bInteract )
/*****************************************************************************/
{
    if ( bInteract )
    {
        SolNewProjectDlg aNewProjectDlg( mpBaseWin, DtSodResId( RID_SD_DIALOG_NEWPROJECT ));
        if ( aNewProjectDlg.Execute() )
        {
            rBodyText = ByteString( aNewProjectDlg.maEName.GetText(), RTL_TEXTENCODING_UTF8);
//hashtable auf stand halten
            MyHashObject* pHObject;
            ULONG nObjectId = Depper::AddObject( rBodyText, FALSE );
            pHObject = new MyHashObject( nObjectId, ObjIdToPtr( nObjectId ));
            mpSolIdMapper->Insert( rBodyText, pHObject );

            ByteString sTokenLine( aNewProjectDlg.maEShort.GetText(), RTL_TEXTENCODING_UTF8 );
            sTokenLine += '\t';
            sTokenLine += ByteString( aNewProjectDlg.maEName.GetText(), RTL_TEXTENCODING_UTF8 );
            sTokenLine += "\t:\t";

            ByteString sDeps = ByteString( aNewProjectDlg.maEDeps.GetText(), RTL_TEXTENCODING_UTF8 );

            if ( sDeps != "" )
            {
                USHORT i;
                ByteString sDepName;
                USHORT nToken = sDeps.GetTokenCount(' ');
                for ( i = 0 ; i < nToken ; i++)
                {
                    sDepName =  sDeps.GetToken( i, ' ' );
                    sTokenLine += sDepName;
                    sTokenLine +='\t';
                }
            }
            sTokenLine +="NULL";

            mpStarWriter->InsertTokenLine( sTokenLine );
            mpStarWriter->InsertTokenLine( sDelimiterLine );

            if ( sDeps != "" )
            {
                USHORT i;
                ByteString sDepName;
                ULONG nObjectId, nHashedId;
                MyHashObject* pHObject;
                USHORT nToken = sDeps.GetTokenCount(' ');
                for ( i = 0 ; i < nToken ; i++)
                {
                    sDepName =  sDeps.GetToken( i, ' ' );

                    pHObject = mpSolIdMapper->Find( sDepName );
                    if ( !pHObject )
                    {
                        String sMessage;
                        sMessage += String::CreateFromAscii("can't find ");
                        sMessage += String( sDepName, RTL_TEXTENCODING_UTF8 );
                        sMessage += String::CreateFromAscii(".\ndependency ignored");
                        WarningBox aBox( mpBaseWin, WB_OK, sMessage);
                        aBox.Execute();
                    }
                    else
                    {
                        nHashedId = pHObject->GetId();
                        pHObject = mpSolIdMapper->Find( rBodyText );
                        nObjectId = pHObject->GetId();
                        Depper::AddConnector( nHashedId, nObjectId );
                    }
                }
            }
            return nObjectId;
        }
        return 0;
    }
    else
    {
//hashtable auf stand halten
        MyHashObject* pHObject;
        ULONG nObjectId = Depper::AddObject( rBodyText, FALSE );
        pHObject = new MyHashObject( nObjectId, ObjIdToPtr( nObjectId ));
        mpSolIdMapper->Insert( rBodyText, pHObject );

        return nObjectId;
    }
}

/*****************************************************************************/
USHORT SolDep::AddConnector( ObjectWin* pStartWin, ObjectWin* pEndWin )
/*****************************************************************************/
{
//  DBG_ASSERT( FALSE , "not yet" );
    ByteString sEndName = pEndWin->GetBodyText();
    ByteString sStartName = pStartWin->GetBodyText();

    Prj* pPrj = mpStarWriter->GetPrj( sEndName );
    if ( pPrj )
    {
        pPrj->AddDependencies( sStartName );
        return Depper::AddConnector( pStartWin, pEndWin );
    }
    else
    {
        DBG_ASSERT( FALSE , "non existing Project" );
        return 1;
    }
}

/*****************************************************************************/
USHORT SolDep::RemoveConnector( ObjectWin* pStartWin, ObjectWin* pEndWin )
/*****************************************************************************/
{
    SByteStringList* pPrjDeps = NULL;
    ByteString sEndName = pEndWin->GetBodyText();
    ByteString sStartName = pStartWin->GetBodyText();

    Prj* pPrj = mpStarWriter->GetPrj( sEndName );
    pPrjDeps = pPrj->GetDependencies( FALSE );
    if ( pPrjDeps )
    {
        ByteString* pString;
        ULONG nPrjDepsCount = pPrjDeps->Count();
        for ( ULONG j = nPrjDepsCount; j > 0; j-- )
        {
            pString = pPrjDeps->GetObject( j - 1 );
            if ( pString->GetToken( 0, '.') == sStartName )
                pPrjDeps->Remove( pString );
        }
    }

    return Depper::RemoveConnector( pStartWin, pEndWin );
}

/*****************************************************************************/
void SolDep::RemoveAllObjects( ObjWinList* pObjLst )
/*****************************************************************************/
{

    Depper::RemoveAllObjects( pObjLst );

    if ( mpSolIdMapper )
    {
        delete mpSolIdMapper;
        mpSolIdMapper = NULL;
    }
    if ( mpStarWriter )
    {
        delete mpStarWriter;
        mpStarWriter = NULL;
    }
}

/*****************************************************************************/
ULONG SolDep::GetStart()
/*****************************************************************************/
{
//  DBG_ASSERT( FALSE , "soldep" );
    MyHashObject* pHObject = mpSolIdMapper->Find( "null_project" );

    if ( !pHObject ) {
        ByteString sNullProject = ByteString( "null_project" );
        ULONG nObjectId = AddObject( sNullProject, FALSE );
        ObjIdToPtr( nObjectId )->SetViewMask( 1 );
        return nObjectId;
    }

    return pHObject->GetId();
}

/*****************************************************************************/
USHORT SolDep::OpenSource()
/*****************************************************************************/
{
    if ( pStandLst ) {
        if ( GetVersion())
            return ReadSource();
    }
    return 0;
}

/*****************************************************************************/
USHORT SolDep::ReadSource()
/*****************************************************************************/
{
    mpBaseWin->EnablePaint( FALSE );
    ULONG nObjectId, nHashedId;
    ULONG i;
    MyHashObject* pHObject;
    ByteString* pStr;
    ObjectWin *pStartWin, *pEndWin;

    RemoveAllObjects( pObjectList );
    delete mpSolIdMapper;
    delete mpStarWriter;

    mpSolIdMapper = new SolIdMapper( 63997 );
    mpStarWriter = new StarWriter( pStandLst, msVersion, TRUE, getenv(SOURCEROOT) );


    ByteString sTitle( SOLDEPL_NAME );
    if ( mpStarWriter->GetMode() == STAR_MODE_SINGLE_PARSE ) {
        sTitle += ByteString( " - mode: single file [" );
           sTitle += (ByteString) mpStarWriter->GetName();
        sTitle += ByteString( "]" );
    }
    else if ( mpStarWriter->GetMode() == STAR_MODE_MULTIPLE_PARSE ) {
        sTitle += ByteString( " - mode: multiple files [" );
        sTitle += ByteString(getenv(SOURCEROOT));
        sTitle += ByteString( "]" );
    }
    SetTitle( String( sTitle, RTL_TEXTENCODING_UTF8) );

    ULONG nCount = mpStarWriter->Count();
    for ( i=0; i<nCount; i++ )
    {
        Prj *pPrj = mpStarWriter->GetObject(i);
        ByteString sProjectName = pPrj->GetProjectName();
        nObjectId = AddObject( sProjectName, FALSE );
        ObjIdToPtr( nObjectId )->SetViewMask( 1 );
    }
    for ( i=0; i<nCount; i++ )
    {
        Prj *pPrj = mpStarWriter->GetObject(i);
        SByteStringList *pLst = pPrj->GetDependencies( FALSE );
        if ( pLst )
        {
            ULONG nDepCount = pLst->Count();
            for ( ULONG m=0; m<nDepCount; m++)
            {
                pStr = pLst->GetObject(m);
                pHObject = mpSolIdMapper->Find( *pStr );
                if ( !pHObject )
                {
    // create new prj
                    Prj *pNewPrj = new Prj( *pStr );
                    ByteString sNewProjectName = pNewPrj->GetProjectName();
                    nObjectId = AddObject( sNewProjectName, FALSE );
                    pHObject = mpSolIdMapper->Find( *pStr );
                    ObjIdToPtr( nObjectId )->SetViewMask( 2 );
                }

                nHashedId = pHObject->GetId();
                ByteString sF_Os2 = pPrj->GetProjectName();
                pStr = &sF_Os2;
                pHObject = mpSolIdMapper->Find( *pStr );
                nObjectId = pHObject->GetId();
                pStartWin = ObjIdToPtr( nHashedId );
                pEndWin = ObjIdToPtr( nObjectId );
//                  Depper::AddConnector( nHashedId, nObjectId );
                Depper::AddConnector( pStartWin, pEndWin );
            }
        }
    }
    AutoArrange( GetStart(), 0 );
    mpBaseWin->EnablePaint( TRUE );
    return 0;
}

/*****************************************************************************/
USHORT SolDep::WriteSource()
/*****************************************************************************/
{
    USHORT nMode = mpStarWriter->GetMode();
    if ( nMode == STAR_MODE_SINGLE_PARSE ) {
        ByteString sFileName = mpStarWriter->GetName();
        if ( sFileName.Len()) {
            mpStarWriter->Write( String( sFileName, RTL_TEXTENCODING_UTF8) );
            mpStarWriter->RemoveProject( ByteString( "null_project"));
        }
    }
    else if ( nMode == STAR_MODE_MULTIPLE_PARSE ) {
    //*OBO*
    //String sRoot = mpStarWriter->GetSourceRoot();
    //nicht mehr unterstützt mpStarWriter->GetSourceRoot()
        String sRoot = String(getenv(SOURCEROOT), RTL_TEXTENCODING_UTF8);
        ByteString sFileName = mpStarWriter->GetName();
        DirEntry aEntry( sFileName );
        aEntry.ToAbs();
        aEntry = aEntry.GetPath().GetPath().GetPath();

        if ( sRoot.Len()) {
            mpStarWriter->RemoveProject( ByteString( "null_project"));
            mpStarWriter->WriteMultiple( sRoot );
        }
    }

    return 1;
}

/*****************************************************************************/
BOOL SolDep::ViewContent( ByteString& rObjectName )
/*****************************************************************************/
{
    pFocusWin = NULL;
    for ( ULONG i = 0; i < pObjectList->Count() && !pFocusWin; i++ )
        if ( pObjectList->GetObject( i )->HasFocus())
            pFocusWin = pObjectList->GetObject( i );

    if ( mpPrjDep )
        delete mpPrjDep;

    mpGraphWin->Hide();
    mpBaseWin->Hide();

    mpPrjDep = new PrjDep( mpProcessWin );
    mpPrjDep->SetCloseHdl( LINK( this, SolDep, PrjCloseHdl ));

    mpProcessWin->Resize();

    return mpPrjDep->Init( rObjectName, mpStarWriter );
}

/*****************************************************************************/
IMPL_LINK( SolDep, PrjCloseHdl, PrjDep *, pPrjDep )
/*****************************************************************************/
{
    delete mpPrjDep;
    mpPrjDep = NULL;

    mpGraphWin->Show();
    mpBaseWin->Show();

    if ( pFocusWin ) {
        pFocusWin->GrabFocus();
        pFocusWin = NULL;
    }

    return 0;
}

/*****************************************************************************/
USHORT SolDep::CloseWindow()
/*****************************************************************************/
{

    ((SystemWindow*)mpProcessWin)->Close();
    return 0;
}

/*****************************************************************************/
void SolDep::ShowHelp()
/*****************************************************************************/
{
    SvFileStream aHelpFile( String::CreateFromAscii( "g:\\soldep.hlp" ), STREAM_READ );
    String aHelpText;
    String aGetStr;
    ByteString sRead;

    if ( aHelpFile.IsOpen() )
    {
        while ( aHelpFile.ReadLine( sRead ) )
        {
            aGetStr = String( sRead, RTL_TEXTENCODING_UTF8 );
            aHelpText += aGetStr;
            aHelpText += String::CreateFromAscii("\n");
        }
    }
    else
        aHelpText = String::CreateFromAscii("No Helpfile found.");

    SolHelpDlg aHelpDlg( mpBaseWin, DtSodResId( RID_SD_DIALOG_HELP ));
    aHelpDlg.maMLEHelp.SetText( aHelpText );
    aHelpDlg.maMLEHelp.SetReadOnly();
    aHelpDlg.maMLEHelp.EnableFocusSelectionHide( TRUE );
    aHelpDlg.Execute();
}

/*****************************************************************************/
void SolDep::test()
/*****************************************************************************/
{
    FileDialog aTestDlg( mpBaseWin, WB_STDDIALOG );
    aTestDlg.SetDefaultExt( String::CreateFromAscii( "lst" ));

    if ( aTestDlg.Execute() )
    {
        WarningBox aBox( mpBaseWin, WB_OK, aTestDlg.GetPath());
        aBox.Execute();
    }
}


