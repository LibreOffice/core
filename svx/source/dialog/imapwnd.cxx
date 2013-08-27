/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/help.hxx>
#include <sfx2/sfxsids.hrc>     // SID_ATTR_MACROITEM
#include <svtools/imaprect.hxx>
#include <svtools/imapcirc.hxx>
#include <svtools/imappoly.hxx>
#include <svl/urlbmk.hxx>

#include <svx/xoutbmp.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>
#include <imapdlg.hrc>
#include <imapwnd.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdopath.hxx>
#include <svx/xfltrit.hxx>
#include <svx/svdpagv.hxx>
#include <svl/urihelper.hxx>
#include <svx/xfillit.hxx>
#include <svx/xlineit.hxx>
#include <sfx2/evntconf.hxx>

#include <sot/formats.hxx>

#include <svx/svxdlg.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

using ::com::sun::star::frame::XFrame;
using ::com::sun::star::uno::Reference;

#define TRANSCOL Color( COL_WHITE )

IMapWindow::IMapWindow( Window* pParent, const ResId& rResId, const Reference< XFrame >& rxDocumentFrame ) :
            GraphCtrl( pParent, rResId ),
            DropTargetHelper( this ),
            mxDocumentFrame( rxDocumentFrame )
{
    SetWinStyle( WB_SDRMODE );

    pItemInfo = new SfxItemInfo[ 1 ];
    memset( pItemInfo, 0, sizeof( SfxItemInfo ) );
    pIMapPool = new SfxItemPool( OUString("IMapItemPool"),
                                 SID_ATTR_MACROITEM, SID_ATTR_MACROITEM, pItemInfo );
    pIMapPool->FreezeIdRanges();
}

IMapWindow::~IMapWindow()
{
    SfxItemPool::Free(pIMapPool);
    delete[] pItemInfo;
}

void IMapWindow::SetImageMap( const ImageMap& rImageMap )
{
    ReplaceImageMap( rImageMap, sal_False );
}

void IMapWindow::ReplaceImageMap( const ImageMap& rImageMap, sal_Bool /*bScaleToGraphic*/ )
{
    SdrPage* pPage = 0;
    aIMap = rImageMap;

    if(GetSdrModel())
    {
        // try to access page
        pPage = GetSdrModel()->GetPage(0L);
    }

    if(pPage)
    {
        // clear all draw objects
        pPage->Clear();
    }

    if(GetSdrView())
    {
        // #i63762# reset selection at view
        GetSdrView()->UnmarkAllObj();
    }

    // create new drawing objects
    const sal_uInt16 nCount(rImageMap.GetIMapObjectCount());

    for ( sal_uInt16 i(nCount); i > 0; i-- )
    {
        SdrObject* pNewObj = CreateObj( rImageMap.GetIMapObject( i - 1 ) );

        if ( pNewObj )
        {
            pPage->InsertObject( pNewObj );
        }
    }
}

sal_Bool IMapWindow::ReplaceActualIMapInfo( const NotifyInfo& rNewInfo )
{
    const SdrObject*    pSdrObj = GetSelectedSdrObject();
    IMapObject*         pIMapObj;
    sal_Bool                bRet = sal_False;

    if ( pSdrObj && ( ( pIMapObj = GetIMapObj( pSdrObj ) ) != NULL ) )
    {
        pIMapObj->SetURL( rNewInfo.aMarkURL );
        pIMapObj->SetAltText( rNewInfo.aMarkAltText );
        pIMapObj->SetTarget( rNewInfo.aMarkTarget );
        pModel->SetChanged( sal_True );
        UpdateInfo( sal_False );

        bRet = sal_True;
    }

    return bRet;
}

const ImageMap& IMapWindow::GetImageMap()
{
    if ( pModel->IsChanged() )
    {
        SdrPage* pPage = (SdrPage*) pModel->GetPage( 0 );

        if ( pPage )
        {
            const long nCount = pPage->GetObjCount();

            aIMap.ClearImageMap();

            for ( long i = nCount - 1; i > -1; i-- )
                aIMap.InsertIMapObject( *( ( (IMapUserData*) pPage->GetObj( i )->GetUserData( 0 ) )->GetObject() ) );
        }

        pModel->SetChanged( sal_False );
    }

    return aIMap;
}

