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


#include <stdio.h>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/prgsbar.hxx>

#include <soldep/depper.hxx>
#include <soldep/objwin.hxx>

#include "math.h"
#include "time.h"
#include "stdlib.h"
#include "dtsodcmp.hrc"

#include <vcl/svapp.hxx>

#define MIN(a,b) (a)<(b)?(a):(b)
#define MAX(a,b) (a)>(b)?(a):(b)

ByteString sDelimiterLine("#==========================================================================");




Depper::Depper( Window* pBaseWindow ):
    Window( pBaseWindow ),
    mbIsPrjView(sal_False),
    maDefPos( 50, 50 ),
    maDefSize( 60, 25 ),
    mnViewMask( 1 ),
    pSubBar( NULL ),
    pMainBar( NULL ),
    pSubText( NULL ),
    pMainText( NULL ),
    maArrangeDlg( pBaseWindow ),
    mpPrjIdMapper( NULL ),
    maToolBox( pBaseWindow, DtSodResId(TID_SOLDEP_MAIN) ),
    maTaskBarFrame( pBaseWindow, 0)
{
    maArrangeDlg.Hide();

    mpProcessWin = pBaseWindow;
    mpGraphWin = new GraphWin( mpProcessWin, this );
    mpGraphPrjWin = new GraphWin( mpProcessWin, this );
    //mpBaseWin paints into virtual OutputDevice
    mpBaseWin = new DepWin( mpGraphWin->GetBufferWindow(), WB_NOBORDER | WB_SIZEABLE | WB_AUTOSIZE );
    mpBasePrjWin = new DepWin( mpGraphPrjWin->GetBufferWindow(), WB_NOBORDER | WB_SIZEABLE | WB_AUTOSIZE );

    mpGraphWin->SetBackground( Wallpaper( Color( COL_RED )));
    mpGraphPrjWin->SetBackground( Wallpaper( Color( COL_BLUE )));
    mpGraphPrjWin->Hide();
    mpBaseWin->SetBackground( Wallpaper( Color( COL_WHITE )));
    mpBasePrjWin->SetBackground( Wallpaper( Color( COL_LIGHTGRAY )));
/*
    mpGraphWin->SetZOrder( NULL, WINDOW_ZORDER_FIRST );
    mpGraphPrjWin->SetZOrder( NULL, WINDOW_ZORDER_FIRST );
    mpBaseWin->SetZOrder( NULL, WINDOW_ZORDER_FIRST );
*/
    mpGraphWin->EnableClipSiblings();
    mpGraphPrjWin->EnableClipSiblings();

    mpParentProcessWin = mpProcessWin->GetParent();
    mpBaseWin->Show();
    nZoomed = 0;

    mpBaseWin->SetPopupHdl( this );
    mpBasePrjWin->SetPopupHdl( this );

    mpObjectList = new ObjectList(); //ObjectList unbekannt
    mpObjectPrjList = new ObjectList();
}

Depper::~Depper()
{
//  delete mpObjectList;
    delete mpBaseWin;
    delete mpGraphWin;
}

sal_Bool Depper::TogglePrjViewStatus()
{
    GetDepWin()->Hide();            //Hide old content window
    GetGraphWin()->Hide();
    mbIsPrjView = !mbIsPrjView;     //switch now
    GetDepWin()->Invalidate();
    GetGraphWin()->Show();
    GetDepWin()->Show();            //Show new content window
    return mbIsPrjView;
}

void Depper::HideObjectsAndConnections( ObjectList* pObjLst )
{
    for ( sal_uIntPtr i = 0; i < pObjLst->Count(); i ++ ) {
        ObjectWin *pWin = pObjLst->GetObject( i );
        pWin->Hide();
    }
    mpGraphWin->Hide();
    mpBaseWin->Hide();
    mpGraphPrjWin->Show();
    mpBasePrjWin->Show();
}

void Depper::ShowObjectsAndConnections( ObjectList* pObjLst )
{
    for ( sal_uIntPtr i = 0; i < pObjLst->Count(); i ++ ) {
        ObjectWin *pWin = pObjLst->GetObject( i );
        pWin->Show();
    }
    mpBasePrjWin->Hide();
    mpGraphPrjWin->Hide();
    mpGraphWin->Show();
    mpBaseWin->Show();
}


