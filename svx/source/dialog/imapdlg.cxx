/*************************************************************************
 *
 *  $RCSfile: imapdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:09 $
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

#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
#define _SDR_NOOBJECTS
#define _SDR_NOVIEWMARKER
#define _SDR_NODRAGMETHODS
#define _SDR_NOUNDO
#define _SDR_NOXOUTDEV

#include <stdlib.h>

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _EINF_HXX
#include <tools/errinf.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINIMGR_HXX
#include <svtools/iniman.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _IODLG_HXX
#include <sfx2/iodlg.hxx>
#endif
#ifndef _SFX_SAVEOPT_HXX
#include <sfx2/saveopt.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#pragma hdrstop

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#include <sfx2/viewfrm.hxx>

#define _IMAPDLG_PRIVATE
#include "imapdlg.hxx"

#include "dialmgr.hxx"
#include "dialogs.hrc"
#include "svxids.hrc"
#include "imapdlg.hrc"
#include "imapwnd.hxx"
#include "imapimp.hxx"
#include "dlgutil.hxx"
#include "svdtrans.hxx"
#include "svdopath.hxx"

#define SELF_TARGET         "_self"
#define IMAP_ALL_FILTER     "<Alle>"
#define IMAP_CERN_FILTER    "MAP - CERN"
#define IMAP_NCSA_FILTER    "MAP - NCSA"
#define IMAP_BINARY_FILTER  "SIP - StarView ImageMap"
#define IMAP_ALL_TYPE       "*.*"
#define IMAP_BINARY_TYPE    "*.sip"
#define IMAP_CERN_TYPE      "*.map"
#define IMAP_NCSA_TYPE      "*.map"
#define IMAP_BINARY_EXT     "sip"
#define IMAP_CERN_EXT       "map"
#define IMAP_NCSA_EXT       "map"

/******************************************************************************/

SFX_IMPL_FLOATINGWINDOW( SvxIMapDlgChildWindow, SID_IMAP );

/******************************************************************************/

inline String GetUnitString( long nVal_100, FieldUnit eFieldUnit, sal_Unicode cSep )
{
    String aVal = UniString::CreateFromInt32( MetricField::ConvertValue( nVal_100, 2, MAP_100TH_MM, eFieldUnit ) );

    while( aVal.Len() < 3 )
        aVal.Insert( sal_Unicode('0'), 0 );

    aVal.Insert( cSep, aVal.Len() - 2 );
    aVal += sal_Unicode(' ');
    aVal += SdrFormatter::GetUnitStr( eFieldUnit );

    return aVal;
}

/*************************************************************************
|*
|* ControllerItem
|*
\************************************************************************/

SvxIMapDlgItem::SvxIMapDlgItem( USHORT nId, SvxIMapDlg& rIMapDlg, SfxBindings& rBindings ) :
            SfxControllerItem   ( nId, rBindings ),
            rIMap               ( rIMapDlg )
{
}

/*************************************************************************
|*
|* StateChanged-Methode
|*
\************************************************************************/

