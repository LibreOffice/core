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
#include <sfx2/sfxsids.hrc>
#include <svtools/imaprect.hxx>
#include <svtools/imapcirc.hxx>
#include <svtools/imappoly.hxx>
#include <svl/urlbmk.hxx>

#include <svx/xoutbmp.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
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
#include <memory>

using namespace com::sun::star;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::uno::Reference;

#define TRANSCOL Color( COL_WHITE )

IMapWindow::IMapWindow( vcl::Window* pParent, WinBits nBits, const Reference< XFrame >& rxDocumentFrame ) :
            GraphCtrl( pParent, nBits ),
            DropTargetHelper( this ),
            mxDocumentFrame( rxDocumentFrame )
{
    SetSdrMode(true);

    pItemInfo = new SfxItemInfo[ 1 ];
    memset( pItemInfo, 0, sizeof( SfxItemInfo ) );
    pIMapPool = new SfxItemPool( "IMapItemPool",
                                 SID_ATTR_MACROITEM, SID_ATTR_MACROITEM, pItemInfo );
    pIMapPool->FreezeIdRanges();
}

IMapWindow::~IMapWindow()
{
    disposeOnce();
}

void IMapWindow::dispose()
{
    SfxItemPool::Free(pIMapPool);
    delete[] pItemInfo;
    GraphCtrl::dispose();
}

Size IMapWindow::GetOptimalSize() const
{
    return LogicToPixel(Size(270, 170), MapUnit::MapAppFont);
}

void IMapWindow::SetImageMap( const ImageMap& rImageMap )
{
    ReplaceImageMap( rImageMap );
}

void IMapWindow::ReplaceImageMap( const ImageMap& rImageMap )
{
    SdrPage* pPage = nullptr;
    aIMap = rImageMap;

    if(GetSdrModel())
    {
        // try to access page
        pPage = GetSdrModel()->GetPage(0);
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

        if (pNewObj && pPage)
        {
            pPage->InsertObject( pNewObj );
        }
    }
}

void IMapWindow::ReplaceActualIMapInfo( const NotifyInfo& rNewInfo )
{
    const SdrObject*    pSdrObj = GetSelectedSdrObject();
    IMapObject*         pIMapObj;

    if ( pSdrObj && ( ( pIMapObj = GetIMapObj( pSdrObj ) ) != nullptr ) )
    {
        pIMapObj->SetURL( rNewInfo.aMarkURL );
        pIMapObj->SetAltText( rNewInfo.aMarkAltText );
        pIMapObj->SetTarget( rNewInfo.aMarkTarget );
        pModel->SetChanged();
        UpdateInfo( false );
    }
}

const ImageMap& IMapWindow::GetImageMap()
{
    if ( pModel->IsChanged() )
    {
        SdrPage* pPage = pModel->GetPage( 0 );

        if ( pPage )
        {
            const size_t nCount = pPage->GetObjCount();

            aIMap.ClearImageMap();

            for ( size_t i = nCount; i; )
            {
                --i;
                aIMap.InsertIMapObject( *( static_cast<IMapUserData*>( pPage->GetObj( i )->GetUserData( 0 ) )->GetObject() ) );
            }
        }

        pModel->SetChanged( false );
    }

    return aIMap;
}

void IMapWindow::SetTargetList( TargetList& rTargetList )
{
    // Delete old List
    aTargetList.clear();

    // Fill with the provided list
    for(const OUString & s : rTargetList)
        aTargetList.push_back( s );

    pModel->SetChanged( false );
}

