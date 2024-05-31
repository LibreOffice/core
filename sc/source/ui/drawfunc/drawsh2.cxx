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

#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <editeng/eeitem.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xdef.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/ptitem.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdouno.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <svx/sidebar/SelectionChangeHandler.hxx>
#include <svx/sidebar/SelectionAnalyzer.hxx>
#include <svx/sidebar/ContextChangeEventMultiplexer.hxx>
#include <svx/unomid.hxx>

#include <drawsh.hxx>
#include <drawview.hxx>
#include <viewdata.hxx>
#include <sc.hrc>
#include <tabvwsh.hxx>
#include <document.hxx>
#include <drwlayer.hxx>
#include <drtxtob.hxx>
#include <gridwin.hxx>
#include <svx/svdoole2.hxx>
#include <svx/xflgrit.hxx>
#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <svx/xflclit.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <sfx2/ipclient.hxx>

using namespace com::sun::star;


ScDrawShell::ScDrawShell( ScViewData& rData ) :
    SfxShell(rData.GetViewShell()),
    rViewData( rData ),
    mpSelectionChangeHandler(new svx::sidebar::SelectionChangeHandler(
            [this] () { return this->GetSidebarContextName(); },
            GetFrame()->GetFrame().GetController(),
            vcl::EnumContext::Context::Cell))
{
    SetPool( &rViewData.GetScDrawView()->GetModel().GetItemPool() );
    SfxUndoManager* pMgr = rViewData.GetSfxDocShell()->GetUndoManager();
    SetUndoManager( pMgr );
    if ( !rViewData.GetDocument().IsUndoEnabled() )
    {
        pMgr->SetMaxUndoActionCount( 0 );
    }
    SetName(u"Drawing"_ustr);

    mpSelectionChangeHandler->Connect();
}

ScDrawShell::~ScDrawShell()
{
    mpSelectionChangeHandler->Disconnect();
}

void ScDrawShell::GetState( SfxItemSet& rSet )          // Conditions / Toggles
{
    ScDrawView* pView    = rViewData.GetScDrawView();
    SdrDragMode eMode    = pView->GetDragMode();

    rSet.Put( SfxBoolItem( SID_OBJECT_ROTATE, eMode == SdrDragMode::Rotate ) );
    rSet.Put( SfxBoolItem( SID_OBJECT_MIRROR, eMode == SdrDragMode::Mirror ) );
    rSet.Put( SfxBoolItem( SID_BEZIER_EDIT, !pView->IsFrameDragSingles() ) );

    sal_uInt16 nFWId = ScGetFontWorkId();
    SfxViewFrame& rViewFrm = rViewData.GetViewShell()->GetViewFrame();
    rSet.Put(SfxBoolItem(SID_FONTWORK, rViewFrm.HasChildWindow(nFWId)));

        // Notes always default to Page anchor.
    bool bDisableAnchor = false;
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if ( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
        if( ScDrawLayer::IsNoteCaption( pObj ) )
        {
            bDisableAnchor = true;
            rSet.DisableItem( SID_ANCHOR_PAGE );
            rSet.DisableItem( SID_ANCHOR_CELL );
            rSet.DisableItem( SID_ANCHOR_CELL_RESIZE );
        }
    }

    if ( bDisableAnchor )
        return;

    switch( pView->GetAnchorType() )
    {
    case SCA_PAGE:
        rSet.Put( SfxBoolItem( SID_ANCHOR_PAGE, true ) );
        rSet.Put( SfxBoolItem( SID_ANCHOR_CELL, false ) );
        rSet.Put( SfxBoolItem( SID_ANCHOR_CELL_RESIZE, false ) );
    break;

    case SCA_CELL:
        rSet.Put( SfxBoolItem( SID_ANCHOR_PAGE, false ) );
        rSet.Put( SfxBoolItem( SID_ANCHOR_CELL, true ) );
        rSet.Put( SfxBoolItem( SID_ANCHOR_CELL_RESIZE, false ) );
    break;

    case SCA_CELL_RESIZE:
        rSet.Put( SfxBoolItem( SID_ANCHOR_PAGE, false ) );
        rSet.Put( SfxBoolItem( SID_ANCHOR_CELL, false ) );
        rSet.Put( SfxBoolItem( SID_ANCHOR_CELL_RESIZE, true ) );
    break;

    default:
        rSet.Put( SfxBoolItem( SID_ANCHOR_PAGE, false ) );
        rSet.Put( SfxBoolItem( SID_ANCHOR_CELL, false ) );
        rSet.Put( SfxBoolItem( SID_ANCHOR_CELL_RESIZE, false ) );
    break;
    }
}

