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
#include <vcl/commandevent.hxx>
#include <vcl/imaprect.hxx>
#include <vcl/imapcirc.hxx>
#include <vcl/imappoly.hxx>
#include <vcl/svapp.hxx>
#include <svl/urlbmk.hxx>

#include <svx/svxids.hrc>
#include "imapwnd.hxx"
#include <svx/svdpage.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdopath.hxx>
#include <svx/xfltrit.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnclit.hxx>

#include <sfx2/evntconf.hxx>

#include <sot/formats.hxx>

#include <svx/svxdlg.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <memory>

using namespace com::sun::star;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::uno::Reference;

#define TRANSCOL COL_WHITE

static ItemInfoPackage& getItemInfoPackageIMapWindow()
{
    class ItemInfoPackageIMapWindow : public ItemInfoPackage
    {
        typedef std::array<ItemInfoStatic, 1> ItemInfoArrayIMapWindow;
        ItemInfoArrayIMapWindow maItemInfos {{
            // m_nWhich, m_pItem, m_nSlotID, m_nItemInfoFlags
            { SID_ATTR_MACROITEM, new SvxMacroItem(SID_ATTR_MACROITEM), 0, SFX_ITEMINFOFLAG_NONE }
        }};

        virtual const ItemInfoStatic& getItemInfoStatic(size_t nIndex) const override { return maItemInfos[nIndex]; }

    public:
        virtual size_t size() const override { return maItemInfos.size(); }
        virtual const ItemInfo& getItemInfo(size_t nIndex, SfxItemPool& /*rPool*/) override { return maItemInfos[nIndex]; }
    };

    static std::unique_ptr<ItemInfoPackageIMapWindow> g_aItemInfoPackageIMapWindow;
    if (!g_aItemInfoPackageIMapWindow)
        g_aItemInfoPackageIMapWindow.reset(new ItemInfoPackageIMapWindow);
    return *g_aItemInfoPackageIMapWindow;
}

IMapWindow::IMapWindow(const Reference< XFrame >& rxDocumentFrame, weld::Dialog* pDialog)
    : GraphCtrl(pDialog)
    , mxDocumentFrame(rxDocumentFrame)
{
    pIMapPool = new SfxItemPool(u"IMapItemPool"_ustr);
    pIMapPool->registerItemInfoPackage(getItemInfoPackageIMapWindow());
}

IMapWindow::~IMapWindow()
{
}

void IMapWindow::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    weld::CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aSize(pDrawingArea->get_ref_device().LogicToPixel(Size(270, 170), MapMode(MapUnit::MapAppFont)));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    SetOutputSizePixel(aSize);

    SetSdrMode(true);

    mxDropTargetHelper.reset(new IMapDropTargetHelper(*this));
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
        // clear SdrObjects with broadcasting
        pPage->ClearSdrObjList();
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
        rtl::Reference<SdrObject> pNewObj = CreateObj( rImageMap.GetIMapObject( i - 1 ) );

        if (pNewObj && pPage)
        {
            pPage->InsertObject( pNewObj.get() );
        }
    }
}