SdrObject* IMapWindow::CreateObj( const IMapObject* pIMapObj )
{
    Point       aPoint;
    tools::Rectangle   aClipRect( aPoint, GetGraphicSize() );
    SdrObject*  pSdrObj = nullptr;
    IMapObjectPtr pCloneIMapObj;

    switch( pIMapObj->GetType() )
    {
        case IMAP_OBJ_RECTANGLE:
        {
            const IMapRectangleObject* pIMapRectObj = static_cast<const IMapRectangleObject*>(pIMapObj);
            tools::Rectangle               aDrawRect( pIMapRectObj->GetRectangle( false ) );

            // clipped on CanvasPane
            aDrawRect.Intersection( aClipRect );

            pSdrObj = static_cast<SdrObject*>(new SdrRectObj( aDrawRect ));
            pCloneIMapObj.reset(static_cast<IMapObject*>(new IMapRectangleObject( *pIMapRectObj )));
        }
        break;

        case IMAP_OBJ_CIRCLE:
        {
            const IMapCircleObject*   pIMapCircleObj = static_cast<const IMapCircleObject*>(pIMapObj);
            const Point         aCenter( pIMapCircleObj->GetCenter( false ) );
            const long          nRadius = pIMapCircleObj->GetRadius( false );
            const Point         aOffset( nRadius, nRadius );
            tools::Rectangle           aCircle( aCenter - aOffset, aCenter + aOffset );

            // limited to CanvasPane
            aCircle.Intersection( aClipRect );

            pSdrObj = static_cast<SdrObject*>(new SdrCircObj( OBJ_CIRC, aCircle, 0, 36000 ));
            pCloneIMapObj.reset(static_cast<IMapObject*>(new IMapCircleObject( *pIMapCircleObj )));
        }
        break;

        case IMAP_OBJ_POLYGON:
        {
            const IMapPolygonObject*  pIMapPolyObj = static_cast<const IMapPolygonObject*>(pIMapObj);

            // If it actually is an ellipse, then another ellipse is created again
            if ( pIMapPolyObj->HasExtraEllipse() )
            {
                tools::Rectangle aDrawRect( pIMapPolyObj->GetExtraEllipse() );

                // clipped on CanvasPane
                aDrawRect.Intersection( aClipRect );

                pSdrObj = static_cast<SdrObject*>(new SdrCircObj( OBJ_CIRC, aDrawRect, 0, 36000 ));
            }
            else
            {
                const tools::Polygon& rPoly = pIMapPolyObj->GetPolygon( false );
                tools::Polygon aDrawPoly( rPoly );

                // clipped on CanvasPane
                aDrawPoly.Clip( aClipRect );

                basegfx::B2DPolygon aPolygon;
                aPolygon.append(aDrawPoly.getB2DPolygon());
                pSdrObj = static_cast<SdrObject*>(new SdrPathObj(OBJ_POLY, basegfx::B2DPolyPolygon(aPolygon)));
            }

            pCloneIMapObj.reset(static_cast<IMapObject*>(new IMapPolygonObject( *pIMapPolyObj )));
        }
        break;

        default:
        break;
    }

    if ( pSdrObj )
    {
        SfxItemSet aSet( pModel->GetItemPool() );

        aSet.Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
        aSet.Put( XFillColorItem( "", TRANSCOL ) );

        if ( !pIMapObj->IsActive() )
        {
            aSet.Put( XFillTransparenceItem( 100 ) );
            aSet.Put( XLineColorItem( "", Color( COL_RED ) ) );
        }
        else
        {
            aSet.Put( XFillTransparenceItem( 50 ) );
            aSet.Put( XLineColorItem( "", Color( COL_BLACK ) ) );
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

    aSet.Put( XFillColorItem( "", TRANSCOL ) );
    aSet.Put( XFillTransparenceItem( 50 ) );
    pView->SetAttributes( aSet );
    pView->SetFrameDragSingles();
}

void IMapWindow::SdrObjCreated( const SdrObject& rObj )
{
    switch( rObj.GetObjIdentifier() )
    {
        case OBJ_RECT:
        {
            SdrRectObj*    pRectObj = const_cast<SdrRectObj*>(static_cast<const SdrRectObj*>(&rObj));
            IMapRectangleObject* pObj = new IMapRectangleObject( pRectObj->GetLogicRect(), "", "", "", "", "", true, false );

            pRectObj->AppendUserData( new IMapUserData( IMapObjectPtr(pObj) ) );
        }
        break;

        case OBJ_CIRC:
        {
            SdrCircObj* pCircObj = const_cast<SdrCircObj*>( static_cast<const SdrCircObj*>(&rObj) );
            SdrPathObj* pPathObj = static_cast<SdrPathObj*>( pCircObj->ConvertToPolyObj( false, false ) );
            tools::Polygon aPoly(pPathObj->GetPathPoly().getB2DPolygon(0));
            delete pPathObj;

            IMapPolygonObject* pObj = new IMapPolygonObject( aPoly, "", "", "", "", "", true, false );
            pObj->SetExtraEllipse( aPoly.GetBoundRect() );
            pCircObj->AppendUserData( new IMapUserData( IMapObjectPtr(pObj) ) );
        }
        break;

        case OBJ_POLY:
        case OBJ_FREEFILL:
        case OBJ_PATHPOLY:
        case OBJ_PATHFILL:
        {
            SdrPathObj* pPathObj = const_cast<SdrPathObj*>( static_cast<const SdrPathObj*>(&rObj) );
            const basegfx::B2DPolyPolygon& rXPolyPoly = pPathObj->GetPathPoly();

            if ( rXPolyPoly.count() )
            {
                tools::Polygon aPoly(rXPolyPoly.getB2DPolygon(0));
                IMapPolygonObject* pObj = new IMapPolygonObject( aPoly, "", "", "", "", "", true, false );
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
    IMapUserData* pUserData = static_cast<IMapUserData*>( rObj.GetUserData( 0 ) );

    if ( pUserData )
    {
        OUString        aURL;
        OUString        aAltText;
        OUString        aDesc;
        OUString        aTarget;
        IMapObjectPtr   pIMapObj = pUserData->GetObject();
        bool        bActive = true;

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
            case OBJ_RECT:
            {
                pUserData->ReplaceObject( IMapObjectPtr(new IMapRectangleObject( static_cast<const SdrRectObj&>(rObj).GetLogicRect(),
                          aURL, aAltText, aDesc, aTarget, "", bActive, false ) ) );
            }
            break;

            case OBJ_CIRC:
            {
                const SdrCircObj& rCircObj = static_cast<const SdrCircObj&>(rObj);
                SdrPathObj* pPathObj = static_cast<SdrPathObj*>( rCircObj.ConvertToPolyObj( false, false ) );
                tools::Polygon aPoly(pPathObj->GetPathPoly().getB2DPolygon(0));

                IMapPolygonObject* pObj = new IMapPolygonObject( aPoly, aURL, aAltText, aDesc, aTarget, "", bActive, false );
                pObj->SetExtraEllipse( aPoly.GetBoundRect() );

                // was only created by us temporarily
                delete pPathObj;
                pUserData->ReplaceObject( IMapObjectPtr(pObj) );
            }
            break;

            case OBJ_POLY:
            case OBJ_FREEFILL:
            case OBJ_PATHPOLY:
            case OBJ_PATHFILL:
            {
                const SdrPathObj& rPathObj = static_cast<const SdrPathObj&>(rObj);
                const basegfx::B2DPolyPolygon& rXPolyPoly = rPathObj.GetPathPoly();

                if ( rXPolyPoly.count() )
                {
                    tools::Polygon aPoly(rPathObj.GetPathPoly().getB2DPolygon(0));
                    IMapPolygonObject*  pObj = new IMapPolygonObject( aPoly, aURL, aAltText, aDesc, aTarget, "", bActive, false );
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
    UpdateInfo( true );
}

void IMapWindow::MarkListHasChanged()
{
    GraphCtrl::MarkListHasChanged();
    UpdateInfo( false );
}

SdrObject* IMapWindow::GetHitSdrObj( const Point& rPosPixel ) const
{
    SdrObject*  pObj = nullptr;
    Point       aPt = PixelToLogic( rPosPixel );

    if ( tools::Rectangle( Point(), GetGraphicSize() ).IsInside( aPt ) )
    {
        SdrPage* pPage = pModel->GetPage( 0 );
        if ( pPage )
        {
            for ( size_t i = pPage->GetObjCount(); i > 0; )
            {
                --i;
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

IMapObject* IMapWindow::GetIMapObj( const SdrObject* pSdrObj )
{
    IMapObject* pIMapObj = nullptr;

    if ( pSdrObj )
    {
        IMapUserData* pUserData = static_cast<IMapUserData*>( pSdrObj->GetUserData( 0 ) );

        if ( pUserData )
            pIMapObj = pUserData->GetObject().get();
    }

    return pIMapObj;
}

void IMapWindow::Command(const CommandEvent& rCEvt)
{
    vcl::Region  aRegion;

    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        VclBuilder aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "svx/ui/imapmenu.ui", "");
        VclPtr<PopupMenu> aMenu(aBuilder.get_menu("menu"));
        const SdrMarkList&  rMarkList = pView->GetMarkedObjectList();
        const size_t nMarked = rMarkList.GetMarkCount();

        aMenu->EnableItem(aMenu->GetItemId("url"), false);
        aMenu->EnableItem(aMenu->GetItemId("active"), false);
        aMenu->EnableItem(aMenu->GetItemId("macro"), false);
        aMenu->EnableItem(aMenu->GetItemId("selectall"), pModel->GetPage(0)->GetObjCount() != pView->GetMarkedObjectCount());

        if ( !nMarked )
        {
            aMenu->EnableItem(aMenu->GetItemId("arrange"), false);
            aMenu->EnableItem(aMenu->GetItemId("delete"), false);
        }
        else
        {
            if ( nMarked == 1 )
            {
                SdrObject*  pSdrObj = GetSelectedSdrObject();

                aMenu->EnableItem(aMenu->GetItemId("url"));
                aMenu->EnableItem(aMenu->GetItemId("active"));
                aMenu->EnableItem(aMenu->GetItemId("macro"));
                aMenu->CheckItem(aMenu->GetItemId("active"), GetIMapObj(pSdrObj)->IsActive());
            }

            aMenu->EnableItem(aMenu->GetItemId("arrange"));
            aMenu->EnableItem(aMenu->GetItemId("delete"));
        }

        aMenu->SetSelectHdl( LINK( this, IMapWindow, MenuSelectHdl ) );
        aMenu->Execute( this, rCEvt.GetMousePosPixel() );
    }
    else
        Window::Command(rCEvt);
}

sal_Int8 IMapWindow::AcceptDrop( const AcceptDropEvent& rEvt )
{
    return( ( GetHitSdrObj( rEvt.maPosPixel ) != nullptr ) ? rEvt.mnAction : DND_ACTION_NONE );
}

sal_Int8 IMapWindow::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( IsDropFormatSupported( SotClipboardFormatId::NETSCAPE_BOOKMARK ) )
    {
        const OUString  aString;
        INetBookmark    aBookMark( aString, aString );
        SdrObject*      pSdrObj = GetHitSdrObj( rEvt.maPosPixel );

        if( pSdrObj && TransferableDataHelper( rEvt.maDropEvent.Transferable ).GetINetBookmark( SotClipboardFormatId::NETSCAPE_BOOKMARK, aBookMark ) )
        {
            IMapObject* pIMapObj = GetIMapObj( pSdrObj );

            pIMapObj->SetURL( aBookMark.GetURL() );
            pIMapObj->SetAltText( aBookMark.GetDescription() );
            pModel->SetChanged();
            pView->UnmarkAll();
            pView->MarkObj( pSdrObj, pView->GetSdrPageView() );
            UpdateInfo( true );
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
        SdrPageView* pPageView = nullptr;
        SdrObject* pSdrObj = pView->PickObj(aPos, pView->getHitTolLog(), pPageView);
        if (pSdrObj)
        {
            const IMapObject*   pIMapObj = GetIMapObj( pSdrObj );
            OUString            aStr;

            if ( pIMapObj && !( aStr = pIMapObj->GetURL() ).isEmpty() )
            {
                tools::Rectangle   aLogicPix( LogicToPixel( tools::Rectangle( Point(), GetGraphicSize() ) ) );
                tools::Rectangle   aScreenRect( OutputToScreenPixel( aLogicPix.TopLeft() ),
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

void IMapWindow::SetCurrentObjState( bool bActive )
{
    SdrObject* pObj = GetSelectedSdrObject();

    if ( pObj )
    {
        SfxItemSet aSet( pModel->GetItemPool() );

        GetIMapObj( pObj )->SetActive( bActive );

        aSet.Put( XFillColorItem( "", TRANSCOL ) );

        if ( !bActive )
        {
            aSet.Put( XFillTransparenceItem( 100 ) );
            aSet.Put( XLineColorItem( "", Color( COL_RED ) ) );
        }
        else
        {
            aSet.Put( XFillTransparenceItem( 50 ) );
            aSet.Put( XLineColorItem( "", Color( COL_BLACK ) ) );
        }

        pView->SetAttributes( aSet );
    }
}

void IMapWindow::UpdateInfo( bool bNewObj )
{
    if ( aInfoLink.IsSet() )
    {
        const SdrObject*    pSdrObj = GetSelectedSdrObject();
        const IMapObject*   pIMapObj = pSdrObj ? GetIMapObj( pSdrObj ) : nullptr;

        aInfo.bNewObj = bNewObj;

        if ( pIMapObj )
        {
            aInfo.bOneMarked = true;
            aInfo.aMarkURL = pIMapObj->GetURL();
            aInfo.aMarkAltText = pIMapObj->GetAltText();
            aInfo.aMarkTarget = pIMapObj->GetTarget();
            aInfo.bActivated = pIMapObj->IsActive();
            aInfoLink.Call( *this );
        }
        else
        {
            aInfo.aMarkURL.clear();
            aInfo.aMarkAltText.clear();
            aInfo.aMarkTarget.clear();
            aInfo.bOneMarked = false;
            aInfo.bActivated = false;
        }

        aInfoLink.Call( *this );
    }
}

void IMapWindow::DoMacroAssign()
{
    SdrObject*  pSdrObj = GetSelectedSdrObject();

    if ( pSdrObj )
    {
        SfxItemSet      aSet( *pIMapPool, svl::Items<SID_ATTR_MACROITEM, SID_ATTR_MACROITEM, SID_EVENTCONFIG, SID_EVENTCONFIG>{} );

        SfxEventNamesItem aNamesItem(SID_EVENTCONFIG);
        aNamesItem.AddEvent( "MouseOver", "", SvMacroItemId::OnMouseOver );
        aNamesItem.AddEvent( "MouseOut", "", SvMacroItemId::OnMouseOut );
        aSet.Put( aNamesItem );

        SvxMacroItem    aMacroItem(SID_ATTR_MACROITEM);
        IMapObject*     pIMapObj = GetIMapObj( pSdrObj );
        aMacroItem.SetMacroTable( pIMapObj->GetMacroTable() );
        aSet.Put( aMacroItem );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<SfxAbstractDialog> pMacroDlg(pFact->CreateSfxDialog( this, aSet, mxDocumentFrame, SID_EVENTCONFIG ));

        if ( pMacroDlg && pMacroDlg->Execute() == RET_OK )
        {
            const SfxItemSet* pOutSet = pMacroDlg->GetOutputItemSet();
            pIMapObj->SetMacroTable( static_cast<const SvxMacroItem& >(pOutSet->Get( SID_ATTR_MACROITEM )).GetMacroTable() );
            pModel->SetChanged();
            UpdateInfo( false );
        }
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
            ScopedVclPtr<AbstractURLDlg> aDlg(pFact->CreateURLDialog( this, pIMapObj->GetURL(), pIMapObj->GetAltText(), pIMapObj->GetDesc(),
                                            pIMapObj->GetTarget(), pIMapObj->GetName(), aTargetList ));
            DBG_ASSERT(aDlg, "Dialog creation failed!");
            if ( aDlg->Execute() == RET_OK )
            {
                const OUString aURLText( aDlg->GetURL() );

                if ( !aURLText.isEmpty() )
                {
                    INetURLObject aObj( aURLText, INetProtocol::File );
                    DBG_ASSERT( aObj.GetProtocol() != INetProtocol::NotValid, "Invalid URL" );
                    pIMapObj->SetURL( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
                }
                else
                    pIMapObj->SetURL( aURLText );

                pIMapObj->SetAltText( aDlg->GetAltText() );
                pIMapObj->SetDesc( aDlg->GetDesc() );
                pIMapObj->SetTarget( aDlg->GetTarget() );
                pIMapObj->SetName( aDlg->GetName() );
                pModel->SetChanged();
                UpdateInfo( true );
            }
        }
    }
}

IMPL_LINK( IMapWindow, MenuSelectHdl, Menu*, pMenu, bool )
{
    if (!pMenu)
        return false;

    OString sId = pMenu->GetCurItemIdent();

    if (sId == "url")
        DoPropertyDialog();
    else if (sId == "macro")
        DoMacroAssign();
    else if (sId == "active")
    {
        const sal_uInt16 nActiveId = pMenu->GetItemId(sId);
        const bool bNewState = !pMenu->IsItemChecked(nActiveId);
        pMenu->CheckItem(nActiveId, bNewState);
        SetCurrentObjState(bNewState);
        UpdateInfo( false );
    }
    else if (sId == "front")
        pView->PutMarkedToTop();
    else if (sId == "forward")
        pView->MovMarkedToTop();
    else if (sId == "backward")
        pView->MovMarkedToBtm();
    else if (sId == "back")
        pView->PutMarkedToBtm();
    else if (sId == "selectall")
        pView->MarkAll();
    else if (sId == "delete")
        pView->DeleteMarked();

    return false;
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
        tools::Rectangle aNewObjectRectangle(aPagePos, Size(nDefaultObjectSizeWidth, nDefaultObjectSizeHeight));

        SdrObject* pObj = SdrObjFactory::MakeNewObject( pView->GetCurrentObjInventor(), pView->GetCurrentObjIdentifier(), nullptr, pModel);
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
                static_cast<SdrPathObj*>(pObj)->SetPathPoly(basegfx::B2DPolyPolygon(aInnerPoly));
                break;
            }
        case OBJ_FREEFILL:
        case OBJ_PATHFILL:
            {
                sal_Int32 nWdt(aNewObjectRectangle.GetWidth() / 2);
                sal_Int32 nHgt(aNewObjectRectangle.GetHeight() / 2);
                basegfx::B2DPolygon aInnerPoly(XPolygon(aNewObjectRectangle.Center(), nWdt, nHgt).getB2DPolygon());
                static_cast<SdrPathObj*>(pObj)->SetPathPoly(basegfx::B2DPolyPolygon(aInnerPoly));
                break;
            }

        }

        pView->InsertObjectAtView(pObj, *pPageView);
        SdrObjCreated( *pObj );
        SetCurrentObjState( true );
        pView->MarkObj( pObj, pPageView );
    }
}

void IMapWindow::SelectFirstObject()
{
    SdrPage*    pPage = pModel->GetPage( 0 );
    if( pPage->GetObjCount() != 0 )
    {
        GrabFocus();
        pView->UnmarkAllObj();
        pView->MarkNextObj(true);
    }
}

void IMapWindow::StartPolyEdit()
{
    GrabFocus();

    if( !pView->AreObjectsMarked() )
        pView->MarkNextObj(true);

    const SdrHdlList& rHdlList = pView->GetHdlList();
    SdrHdl* pHdl = rHdlList.GetFocusHdl();

    if(!pHdl)
    {
        const_cast<SdrHdlList&>(rHdlList).TravelFocusHdl(true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