void ScDrawShell::GetDrawFuncState( SfxItemSet& rSet )      // disable functions
{
    ScDrawView* pView = rViewData.GetScDrawView();

    //  call IsMirrorAllowed first to make sure ForcePossibilities (and thus CheckMarked)
    //  is called before GetMarkCount, so the nMarkCount value is valid for the rest of this method.
    if (!pView->IsMirrorAllowed(true,true))
    {
        rSet.DisableItem( SID_MIRROR_HORIZONTAL );
        rSet.DisableItem( SID_MIRROR_VERTICAL );
        rSet.DisableItem( SID_FLIP_HORIZONTAL );
        rSet.DisableItem( SID_FLIP_VERTICAL );
    }


    if (GetObjectShell()->isContentExtractionLocked())
    {
        rSet.DisableItem(SID_COPY);
        rSet.DisableItem(SID_CUT);
    }

    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    const size_t nMarkCount = rMarkList.GetMarkCount();

    if ( nMarkCount <= 1 || !pView->IsGroupPossible() )
        rSet.DisableItem( SID_GROUP );
    if ( nMarkCount == 0 || !pView->IsUnGroupPossible() )
        rSet.DisableItem( SID_UNGROUP );
    if ( nMarkCount != 1 || !pView->IsGroupEnterPossible() )
        rSet.DisableItem( SID_ENTER_GROUP );
    if ( !pView->IsGroupEntered() )
        rSet.DisableItem( SID_LEAVE_GROUP );

    if ( nMarkCount <= 1 )                      // Nothing or only one object selected
    {
            //  alignment
        rSet.DisableItem( SID_OBJECT_ALIGN_LEFT );      // no alignment on the side
        rSet.DisableItem( SID_OBJECT_ALIGN_CENTER );
        rSet.DisableItem( SID_OBJECT_ALIGN_RIGHT );
        rSet.DisableItem( SID_OBJECT_ALIGN_UP );
        rSet.DisableItem( SID_OBJECT_ALIGN_MIDDLE );
        rSet.DisableItem( SID_OBJECT_ALIGN_DOWN );
        rSet.DisableItem( SID_OBJECT_ALIGN );

        // pseudo slots for Format menu
        rSet.DisableItem( SID_ALIGN_ANY_LEFT );
        rSet.DisableItem( SID_ALIGN_ANY_HCENTER );
        rSet.DisableItem( SID_ALIGN_ANY_RIGHT );
        rSet.DisableItem( SID_ALIGN_ANY_TOP );
        rSet.DisableItem( SID_ALIGN_ANY_VCENTER );
        rSet.DisableItem( SID_ALIGN_ANY_BOTTOM );
    }

    // do not change layer of form controls
    // #i83729# do not change layer of cell notes (on internal layer)
    if ( !nMarkCount || pView->HasMarkedControl() || pView->HasMarkedInternal() )
    {
        rSet.DisableItem( SID_OBJECT_HEAVEN );
        rSet.DisableItem( SID_OBJECT_HELL );
    }
    else
    {
        if(AreAllObjectsOnLayer(SC_LAYER_FRONT,rMarkList))
        {
            rSet.DisableItem( SID_OBJECT_HEAVEN );
        }
        else if(AreAllObjectsOnLayer(SC_LAYER_BACK,rMarkList))
        {
            rSet.DisableItem( SID_OBJECT_HELL );
        }
    }

    bool bCanRename = false;
    if ( nMarkCount > 1 )
    {
        // no hyperlink options for a selected group
        rSet.DisableItem( SID_EDIT_HYPERLINK );
        rSet.DisableItem( SID_REMOVE_HYPERLINK );
        rSet.DisableItem( SID_OPEN_HYPERLINK );
        rSet.DisableItem( SID_COPY_HYPERLINK_LOCATION );
        // Fit to cell only works with a single graphic
        rSet.DisableItem( SID_FITCELLSIZE );
    }
    else if ( nMarkCount == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
        if (pObj->getHyperlink().isEmpty())
        {
            rSet.DisableItem( SID_EDIT_HYPERLINK );
            rSet.DisableItem( SID_OPEN_HYPERLINK );
            rSet.DisableItem( SID_REMOVE_HYPERLINK );
            rSet.DisableItem( SID_COPY_HYPERLINK_LOCATION );
        }
        SdrLayerID nLayerID = pObj->GetLayer();
        if ( nLayerID != SC_LAYER_INTERN )
            bCanRename = true;                          // #i51351# anything except internal objects can be renamed

        // #91929#; don't show original size entry if not possible
        SdrObjKind nObjType = pObj->GetObjIdentifier();
        if ( nObjType == SdrObjKind::OLE2 )
        {
            SdrOle2Obj* pOleObj = static_cast<SdrOle2Obj*>(rMarkList.GetMark( 0 )->GetMarkedSdrObj());
            if (pOleObj->GetObjRef().is() &&
                (pOleObj->GetObjRef()->getStatus( pOleObj->GetAspect() ) & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE) )
                //TODO/LATER: why different slots in Draw and Calc?
                rSet.DisableItem(SID_ORIGINALSIZE);
        }
        else if ( nObjType == SdrObjKind::Caption )
        {
            if ( nLayerID == SC_LAYER_INTERN )
            {
                // SdrCaptionObj() Notes cannot be cut/copy in isolation from
                // their cells.
                rSet.DisableItem( SID_CUT );
                rSet.DisableItem( SID_COPY );
                // Notes always default to Page anchor.
                rSet.DisableItem( SID_ANCHOR_TOGGLE );
                rSet.DisableItem( SID_ANCHOR_MENU );
            }
        }

        // Fit to cell is only available for cell anchored graphics obviously
        if (pView->GetAnchorType() != SCA_CELL &&
            pView->GetAnchorType() != SCA_CELL_RESIZE)
            rSet.DisableItem( SID_FITCELLSIZE );

        // Support advanced DiagramHelper
        if (!pObj->isDiagram())
        {
            rSet.DisableItem( SID_REGENERATE_DIAGRAM );
            rSet.DisableItem( SID_EDIT_DIAGRAM );
        }
    }
    if ( !bCanRename )
    {
        // #i68101#
        rSet.DisableItem( SID_RENAME_OBJECT );
        rSet.DisableItem( SID_TITLE_DESCRIPTION_OBJECT );
    }

    if ( !nMarkCount )                          // nothing selected
    {
            //  Arrangement
        rSet.DisableItem( SID_FRAME_UP );
        rSet.DisableItem( SID_FRAME_DOWN );
        rSet.DisableItem( SID_FRAME_TO_TOP );
        rSet.DisableItem( SID_FRAME_TO_BOTTOM );
            //  Clipboard / delete
        rSet.DisableItem( SID_DELETE );
        rSet.DisableItem( SID_DELETE_CONTENTS );
        rSet.DisableItem( SID_CUT );
        rSet.DisableItem( SID_COPY );
            //  other
        rSet.DisableItem( SID_ANCHOR_TOGGLE );
        rSet.DisableItem( SID_ANCHOR_MENU );
        rSet.DisableItem( SID_ORIGINALSIZE );
        rSet.DisableItem( SID_FITCELLSIZE );
        rSet.DisableItem( SID_ATTR_TRANSFORM );
    }

    if ( rSet.GetItemState( SID_ENABLE_HYPHENATION ) != SfxItemState::UNKNOWN )
    {
        SfxItemSet aAttrs( pView->GetModel().GetItemPool() );
        pView->GetAttributes( aAttrs );
        if( aAttrs.GetItemState( EE_PARA_HYPHENATE ) >= SfxItemState::DEFAULT )
        {
            bool bValue = aAttrs.Get( EE_PARA_HYPHENATE ).GetValue();
            rSet.Put( SfxBoolItem( SID_ENABLE_HYPHENATION, bValue ) );
        }
    }

    svx::ExtrusionBar::getState( pView, rSet );
    svx::FontworkBar::getState( pView, rSet );
}