void IMapWindow::SetTargetList( TargetList& rTargetList )
{
    // Delete old List
    aTargetList.clear();

    // Fill with the provided list
    for( size_t i = 0, n = rTargetList.size(); i < n; ++i )
        aTargetList.push_back( rTargetList[ i ] );

    pModel->SetChanged( sal_False );
}

SdrObject* IMapWindow::CreateObj( const IMapObject* pIMapObj )
{
    Point       aPoint;
    Rectangle   aClipRect( aPoint, GetGraphicSize() );
    SdrObject*  pSdrObj = NULL;
    IMapObjectPtr pCloneIMapObj;

    switch( pIMapObj->GetType() )
    {
        case( IMAP_OBJ_RECTANGLE ):
        {
            IMapRectangleObject*    pIMapRectObj = (IMapRectangleObject*) pIMapObj;
            Rectangle               aDrawRect( pIMapRectObj->GetRectangle( sal_False ) );

            // clipped on CanvasPane
            aDrawRect.Intersection( aClipRect );

            pSdrObj = (SdrObject*) new SdrRectObj( aDrawRect );
            pCloneIMapObj.reset((IMapObject*) new IMapRectangleObject( *pIMapRectObj ));
        }
        break;

        case( IMAP_OBJ_CIRCLE ):
        {
            IMapCircleObject*   pIMapCircleObj = (IMapCircleObject*) pIMapObj;
            const Point         aCenter( pIMapCircleObj->GetCenter( sal_False ) );
            const long          nRadius = pIMapCircleObj->GetRadius( sal_False );
            const Point         aOffset( nRadius, nRadius );
            Rectangle           aCircle( aCenter - aOffset, aCenter + aOffset );

            // limited to CanvasPane
            aCircle.Intersection( aClipRect );

            pSdrObj = (SdrObject*) new SdrCircObj( OBJ_CIRC, aCircle, 0, 36000 );
            pCloneIMapObj.reset((IMapObject*) new IMapCircleObject( *pIMapCircleObj ));
        }
        break;

        case( IMAP_OBJ_POLYGON ):
        {
            IMapPolygonObject*  pIMapPolyObj = (IMapPolygonObject*) pIMapObj;

            // If it actually is an ellipse, then another ellipse is created again
            if ( pIMapPolyObj->HasExtraEllipse() )
            {
                Rectangle aDrawRect( pIMapPolyObj->GetExtraEllipse() );

                // clipped on CanvasPane
                aDrawRect.Intersection( aClipRect );

                pSdrObj = (SdrObject*) new SdrCircObj( OBJ_CIRC, aDrawRect, 0, 36000 );
            }
            else
            {
                const Polygon&  rPoly = pIMapPolyObj->GetPolygon( sal_False );
                Polygon         aDrawPoly( rPoly );

                // clipped on CanvasPane
                aDrawPoly.Clip( aClipRect );

                basegfx::B2DPolygon aPolygon;
                aPolygon.append(aDrawPoly.getB2DPolygon());
                pSdrObj = (SdrObject*)new SdrPathObj(OBJ_POLY, basegfx::B2DPolyPolygon(aPolygon));
            }

            pCloneIMapObj.reset((IMapObject*) new IMapPolygonObject( *pIMapPolyObj ));
        }
        break;

        default:
        break;
    }

    if ( pSdrObj )
    {
        SfxItemSet aSet( pModel->GetItemPool() );

        aSet.Put( XFillStyleItem( XFILL_SOLID ) );
        aSet.Put( XFillColorItem( String(), TRANSCOL ) );

        if ( !pIMapObj->IsActive() )
        {
            aSet.Put( XFillTransparenceItem( 100 ) );
            aSet.Put( XLineColorItem( String(), Color( COL_RED ) ) );
        }
        else
        {
            aSet.Put( XFillTransparenceItem( 50 ) );
            aSet.Put( XLineColorItem( String(), Color( COL_BLACK ) ) );
        }

        pSdrObj->SetMergedItemSetAndBroadcast(aSet);

        pSdrObj->AppendUserData( new IMapUserData( pCloneIMapObj ) );
        pSdrObj->SetUserCall( GetSdrUserCall() );
    }

    return pSdrObj;
}

