
/*************************************************************************
 *
 *  $RCSfile: depper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: obo $ $Date: 2004-02-26 14:48:13 $
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


#include <stdio.h>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/prgsbar.hxx>

#include "depper.hxx"
#include "objwin.hxx"
#include "soldlg.hxx"

#include "math.h"
#include "time.h"
#include "stdlib.h"

#include <vcl/svapp.hxx>

#define MIN(a,b) (a)<(b)?(a):(b)
#define MAX(a,b) (a)>(b)?(a):(b)

ResMgr *DtSodResId::pResMgr = NULL;
DtSodResId::DtSodResId( USHORT nId ) :
    ResId( nId, pResMgr )
{
}

Depper::Depper( Window* pBaseWindow ):
    maDefPos( 50, 50 ),
    maDefSize( 60, 25 ),
    mpTravellerList( NULL ),
    nViewMask( 1 ),
    pMainBar( NULL ),
    pSubBar( NULL ),
    pMainText( NULL ),
    pSubText( NULL )
{
    mpProcessWin = pBaseWindow;
    mpGraphWin = new GraphWin( mpProcessWin );
    mpBaseWin = new DepWin( mpGraphWin->GetBufferWindow(), WB_NOBORDER | WB_SIZEABLE | WB_AUTOSIZE );

    mpGraphWin->SetBackgroundBrush( Brush( Color( COL_WHITE )));
    mpBaseWin->SetBackgroundBrush( Brush( Color( COL_WHITE )));

    mpBaseWin->mpDepperDontuseme = this;

    mpGraphWin->SetZOrder( NULL, WINDOW_ZORDER_FIRST );
    mpBaseWin->SetZOrder( NULL, WINDOW_ZORDER_FIRST );

    mpGraphWin->EnableClipSiblings();

    mpBaseWin->mpPopup->InsertItem( DEPPOPUP_AUTOARRANGE, String::CreateFromAscii("Autoarrange")) ;
#ifdef HJS_TEST
    //  not realy working
    // mpBaseWin->mpPopup->InsertItem( DEPPOPUP_LOAD, "Load" );
    // mpBaseWin->mpPopup->InsertItem( DEPPOPUP_SAVE, "Save" );
#endif
#ifdef HJS_TEST
    mpBaseWin->mpPopup->InsertItem( DEPPOPUP_TEST, "TEST" );
#endif

    mpBaseWin->Show();
    mnWinCount = 0;
    mnLastId = 0;
    nZoomed = 0;

    mpBaseWin->SetPopupHdl( this );
    pObjectList = new ObjWinList();
}

Depper::~Depper()
{
    delete pObjectList;
    delete mpBaseWin;
    delete mpGraphWin;
}

void Depper::SetViewMask( ULONG nMask )
{
    nViewMask = nMask;
    for ( ULONG i = 0; i < pObjectList->Count(); i ++ ) {
        ObjectWin *pWin = pObjectList->GetObject( i );
        if ( pWin->IsVisible())
            pWin->Show();
        else
            pWin->Hide();
    }
}

ULONG Depper::AddObject( ByteString& rBodyText, BOOL bInteract )
{
    Point aPos;
    Size aSize = maDefSize;

    aPos = mpBaseWin->LogicToPixel( aPos );
    aSize = mpBaseWin->LogicToPixel( aSize );
    return AddObject( rBodyText, aPos, aSize );
}

ULONG Depper::AddObject( ByteString& rBodyText, Point& rPos, Size& rSize )
{
    ObjectWin* pWin = new ObjectWin( mpBaseWin, WB_BORDER );
    pWin->mpDepperDontuseme = this;

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

    pObjectList->Insert( pWin, LIST_APPEND );
    pWin->SetId( mnLastId );
    mnLastId++;
    mnWinCount++;
    pWin->SetBodyText( rBodyText );
//  pWin->Show();
    return pWin->GetId();
}

ObjectWin*  Depper::RemoveObject( USHORT nId, BOOL bDelete )
{
    ObjectWin* pWin = ObjIdToPtr( nId );

    if ( pWin )
    {
        pObjectList ->Remove( pWin );
        mnWinCount--;
        if( bDelete )
            delete pWin;
        return pWin;
    }
    else
        return NULL;
}

void Depper::RemoveAllObjects( ObjWinList* pObjLst )
{
    ULONG i;

    for ( i = pObjLst->Count(); i > 0; i-- )
        delete pObjLst->GetObject( i - 1 );
    pObjLst->Clear();
}

USHORT Depper::AddConnector( ULONG nStartId, ULONG nEndId )
{
    ObjectWin* pStartWin = ObjIdToPtr( nStartId );

    if ( !pStartWin )
        return DEP_STARTID_NOT_FOUND;

    ObjectWin* pEndWin = ObjIdToPtr( nEndId );

    if ( !pEndWin )
        return DEP_STARTID_NOT_FOUND;

    return AddConnector( pStartWin, pEndWin );
}

USHORT Depper::RemoveConnector( ULONG nStartId, ULONG nEndId )
{
//  DBG_ASSERT( FALSE , "noch nicht" );
    ObjectWin* pStartWin = ObjIdToPtr( nStartId );

    if ( !pStartWin )
        return DEP_STARTID_NOT_FOUND;

    ObjectWin* pEndWin = ObjIdToPtr( nEndId );

    if ( !pEndWin )
        return DEP_STARTID_NOT_FOUND;

    return RemoveConnector( pStartWin, pEndWin );
}

USHORT Depper::AddConnector( ObjectWin* pStartWin, ObjectWin* pEndWin )
{
    if ( pStartWin->ConnectionExistsInAnyDirection( pEndWin ))
        return 0;

    Connector* pCon = new Connector( mpBaseWin, WB_NOBORDER );
    pCon->Initialize( pStartWin, pEndWin );

    return 0;
}

USHORT Depper::RemoveConnector( ObjectWin* pStartWin, ObjectWin* pEndWin )
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

USHORT Depper::Save( const ByteString& rFileName )
{
    DBG_ASSERT( FALSE , "you are dead!" );
    SvFileStream aOutFile( String( rFileName, RTL_TEXTENCODING_UTF8 ), STREAM_WRITE );
    depper_head dh;
    USHORT i;
    ULONG nObjCount = pObjectList->Count();

    ConnectorList* pConList = mpBaseWin->GetConnectorList();
    ULONG nCnctrCount = pConList->Count();

    dh.nID = DEPPER_ID;
    dh.nObjectCount = nObjCount;
    dh.nCnctrCount = nCnctrCount;

    aOutFile.Write( &dh, sizeof( dh ));

    for ( i=0; i < nObjCount ; i++ )
    {
        pObjectList->GetObject( i )->Save( aOutFile );
    }

    for ( i=0; i < nCnctrCount ; i++ )
    {
        pConList->GetObject( i )->Save( aOutFile );
    }

    return 0;
}

USHORT Depper::Load( const ByteString& rFileName )
{
    DBG_ASSERT( FALSE , "you are dead!" );
    SvFileStream aInFile( String( rFileName, RTL_TEXTENCODING_UTF8 ), STREAM_READ );
    depper_head dh;
    ULONG i;
    ULONG nLoadOffs = mnLastId;
    ObjectWin* pNewWin;
    aInFile.Read( &dh, sizeof( dh ));

    ULONG nObjCount = dh.nObjectCount;
    ULONG nCnctrCount = dh.nCnctrCount;

    for ( i=0; i < nObjCount ; i++ )
    {
        ObjectWin* pWin = new ObjectWin( mpBaseWin, WB_BORDER );
        pWin->Load( aInFile );
        pNewWin = ObjIdToPtr( AddObject( pWin->GetBodyText(), FALSE ));
        pNewWin->SetId( nLoadOffs + pWin->GetId());
        pNewWin->SetPosPixel( pWin->GetPosPixel());
        pNewWin->SetSizePixel( pWin->GetSizePixel());
    }

    ULONG nStartId;
    ULONG nEndId;
//  ueber addconnector fuehren!
    for ( i=0; i < nCnctrCount ; i++ )
    {
        Connector* pCon = new Connector( mpBaseWin, WB_NOBORDER );
        pCon->Load( aInFile );

        nStartId = nLoadOffs + pCon->GetStartId();
        nEndId = nLoadOffs + pCon->GetEndId();

        ObjectWin* pStartWin = ObjIdToPtr( nStartId );
        ObjectWin* pEndWin = ObjIdToPtr( nEndId );

        pCon->Initialize( pStartWin, pEndWin );
    }


    return 0;
}
USHORT  Depper::WriteSource()
{
    DBG_ASSERT( FALSE , "overload it!" );
    return 0;
};

USHORT  Depper::ReadSource()
{
    DBG_ASSERT( FALSE , "overload it!" );
    return 0;
};

USHORT  Depper::OpenSource()
{
    DBG_ASSERT( FALSE , "overload it!" );
    return 0;
};

ObjectWin* Depper::ObjIdToPtr( ULONG nId )
{
    ULONG nObjCount = pObjectList->Count();
    ULONG i = 0;
    ObjectWin* pWin;

    do
    {
        pWin = pObjectList->GetObject( i );
        i++;
    }
    while( i < nObjCount && pWin->GetId() != nId );
    if ( pWin->GetId() == nId )
        return pWin;
    else
        return NULL;
}

USHORT Depper::Impl_Traveller( ObjectWin* pWin, USHORT nDepth )
{
    USHORT i = 0;
    ObjectWin* pNewWin;
    Connector* pCon;

    nDepth++;

    USHORT nMaxDepth = nDepth;

    pWin->mbVisited = TRUE;
    pWin->mnRootDist = Max ( nDepth, pWin-> mnRootDist );
    if ( nDepth > DEPPER_MAX_DEPTH )
    {
        DBG_ASSERT( nDepth != DEPPER_MAX_DEPTH + 1, "ring abh." );
        nDepth++;
        return DEP_ENDLES_RECURSION_FOUND;
    }

    while ( pCon = pWin->GetConnector( i ) )
    {
        if ( pCon->IsStart( pWin ) && pCon->IsVisible())
        {
            pNewWin = pCon->GetOtherWin( pWin );
            nMaxDepth = Max( Impl_Traveller( pNewWin, nDepth ), nMaxDepth );
            if( nMaxDepth == DEP_ENDLES_RECURSION_FOUND )
            {
                mpTravellerList->Insert( pWin, LIST_APPEND );
                return DEP_ENDLES_RECURSION_FOUND;
            }
        }
        i++;
    }
    pWin->mnHeadDist = MAX( pWin->mnHeadDist, nMaxDepth - nDepth );
    return nMaxDepth;
}

double Depper::CalcDistSum( ObjWinList* pObjList, DistType eDistType )
{
    ObjectWin* pWin;
    Connector* pCon;
    ULONG nObjCount = pObjList->Count();
    double dRetVal = 0;
    double dWinVal;
    USHORT i, j;
    BOOL bIsStart;

    for ( i = 0; i < nObjCount; i++ )
    {
        pWin = pObjList->GetObject( i );

        if ( pWin && pWin->IsVisible())
        {
            j = 0;
            dWinVal = 0;
            while ( pCon = pWin->GetConnector( j ) )
            {
                if ( pCon->IsVisible()) {
                    bIsStart = pCon->IsStart( pWin );
                    if ( eDistType != BOTH )
                        if ( eDistType == TOPDOWN )
                        {
                            if ( bIsStart )
                            {
                                pCon->UpdatePosition( pWin, FALSE );
                                dWinVal += pCon->GetLen() * pWin->mnHeadDist;
                            }
                        }
                        else
                        {
                            if ( !bIsStart )
                            {
                                pCon->UpdatePosition( pWin, FALSE );
                                dWinVal += pCon->GetLen() * pWin->mnRootDist;
                            }

                        }
                    else
                    {
                        pCon->UpdatePosition( pWin, FALSE );
                        if ( !bIsStart )
                            dWinVal += pCon->GetLen() * ( pWin->mnHeadDist + 1 );
                        else
                            dWinVal += pCon->GetLen() * pWin->mnRootDist;
                    }
                }
                j++;
            }
//          if ( j != 0 )
//              dWinVal /= j;
            dRetVal += dWinVal;
        }
    }

    return dRetVal;
}
ULONG Depper::CalcXOffset( ULONG nObjectsToFit )
{
    long nDynXOffs;
    long nXMiddle;
    ULONG nTrigger;

    nXMiddle = mpBaseWin->PixelToLogic( mpBaseWin->GetSizePixel()).Width() / 2;
    if ( nObjectsToFit > DEPPER_MAX_WIDTH )
        nObjectsToFit = DEPPER_MAX_WIDTH - 1 + DEPPER_MAX_WIDTH % 2;
    nTrigger = ( nObjectsToFit - 1 ) / 2;
    nDynXOffs = ( maDefSize.Width() + OBJWIN_X_SPACING ) * nTrigger;
    ULONG nXOffs = nXMiddle - nDynXOffs;

    if ( nXMiddle - nDynXOffs < mnMinDynXOffs )
        mnMinDynXOffs = nXMiddle - nDynXOffs;

    return nXOffs;

}

double Depper::Impl_PermuteMin( ObjWinList& rObjList, Point* pPosArray, ObjWinList& rResultList, double dMinDist, ULONG nStart, ULONG nSize, DistType eDistType )
{

    ULONG i, j, l;
    ULONG nEnd = nStart + nSize;
    ObjectWin* pSwapWin;
    ULONG nLevelObjCount = rObjList.Count();

//dont use full recusion for more than 6 objects
    if ( nLevelObjCount > 6 )
    {
        srand(( unsigned ) time( NULL ));

        ULONG nIdx1, nIdx2;
        for ( i = 0; i < 101; i++ )
        {
            if ( pSubBar ) {
#ifndef SOLARIS
                pSubBar->SetValue( i );
#endif
                pSubBar->Update();
                GetpApp()->Reschedule();
            }
            for ( j = 0; j < 100; j++ )
            {
                nIdx1 = (ULONG) ( double( rand() ) / RAND_MAX * nLevelObjCount );
                while ( rObjList.GetObject( nIdx1 ) == NULL )
                    nIdx1 = (ULONG) ( double( rand() ) / RAND_MAX * nLevelObjCount );
                nIdx2 = (ULONG) ( double( rand() ) / RAND_MAX * nLevelObjCount );
                while ( nIdx1 == nIdx2 || nIdx2 == nLevelObjCount )
                    nIdx2 = (ULONG) ( double( rand() ) / RAND_MAX * nLevelObjCount );

                pSwapWin = rObjList.GetObject( nIdx1 );
                if ( pSwapWin )
                    pSwapWin->SetCalcPosPixel( pPosArray[ nIdx2 ] );
                pSwapWin = rObjList.Replace( pSwapWin, nIdx2 );
                if ( pSwapWin )
                    pSwapWin->SetCalcPosPixel( pPosArray[ nIdx1 ] );
                rObjList.Replace( pSwapWin, nIdx1 );

                double dCurDist = CalcDistSum( &rObjList, eDistType );

                if ( dCurDist < dMinDist )
                {
                    dMinDist = dCurDist;
                    rResultList.Clear();
                    for ( l = 0; l < nLevelObjCount; l++ )
                    {
                        pSwapWin = rObjList.GetObject( l );
                        rResultList.Insert( pSwapWin, LIST_APPEND);
                    }
                }
//              if ( dCurDist > dMinDist * 1.5 )
                if ( dCurDist > dMinDist * 15 )
                {
                    pSwapWin = rObjList.GetObject( nIdx1 );
                    if ( pSwapWin )
                        pSwapWin->SetCalcPosPixel( pPosArray[ nIdx2 ] );
                    pSwapWin = rObjList.Replace( pSwapWin, nIdx2 );
                    if ( pSwapWin )
                        pSwapWin->SetCalcPosPixel( pPosArray[ nIdx1 ] );
                    rObjList.Replace( pSwapWin, nIdx1 );
                }
            }
        }
    }
    else
    {
        for ( i = nStart ; i < nEnd; i++)
        {
            if ( nSize > 1 )
            {
                pSwapWin = rObjList.GetObject( i );
                pSwapWin = rObjList.Replace( pSwapWin, nStart );
                rObjList.Replace( pSwapWin, i );

                dMinDist = MIN( dMinDist, Impl_PermuteMin( rObjList, pPosArray, rResultList, dMinDist, nStart + 1, nSize - 1, eDistType ));
                pSwapWin = rObjList.GetObject( i );
                pSwapWin = rObjList.Replace( pSwapWin, nStart );
                rObjList.Replace( pSwapWin, i );

            }
            else
            {
                for ( l = 0; l < nLevelObjCount; l++ )
                {
                    pSwapWin = rObjList.GetObject( l );
                    if ( pSwapWin )
                        pSwapWin->SetCalcPosPixel( pPosArray[ l ] );
                }

                double dCurDist = CalcDistSum( &rObjList, eDistType );

                if ( dCurDist < dMinDist )
                {
                    dMinDist = dCurDist;
                    rResultList.Clear();
                    for ( l = 0; l < nLevelObjCount; l++ )
                    {
                        pSwapWin = rObjList.GetObject( l );
                        rResultList.Insert( pSwapWin, LIST_APPEND);
                    }
                }

            }
        }
    }

    return dMinDist;
}

Point Depper::CalcPos( USHORT nSet, USHORT nIndex )
{
    int nRowIndex = nIndex / DEPPER_MAX_WIDTH;
    ULONG nPosX = mnXOffset + nRowIndex % 3 * maDefSize.Width() / 3 + ( nIndex - ( DEPPER_MAX_WIDTH * nRowIndex )) * (maDefSize.Width() + OBJWIN_X_SPACING );

    ULONG nPosY = ( nSet + mnLevelOffset + nRowIndex ) * ( maDefSize.Height() + OBJWIN_Y_SPACING ) + OBJWIN_Y_SPACING;
    Point aPos( nPosX, nPosY );
    return aPos;
}

USHORT Depper::AutoArrange( ULONG nTopId, ULONG nBottmId )
{
    SolAutoarrangeDlg aArrangeDlg( mpProcessWin );
    pSubBar = aArrangeDlg.GetSubBar();
    pMainBar = aArrangeDlg.GetMainBar();
    pSubText = aArrangeDlg.GetSubText();
    pMainText = aArrangeDlg.GetMainText();
    pMainText->SetText( String::CreateFromAscii( "Overall status" ));
    mpBaseWin->Enable( FALSE );
    OptimizePos( GetStart(), 0 );
    mpBaseWin->Enable( TRUE );
    pSubBar = NULL;
    pMainBar = NULL;
    pSubText = NULL;
    pMainText = NULL;

    return 0;
}

USHORT Depper::OptimizePos( ULONG nTopId, ULONG nBottmId )
{
    ObjWinList aWorkList;
    ObjectWin* pWin;
    Connector* pCon;
    USHORT nRootDist = -1;
    USHORT i, j, k, l, nRetVal;
    USHORT LevelUse[ DEPPER_MAX_DEPTH ];
    USHORT LevelSecUse[ DEPPER_MAX_DEPTH ];
    ObjWinList* LevelList[ DEPPER_MAX_DEPTH ];
    ObjWinList* LevelSecList[ DEPPER_MAX_DEPTH ];
    Point aPosArray[ DEPPER_MAX_LEVEL_WIDTH * DEPPER_MAX_WIDTH ];

    mnMinDynXOffs = 0xffff;

    for ( i = 0; i < DEPPER_MAX_DEPTH; i++ )
    {
        LevelUse[ i ] = 0;
        LevelList[ i ] = NULL;
        LevelSecUse[ i ] = 0;
        LevelSecList[ i ] = NULL;
    }

    mpBaseWin->EnablePaint( FALSE );

    ULONG nObjCount = pObjectList->Count();
    for ( i = 0; i < nObjCount; i++ )
    {
        pWin = pObjectList->GetObject( i );
        if ( pWin->IsVisible()) {
            pWin->mbVisited = FALSE;
            pWin->mnHeadDist = 0;
            pWin->mnRootDist = 0;

            // find initial objects which need to be connected with
            // root object
            j = 0;
            USHORT nStartCount = 0;
            USHORT nEndCount = 0;
            while ( pCon = pWin->GetConnector( j ) )
            {
                if ( pCon->IsVisible()) {
                    if( pCon->IsStart( pWin ))
                        nStartCount++;
                    else
                    {
                        nEndCount = 1;
                        break;
                    }
                }
                j++;
            }
            if ( nStartCount > 0 && nEndCount == 0 )
                if ( nTopId != pWin->GetId())
                    Depper::AddConnector( nTopId, pWin->GetId());
        }
    }

    pWin = ObjIdToPtr( nTopId );

    if ( mpTravellerList )
    {
        mpTravellerList->Clear();
        delete mpTravellerList;
    }
    mpTravellerList = new ObjWinList();
    // set root and top distance
    nRetVal = Impl_Traveller( pWin, nRootDist );

    DBG_ASSERT( nRetVal < DEPPER_MAX_DEPTH , "zu tief" );
    if ( nRetVal == DEP_ENDLES_RECURSION_FOUND )
    {
        WarningBox aWBox( mpBaseWin, WB_OK, String::CreateFromAscii("graph too deep! dat geiht nich gut.\nlook at depper.err in your Tmp-directory\nfor list of objects"));
        aWBox.Execute();
        char *tmpdir = getenv("TMP");
        char *errfilebasename = "depper.err";
        char *ErrFileName = (char*) malloc( strlen( tmpdir ) + strlen( errfilebasename) + 3 );
        *ErrFileName = '\0';
        strcat( ErrFileName, tmpdir );
        strcat( ErrFileName, "\\" );
        strcat( ErrFileName, errfilebasename );
        FILE* pErrFile = fopen( "depper.err", "w+" );
        if ( pErrFile )
        {
            for ( USHORT i = 0; i < mpTravellerList->Count(); i++ )
            {
                pWin = mpTravellerList->GetObject( i );
                fprintf( pErrFile, " %s -> \n", (pWin->GetBodyText()).GetBuffer());
            }
            fclose( pErrFile );
        }
        return nRetVal;
    }

    ULONG nUnvisited = 0;
    ULONG nUnvisYOffs = 0;

    // seperate mainstream, secondary and unconnected
    for ( i = 0; i < nObjCount; i++ )
    {
        pWin = pObjectList->GetObject( i );
        if ( pWin->IsVisible()) {
            if (( pWin->mnHeadDist + pWin->mnRootDist ) == nRetVal )
            {
                if ( !LevelList[ pWin->mnHeadDist ] )
                        LevelList[ pWin->mnHeadDist ] = new ObjWinList;
                LevelList[ pWin->mnHeadDist ]->Insert( pWin );
                LevelUse[ pWin->mnHeadDist ]++;
            }
            else
                if ( pWin->mbVisited )
                {
                    if ( !LevelSecList[ nRetVal - pWin->mnRootDist ] )
                        LevelSecList[ nRetVal - pWin->mnRootDist ] = new ObjWinList;
                    LevelSecList[ nRetVal - pWin->mnRootDist ]->Insert( pWin );
                    LevelSecUse[ nRetVal - pWin->mnRootDist ]++;
                }
                else
                {
    //              need to be arranged more intelligent...
                    Point aPos( 5, nUnvisYOffs );
                    pWin->SetCalcPosPixel( aPos );

                    Point aTmpPos = pWin->GetCalcPosPixel();
                    pWin->SetPosPixel( mpBaseWin->LogicToPixel( aTmpPos ));

                    nUnvisYOffs += pWin->PixelToLogic( pWin->GetSizePixel()).Height();
                    nUnvisited++;
                }
        }
    }

    mnLevelOffset = 0;

    USHORT nScaleVal;

    if ( nRetVal == 0 )
        nScaleVal = 1;
    else
        nScaleVal = nRetVal;

    i = 0;

    USHORT nStep = 0;

    while ( LevelList[ i ] )
    {
        if ( pMainBar ) {
#ifndef SOLARIS
            pMainBar->SetValue( i * 50 / nScaleVal );
#endif
            pMainBar->Update();
            String sText( String::CreateFromAscii( "Optimize step " ));
            sText += String::CreateFromInt32( ++nStep );
            pSubText->SetText( sText );
        }

        DBG_ASSERT( LevelUse[ i ] == LevelList[ i ]->Count() , "level index im a..." );
        ObjectWin* pSwapWin;
        ULONG nLevelObjCount = LevelList[ i ]->Count();

        if ( nLevelObjCount % 2 == 0 )
        {
            LevelList[ i ]->Insert( NULL, LIST_APPEND );
            nLevelObjCount++;
//          LevelUse bleibt orginal...
//          LevelUse[ i ]++;
        }

// catch too big lists
        DBG_ASSERT( nLevelObjCount < DEPPER_MAX_LEVEL_WIDTH * DEPPER_MAX_WIDTH , "graph zu breit! dat geiht nich gut. breaking" );
        if ( nLevelObjCount >= DEPPER_MAX_LEVEL_WIDTH * DEPPER_MAX_WIDTH )
        {
            WarningBox aWBox( mpBaseWin, WB_OK, String::CreateFromAscii("graph zu breit! dat geiht nich gut. breaking"));
            aWBox.Execute();
            break;
        }
        mnXOffset = CalcXOffset( nLevelObjCount );
        aWorkList.Clear();

        // initial positioning for mainstream
        for ( j = 0; j < nLevelObjCount; j++ )
        {
            pSwapWin = LevelList[ i ]->GetObject( j );
            aWorkList.Insert( pSwapWin, LIST_APPEND);
            Point aPos = CalcPos( i, j );
            aPosArray[ j ] = aPos;
            if ( pSwapWin )
                pSwapWin->SetCalcPosPixel( aPosArray[ j ] );
        }

        double dMinDist = CalcDistSum( LevelList[ i ] );

        // optimize mainstream order and return best matching list in "aWorkList"
        dMinDist = MIN( dMinDist, Impl_PermuteMin( *(LevelList[ i ]), aPosArray, aWorkList, dMinDist, 0, nLevelObjCount ));

        // set optimized positions - may still be wrong from later tries
        for ( j = 0; j < nLevelObjCount; j++ )
        {
            pSwapWin = aWorkList.GetObject( j );
            if ( pSwapWin )
                pSwapWin->SetCalcPosPixel(  aPosArray[ j ] );
        }

        if ( LevelSecList[ i ] != NULL )
        {
            ULONG nLevelSecObjCount = LevelSecList[ i ]->Count();
            // expand list for better positioning
            while ( nLevelSecObjCount + LevelUse[ i ] < DEPPER_MAX_WIDTH - 1 )
            {
                LevelSecList[ i ]->Insert( NULL, LIST_APPEND );
                nLevelSecObjCount++;
            }
            if ( ( nLevelSecObjCount + LevelUse[ i ])% 2 == 0 )
            {
                LevelSecList[ i ]->Insert( NULL, LIST_APPEND );
                nLevelSecObjCount++;
            }

            DBG_ASSERT( nLevelSecObjCount < DEPPER_MAX_LEVEL_WIDTH * DEPPER_MAX_WIDTH , "graph zu breit! dat geiht nich gut. breaking" );
            if ( nLevelObjCount >= DEPPER_MAX_LEVEL_WIDTH * DEPPER_MAX_WIDTH )
            {
                WarningBox aWBox( mpBaseWin, WB_OK, String::CreateFromAscii("graph zu breit! dat geiht nich gut. breaking"));
                aWBox.Execute();
                break;
            }
            mnXOffset = CalcXOffset( LevelUse[ i ] + nLevelSecObjCount );
            aWorkList.Clear();

            l = 0;
            BOOL bUsedPos;

            // find free positions for secondary objects
            for ( j = 0; j < ( LevelUse[ i ] + nLevelSecObjCount ) ; j++ )
            {
                Point aPos = CalcPos( i, j );
                bUsedPos = FALSE;
                // is already occupied?
                for ( k = 0; k < nLevelObjCount; k++ )
                {
                    if ( LevelList[ i ]->GetObject( k ) )
                        if ( aPos == LevelList[ i ]->GetObject( k )->GetCalcPosPixel() )
                            bUsedPos = TRUE;
                }
                // if its free, add to pool
                if ( !bUsedPos )
                {
                    aPosArray[ l ] = aPos;
                    l++;
                }
            }

            // initial positioning for secodaries
            for ( j = 0 ; j < nLevelSecObjCount ; j++ )
            {
                pSwapWin = LevelSecList[ i ]->GetObject( j );
                aWorkList.Insert( pSwapWin, LIST_APPEND);
                if ( pSwapWin )
                    pSwapWin->SetCalcPosPixel( aPosArray[ j ] );
            }
            dMinDist = CalcDistSum( LevelSecList[ i ] );

            dMinDist = MIN( dMinDist, Impl_PermuteMin( *(LevelSecList[ i ]), aPosArray, aWorkList, dMinDist, 0, nLevelSecObjCount ));

            // set optimized positions - may still be wrong from later tries
            for ( j = 0; j < nLevelSecObjCount; j++ )
            {
                pSwapWin = aWorkList.GetObject( j );
                if ( pSwapWin )
                    pSwapWin->SetCalcPosPixel(  aPosArray[ j ] );
            }
            if ( LevelUse[ i ] + LevelSecUse[ i ] > DEPPER_MAX_WIDTH )
                mnLevelOffset++;
        }
        if ( LevelUse[ i ] + LevelSecUse[ i ] > DEPPER_MAX_WIDTH )
            mnLevelOffset+= ( LevelUse[ i ] + LevelSecUse[ i ] ) / DEPPER_MAX_WIDTH ;
        i++;
    }

    mnMinDynXOffs = 0xffff;

// and back again...
/**/
    // get better results form already preoptimized upper and lower rows