void Depper::RemoveAllObjects( ObjectList* pObjLst )
{
    sal_uIntPtr i;

    for ( i = pObjLst->Count(); i > 0; i-- )
        delete pObjLst->GetObject( i - 1 );
    pObjLst->Clear();
}

sal_uInt16 Depper::Save( const ByteString& rFileName )
{
    DBG_ASSERT( sal_False , "you are dead!" );
    SvFileStream aOutFile( String( rFileName, RTL_TEXTENCODING_UTF8 ), STREAM_WRITE );
    depper_head dh;
    sal_uInt16 i;
    sal_uIntPtr nObjCount = mpObjectList->Count();

    ConnectorList* pConList = GetDepWin()->GetConnectorList();
    sal_uIntPtr nCnctrCount = pConList->Count();

    dh.nID = DEPPER_ID;
    dh.nObjectCount = nObjCount;
    dh.nCnctrCount = nCnctrCount;

    aOutFile.Write( &dh, sizeof( dh ));

    for ( i=0; i < nObjCount ; i++ )
    {
        mpObjectList->GetObject( i )->Save( aOutFile );
    }

    for ( i=0; i < nCnctrCount ; i++ )
    {
        pConList->GetObject( i )->Save( aOutFile );
    }

    return 0;
}


sal_uInt16  Depper::WriteSource()
{
    DBG_ASSERT( sal_False , "overload it!" );
    return 0;
};

sal_uInt16  Depper::ReadSource( sal_Bool bUpdater )
{
    DBG_ASSERT( sal_False , "overload it!" );
    return 0;
};

sal_uInt16  Depper::OpenSource()
{
    DBG_ASSERT( sal_False , "overload it!" );
    return 0;
};

ObjectWin* Depper::ObjIdToPtr( ObjectList* pObjLst, sal_uIntPtr nId )
{
    sal_uIntPtr nObjCount = pObjLst->Count();
    sal_uIntPtr i = 0;
    ObjectWin* pWin;
    sal_uIntPtr nWinId = 0;

    do
    {
        pWin = pObjLst->GetObject( i );
        nWinId = pWin->GetId();
        i++;
    }
    while( i < nObjCount && pWin->GetId() != nId );
    if ( pWin->GetId() == nId )
        return pWin;
    else
        return NULL;
}

void Depper::SetMainProgressBar( sal_uInt16 i)
{
    if ( pMainBar ) {
        pMainBar->SetValue( 100 );
        pMainBar->Update();
    }
}

void Depper::UpdateMainProgressBar(sal_uInt16 i, sal_uInt16 nScaleVal, sal_uInt16 &nStep, sal_Bool bCountingDown )
{
    sal_uIntPtr val = 0;
    if ( pMainBar ) {
 //       val = i * 50 / nScaleVal + 1;
        val = bCountingDown ? 50 + ( nScaleVal - i ) * 50 / nScaleVal : i * 50 / nScaleVal;

        pMainBar->SetValue( val );
        pMainBar->Update();
        String sText( String::CreateFromAscii( "Optimize step " ));
        sText += String::CreateFromInt32( ++nStep );
        pSubText->SetText( sText );
    }
}

void Depper::UpdateSubProgrssBar(sal_uIntPtr i)
{
    if ( pSubBar )
    {
        pSubBar->SetValue( i );
        pSubBar->Update();
        GetpApp()->Reschedule();
    }
}


sal_uInt16 Depper::AutoArrangeDlgStart()
{
    pSubBar = maArrangeDlg.GetSubBar();
    pMainBar = maArrangeDlg.GetMainBar();
    pSubText = maArrangeDlg.GetSubText();
    pMainText = maArrangeDlg.GetMainText();
    pMainText->SetText( String::CreateFromAscii( "Overall status" ));
    maArrangeDlg.Show();
    GetDepWin()->Enable( sal_False );
    GetDepWin()->Hide();
    return 0;
}