void IMapWindow::InitSdrModel()
{
    GraphCtrl::InitSdrModel();

    SfxItemSet aSet( pModel->GetItemPool() );

    aSet.Put( XFillColorItem( String(), TRANSCOL ) );
    aSet.Put( XFillTransparenceItem( 50 ) );
    pView->SetAttributes( aSet );
    pView->SetFrameDragSingles( sal_True );
}

void IMapWindow::SdrObjCreated( const SdrObject& rObj )
{
    switch( rObj.GetObjIdentifier() )
    {
        case( OBJ_RECT ):
        {
            SdrRectObj*          pRectObj = (SdrRectObj*) &rObj;
            IMapRectangleObject* pObj = new IMapRectangleObject( pRectObj->GetLogicRect(),
                String(), String(), String(), String(), String(), sal_True, sal_False );

            pRectObj->AppendUserData( new IMapUserData( IMapObjectPtr(pObj) ) );
        }
        break;

        case( OBJ_CIRC ):
        {
            SdrCircObj* pCircObj = (SdrCircObj*) &rObj;
            SdrPathObj* pPathObj = (SdrPathObj*) pCircObj->ConvertToPolyObj( sal_False, sal_False );
            Polygon aPoly(pPathObj->GetPathPoly().getB2DPolygon(0L));
            delete pPathObj;

            IMapPolygonObject* pObj = new IMapPolygonObject( Polygon(aPoly), String(), String(), String(), String(), String(),  sal_True, sal_False );
            pObj->SetExtraEllipse( aPoly.GetBoundRect() );
            pCircObj->AppendUserData( new IMapUserData( IMapObjectPtr(pObj) ) );
        }
        break;

        case( OBJ_POLY ):
        case( OBJ_FREEFILL ):
        case( OBJ_PATHPOLY ):
        case( OBJ_PATHFILL ):
        {
            SdrPathObj* pPathObj = (SdrPathObj*) &rObj;
            const basegfx::B2DPolyPolygon& rXPolyPoly = pPathObj->GetPathPoly();

            if ( rXPolyPoly.count() )
            {
                Polygon aPoly(rXPolyPoly.getB2DPolygon(0L));
                IMapPolygonObject* pObj = new IMapPolygonObject( aPoly, String(), String(), String(), String(), String(),  sal_True, sal_False );
                pPathObj->AppendUserData( new IMapUserData( IMapObjectPtr(pObj) ) );
            }
        }
        break;

        default:
        break;
    }
}

void IMapWindow::SdrObjChanged( const SdrObject& rObj )
{
    IMapUserData* pUserData = (IMapUserData*) rObj.GetUserData( 0 );

    if ( pUserData )
    {
        String          aURL;
        String          aAltText;
        String          aDesc;
        String          aTarget;
        IMapObjectPtr   pIMapObj = pUserData->GetObject();
        sal_Bool            bActive = sal_True;

        if ( pIMapObj.get() )
        {
            aURL = pIMapObj->GetURL();
            aAltText = pIMapObj->GetAltText();
            aDesc = pIMapObj->GetDesc();
            aTarget = pIMapObj->GetTarget();
            bActive = pIMapObj->IsActive();
        }

        switch( rObj.GetObjIdentifier() )
        {
            case( OBJ_RECT ):
            {
                pUserData->ReplaceObject( IMapObjectPtr(new IMapRectangleObject( ( (const SdrRectObj&) rObj ).GetLogicRect(),
                          aURL, aAltText, aDesc, aTarget, String(), bActive, sal_False ) ) );
            }
            break;

            case( OBJ_CIRC ):
            {
                const SdrCircObj& rCircObj = (const SdrCircObj&) rObj;
                SdrPathObj* pPathObj = (SdrPathObj*) rCircObj.ConvertToPolyObj( sal_False, sal_False );
                Polygon aPoly(pPathObj->GetPathPoly().getB2DPolygon(0L));

                IMapPolygonObject* pObj = new IMapPolygonObject( aPoly, aURL, aAltText, aDesc, aTarget, String(), bActive, sal_False );
                pObj->SetExtraEllipse( aPoly.GetBoundRect() );

                // was only created by us temporarily
                delete pPathObj;
                pUserData->ReplaceObject( IMapObjectPtr(pObj) );
            }
            break;

            case( OBJ_POLY ):
            case( OBJ_FREEFILL ):
            case( OBJ_PATHPOLY ):
            case( OBJ_PATHFILL ):
            {
                const SdrPathObj& rPathObj = (const SdrPathObj&) rObj;
                const basegfx::B2DPolyPolygon& rXPolyPoly = rPathObj.GetPathPoly();

                if ( rXPolyPoly.count() )
                {
                    Polygon aPoly(rPathObj.GetPathPoly().getB2DPolygon(0L));
                    IMapPolygonObject*  pObj = new IMapPolygonObject( aPoly, aURL, aAltText, aDesc, aTarget, String(), bActive, sal_False );
                    pUserData->ReplaceObject( IMapObjectPtr(pObj) );
                }
            }
            break;

            default:
            break;
        }
    }
}