//  i--;
//  while ( LevelList[ i ] &&  i < 60000 )
    do
    {
        i--;
        if ( pMainBar )
        {
#ifndef SOLARIS
            pMainBar->SetValue( 50 + ( nScaleVal - i ) * 50 / nScaleVal );
#endif
            pMainBar->Update();
            String sText( String::CreateFromAscii( "Optimize step " ));
            sText += String::CreateFromInt32( ++nStep );
            pSubText->SetText( sText );
        }
        if ( LevelUse[ i ] + LevelSecUse[ i ] > DEPPER_MAX_WIDTH )
            mnLevelOffset-= ( LevelUse[ i ] + LevelSecUse[ i ] ) / DEPPER_MAX_WIDTH ;
        ObjectWin* pSwapWin;
        ULONG nLevelObjCount = LevelList[ i ]->Count();
        mnXOffset = CalcXOffset( nLevelObjCount );
        aWorkList.Clear();

        for ( j = 0; j < nLevelObjCount; j++ )
        {
            pSwapWin = LevelList[ i ]->GetObject( j );
            aWorkList.Insert( pSwapWin, LIST_APPEND);
            Point aPos = CalcPos( i, j );
            aPosArray[ j ] = aPos;
//no need to do this stuff.......   ?????
            if ( pSwapWin )
                pSwapWin->SetCalcPosPixel( aPosArray[ j ] );
        }

        double dMinDist = CalcDistSum( LevelList[ i ], BOTH );

        dMinDist = MIN( dMinDist, Impl_PermuteMin( *(LevelList[ i ]), aPosArray, aWorkList, dMinDist, 0, nLevelObjCount, BOTH ));
// wrong position for remaping - keep old positions for comparing
        for ( j = 0; j < nLevelObjCount; j++ )
        {
            pSwapWin = aWorkList.GetObject( j );
            if ( pSwapWin )
//              pSwapWin->SetCalcPosPixel( mpBaseWin->LogicToPixel( aPosArray[ j ] ));
                pSwapWin->SetCalcPosPixel( aPosArray[ j ] );
        }

        if ( LevelSecList[ i ] != NULL )
        {
            ULONG nLevelSecObjCount = LevelSecList[ i ]->Count();
            mnXOffset = CalcXOffset( LevelUse[ i ] + nLevelSecObjCount );
            aWorkList.Clear();

            l = 0;
            BOOL bUsedPos;

            for ( j = 0; j < ( LevelUse[ i ] + nLevelSecObjCount ) ; j++ )
            {
                Point aPos = CalcPos( i, j );
                bUsedPos = FALSE;
// could be faster
                for ( k = 0; k < nLevelObjCount; k++ )
                {
                    if ( LevelList[ i ]->GetObject( k ) )
                        if ( aPos == LevelList[ i ]->GetObject( k )->GetCalcPosPixel() )
                            bUsedPos = TRUE;
                }
                if ( !bUsedPos )
                {
                    aPosArray[ l ] = aPos;
                    l++;
                }
            }

            for ( j = 0 ; j < nLevelSecObjCount ; j++ )
            {
                pSwapWin = LevelSecList[ i ]->GetObject( j );
                aWorkList.Insert( pSwapWin, LIST_APPEND);
                if ( pSwapWin )
                    pSwapWin->SetCalcPosPixel( aPosArray[ j ] );
            }
            dMinDist = CalcDistSum( LevelSecList[ i ], BOTH );

            dMinDist = MIN( dMinDist, Impl_PermuteMin( *(LevelSecList[ i ]), aPosArray, aWorkList, dMinDist, 0, nLevelSecObjCount, BOTH ));
// wrong position for remaping - keep old positions for comparing
            for ( j = 0; j < nLevelSecObjCount; j++ )
            {
                pSwapWin = aWorkList.GetObject( j );
                if ( pSwapWin )
                    pSwapWin->SetCalcPosPixel( aPosArray[ j ] );
            }
        }
//      i--;
    } while ( i != 0 );
    if ( pMainBar ) {
#ifndef SOLARIS
        pMainBar->SetValue( 100 );
#endif
        pMainBar->Update();
    }

    ULONG nNewXSize = ( DEPPER_MAX_WIDTH + 1 )  * ( OBJWIN_X_SPACING + maDefSize.Width() );
    ULONG nNewYSize = ObjIdToPtr( GetStart())->GetCalcPosPixel().Y() + maDefSize.Height() + 2 * OBJWIN_Y_SPACING;
    if (( nUnvisYOffs + maDefSize.Height()) > nNewYSize )
        nNewYSize = nUnvisYOffs + maDefSize.Height();

    MapMode aMapMode = mpBaseWin->GetMapMode();
    Size aTmpSize( double(nNewXSize) * double( aMapMode.GetScaleX()), double( nNewYSize) * double( aMapMode.GetScaleY()));

    Size aNowSize( mpGraphWin->GetSizePixel());

    if ( mpBaseWin->LogicToPixel( aNowSize ).Width() > aTmpSize.Width() )
        aTmpSize.Width() = mpBaseWin->LogicToPixel( aNowSize ).Width() ;

    if ( mpBaseWin->LogicToPixel( aNowSize ).Height()  > aTmpSize.Height() )
        aTmpSize.Height() = mpBaseWin->LogicToPixel( aNowSize ).Height() ;

    if ( nZoomed <= 0 )
    {
//      mpBaseWin->SetSizePixel( aTmpSize );
//      mpGraphWin->SetTotalSize( aTmpSize );
//      mpGraphWin->EndScroll( 0, 0 );
    }