static void setupFillColorForChart(const SfxViewShell* pShell, SfxItemSet& rSet)
{
    if (!pShell)
        return;

    SfxInPlaceClient* pIPClient = pShell->GetIPClient();
    if (!pIPClient)
        return;

    const css::uno::Reference<::css::embed::XEmbeddedObject>& xEmbObj = pIPClient->GetObject();
    if( !xEmbObj.is() )
        return;

    ::css::uno::Reference<::css::chart2::XChartDocument> xChart( xEmbObj->getComponent(), uno::UNO_QUERY );
    if( !xChart.is() )
        return;

    css::uno::Reference<css::beans::XPropertySet> xPropSet = xChart->getPageBackground();
    if (!xPropSet.is())
        return;

    css::uno::Reference<css::beans::XPropertySetInfo> xInfo(xPropSet->getPropertySetInfo());
    if (!xInfo.is())
        return;

    if (xInfo->hasPropertyByName(u"FillColor"_ustr))
    {
        sal_uInt32 nFillColor = 0;
        xPropSet->getPropertyValue(u"FillColor"_ustr) >>= nFillColor;

        XFillColorItem aFillColorItem(u""_ustr, Color(ColorTransparency, nFillColor));
        rSet.Put(aFillColorItem);

        if (comphelper::LibreOfficeKit::isActive())
            pShell->libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED,
                    (".uno:FillColor=" + OString::number(nFillColor)));
    }

    if (!(comphelper::LibreOfficeKit::isActive() && xInfo->hasPropertyByName(u"FillGradientName"_ustr)))
        return;

    OUString aGradientName;
    xPropSet->getPropertyValue(u"FillGradientName"_ustr) >>= aGradientName;

    ::css::uno::Reference< ::css::frame::XController > xChartController = xChart->getCurrentController();
    if( !xChartController.is() )
        return;

    css::uno::Reference<css::lang::XMultiServiceFactory> xFact(xChartController->getModel(), css::uno::UNO_QUERY);

    if (!xFact.is())
        return;

    css::uno::Reference<css::container::XNameAccess> xNameAccess(
        xFact->createInstance(u"com.sun.star.drawing.GradientTable"_ustr), css::uno::UNO_QUERY);

    if (xNameAccess.is() && xNameAccess->hasByName(aGradientName))
    {
        css::uno::Any aAny = xNameAccess->getByName(aGradientName);

        XFillGradientItem aItem;
        aItem.SetName(aGradientName);
        aItem.PutValue(aAny, MID_FILLGRADIENT);

        rSet.Put(aItem);
    }
}