void IMapWindow::MouseButtonUp(const MouseEvent& rMEvt)
{
    GraphCtrl::MouseButtonUp( rMEvt );
    UpdateInfo( sal_True );
}

void IMapWindow::MarkListHasChanged()
{
    GraphCtrl::MarkListHasChanged();
    UpdateInfo( sal_False );
}

SdrObject* IMapWindow::GetHitSdrObj( const Point& rPosPixel ) const
{
    SdrObject*  pObj = NULL;
    Point       aPt = PixelToLogic( rPosPixel );

    if ( Rectangle( Point(), GetGraphicSize() ).IsInside( aPt ) )
    {
        SdrPage* pPage = (SdrPage*) pModel->GetPage( 0 );
        sal_uIntPtr  nCount;

        if ( pPage && ( ( nCount = pPage->GetObjCount() ) > 0 ) )
        {
            for ( long i = nCount - 1; i >= 0; i-- )
            {
                SdrObject*  pTestObj = pPage->GetObj( i );
                IMapObject* pIMapObj = GetIMapObj( pTestObj );

                if ( pIMapObj && pIMapObj->IsHit( aPt ) )
                {
                    pObj = pTestObj;
                    break;
                }
            }
        }
    }

    return pObj;
}

IMapObject* IMapWindow::GetIMapObj( const SdrObject* pSdrObj ) const
{
    IMapObject* pIMapObj = NULL;

    if ( pSdrObj )
    {
        IMapUserData* pUserData = (IMapUserData*) pSdrObj->GetUserData( 0 );

        if ( pUserData )
            pIMapObj = pUserData->GetObject().get();
    }

    return pIMapObj;
}

void IMapWindow::Command(const CommandEvent& rCEvt)
{
    Region  aRegion;

    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        PopupMenu           aMenu( SVX_RES( RID_SVXMN_IMAP ) );
        const SdrMarkList&  rMarkList = pView->GetMarkedObjectList();
        sal_uIntPtr             nMarked = rMarkList.GetMarkCount();

        aMenu.EnableItem( MN_URL, sal_False );
        aMenu.EnableItem( MN_ACTIVATE, sal_False );
        aMenu.EnableItem( MN_MACRO, sal_False );
        aMenu.EnableItem( MN_MARK_ALL, pModel->GetPage( 0 )->GetObjCount() != pView->GetMarkedObjectCount() );

        if ( !nMarked )
        {
            aMenu.EnableItem( MN_POSITION, sal_False );
            aMenu.EnableItem( MN_FRAME_TO_TOP, sal_False );
            aMenu.EnableItem( MN_MOREFRONT, sal_False );
            aMenu.EnableItem( MN_MOREBACK, sal_False );
            aMenu.EnableItem( MN_FRAME_TO_BOTTOM, sal_False );
            aMenu.EnableItem( MN_DELETE1, sal_False );
        }
        else
        {
            if ( nMarked == 1 )
            {
                SdrObject*  pSdrObj = GetSelectedSdrObject();

                aMenu.EnableItem( MN_URL, sal_True );
                aMenu.EnableItem( MN_ACTIVATE, sal_True );
                aMenu.EnableItem( MN_MACRO, sal_True );
                aMenu.CheckItem( MN_ACTIVATE, GetIMapObj( pSdrObj )->IsActive() );
            }

            aMenu.EnableItem( MN_POSITION, sal_True );
            aMenu.EnableItem( MN_FRAME_TO_TOP, sal_True );
            aMenu.EnableItem( MN_MOREFRONT, sal_True );
            aMenu.EnableItem( MN_MOREBACK, sal_True );
            aMenu.EnableItem( MN_FRAME_TO_BOTTOM, sal_True );
            aMenu.EnableItem( MN_DELETE1, sal_True );
        }

        aMenu.SetSelectHdl( LINK( this, IMapWindow, MenuSelectHdl ) );
        aMenu.Execute( this, rCEvt.GetMousePosPixel() );
    }
    else
        Window::Command(rCEvt);
}