// now remap all objects
    ULONG nAllObjCount = pObjectList->Count();
    Point aTmpPos;
    for ( j = 0; j < nAllObjCount; j++ )
    {
        pWin = pObjectList->GetObject( j );
        if ( pWin->IsVisible()) {
            aTmpPos = pWin->GetCalcPosPixel();
            if ( pWin->mbVisited )
            {
// reserve space for unconnected
                aTmpPos.X() -= mnMinDynXOffs;
                aTmpPos.X() += maDefSize.Width() + OBJWIN_X_SPACING;
// center window
                aTmpPos.X() += maDefSize.Width() / 2;
                aTmpPos.X() -= pWin->PixelToLogic( pWin->GetSizePixel()).Width() / 2 ;
            }
            pWin->SetPosPixel( mpBaseWin->LogicToPixel( aTmpPos ));
        }
    }
    aWorkList.Clear();
    mpBaseWin->EnablePaint( TRUE );
    mpBaseWin->Invalidate();
//LevelListen loeschen

    ObjectWin* pObject1;
    for ( i = 0 ; i < nObjCount ; i++)
    {
        pObject1 = pObjectList->GetObject( i );
        if ( pObject1->IsVisible())
            pObject1->UpdateConnectors();
    };
    return 0;
}

ULONG Depper::GetStart()
{
    DBG_ASSERT( FALSE , "overload it!" );
    return 0;
}