sal_uInt16 Depper::AutoArrangeDlgStop()
{
    maArrangeDlg.Hide();
    GetDepWin()->Enable( sal_True );
    GetDepWin()->Show();
    pSubBar = NULL;
    pMainBar = NULL;
    pSubText = NULL;
    pMainText = NULL;
    return 0;
}



sal_uInt16 Depper::Zoom( MapMode& rMapMode )
{
    sal_uIntPtr i;
    ObjectWin* pWin;
    Point aPos;
    Size aSize;
    ObjectList* pObjList;
    pObjList = GetObjectList();
//  aSize = mpBaseWin->GetSizePixel();
//  mpGraphWin->SetTotalSize( aSize );
//  mpGraphWin->EndScroll( 0, 0 );

    for ( i = pObjList->Count(); i > 0; i-- )
    {
        pWin = pObjList->GetObject( i - 1 );
        aPos = pWin->PixelToLogic( pWin->GetPosPixel());
        aSize = pWin->PixelToLogic( pWin->GetSizePixel());
        pWin->SetMapMode( rMapMode );
        aPos = pWin->LogicToPixel( aPos );
        aSize = pWin->LogicToPixel( aSize );
        pWin->SetPosSizePixel( aPos, aSize );
    }
    GetDepWin()->Invalidate();
    return 0;
}

sal_uIntPtr Depper::AddObjectToList( DepWin* pParentWin, ObjectList* pObjLst, sal_uIntPtr &LastID, sal_uIntPtr &WinCount, ByteString& rBodyText, sal_Bool bInteract )
{
    Point aPos;
    Size aSize = GetDefSize();

    aPos = GetDepWin()->LogicToPixel( aPos );
    aSize = GetDepWin()->LogicToPixel( aSize );
    return AddObjectToList( pParentWin, pObjLst, LastID, WinCount, rBodyText, aPos, aSize );
}

sal_uIntPtr Depper::AddObjectToList( DepWin* pParentWin, ObjectList* pObjLst, sal_uIntPtr &LastID, sal_uIntPtr &WinCount, ByteString& rBodyText, Point& rPos, Size& rSize )
{
    ObjectWin* pWin = new ObjectWin( pParentWin, WB_BORDER );
    //pWin->mpDepperDontuseme = this;
    pWin->SetGlobalViewMask(mnViewMask);    // Set ViewMask for all ObjectWin and Connector objects

    Size aNewSize;
    aNewSize.Width() = pWin->GetTextWidth( String( rBodyText, RTL_TEXTENCODING_UTF8 ));
    aNewSize.Height() = pWin->GetTextHeight();
    if ( aNewSize.Width() > rSize.Width() - 8 )
    {
        aNewSize.Width() = aNewSize.Width() + 8;
        aNewSize.Height() = rSize.Height();
    }
    else
        aNewSize = rSize;
    pWin->SetPosSizePixel( rPos,aNewSize);

    MapMode aMapMode = mpBaseWin->GetMapMode();
    pWin->SetMapMode( aMapMode );

    pObjLst->Insert( pWin, LIST_APPEND );
    pWin->SetId( LastID );
    LastID++;
    WinCount++;
    pWin->SetBodyText( rBodyText );
//  pWin->Show();
    return pWin->GetId();
}

/*void Depper::DrawOutput( OutputDevice* pDevice )
{
    GetDepWin()->DrawOutput( pDevice );
    ObjectList* pObjList = GetObjectList();
    for ( sal_uInt16 i = pObjList->Count(); i > 0; i-- )
    {
        ObjectWin* pWin = pObjList->GetObject( i - 1 );
        pWin->DrawOutput( pDevice );
    }
}*/

ObjectWin* Depper::RemoveObjectFromList( ObjectList* pObjLst, sal_uIntPtr &WinCount, sal_uInt16 nId, sal_Bool bDelete )
{
    ObjectWin* pWin = ObjIdToPtr( pObjLst, nId );

    if ( pWin )
    {
        pObjLst->Remove( pWin );
        WinCount--;
        if( bDelete )
            delete pWin;
        return pWin;
    }
    else
        return NULL;
}
sal_uInt16 Depper::AddConnectorToObjects( ObjectList* pObjLst, sal_uIntPtr nStartId, sal_uIntPtr nEndId )
{
    ObjectWin* pStartWin = ObjIdToPtr( pObjLst, nStartId );

    if ( !pStartWin )
        return DEP_STARTID_NOT_FOUND;

    ObjectWin* pEndWin = ObjIdToPtr( pObjLst, nEndId );

    if ( !pEndWin )
        return DEP_STARTID_NOT_FOUND;

    return AddConnectorToObjects( pStartWin, pEndWin );
}