sal_Int8 IMapWindow::AcceptDrop( const AcceptDropEvent& rEvt )
{
    return( ( GetHitSdrObj( rEvt.maPosPixel ) != NULL ) ? rEvt.mnAction : DND_ACTION_NONE );
}

sal_Int8 IMapWindow::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( IsDropFormatSupported( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK ) )
    {
        const String    aString;
        INetBookmark    aBookMark( aString, aString );
        SdrObject*      pSdrObj = GetHitSdrObj( rEvt.maPosPixel );

        if( pSdrObj && TransferableDataHelper( rEvt.maDropEvent.Transferable ).GetINetBookmark( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK, aBookMark ) )
        {
            IMapObject* pIMapObj = GetIMapObj( pSdrObj );

            pIMapObj->SetURL( aBookMark.GetURL() );
            pIMapObj->SetAltText( aBookMark.GetDescription() );
            pModel->SetChanged( sal_True );
            pView->UnmarkAll();
            pView->MarkObj( pSdrObj, pView->GetSdrPageView() );
            UpdateInfo( sal_True );
            nRet =  rEvt.mnAction;
        }
    }

    return nRet;
}

void IMapWindow::RequestHelp( const HelpEvent& rHEvt )
{
    Point               aPos = PixelToLogic( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );

    if ( Help::IsBalloonHelpEnabled() || Help::IsQuickHelpEnabled() )
    {
        SdrObject*          pSdrObj = NULL;
        SdrPageView*        pPageView = NULL;
        if ( pView->PickObj( aPos, pView->getHitTolLog(), pSdrObj, pPageView ) )
        {
            const IMapObject*   pIMapObj = GetIMapObj( pSdrObj );
            String              aStr;

            if ( pIMapObj && ( aStr = pIMapObj->GetURL() ).Len() )
            {
                Rectangle   aLogicPix( LogicToPixel( Rectangle( Point(), GetGraphicSize() ) ) );
                Rectangle   aScreenRect( OutputToScreenPixel( aLogicPix.TopLeft() ),
                                         OutputToScreenPixel( aLogicPix.BottomRight() ) );

                if ( Help::IsBalloonHelpEnabled() )
                    Help::ShowBalloon( this, rHEvt.GetMousePosPixel(), aScreenRect, aStr );
                else if ( Help::IsQuickHelpEnabled() )
                    Help::ShowQuickHelp( this, aScreenRect, aStr );
            }
        }
        else
            Window::RequestHelp( rHEvt );
    }
}

void IMapWindow::SetCurrentObjState( sal_Bool bActive )
{
    SdrObject* pObj = GetSelectedSdrObject();

    if ( pObj )
    {
        SfxItemSet aSet( pModel->GetItemPool() );

        GetIMapObj( pObj )->SetActive( bActive );

        aSet.Put( XFillColorItem( String(), TRANSCOL ) );

        if ( !bActive )
        {
            aSet.Put( XFillTransparenceItem( 100 ) );
            aSet.Put( XLineColorItem( String(), Color( COL_RED ) ) );
        }
        else
        {
            aSet.Put( XFillTransparenceItem( 50 ) );
            aSet.Put( XLineColorItem( String(), Color( COL_BLACK ) ) );
        }

        pView->SetAttributes( aSet, sal_False );
    }
}

void IMapWindow::UpdateInfo( sal_Bool bNewObj )
{
    if ( aInfoLink.IsSet() )
    {
        const SdrObject*    pSdrObj = GetSelectedSdrObject();
        const IMapObject*   pIMapObj = pSdrObj ? GetIMapObj( pSdrObj ) : NULL;

        aInfo.bNewObj = bNewObj;

        if ( pIMapObj )
        {
            aInfo.bOneMarked = sal_True;
            aInfo.aMarkURL = pIMapObj->GetURL();
            aInfo.aMarkAltText = pIMapObj->GetAltText();
            aInfo.aMarkTarget = pIMapObj->GetTarget();
            aInfo.bActivated = pIMapObj->IsActive();
            aInfoLink.Call( this );
        }
        else
        {
            aInfo.aMarkURL = aInfo.aMarkAltText = aInfo.aMarkTarget = String();
            aInfo.bOneMarked = sal_False;
            aInfo.bActivated = sal_False;
        }

        aInfoLink.Call( this );
    }
}