void IMapWindow::ReplaceActualIMapInfo( const NotifyInfo& rNewInfo )
{
    const SdrObject*    pSdrObj = GetSelectedSdrObject();

    if ( pSdrObj )
    {
        IMapObject* pIMapObj = GetIMapObj( pSdrObj );
        if (pIMapObj)
        {
            pIMapObj->SetURL( rNewInfo.aMarkURL );
            pIMapObj->SetAltText( rNewInfo.aMarkAltText );
            pIMapObj->SetTarget( rNewInfo.aMarkTarget );
            pModel->SetChanged();
            UpdateInfo( false );
        }
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

void IMapWindow::SetTargetList( const TargetList& rTargetList )
{
    // Delete old List
    // Fill with the provided list
    aTargetList = rTargetList;

    pModel->SetChanged( false );
}

rtl::Reference<SdrObject> IMapWindow::CreateObj( const IMapObject* pIMapObj )
{
    tools::Rectangle   aClipRect( Point(), GetGraphicSize() );
    rtl::Reference<SdrObject> pSdrObj;
    IMapObjectPtr pCloneIMapObj;

    switch( pIMapObj->GetType() )
    {
        case IMapObjectType::Rectangle:
        {
            const IMapRectangleObject* pIMapRectObj = static_cast<const IMapRectangleObject*>(pIMapObj);
            tools::Rectangle               aDrawRect( pIMapRectObj->GetRectangle( false ) );

            // clipped on CanvasPane
            aDrawRect.Intersection( aClipRect );

            pSdrObj = new SdrRectObj(*pModel, aDrawRect);
            pCloneIMapObj.reset(static_cast<IMapObject*>(new IMapRectangleObject( *pIMapRectObj )));
        }
        break;

        case IMapObjectType::Circle:
        {
            const IMapCircleObject*   pIMapCircleObj = static_cast<const IMapCircleObject*>(pIMapObj);
            const Point         aCenter( pIMapCircleObj->GetCenter( false ) );
            const tools::Long          nRadius = pIMapCircleObj->GetRadius( false );
            const Point         aOffset( nRadius, nRadius );
            tools::Rectangle           aCircle( aCenter - aOffset, aCenter + aOffset );

            // limited to CanvasPane
            aCircle.Intersection( aClipRect );

            pSdrObj = new SdrCircObj(
                    *pModel,
                    SdrCircKind::Full,
                    aCircle,
                    0_deg100,
                    36000_deg100);
            pCloneIMapObj.reset(static_cast<IMapObject*>(new IMapCircleObject( *pIMapCircleObj )));
        }
        break;

        case IMapObjectType::Polygon:
        {
            const IMapPolygonObject*  pIMapPolyObj = static_cast<const IMapPolygonObject*>(pIMapObj);

            // If it actually is an ellipse, then another ellipse is created again
            if ( pIMapPolyObj->HasExtraEllipse() )
            {
                tools::Rectangle aDrawRect( pIMapPolyObj->GetExtraEllipse() );

                // clipped on CanvasPane
                aDrawRect.Intersection( aClipRect );

                pSdrObj = new SdrCircObj(
                        *pModel,
                        SdrCircKind::Full,
                        aDrawRect,
                        0_deg100,
                        36000_deg100);
            }
            else
            {
                const tools::Polygon aPoly = pIMapPolyObj->GetPolygon( false );
                tools::Polygon aDrawPoly( aPoly );

                // clipped on CanvasPane
                aDrawPoly.Clip( aClipRect );

                basegfx::B2DPolygon aPolygon;
                aPolygon.append(aDrawPoly.getB2DPolygon());
                pSdrObj = new SdrPathObj(
                        *pModel,
                        SdrObjKind::Polygon,
                        basegfx::B2DPolyPolygon(aPolygon));
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
        aSet.Put( XFillColorItem( u""_ustr, TRANSCOL ) );

        if ( !pIMapObj->IsActive() )
        {
            aSet.Put( XFillTransparenceItem( 100 ) );
            aSet.Put( XLineColorItem( u""_ustr, COL_RED ) );
        }
        else
        {
            aSet.Put( XFillTransparenceItem( 50 ) );
            aSet.Put( XLineColorItem( u""_ustr, COL_BLACK ) );
        }

        pSdrObj->SetMergedItemSetAndBroadcast(aSet);

        pSdrObj->AppendUserData( std::unique_ptr<SdrObjUserData>(new IMapUserData( std::move(pCloneIMapObj) )) );
        pSdrObj->SetUserCall( GetSdrUserCall() );
    }

    return pSdrObj;
}

void IMapWindow::InitSdrModel()
{
    GraphCtrl::InitSdrModel();

    SfxItemSet aSet( pModel->GetItemPool() );

    aSet.Put( XFillColorItem( u""_ustr, TRANSCOL ) );
    aSet.Put( XFillTransparenceItem( 50 ) );
    pView->SetAttributes( aSet );
    pView->SetFrameDragSingles();
}

void IMapWindow::SdrObjCreated( const SdrObject& rObj )
{
    switch( rObj.GetObjIdentifier() )
    {
        case SdrObjKind::Rectangle:
        {
            SdrRectObj*    pRectObj = const_cast<SdrRectObj*>(static_cast<const SdrRectObj*>(&rObj));
            auto pObj = std::make_shared<IMapRectangleObject>( pRectObj->GetLogicRect(), "", "", "", "", "", true, false );

            pRectObj->AppendUserData( std::unique_ptr<SdrObjUserData>(new IMapUserData( pObj )) );
        }
        break;

        case SdrObjKind::CircleOrEllipse:
        {
            SdrCircObj* pCircObj = const_cast<SdrCircObj*>( static_cast<const SdrCircObj*>(&rObj) );
            rtl::Reference<SdrPathObj> pPathObj = static_cast<SdrPathObj*>( pCircObj->ConvertToPolyObj( false, false ).get() );
            tools::Polygon aPoly(pPathObj->GetPathPoly().getB2DPolygon(0));

            pPathObj.clear();

            auto pObj = std::make_shared<IMapPolygonObject>( aPoly, "", "", "", "", "", true, false );
            pObj->SetExtraEllipse( aPoly.GetBoundRect() );
            pCircObj->AppendUserData( std::unique_ptr<SdrObjUserData>(new IMapUserData( pObj )) );
        }
        break;

        case SdrObjKind::Polygon:
        case SdrObjKind::FreehandFill:
        case SdrObjKind::PathPoly:
        case SdrObjKind::PathFill:
        {
            SdrPathObj* pPathObj = const_cast<SdrPathObj*>( static_cast<const SdrPathObj*>(&rObj) );
            const basegfx::B2DPolyPolygon& rXPolyPoly = pPathObj->GetPathPoly();

            if ( rXPolyPoly.count() )
            {
                tools::Polygon aPoly(rXPolyPoly.getB2DPolygon(0));
                auto pObj = std::make_shared<IMapPolygonObject>( aPoly, "", "", "", "", "", true, false );
                pPathObj->AppendUserData( std::unique_ptr<SdrObjUserData>(new IMapUserData( pObj )) );
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

    if ( !pUserData )
        return;

    OUString        aURL;
    OUString        aAltText;
    OUString        aDesc;
    OUString        aTarget;
    IMapObjectPtr   pIMapObj = pUserData->GetObject();
    bool        bActive = true;

    if ( pIMapObj )
    {
        aURL = pIMapObj->GetURL();
        aAltText = pIMapObj->GetAltText();
        aDesc = pIMapObj->GetDesc();
        aTarget = pIMapObj->GetTarget();
        bActive = pIMapObj->IsActive();
    }

    switch( rObj.GetObjIdentifier() )
    {
        case SdrObjKind::Rectangle:
        {
            pUserData->ReplaceObject( std::make_shared<IMapRectangleObject>( static_cast<const SdrRectObj&>(rObj).GetLogicRect(),
                      aURL, aAltText, aDesc, aTarget, "", bActive, false ) );
        }
        break;

        case SdrObjKind::CircleOrEllipse:
        {
            const SdrCircObj& rCircObj = static_cast<const SdrCircObj&>(rObj);
            rtl::Reference<SdrPathObj> pPathObj = static_cast<SdrPathObj*>( rCircObj.ConvertToPolyObj( false, false ).get() );
            tools::Polygon aPoly(pPathObj->GetPathPoly().getB2DPolygon(0));

            auto pObj = std::make_shared<IMapPolygonObject>( aPoly, aURL, aAltText, aDesc, aTarget, "", bActive, false );
            pObj->SetExtraEllipse( aPoly.GetBoundRect() );

            pPathObj.clear();

            pUserData->ReplaceObject( pObj );
        }
        break;

        case SdrObjKind::Polygon:
        case SdrObjKind::FreehandFill:
        case SdrObjKind::PathPoly:
        case SdrObjKind::PathFill:
        {
            const SdrPathObj& rPathObj = static_cast<const SdrPathObj&>(rObj);
            const basegfx::B2DPolyPolygon& rXPolyPoly = rPathObj.GetPathPoly();

            if ( rXPolyPoly.count() )
            {
                tools::Polygon aPoly(rPathObj.GetPathPoly().getB2DPolygon(0));
                auto pObj = std::make_shared<IMapPolygonObject>( aPoly, aURL, aAltText, aDesc, aTarget, "", bActive, false );
                pUserData->ReplaceObject( pObj );
            }
        }
        break;

        default:
        break;
    }
}

bool IMapWindow::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bRet = GraphCtrl::MouseButtonUp( rMEvt );
    UpdateInfo( true );
    return bRet;
}

void IMapWindow::MarkListHasChanged()
{
    GraphCtrl::MarkListHasChanged();
    UpdateInfo( false );
}

SdrObject* IMapWindow::GetHitSdrObj( const Point& rPosPixel ) const
{
    OutputDevice& rDevice = GetDrawingArea()->get_ref_device();

    SdrObject*  pObj = nullptr;
    Point       aPt = rDevice.PixelToLogic( rPosPixel );

    if ( tools::Rectangle( Point(), GetGraphicSize() ).Contains( aPt ) )
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

bool IMapWindow::Command(const CommandEvent& rCEvt)
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetDrawingArea(), u"svx/ui/imapmenu.ui"_ustr));
        mxPopupMenu = xBuilder->weld_menu(u"menu"_ustr);
        const SdrMarkList&  rMarkList = pView->GetMarkedObjectList();
        const size_t nMarked = rMarkList.GetMarkCount();

        mxPopupMenu->set_sensitive(u"url"_ustr, false);
        mxPopupMenu->set_sensitive(u"active"_ustr, false);
        mxPopupMenu->set_sensitive(u"macro"_ustr, false);
        mxPopupMenu->set_sensitive(u"selectall"_ustr, pModel->GetPage(0)->GetObjCount() != rMarkList.GetMarkCount());

        if ( !nMarked )
        {
            mxPopupMenu->set_sensitive(u"arrange"_ustr, false);
            mxPopupMenu->set_sensitive(u"delete"_ustr, false);
        }
        else
        {
            if ( nMarked == 1 )
            {
                SdrObject*  pSdrObj = GetSelectedSdrObject();

                mxPopupMenu->set_sensitive(u"url"_ustr, true);
                mxPopupMenu->set_sensitive(u"active"_ustr, true);
                mxPopupMenu->set_sensitive(u"macro"_ustr, true);
                mxPopupMenu->set_active(u"active"_ustr, GetIMapObj(pSdrObj)->IsActive());
            }

            mxPopupMenu->set_sensitive(u"arrange"_ustr, true);
            mxPopupMenu->set_sensitive(u"delete"_ustr, true);
        }

        MenuSelectHdl(mxPopupMenu->popup_at_rect(GetDrawingArea(), tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1))));

        mxPopupMenu.reset();

        return true;
    }
    return CustomWidgetController::Command(rCEvt);
}

IMapDropTargetHelper::IMapDropTargetHelper(IMapWindow& rImapWindow)
    : DropTargetHelper(rImapWindow.GetDrawingArea()->get_drop_target())
    , m_rImapWindow(rImapWindow)
{
}

sal_Int8 IMapDropTargetHelper::AcceptDrop( const AcceptDropEvent& rEvt )
{
    return m_rImapWindow.AcceptDrop(rEvt);
}

sal_Int8 IMapDropTargetHelper::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    return m_rImapWindow.ExecuteDrop(rEvt);
}