sal_uInt16 Depper::RemoveConnectorFromObjects( ObjectList* pObjLst, sal_uIntPtr nStartId, sal_uIntPtr nEndId )
{
//  DBG_ASSERT( FALSE , "noch nicht" );
    ObjectWin* pStartWin = ObjIdToPtr( pObjLst, nStartId );

    if ( !pStartWin )
        return DEP_STARTID_NOT_FOUND;

    ObjectWin* pEndWin = ObjIdToPtr( pObjLst, nEndId );

    if ( !pEndWin )
        return DEP_STARTID_NOT_FOUND;

    return RemoveConnectorFromObjects( pStartWin, pEndWin );
}

sal_uInt16 Depper::AddConnectorToObjects( ObjectWin* pStartWin, ObjectWin* pEndWin )
{
    if ( pStartWin->ConnectionExistsInAnyDirection( pEndWin ))
        return 0;

    Connector* pCon = new Connector( GetDepWin(), WB_NOBORDER );
//  pCon->Initialize( pStartWin, pEndWin );
    if (pStartWin->IsNullObject()) //null_project
        pCon->Initialize( pStartWin, pEndWin, sal_False );
    else
        pCon->Initialize( pStartWin, pEndWin, sal_True );
    return 0;
}

sal_uInt16 Depper::RemoveConnectorFromObjects( ObjectWin* pStartWin, ObjectWin* pEndWin )
{
    Connector* pCon = pStartWin->GetConnector( pStartWin->GetId(), pEndWin->GetId() );

    if ( !pCon )
        return DEP_CONNECTOR_NOT_FOUND;
    else
    {
        delete pCon;
        return DEP_OK;
    }
}

sal_uIntPtr Depper::HandleNewPrjDialog( ByteString &rBodyText )
{
        SolNewProjectDlg aNewProjectDlg( GetDepWin(), DtSodResId( RID_SD_DIALOG_NEWPROJECT ));
        if ( aNewProjectDlg.Execute() )
        {
            rBodyText = ByteString( aNewProjectDlg.maEName.GetText(), RTL_TEXTENCODING_UTF8);
//hashtable auf stand halten
            MyHashObject* pHObject;
            sal_uIntPtr nObjectId = AddObjectToList( mpBaseWin, mpObjectList, mnSolLastId, mnSolWinCount, rBodyText, sal_False );
            pHObject = new MyHashObject( nObjectId, ObjIdToPtr( mpObjectList, nObjectId ));
            mpSolIdMapper->Insert( rBodyText, pHObject );

            ByteString sTokenLine( aNewProjectDlg.maEShort.GetText(), RTL_TEXTENCODING_UTF8 );
            sTokenLine += '\t';
            sTokenLine += ByteString( aNewProjectDlg.maEName.GetText(), RTL_TEXTENCODING_UTF8 );
            sTokenLine += "\t:\t";

            ByteString sDeps = ByteString( aNewProjectDlg.maEDeps.GetText(), RTL_TEXTENCODING_UTF8 );

            if ( sDeps != "" )
            {
                sal_uInt16 i;
                ByteString sDepName;
                sal_uInt16 nToken = sDeps.GetTokenCount(' ');
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
                sal_uInt16 i;
                ByteString sDepName;
                sal_uIntPtr nObjectId_l, nHashedId;
                MyHashObject* pHObject_l;
                sal_uInt16 nToken = sDeps.GetTokenCount(' ');
                for ( i = 0 ; i < nToken ; i++)
                {
                    sDepName =  sDeps.GetToken( i, ' ' );

                    pHObject_l = mpSolIdMapper->Find( sDepName );
                    if ( !pHObject_l )
                    {
                        String sMessage;
                        sMessage += String::CreateFromAscii("can't find ");
                        sMessage += String( sDepName, RTL_TEXTENCODING_UTF8 );
                        sMessage += String::CreateFromAscii(".\ndependency ignored");
                        WarningBox aBox( GetDepWin(), WB_OK, sMessage);
                        aBox.Execute();
                    }
                    else
                    {
                        nHashedId = pHObject_l->GetId();
                        pHObject_l = mpSolIdMapper->Find( rBodyText );
                        nObjectId_l = pHObject_l->GetId();
                        AddConnectorToObjects( mpObjectList, nHashedId, nObjectId_l );
                    }
                }
            }
            return nObjectId;
        }
        return 0;
}