void IMapWindow::DoMacroAssign()
{
    SdrObject*  pSdrObj = GetSelectedSdrObject();

    if ( pSdrObj )
    {
        SfxItemSet      aSet( *pIMapPool, SID_ATTR_MACROITEM, SID_ATTR_MACROITEM, SID_EVENTCONFIG, SID_EVENTCONFIG, 0 );

        SfxEventNamesItem aNamesItem(SID_EVENTCONFIG);
        aNamesItem.AddEvent( OUString("MouseOver"), String(), SFX_EVENT_MOUSEOVER_OBJECT );
        aNamesItem.AddEvent( OUString("MouseOut"), String(), SFX_EVENT_MOUSEOUT_OBJECT );
        aSet.Put( aNamesItem );

        SvxMacroItem    aMacroItem(SID_ATTR_MACROITEM);
        IMapObject*     pIMapObj = GetIMapObj( pSdrObj );
        aMacroItem.SetMacroTable( pIMapObj->GetMacroTable() );
        aSet.Put( aMacroItem, SID_ATTR_MACROITEM );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        SfxAbstractDialog* pMacroDlg = pFact->CreateSfxDialog( this, aSet, mxDocumentFrame, SID_EVENTCONFIG );

        if ( pMacroDlg && pMacroDlg->Execute() == RET_OK )
        {
            const SfxItemSet* pOutSet = pMacroDlg->GetOutputItemSet();
            pIMapObj->SetMacroTable( ((const SvxMacroItem& )pOutSet->Get( SID_ATTR_MACROITEM )).GetMacroTable() );
            pModel->SetChanged( sal_True );
            UpdateInfo( sal_False );
        }

        delete pMacroDlg;
    }
}

void IMapWindow::DoPropertyDialog()
{
    SdrObject*  pSdrObj = GetSelectedSdrObject();

    if ( pSdrObj )
    {
        IMapObject* pIMapObj = GetIMapObj( pSdrObj );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            AbstractURLDlg* aDlg = pFact->CreateURLDialog( this, pIMapObj->GetURL(), pIMapObj->GetAltText(), pIMapObj->GetDesc(),
                                            pIMapObj->GetTarget(), pIMapObj->GetName(), aTargetList );
            DBG_ASSERT(aDlg, "Dialogdiet fail!");
            if ( aDlg->Execute() == RET_OK )
            {
                const String aURLText( aDlg->GetURL() );

                if ( aURLText.Len() )
                {
                    INetURLObject aObj( aURLText, INET_PROT_FILE );
                    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL" );
                    pIMapObj->SetURL( aObj.GetMainURL( INetURLObject::NO_DECODE ) );
                }
                else
                    pIMapObj->SetURL( aURLText );

                pIMapObj->SetAltText( aDlg->GetAltText() );
                pIMapObj->SetDesc( aDlg->GetDesc() );
                pIMapObj->SetTarget( aDlg->GetTarget() );
                pIMapObj->SetName( aDlg->GetName() );
                pModel->SetChanged( sal_True );
                UpdateInfo( sal_True );
            }
            delete aDlg;
        }
    }
}

IMPL_LINK( IMapWindow, MenuSelectHdl, Menu*, pMenu )
{
    if (pMenu)
    {
        sal_uInt16  nId = pMenu->GetCurItemId();

        switch(nId)
        {
            case( MN_URL ):
                DoPropertyDialog();
            break;

            case( MN_MACRO ):
                DoMacroAssign();
            break;

            case( MN_ACTIVATE ):
            {
                const sal_Bool bNewState = !pMenu->IsItemChecked( MN_ACTIVATE );

                pMenu->CheckItem( MN_ACTIVATE, bNewState );
                SetCurrentObjState( bNewState );
                UpdateInfo( sal_False );
            }

            case( MN_FRAME_TO_TOP ):
                pView->PutMarkedToTop();
            break;

            case( MN_MOREFRONT ):
                pView->MovMarkedToTop();
            break;

            case( MN_MOREBACK ):
                pView->MovMarkedToBtm();
            break;

            case( MN_FRAME_TO_BOTTOM ):
                pView->PutMarkedToBtm();
            break;

            case( MN_MARK_ALL ):
                pView->MarkAll();
            break;

            case( MN_DELETE1 ):
                pView->DeleteMarked();

            default :
            break;
        }
    }

    return 0;
}