sal_Int8 IMapWindow::AcceptDrop( const AcceptDropEvent& rEvt )
{
    return( ( GetHitSdrObj( rEvt.maPosPixel ) != nullptr ) ? rEvt.mnAction : DND_ACTION_NONE );
}

sal_Int8 IMapWindow::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if (mxDropTargetHelper->IsDropFormatSupported(SotClipboardFormatId::NETSCAPE_BOOKMARK))
    {
        INetBookmark    aBookMark( u""_ustr, u""_ustr );
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

OUString IMapWindow::RequestHelp(tools::Rectangle& rHelpArea)
{
    OutputDevice& rDevice = GetDrawingArea()->get_ref_device();

    Point aPos = rDevice.PixelToLogic(rHelpArea.TopLeft());

    SdrPageView* pPageView = nullptr;
    SdrObject* pSdrObj = pView->PickObj(aPos, pView->getHitTolLog(), pPageView);
    if (pSdrObj)
    {
        const IMapObject*   pIMapObj = GetIMapObj( pSdrObj );
        if ( pIMapObj )
        {
            OUString aStr = pIMapObj->GetURL();
            if ( !aStr.isEmpty() )
            {
                rHelpArea = rDevice.LogicToPixel(tools::Rectangle( Point(), GetGraphicSize()));
                return aStr;
            }
        }
    }

    return OUString();
}

void IMapWindow::SetCurrentObjState( bool bActive )
{
    SdrObject* pObj = GetSelectedSdrObject();

    if ( !pObj )
        return;

    SfxItemSet aSet( pModel->GetItemPool() );

    GetIMapObj( pObj )->SetActive( bActive );

    aSet.Put( XFillColorItem( u""_ustr, TRANSCOL ) );

    if ( !bActive )
    {
        aSet.Put( XFillTransparenceItem( 100 ) );
        aSet.Put( XLineColorItem( u""_ustr, COL_RED ) );
    }
    else
    {
        aSet.Put( XFillTransparenceItem( 50 ) );
        aSet.Put( XLineColorItem( u""_ustr, COL_BLACK ) );
    }

    pView->SetAttributes( aSet );
}

void IMapWindow::UpdateInfo( bool bNewObj )
{
    if ( !aInfoLink.IsSet() )
        return;

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

void IMapWindow::DoMacroAssign()
{
    SdrObject*  pSdrObj = GetSelectedSdrObject();

    if ( !pSdrObj )
        return;

    auto xSet = std::make_unique<SfxItemSetFixed<SID_ATTR_MACROITEM, SID_ATTR_MACROITEM, SID_EVENTCONFIG, SID_EVENTCONFIG>>
        (*pIMapPool);

    SfxEventNamesItem aNamesItem(SID_EVENTCONFIG);
    aNamesItem.AddEvent( u"MouseOver"_ustr, u""_ustr, SvMacroItemId::OnMouseOver );
    aNamesItem.AddEvent( u"MouseOut"_ustr, u""_ustr, SvMacroItemId::OnMouseOut );
    xSet->Put( aNamesItem );

    SvxMacroItem    aMacroItem(SID_ATTR_MACROITEM);
    IMapObject*     pIMapObj = GetIMapObj( pSdrObj );
    aMacroItem.SetMacroTable( pIMapObj->GetMacroTable() );
    xSet->Put( aMacroItem );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    VclPtr<SfxAbstractDialog> pMacroDlg(pFact->CreateEventConfigDialog(GetDrawingArea(), std::move(xSet), mxDocumentFrame));

    pMacroDlg->StartExecuteAsync(
        [this, pMacroDlg, pIMapObj] (sal_Int32 nResult)->void
        {
            if (nResult == RET_OK)
            {
                const SfxItemSet* pOutSet = pMacroDlg->GetOutputItemSet();
                pIMapObj->SetMacroTable( pOutSet->Get( SID_ATTR_MACROITEM ).GetMacroTable() );
                pModel->SetChanged();
                UpdateInfo( false );
            }
            pMacroDlg->disposeOnce();
        }
    );
}

void IMapWindow::DoPropertyDialog()
{
    SdrObject*  pSdrObj = GetSelectedSdrObject();

    if ( !pSdrObj )
        return;

    IMapObject* pIMapObj = GetIMapObj( pSdrObj );
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractURLDlg> aDlg(pFact->CreateURLDialog(GetDrawingArea(), pIMapObj->GetURL(), pIMapObj->GetAltText(), pIMapObj->GetDesc(),
                                    pIMapObj->GetTarget(), pIMapObj->GetName(), aTargetList));
    if ( aDlg->Execute() != RET_OK )
        return;

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

void IMapWindow::MenuSelectHdl(const OUString& rId)
{
    if (rId == "url")
        DoPropertyDialog();
    else if (rId == "macro")
        DoMacroAssign();
    else if (rId == "active")
    {
        const bool bNewState = !mxPopupMenu->get_active(rId);
        SetCurrentObjState(bNewState);
        UpdateInfo( false );
    }
    else if (rId == "front")
        pView->PutMarkedToTop();
    else if (rId == "forward")
        pView->MovMarkedToTop();
    else if (rId == "backward")
        pView->MovMarkedToBtm();
    else if (rId == "back")
        pView->PutMarkedToBtm();
    else if (rId == "selectall")
        pView->MarkAll();
    else if (rId == "delete")
        pView->DeleteMarked();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