//          Attributes for Drawing-Objects

void ScDrawShell::GetDrawAttrState( SfxItemSet& rSet )
{
    Point       aMousePos   = rViewData.GetMousePosPixel();
    vcl::Window*     pWindow     = rViewData.GetActiveWin();
    ScDrawView* pDrView     = rViewData.GetScDrawView();
    Point       aPos        = pWindow->PixelToLogic(aMousePos);
    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
    bool        bHasMarked  = rMarkList.GetMarkCount() != 0;

    if( bHasMarked )
    {
        SfxAllItemSet aSet(pDrView->GetAttrFromMarked(false));
        if (const SfxPoolItem* pItem = nullptr;
            aSet.GetItemState(SDRATTR_TEXTCOLUMNS_NUMBER, false, &pItem) >= SfxItemState::DEFAULT
            && pItem)
        {
            aSet.Put(pItem->CloneSetWhich(SID_ATTR_TEXTCOLUMNS_NUMBER));
        }
        if (const SfxPoolItem* pItem = nullptr;
            aSet.GetItemState(SDRATTR_TEXTCOLUMNS_SPACING, false, &pItem) >= SfxItemState::DEFAULT
            && pItem)
        {
            aSet.Put(pItem->CloneSetWhich(SID_ATTR_TEXTCOLUMNS_SPACING));
        }
        rSet.Put(aSet, false);
    }
    else
    {
        pDrView->GetAttributes(rSet);
    }

    SdrPageView* pPV = pDrView->GetSdrPageView();
    if ( !pPV )
        return;

    // #i52073# when a sheet with an active OLE object is deleted,
    // the slot state is queried without an active page view

    //  Items for position and size (see ScGridWindow::UpdateStatusPosSize, #108137#)

    // #i34458# The SvxSizeItem in SID_TABLE_CELL is no longer needed by
    // SvxPosSizeStatusBarControl, it's enough to have it in SID_ATTR_SIZE.

    bool bActionItem = false;
    if ( pDrView->IsAction() )              // action rectangle
    {
        tools::Rectangle aRect;
        pDrView->TakeActionRect( aRect );
        if ( !aRect.IsEmpty() )
        {
            pPV->LogicToPagePos(aRect);
            rSet.Put( SfxPointItem( SID_ATTR_POSITION, aRect.TopLeft() ) );
            Size aSize( aRect.Right() - aRect.Left(), aRect.Bottom() - aRect.Top() );
            rSet.Put( SvxSizeItem( SID_ATTR_SIZE, aSize ) );
            bActionItem = true;
        }

        // Set correct colors for charts in sidebar
        setupFillColorForChart(pDrView->GetSfxViewShell(), rSet);
    }
    if ( bActionItem )
        return;

    if ( rMarkList.GetMarkCount() != 0 )      // selected objects
    {
        tools::Rectangle aRect = pDrView->GetAllMarkedRect();
        pPV->LogicToPagePos(aRect);
        rSet.Put( SfxPointItem( SID_ATTR_POSITION, aRect.TopLeft() ) );
        Size aSize( aRect.Right() - aRect.Left(), aRect.Bottom() - aRect.Top() );
        rSet.Put( SvxSizeItem( SID_ATTR_SIZE, aSize ) );
    }
    else                                // mouse position
    {
        // aPos is initialized above
        pPV->LogicToPagePos(aPos);
        rSet.Put( SfxPointItem( SID_ATTR_POSITION, aPos ) );
        rSet.Put( SvxSizeItem( SID_ATTR_SIZE, Size( 0, 0 ) ) );
    }
}