void IMapWindow::CreateDefaultObject()
{
    SdrPageView* pPageView = pView->GetSdrPageView();

    if(pPageView)
    {
        // calc position and size
        Point aPagePos(0, 0); // = pPageView->GetOffset();
        Size aPageSize = pPageView->GetPage()->GetSize();
        sal_uInt32 nDefaultObjectSizeWidth = aPageSize.Width() / 4;
        sal_uInt32 nDefaultObjectSizeHeight = aPageSize.Height() / 4;
        aPagePos.X() += (aPageSize.Width() / 2) - (nDefaultObjectSizeWidth / 2);
        aPagePos.Y() += (aPageSize.Height() / 2) - (nDefaultObjectSizeHeight / 2);
        Rectangle aNewObjectRectangle(aPagePos, Size(nDefaultObjectSizeWidth, nDefaultObjectSizeHeight));

        SdrObject* pObj = SdrObjFactory::MakeNewObject( pView->GetCurrentObjInventor(), pView->GetCurrentObjIdentifier(), 0L, pModel);
        pObj->SetLogicRect(aNewObjectRectangle);

        switch( pObj->GetObjIdentifier() )
        {
        case OBJ_POLY:
        case OBJ_PATHPOLY:
            {
                basegfx::B2DPolygon aInnerPoly;
                aInnerPoly.append(basegfx::B2DPoint(aNewObjectRectangle.BottomLeft().X(), aNewObjectRectangle.BottomLeft().Y()));
                aInnerPoly.append(basegfx::B2DPoint(aNewObjectRectangle.TopLeft().X(), aNewObjectRectangle.TopLeft().Y()));
                aInnerPoly.append(basegfx::B2DPoint(aNewObjectRectangle.TopCenter().X(), aNewObjectRectangle.TopCenter().Y()));
                aInnerPoly.append(basegfx::B2DPoint(aNewObjectRectangle.Center().X(), aNewObjectRectangle.Center().Y()));
                aInnerPoly.append(basegfx::B2DPoint(aNewObjectRectangle.RightCenter().X(), aNewObjectRectangle.RightCenter().Y()));
                aInnerPoly.append(basegfx::B2DPoint(aNewObjectRectangle.BottomRight().X(), aNewObjectRectangle.BottomRight().Y()));
                aInnerPoly.setClosed(true);
                ((SdrPathObj*)pObj)->SetPathPoly(basegfx::B2DPolyPolygon(aInnerPoly));
                break;
            }
        case OBJ_FREEFILL:
        case OBJ_PATHFILL:
            {
                sal_Int32 nWdt(aNewObjectRectangle.GetWidth() / 2);
                sal_Int32 nHgt(aNewObjectRectangle.GetHeight() / 2);
                basegfx::B2DPolygon aInnerPoly(XPolygon(aNewObjectRectangle.Center(), nWdt, nHgt).getB2DPolygon());
                ((SdrPathObj*)pObj)->SetPathPoly(basegfx::B2DPolyPolygon(aInnerPoly));
                break;
            }

        }

        pView->InsertObjectAtView(pObj, *pPageView);
        SdrObjCreated( *pObj );
        SetCurrentObjState( true );
        pView->MarkObj( pObj, pPageView );
    }
}

void IMapWindow::KeyInput( const KeyEvent& rKEvt )
{
    GraphCtrl::KeyInput( rKEvt );
}

void IMapWindow::SelectFirstObject()
{
    SdrPage*    pPage = (SdrPage*) pModel->GetPage( 0 );
    if( pPage->GetObjCount() != 0 )
    {
        GrabFocus();
        pView->UnmarkAllObj();
        pView->MarkNextObj(sal_True);
    }
}

void IMapWindow::StartPolyEdit()
{
    GrabFocus();

    if( !pView->AreObjectsMarked() )
        pView->MarkNextObj(sal_True);

    const SdrHdlList& rHdlList = pView->GetHdlList();
    SdrHdl* pHdl = rHdlList.GetFocusHdl();

    if(!pHdl)
    {
        ((SdrHdlList&)rHdlList).TravelFocusHdl(true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