sal_uIntPtr Depper::HandleNewDirectoryDialog(ByteString &rBodyText)
{
        SolNewDirectoryDlg aNewDirectoryDlg( GetDepWin(), DtSodResId( RID_SD_DIALOG_NEWDIRECTORY ));
//todo: set defaults
        if ( aNewDirectoryDlg.Execute() )
        {
            rBodyText = ByteString( aNewDirectoryDlg.maEFlag.GetText(), RTL_TEXTENCODING_UTF8 );
//hashtable auf stand halten
            MyHashObject* pHObject;
            sal_uIntPtr nObjectId = AddObjectToList( GetDepWin(), mpObjectPrjList, mnSolLastId, mnSolWinCount, rBodyText );
            pHObject = new MyHashObject( nObjectId, ObjIdToPtr( mpObjectPrjList, nObjectId ));
            mpPrjIdMapper->Insert( rBodyText, pHObject ); // mpPrjIdMapper

            String sTokenLine;
            sTokenLine = String( mpPrj->GetPreFix(), RTL_TEXTENCODING_UTF8 );
            sTokenLine += '\t';
            String sNameConvert = aNewDirectoryDlg.maEName.GetText();
            sNameConvert.SearchAndReplaceAll( '/', '\\' );
            if ( sNameConvert.GetToken( 0, 0x5c ) != String( mpPrj->GetProjectName(), RTL_TEXTENCODING_UTF8 ))
            {
                sTokenLine += String( mpPrj->GetProjectName(), RTL_TEXTENCODING_UTF8 );
                sTokenLine += String("\\", RTL_TEXTENCODING_UTF8 );
                sTokenLine += sNameConvert;
            }
            else
                sTokenLine += sNameConvert;
            sTokenLine += '\t';
            sTokenLine += aNewDirectoryDlg.maEAction.GetText();
            sTokenLine += String( "\t-\t", RTL_TEXTENCODING_UTF8 );
            sTokenLine += aNewDirectoryDlg.maEEnv.GetText();
            sTokenLine += '\t';
            sTokenLine += aNewDirectoryDlg.maEFlag.GetText();
            sTokenLine += '\t';

            sTokenLine += String( "NULL", RTL_TEXTENCODING_UTF8 );

            ByteString bsTokenLine = ByteString( sTokenLine, RTL_TEXTENCODING_UTF8 );
            mpStarWriter->InsertTokenLine( bsTokenLine );

            if ( aNewDirectoryDlg.maEDeps.GetText() != String( "", RTL_TEXTENCODING_UTF8 ))
            {
                sal_uInt16 i;
                ByteString sDeps = ByteString( aNewDirectoryDlg.maEDeps.GetText(), RTL_TEXTENCODING_UTF8 );
                ByteString sDepName;
                sal_uIntPtr nObjectId_l, nHashedId;
                MyHashObject* pHObject_l;
                sal_uInt16 nToken = sDeps.GetTokenCount(' ');
                for ( i = 0 ; i < nToken ; i++)
                {
                    sDepName =  sDeps.GetToken( i, ' ' );

                    pHObject_l = mpPrjIdMapper->Find( sDepName ); // mpPrjIdMapper
                    if ( !pHObject_l )
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
                        sTokenLine += String( sDepName, RTL_TEXTENCODING_UTF8 );
                        sTokenLine +='\t';
                        nHashedId = pHObject_l->GetId();
                        pHObject_l = mpPrjIdMapper->Find( rBodyText ); // mpPrjIdMapper
                        nObjectId_l = pHObject_l->GetId();
                        AddConnectorToObjects( mpObjectPrjList, nHashedId, nObjectId_l );
                    }
                }
            }

            if ( mpPrj->Count() > 1 )
            {
                CommandData* pData = mpPrj->GetObject( mpPrj->Count() - 1 );
                pData = mpPrj->Replace( pData, mpPrj->Count() - 2 );
                mpPrj->Replace( pData, mpPrj->Count() - 1 );
            }

            return nObjectId;
        }
        return 0;
}