void ScDrawShell::GetAttrFuncState(SfxItemSet &rSet)
{
    //  Disable dialogs for Draw-attributes if necessary

    ScDrawView* pDrView = rViewData.GetScDrawView();
    SfxItemSet aViewSet = pDrView->GetAttrFromMarked(false);
    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
    const size_t nMarkCount = rMarkList.GetMarkCount();
    bool bShowArea = true, bShowMeasure = true;

    for ( size_t i = 0; i < nMarkCount && i < 50; ++i )
    {
        SdrObject* pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
        SdrObjKind nObjType = pObj->GetObjIdentifier();

        if ( nObjType != SdrObjKind::Measure )
            bShowMeasure = false;

        // If marked object is 2D, disable format area command.
        if ( nObjType == SdrObjKind::PolyLine     ||
             nObjType == SdrObjKind::Line     ||
             nObjType == SdrObjKind::PathLine ||
             nObjType == SdrObjKind::FreehandLine ||
             nObjType == SdrObjKind::Edge     ||
             nObjType == SdrObjKind::CircleArc     ||
             bShowMeasure )
            bShowArea = false;

        if ( !bShowArea && !bShowMeasure )
            break;
    }

    if ( !bShowArea )
        rSet.DisableItem( SID_ATTRIBUTES_AREA );

    if ( !bShowMeasure )
        rSet.DisableItem( SID_MEASURE_DLG );

    if ( aViewSet.GetItemState( XATTR_LINESTYLE ) == SfxItemState::DEFAULT )
    {
        rSet.DisableItem( SID_ATTRIBUTES_LINE );
        rSet.DisableItem( SID_ATTR_LINEEND_STYLE );     // Tbx-Controller
    }

    if ( aViewSet.GetItemState( XATTR_FILLSTYLE ) == SfxItemState::DEFAULT )
        rSet.DisableItem( SID_ATTRIBUTES_AREA );
}

bool ScDrawShell::AreAllObjectsOnLayer(SdrLayerID nLayerNo,const SdrMarkList& rMark)
{
    bool bResult=true;
    const size_t nCount = rMark.GetMarkCount();
    for (size_t i=0; i<nCount; ++i)
    {
        SdrObject* pObj = rMark.GetMark(i)->GetMarkedSdrObj();
        assert(pObj);
        if ( dynamic_cast<const SdrUnoObj*>( pObj) ==  nullptr )
        {
            if(nLayerNo!=pObj->GetLayer())
            {
                bResult=false;
                break;
            }
        }
    }
    return bResult;
}

void ScDrawShell::GetDrawAttrStateForIFBX( SfxItemSet& rSet )
{
    ScDrawView* pView = rViewData.GetScDrawView();
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

    if( rMarkList.GetMark(0) != nullptr )
    {
        SfxItemSet aNewAttr(pView->GetGeoAttrFromMarked());
        rSet.Put(aNewAttr, false);
    }
}

void ScDrawShell::Activate (const bool)
{
    ContextChangeEventMultiplexer::NotifyContextChange(
        GetFrame()->GetFrame().GetController(),
        vcl::EnumContext::GetContextEnum(
            GetSidebarContextName()));
}

const OUString & ScDrawShell::GetSidebarContextName()
{
    return vcl::EnumContext::GetContextName(
        svx::sidebar::SelectionAnalyzer::GetContextForSelection_SC(
            GetDrawView()->GetMarkedObjectList()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