USHORT Depper::Zoom( MapMode& rMapMode )
{
    ULONG i;
    ObjectWin* pWin;
    Point aPos;
    Size aSize;

    aSize = mpBaseWin->GetSizePixel();
    mpGraphWin->SetTotalSize( aSize );
    mpGraphWin->EndScroll( 0, 0 );

    for ( i = pObjectList->Count(); i > 0; i-- )
    {
        pWin = pObjectList->GetObject( i - 1 );
        aPos = pWin->PixelToLogic( pWin->GetPosPixel());
        aSize = pWin->PixelToLogic( pWin->GetSizePixel());
        pWin->SetMapMode( rMapMode );
        aPos = pWin->LogicToPixel( aPos );
        aSize = pWin->LogicToPixel( aSize );
        pWin->SetPosSizePixel( aPos, aSize );
    }
    mpBaseWin->Invalidate();
    return 0;
}

IMPL_LINK( Depper, PopupSelected, PopupMenu*, mpPopup )
{
    USHORT nItemId = mpPopup->GetCurItemId();
    ByteString sNew = ByteString("new");

    switch( nItemId )
    {
        case DEPPOPUP_NEW :
//          DBG_ASSERT( FALSE,"new");
            AddObject( sNew , TRUE );
                break;
        case DEPPOPUP_AUTOARRANGE :
        {
//          DBG_ASSERT( FALSE,"autoarrange");
            AutoArrange( GetStart(), 0 );
        };
                break;
        case DEPPOPUP_LOAD :
                Load( ByteString("test.dep"));
//          DBG_ASSERT( FALSE,"load");
                break;
        case DEPPOPUP_SAVE :
                Save( ByteString("test.dep"));
//          DBG_ASSERT( FALSE,"save");
                break;
        case DEPPOPUP_WRITE_SOURCE :
                WriteSource();
//          DBG_ASSERT( FALSE,"write source");
                break;
        case DEPPOPUP_READ_SOURCE :
                ReadSource();
//          DBG_ASSERT( FALSE,"read source");
                break;
        case DEPPOPUP_OPEN_SOURCE :
                OpenSource();
//          DBG_ASSERT( FALSE,"open source");
                break;
        case DEPPOPUP_ZOOMIN :
        {
//          DBG_ASSERT( FALSE,"zoomin");
                MapMode aMapMode = mpBaseWin->GetMapMode();
                aMapMode.SetScaleX( aMapMode.GetScaleX() * Fraction( 1.25 ));
                aMapMode.SetScaleY( aMapMode.GetScaleY() * Fraction( 1.25 ));
                mpBaseWin->SetMapMode( aMapMode );

                if ( nZoomed < 1 )
                {
                    Size aZoomInSize( mpBaseWin->GetSizePixel());
                    aZoomInSize.Width() *= 1.25;
                    aZoomInSize.Height() *= 1.25;
                    mpBaseWin->SetSizePixel( aZoomInSize );
                }
                nZoomed--;

                Zoom( aMapMode );
        };
                break;
        case DEPPOPUP_ZOOMOUT :
        {
//          DBG_ASSERT( FALSE,"zoomout");
                MapMode aMapMode = mpBaseWin->GetMapMode();
                if ( aMapMode.GetScaleX() > Fraction( 0.25 ))
                {
                    aMapMode.SetScaleX( aMapMode.GetScaleX() * Fraction( 0.8 ));
                    aMapMode.SetScaleY( aMapMode.GetScaleY() * Fraction( 0.8 ));
                    mpBaseWin->SetMapMode( aMapMode );

                    if ( nZoomed < 0 )
                    {
                        Size aZoomOutSize( mpBaseWin->GetSizePixel());
                        aZoomOutSize.Width() *= 0.8;
                        aZoomOutSize.Height() *= 0.8;
                        mpBaseWin->SetSizePixel( aZoomOutSize );
                    }
                    nZoomed++;

                    Zoom( aMapMode );
                }
        };
                break;
        case DEPPOPUP_CLEAR :
//          DBG_ASSERT( FALSE,"clear");
                RemoveAllObjects( pObjectList );
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
        default :
            DBG_ASSERT( FALSE, "default" );
                break;
    }
    return 0;
}