// Context-Menue
IMPL_LINK( Depper, PopupSelected, PopupMenu*, mpPopup )
{
    sal_uInt16 nItemId = mpPopup->GetCurItemId();

    switch( nItemId )
    {
        case DEPPOPUP_NEW :
                {
                    ByteString sNewItem = ByteString("new");
                     //AddObject( mpObjectList, sNewItem , sal_True );
                 }
                break;
        case DEPPOPUP_AUTOARRANGE :
                //AutoArrange( mpObjectList, mpObjectList, GetStart(), 0 );
                break;
        case DEPPOPUP_LOAD :
                //Load( ByteString("test.dep"));
                break;
        case DEPPOPUP_SAVE :
                Save( ByteString("test.dep"));
                break;
        case DEPPOPUP_WRITE_SOURCE :
                WriteSource();
                break;
        case DEPPOPUP_READ_SOURCE :
                ReadSource(sal_True);
                break;
        case DEPPOPUP_OPEN_SOURCE :
                OpenSource();
                break;
        case DEPPOPUP_ZOOMIN :
        {
//          DBG_ASSERT( FALSE,"zoomin");
                MapMode aMapMode = GetDepWin()->GetMapMode();
                aMapMode.SetScaleX( aMapMode.GetScaleX() * Fraction( 1.25 ));
                aMapMode.SetScaleY( aMapMode.GetScaleY() * Fraction( 1.25 ));
                GetDepWin()->SetMapMode( aMapMode );

                if ( nZoomed < 1 )
                {
                    Size aZoomInSize( GetDepWin()->GetSizePixel());
                    aZoomInSize.Width() *= 1.25;
                    aZoomInSize.Height() *= 1.25;
                    GetDepWin()->SetSizePixel( aZoomInSize );
                }
                nZoomed--;

                Zoom( aMapMode );
        };
                break;
        case DEPPOPUP_ZOOMOUT :
        {
//          DBG_ASSERT( FALSE,"zoomout");
                MapMode aMapMode = GetDepWin()->GetMapMode();
                if ( aMapMode.GetScaleX() > Fraction( 0.25 ))
                {
                    aMapMode.SetScaleX( aMapMode.GetScaleX() * Fraction( 0.8 ));
                    aMapMode.SetScaleY( aMapMode.GetScaleY() * Fraction( 0.8 ));
                    GetDepWin()->SetMapMode( aMapMode );

                    if ( nZoomed < 0 )
                    {
                        Size aZoomOutSize( mpBaseWin->GetSizePixel());
                        aZoomOutSize.Width() *= 0.8;
                        aZoomOutSize.Height() *= 0.8;
                        GetDepWin()->SetSizePixel( aZoomOutSize );
                    }
                    nZoomed++;

                    Zoom( aMapMode );
                }
        };
                break;
        case DEPPOPUP_CLEAR :
//          DBG_ASSERT( FALSE,"clear");
//              RemoveAllObjects( mpObjectList );
                break;
        case DEPPOPUP_CLOSE :
//          DBG_ASSERT( FALSE,"close");
            CloseWindow();
                break;
        case DEPPOPUP_HELP :
//          DBG_ASSERT( FALSE,"help");
            ShowHelp();
                break;
        case DEPPOPUP_TEST :
//          DBG_ASSERT( FALSE,"TEST!");
//          test();
                break;
        case DEPPOPUP_SHOW_TOOLBOX:
            maToolBox.Show();
            break;
        default :
            DBG_ASSERT( sal_False, "default" );
                break;
    }
    return 0;
}