void SvxIMapDlgItem::StateChanged( USHORT nSID, SfxItemState eState,
                                   const SfxPoolItem* pItem )
{
    if ( ( nSID == SID_IMAP_EXEC ) && pItem )
    {
        const SfxBoolItem* pStateItem = PTR_CAST( SfxBoolItem, pItem );

        DBG_ASSERT( pStateItem || pItem == 0, "SfxBoolItem erwartet");

        // Float ggf. disablen
        rIMap.SetExecState( !pStateItem->GetValue() );
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

SvxIMapDlgChildWindow::SvxIMapDlgChildWindow( Window* pParent, USHORT nId,
                                              SfxBindings* pBindings,
                                              SfxChildWinInfo* pInfo ) :
            SfxChildWindow( pParent, nId )
{
    pWindow = new SvxIMapDlg( pBindings, this, pParent, SVX_RES( RID_SVXDLG_IMAP ) );
    SvxIMapDlg* pDlg = (SvxIMapDlg*) pWindow;

    if ( pInfo->aSize.Width() && pInfo->aSize.Height() )
        pWindow->SetPosSizePixel( pInfo->aPos, pInfo->aSize );
    else
        pWindow->SetPosPixel( Point() );

    if ( pInfo->nFlags & SFX_CHILDWIN_ZOOMIN )
        pDlg->RollUp();

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxIMapDlgChildWindow::UpdateIMapDlg( const Graphic& rGraphic, const ImageMap* pImageMap,
                                           const TargetList* pTargetList, void* pEditingObj )
{
    if ( SfxViewFrame::Current() &&
         SfxViewFrame::Current()->HasChildWindow( SvxIMapDlgChildWindow::GetChildWindowId() ) )
        SVXIMAPDLG()->Update( rGraphic, pImageMap, pTargetList, pEditingObj );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

SvxIMapDlg::SvxIMapDlg( SfxBindings *pBindings, SfxChildWindow *pCW,
                        Window* pParent, const ResId& rResId ) :
        SfxFloatingWindow   ( pBindings, pCW, pParent, rResId ),
        aIMapItem           ( SID_IMAP_EXEC, *this, *pBindings ),
        pOwnData            ( new IMapOwnData( this ) ),
        aTbxIMapDlg1        ( this, SVX_RES( TBX_IMAPDLG1 ) ),
        aStbStatus          ( this, WB_BORDER | WB_3DLOOK | WB_LEFT ),
        pIMapWnd            ( new IMapWindow( this, SVX_RES( RID_SVXCTL_IMAP ) ) ),
        pCheckObj           ( NULL ),
        aFtURL              ( this, SVX_RES( FT_URL ) ),
        aFtText             ( this, SVX_RES( FT_TEXT ) ),
        aCbbURL             ( this, SVX_RES( CBB_URL ) ),
        aEdtText            ( this, SVX_RES( EDT_TEXT ) )
{
    FreeResource();

    // nur temporaer bis Vollupdate
    pOwnData->pFtTarget = new FixedText( this, SVX_RES( RID_SVXCTL_FT_TARGET ) );
    pOwnData->pFtTarget->Show();

    // nur temporaer bis Vollupdate
    pOwnData->pCbbTarget = new ComboBox( this, SVX_RES( RID_SVXCTL_CBB_TARGET ) );
    pOwnData->pCbbTarget->Show();

    pIMapWnd->SetInfoLink( LINK( this, SvxIMapDlg, InfoHdl ) );
    pIMapWnd->SetMousePosLink( LINK( this, SvxIMapDlg, MousePosHdl ) );
    pIMapWnd->SetGraphSizeLink( LINK( this, SvxIMapDlg, GraphSizeHdl ) );
    pIMapWnd->SetUpdateLink( LINK( this, SvxIMapDlg, StateHdl ) );

    aCbbURL.SetModifyHdl( LINK( this, SvxIMapDlg, URLModifyHdl ) );
    aCbbURL.SetSelectHdl( LINK( this, SvxIMapDlg, URLModifyHdl ) );
    aCbbURL.SetLoseFocusHdl( LINK( this, SvxIMapDlg, URLLoseFocusHdl ) );
    aEdtText.SetModifyHdl( LINK( this, SvxIMapDlg, URLModifyHdl ) );
    pOwnData->pCbbTarget->SetLoseFocusHdl( LINK( this, SvxIMapDlg, URLLoseFocusHdl ) );

    aTbxIMapDlg1.SetSizePixel( aTbxIMapDlg1.CalcWindowSizePixel() );
    aTbxIMapDlg1.SetSelectHdl( LINK( this, SvxIMapDlg, TbxClickHdl ) );
    aTbxIMapDlg1.CheckItem( TBI_SELECT, TRUE );
    TbxClickHdl( &aTbxIMapDlg1 );

    SetMinOutputSizePixel( aLastSize = GetOutputSizePixel() );

    aStbStatus.InsertItem( 1, 130, SIB_LEFT | SIB_IN | SIB_AUTOSIZE );
    aStbStatus.InsertItem( 2, 10 + GetTextWidth( String::CreateFromAscii( " 9999,99 cm / 9999,99 cm ") ), SIB_CENTER | SIB_IN );
    aStbStatus.InsertItem( 3, 10 + GetTextWidth( String::CreateFromAscii( " 9999,99 cm x 9999,99 cm ") ), SIB_CENTER | SIB_IN );

    aFtURL.Disable();
    aCbbURL.Disable();
    aFtText.Disable();
    aEdtText.Disable();
    pOwnData->pFtTarget->Disable();
    pOwnData->pCbbTarget->Disable();
    pOwnData->bExecState = FALSE;

    Resize();

    EnableDrop(TRUE);

    pOwnData->aTimer.SetTimeout( 100 );
    pOwnData->aTimer.SetTimeoutHdl( LINK( this, SvxIMapDlg, UpdateHdl ) );

    aTbxIMapDlg1.EnableItem( TBI_ACTIVE, FALSE );
    aTbxIMapDlg1.EnableItem( TBI_MACRO, FALSE );
    aTbxIMapDlg1.EnableItem( TBI_PROPERTY, FALSE );
}


/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SvxIMapDlg::~SvxIMapDlg()
{
    // URL-Liste loeschen
    delete pOwnData->pCbbTarget;
    delete pOwnData->pFtTarget;
    delete pIMapWnd;
    delete pOwnData;
}


/*************************************************************************
|*
|* Resize-Methode
|*
\************************************************************************/

void SvxIMapDlg::Resize()
{
    SfxFloatingWindow::Resize();

    Size aMinSize( GetMinOutputSizePixel() );
    Size aNewSize( GetOutputSizePixel() );

    if ( aNewSize.Height() >= aMinSize.Height() )
    {
        Size    aSize( aStbStatus.GetSizePixel() );
        Point   aPoint( 0, aNewSize.Height() - aSize.Height() );

        // StatusBar positionieren
        aStbStatus.SetPosSizePixel( aPoint, Size( aNewSize.Width(), aSize.Height() ) );
        aStbStatus.Show();

        // EditWindow positionieren
        aSize.Width() = aNewSize.Width() - 18;
        aSize.Height() = aPoint.Y() - pIMapWnd->GetPosPixel().Y() - 6;
        pIMapWnd->SetSizePixel( aSize );

        aLastSize = aNewSize;
    }
}


/*************************************************************************
|*
|* Close-Methode
|*
\************************************************************************/

BOOL SvxIMapDlg::Close()
{
    BOOL bRet = TRUE;

    if ( aTbxIMapDlg1.IsItemEnabled( TBI_APPLY ) )
    {
        QueryBox    aQBox( this, WB_YES_NO_CANCEL | WB_DEF_YES,
                           String( SVX_RES( STR_IMAPDLG_MODIFY ) ) );
        const long  nRet = aQBox.Execute();

        if( nRet == RET_YES )
        {
            SfxBoolItem aBoolItem( SID_IMAP_EXEC, TRUE );
            GetBindings().GetDispatcher()->Execute(
                SID_IMAP_EXEC, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD, &aBoolItem, 0L );
        }
        else if( nRet == RET_CANCEL )
            bRet = FALSE;
    }
    else if( pIMapWnd->IsChanged() )
    {
        QueryBox    aQBox( this, WB_YES_NO_CANCEL | WB_DEF_YES,
                           String( SVX_RES( STR_IMAPDLG_SAVE ) ) );
        const long  nRet = aQBox.Execute();

        if( nRet == RET_YES )
            bRet = DoSave();
        else if( nRet == RET_CANCEL )
            bRet = FALSE;
    }

    return( bRet ? SfxFloatingWindow::Close() : FALSE );
}


/*************************************************************************
|*
|* Enabled oder disabled alle Controls
|*
\************************************************************************/

void SvxIMapDlg::SetExecState( BOOL bEnable )
{
    pOwnData->bExecState = bEnable;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxIMapDlg::SetGraphic( const Graphic& rGraphic )
{
    pIMapWnd->SetGraphic( rGraphic );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxIMapDlg::SetImageMap( const ImageMap& rImageMap )
{
    pIMapWnd->SetImageMap( rImageMap );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

const ImageMap& SvxIMapDlg::GetImageMap() const
{
    return pIMapWnd->GetImageMap();
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxIMapDlg::SetTargetList( const TargetList& rTargetList )
{
    TargetList  aNewList( rTargetList );

    pIMapWnd->SetTargetList( aNewList );

    pOwnData->pCbbTarget->Clear();

    for( String* pStr = aNewList.First(); pStr; pStr = aNewList.Next() )
        pOwnData->pCbbTarget->InsertEntry( *pStr );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

const TargetList& SvxIMapDlg::GetTargetList() const
{
    return pIMapWnd->GetTargetList();
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxIMapDlg::Update( const Graphic& rGraphic, const ImageMap* pImageMap,
                         const TargetList* pTargetList, void* pEditingObj )
{
    pOwnData->aUpdateGraphic = rGraphic;

    if ( pImageMap )
        pOwnData->aUpdateImageMap = *pImageMap;
    else
        pOwnData->aUpdateImageMap.ClearImageMap();

    pOwnData->pUpdateEditingObject = pEditingObj;

    // UpdateTargetList loeschen, da diese Methode
    // vor dem Zuschlagen des Update-Timers noch
    // mehrmals gerufen werden kann( #46540 )
    for( String* pStr = pOwnData->aUpdateTargetList.First(); pStr; pStr = pOwnData->aUpdateTargetList.Next() )
        delete pStr;
    pOwnData->aUpdateTargetList.Clear();

    // TargetListe muss kopiert werden, da sie im
    // Besitz des Aufrufers ist und von ihm nach diesem
    // Aufruf sofort geloescht werden kann;
    // die kopierte Liste wird im Handler wieder geloescht
    if( pTargetList )
    {
        TargetList aTargetList( *pTargetList );

        for( String* pStr = aTargetList.First(); pStr; pStr = aTargetList.Next() )
            pOwnData->aUpdateTargetList.Insert( new String( *pStr ) );
    }

    pOwnData->aTimer.Start();
}


/*************************************************************************
|*
|* Click-Hdl fuer ToolBox
|*
\************************************************************************/

IMPL_LINK( SvxIMapDlg, TbxClickHdl, ToolBox*, pTbx )
{
    USHORT nNewItemId = pTbx->GetCurItemId();

    URLLoseFocusHdl( NULL );

    switch( pTbx->GetCurItemId() )
    {
        case( TBI_APPLY ):
        {
            SfxBoolItem aBoolItem( SID_IMAP_EXEC, TRUE );
            GetBindings().GetDispatcher()->Execute(
                SID_IMAP_EXEC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aBoolItem, 0L );
        }
        break;

        case( TBI_OPEN ):
            DoOpen();
        break;

        case( TBI_SAVEAS ):
            DoSave();
        break;

        case( TBI_SELECT ):
        {
            pTbx->CheckItem( nNewItemId, TRUE );
            pIMapWnd->SetEditMode( TRUE );
        }
        break;

        case( TBI_RECT ):
        {
            pTbx->CheckItem( nNewItemId, TRUE );
            pIMapWnd->SetObjKind( OBJ_RECT );
        }
        break;

        case( TBI_CIRCLE ):
        {
            pTbx->CheckItem( nNewItemId, TRUE );
            pIMapWnd->SetObjKind( OBJ_CIRC );
        }
        break;

        case( TBI_POLY ):
        {
            pTbx->CheckItem( nNewItemId, TRUE );
            pIMapWnd->SetObjKind( OBJ_POLY );
        }
        break;

        case( TBI_FREEPOLY ):
        {
            pTbx->CheckItem( nNewItemId, TRUE );
            pIMapWnd->SetObjKind( OBJ_FREEFILL );
        }
        break;

        case( TBI_ACTIVE ):
        {
            BOOL bNewState = !pTbx->IsItemChecked( TBI_ACTIVE );

            pTbx->CheckItem( TBI_ACTIVE, bNewState );
            pIMapWnd->SetCurrentObjState( !bNewState );
        }
        break;

        case( TBI_MACRO ):
            pIMapWnd->DoMacroAssign();
        break;

        case( TBI_PROPERTY ):
            pIMapWnd->DoPropertyDialog();
        break;

        case( TBI_POLYEDIT ):
            pIMapWnd->SetPolyEditMode( pTbx->IsItemChecked( TBI_POLYEDIT ) ? SID_BEZIER_MOVE : 0 );
        break;

        case( TBI_POLYMOVE ):
            pIMapWnd->SetPolyEditMode( SID_BEZIER_MOVE );
        break;

        case( TBI_POLYINSERT ):
            pIMapWnd->SetPolyEditMode( SID_BEZIER_INSERT );
        break;

        case( TBI_POLYDELETE ):
            pIMapWnd->GetSdrView()->DeleteMarkedPoints();
        break;

        case( TBI_UNDO ):
            pIMapWnd->GetSdrModel()->Undo();
        break;

        case( TBI_REDO ):
            pIMapWnd->GetSdrModel()->Redo();
        break;

        default:
        break;
    }

    return 0;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxIMapDlg::DoOpen()
{
    SfxSimpleFileDialog aDlg( this, WB_3DLOOK | WB_OPEN );
    ImageMap        aLoadIMap;
    const String    aFilter( String::CreateFromAscii( IMAP_ALL_FILTER ) );

#ifdef MAC
    aDlg.AddFilter( aFilter, String::CreateFromAscii( IMAP_ALL_TYPE ), "XXXX0" );
    aDlg.AddFilter( String::CreateFromAscii( IMAP_CERN_FILTER ), String::CreateFromAscii( IMAP_CERN_TYPE ), "XXXX0" );
    aDlg.AddFilter( String::CreateFromAscii( IMAP_NCSA_FILTER ), String::CreateFromAscii( IMAP_NCSA_TYPE ), "XXXX0" );
    aDlg.AddFilter( String::CreateFromAscii( IMAP_BINARY_FILTER ), String::CreateFromAscii( IMAP_BINARY_TYPE ), "XXXX0" );
#else
    aDlg.AddFilter( aFilter, String::CreateFromAscii( IMAP_ALL_TYPE ) );
    aDlg.AddFilter( String::CreateFromAscii( IMAP_CERN_FILTER ), String::CreateFromAscii( IMAP_CERN_TYPE ) );
    aDlg.AddFilter( String::CreateFromAscii( IMAP_NCSA_FILTER ), String::CreateFromAscii( IMAP_NCSA_TYPE ) );
    aDlg.AddFilter( String::CreateFromAscii( IMAP_BINARY_FILTER ), String::CreateFromAscii( IMAP_BINARY_TYPE ) );
#endif

    aDlg.SetCurFilter( aFilter );
    aDlg.SetPath( SFX_INIMANAGER()->Get( SFX_KEY_GRAPHICS_PATH ) );
    aDlg.SetDefaultExt( String::CreateFromAscii( IMAP_BINARY_EXT ) );

    if ( aDlg.Execute() == RET_OK )
    {
        SvFileStream aIStm( aDlg.GetPath(), STREAM_READ );

        if ( aIStm.IsOpen() )
        {
            aLoadIMap.Read( aIStm, IMAP_FORMAT_DETECT );

            if ( !aIStm.GetError() )
                pIMapWnd->SetImageMap( aLoadIMap );
            else
                ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
        }
        else
            ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL SvxIMapDlg::DoSave()
{
    SfxSimpleFileDialog     aDlg( this, WB_3DLOOK | WB_SAVEAS );
    const String    aBinFilter( String::CreateFromAscii( IMAP_BINARY_FILTER ) );
    const String    aCERNFilter( String::CreateFromAscii( IMAP_CERN_FILTER ) );
    const String    aNCSAFilter( String::CreateFromAscii( IMAP_NCSA_FILTER ) );
    SdrModel*       pModel = pIMapWnd->GetSdrModel();
    const BOOL      bChanged = pModel->IsChanged();
    BOOL            bRet;

#ifdef MAC
    aDlg.AddFilter( aCERNFilter, String::CreateFromAscii( IMAP_CERN_TYPE ), "XXXX0" );
    aDlg.AddFilter( aNCSAFilter, String::CreateFromAscii( IMAP_NCSA_TYPE ), "XXXX0" );
    aDlg.AddFilter( aBinFilter, String::CreateFromAscii( IMAP_BINARY_TYPE ), "XXXX0" );
#else
    aDlg.AddFilter( aCERNFilter, String::CreateFromAscii( IMAP_CERN_TYPE ) );
    aDlg.AddFilter( aNCSAFilter, String::CreateFromAscii( IMAP_NCSA_TYPE ) );
    aDlg.AddFilter( aBinFilter, String::CreateFromAscii( IMAP_BINARY_TYPE ) );
#endif

    aDlg.SetCurFilter( aCERNFilter );
    aDlg.SetPath( SFX_INIMANAGER()->Get( SFX_KEY_GRAPHICS_PATH ) );
    aDlg.SetDefaultExt( String::CreateFromAscii( IMAP_BINARY_EXT ) );

    if( aDlg.Execute() == RET_OK )
    {
        String          aPath( aDlg.GetPath() );
        const String    aFilter( aDlg.GetCurFilter() );
        String          aExt;
        ULONG           nFormat;

        if ( aFilter == aBinFilter )
        {
            nFormat = IMAP_FORMAT_BIN;
            aExt = String::CreateFromAscii( IMAP_BINARY_EXT );
        }
        else if ( aFilter == aCERNFilter )
        {
            nFormat = IMAP_FORMAT_CERN;
            aExt = String::CreateFromAscii( IMAP_CERN_EXT );
        }
        else if ( aFilter == aNCSAFilter )
        {
            nFormat = IMAP_FORMAT_NCSA;
            aExt = String::CreateFromAscii( IMAP_NCSA_EXT );
        }

        INetURLObject aURL;
        aURL.SetSmartURL( aPath );

        if( !aURL.getExtension().Len() )
        {
            aURL.setExtension( aExt );
            aPath = aURL.PathToFileName();
        }

        SvFileStream    aOStm( aPath, STREAM_WRITE | STREAM_TRUNC );
        const ImageMap& rImageMap = pIMapWnd->GetImageMap();

        if ( aOStm.IsOpen() )
        {
            rImageMap.Write( aOStm, nFormat );

            if ( aOStm.GetError() )
                ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
        }
        else
            ErrorHandler::HandleError( ERRCODE_IO_GENERAL );

        // Status restaurieren, da 'GetImageMap' den Status zuruecksetzt
        pModel->SetChanged( bChanged );
        bRet = TRUE;
    }
    else
        bRet = FALSE;

    return bRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( SvxIMapDlg, InfoHdl, IMapWindow*, pWnd )
{
    String              aStr;
    const NotifyInfo&   rInfo = pWnd->GetInfo();

    if ( rInfo.bNewObj )
    {
        if( rInfo.aMarkURL.Len() && ( aCbbURL.GetEntryPos( rInfo.aMarkURL ) == LISTBOX_ENTRY_NOTFOUND ) )
            aCbbURL.InsertEntry( rInfo.aMarkURL );

        aCbbURL.SetText( rInfo.aMarkURL );
        aEdtText.SetText( rInfo.aMarkDescription );

        if ( !rInfo.aMarkTarget.Len() )
            pOwnData->pCbbTarget->SetText( String::CreateFromAscii( SELF_TARGET ) );
        else
            pOwnData->pCbbTarget->SetText( rInfo.aMarkTarget );
    }

    if ( !rInfo.bOneMarked )
    {
        aTbxIMapDlg1.CheckItem( TBI_ACTIVE, FALSE );
        aTbxIMapDlg1.EnableItem( TBI_ACTIVE, FALSE );
        aTbxIMapDlg1.EnableItem( TBI_MACRO, FALSE );
        aTbxIMapDlg1.EnableItem( TBI_PROPERTY, FALSE );
        aStbStatus.SetItemText( 1, aStr );

        aFtURL.Disable();
        aCbbURL.Disable();
        aFtText.Disable();
        aEdtText.Disable();
        pOwnData->pFtTarget->Disable();
        pOwnData->pCbbTarget->Disable();

        aCbbURL.SetText( String() );
        aEdtText.SetText( String() );
    }
    else
    {
        aTbxIMapDlg1.EnableItem( TBI_ACTIVE, TRUE );
        aTbxIMapDlg1.CheckItem( TBI_ACTIVE, !rInfo.bActivated );
        aTbxIMapDlg1.EnableItem( TBI_MACRO, TRUE );
        aTbxIMapDlg1.EnableItem( TBI_PROPERTY, TRUE );

        aFtURL.Enable();
        aCbbURL.Enable();
        aFtText.Enable();
        aEdtText.Enable();
        pOwnData->pFtTarget->Enable();
        pOwnData->pCbbTarget->Enable();

        aStbStatus.SetItemText( 1, rInfo.aMarkURL );

        if ( aCbbURL.GetText() != rInfo.aMarkURL )
            aCbbURL.SetText( rInfo.aMarkURL );

        if ( aEdtText.GetText() != rInfo.aMarkDescription )
            aEdtText.SetText( rInfo.aMarkDescription );

        if ( !rInfo.aMarkTarget.Len() )
            pOwnData->pCbbTarget->SetText( String::CreateFromAscii( SELF_TARGET ) );
        else
            pOwnData->pCbbTarget->SetText(  rInfo.aMarkTarget );
    }

    return 0;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( SvxIMapDlg, MousePosHdl, IMapWindow*, pWnd )
{
    String          aStr;
    FieldUnit       eFieldUnit;
    const Point&    rMousePos = pWnd->GetMousePos();
    const sal_Unicode   cSep = International().GetNumDecimalSep();

    GET_MODULE_FIELDUNIT( eFieldUnit );

    aStr.Assign( GetUnitString( rMousePos.X(), eFieldUnit, cSep ) );
    aStr.Append( String::CreateFromAscii( " / " ) );
    aStr.Append( GetUnitString( rMousePos.Y(), eFieldUnit, cSep ) );

    aStbStatus.SetItemText( 2, aStr );

    return 0L;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( SvxIMapDlg, GraphSizeHdl, IMapWindow*, pWnd )
{
    String      aStr;
    FieldUnit   eFieldUnit;
    const Size& rSize = pWnd->GetGraphicSize();
    const sal_Unicode cSep = International().GetNumDecimalSep();

    GET_MODULE_FIELDUNIT( eFieldUnit );

    aStr.Assign( GetUnitString( rSize.Width(), eFieldUnit, cSep ) );
    aStr.Append( String::CreateFromAscii( " x " ) );
    aStr.Append( GetUnitString( rSize.Height(), eFieldUnit, cSep ) );

    aStbStatus.SetItemText( 3, aStr );

    return 0L;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( SvxIMapDlg, URLModifyHdl, void*, p )
{
    NotifyInfo  aNewInfo;

    aNewInfo.aMarkURL = aCbbURL.GetText();
    aNewInfo.aMarkDescription = aEdtText.GetText();
    aNewInfo.aMarkTarget = pOwnData->pCbbTarget->GetText();

    pIMapWnd->ReplaceActualIMapInfo( aNewInfo );

    return 0;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( SvxIMapDlg, URLLoseFocusHdl, void*, p )
{
    NotifyInfo      aNewInfo;
    const String    aURLText( aCbbURL.GetText() );
    const String    aTargetText( pOwnData->pCbbTarget->GetText() );

    if ( aURLText.Len() )
    {
        aNewInfo.aMarkURL = ::URIHelper::SmartRelToAbs( aURLText, FALSE,
                                                        INetURLObject::WAS_ENCODED,
                                                        INetURLObject::DECODE_UNAMBIGUOUS );
    }
    else
        aNewInfo.aMarkURL = aURLText;

    aNewInfo.aMarkDescription = aEdtText.GetText();

    if ( !aTargetText.Len() )
        aNewInfo.aMarkTarget = String::CreateFromAscii( SELF_TARGET );
    else
        aNewInfo.aMarkTarget = aTargetText;

    pIMapWnd->ReplaceActualIMapInfo( aNewInfo );

    return 0;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( SvxIMapDlg, UpdateHdl, Timer*, pTimer )
{
    pOwnData->aTimer.Stop();

    if ( pOwnData->pUpdateEditingObject != pCheckObj )
    {
        if ( pIMapWnd->IsChanged() &&
             ( QueryBox( this, WB_YES_NO | WB_DEF_YES,
             String( SVX_RES( STR_IMAPDLG_SAVE ) ) ).Execute() == RET_YES ) )
        {
            DoSave();
        }

        SetGraphic( pOwnData->aUpdateGraphic );
        SetImageMap( pOwnData->aUpdateImageMap );
        SetTargetList( pOwnData->aUpdateTargetList );
        SetEditingObject( pOwnData->pUpdateEditingObject );

        // Nach Wechsel => default Selektion
        aTbxIMapDlg1.CheckItem( TBI_SELECT, TRUE );
        pIMapWnd->SetEditMode( TRUE );
    }

    // die in der Update-Methode kopierte Liste wieder loeschen
    for( String* pStr = pOwnData->aUpdateTargetList.First(); pStr; pStr = pOwnData->aUpdateTargetList.Next() )
        delete pStr;

    pOwnData->aUpdateTargetList.Clear();

    GetBindings().Invalidate( SID_IMAP_EXEC );

    return 0L;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( SvxIMapDlg, StateHdl, IMapWindow*, pWnd )
{
    const SdrObject*    pObj = pWnd->GetSelectedSdrObject();
    const SdrModel*     pModel = pWnd->GetSdrModel();
    const SdrView*      pView = pWnd->GetSdrView();
    const BOOL          bPolyEdit = ( pObj != NULL ) && pObj->ISA( SdrPathObj );
    const BOOL          bDrawEnabled = !( bPolyEdit && aTbxIMapDlg1.IsItemChecked( TBI_POLYEDIT ) );

    aTbxIMapDlg1.EnableItem( TBI_APPLY, pOwnData->bExecState && pWnd->IsChanged() );

    aTbxIMapDlg1.EnableItem( TBI_SELECT, bDrawEnabled );
    aTbxIMapDlg1.EnableItem( TBI_RECT, bDrawEnabled );
    aTbxIMapDlg1.EnableItem( TBI_CIRCLE, bDrawEnabled );
    aTbxIMapDlg1.EnableItem( TBI_POLY, bDrawEnabled );
    aTbxIMapDlg1.EnableItem( TBI_FREEPOLY, bDrawEnabled );

    // BezierEditor-Stati
    aTbxIMapDlg1.EnableItem( TBI_POLYEDIT, bPolyEdit );
    aTbxIMapDlg1.EnableItem( TBI_POLYMOVE, !bDrawEnabled );
    aTbxIMapDlg1.EnableItem( TBI_POLYINSERT, !bDrawEnabled );
    aTbxIMapDlg1.EnableItem( TBI_POLYDELETE, !bDrawEnabled && pView->IsDeleteMarkedPointsPossible() );

    // Undo/Redo
    aTbxIMapDlg1.EnableItem( TBI_UNDO, pModel->GetUndoActionCount() > 0 );
    aTbxIMapDlg1.EnableItem( TBI_REDO, pModel->GetRedoActionCount() > 0 );

    if ( bPolyEdit )
    {
        USHORT nId = 0;

        switch( pWnd->GetPolyEditMode() )
        {
            case( SID_BEZIER_MOVE ): nId = TBI_POLYMOVE; break;
            case( SID_BEZIER_INSERT ): nId = TBI_POLYINSERT; break;

            default:
            break;
        }

        aTbxIMapDlg1.CheckItem( nId, TRUE );
    }
    else
    {
        aTbxIMapDlg1.CheckItem( TBI_POLYEDIT, FALSE );
        aTbxIMapDlg1.CheckItem( TBI_POLYMOVE, TRUE );
        aTbxIMapDlg1.CheckItem( TBI_POLYINSERT, FALSE );
        pWnd->SetPolyEditMode( 0 );
    }

    return 0L;
}


