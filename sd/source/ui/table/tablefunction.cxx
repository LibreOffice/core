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

#include <sal/config.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/XSelectionFunction.hpp>

#include <comphelper/lok.hxx>

#include <svx/svdotable.hxx>
#include <svx/svxids.hrc>
#include <svx/svdpagv.hxx>
#include <svx/svxdlg.hxx>

#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <svl/style.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <tablefunction.hxx>
#include <DrawViewShell.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <Window.hxx>
#include <drawview.hxx>
#include <sdmod.hxx>

#include <memory>

using namespace ::sd;
using namespace sdr::table;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;

namespace sd
{

static void apply_table_style( SdrTableObj* pObj, SdrModel const * pModel, const OUString& sTableStyle )
{
    if( !(pModel && pObj) )
        return;

    Reference< XNameAccess > xPool(
        static_cast< cppu::OWeakObject* >( pModel->GetStyleSheetPool() ), css::uno::UNO_QUERY );
    if( !xPool.is() )
        return;

    try
    {
        Reference< XNameContainer > xTableFamily( xPool->getByName( u"table"_ustr ), UNO_QUERY_THROW );
        OUString aStdName( u"default"_ustr );
        if( !sTableStyle.isEmpty() )
            aStdName = sTableStyle;
        Reference< XIndexAccess > xStyle( xTableFamily->getByName( aStdName ), UNO_QUERY_THROW );
        pObj->setTableStyle( xStyle );
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::apply_default_table_style()");
    }
}

static void InsertTableImpl(const DrawViewShell* pShell,
                            ::sd::View* pView,
                            sal_Int32 nColumns,
                            sal_Int32 nRows,
                            const OUString& sTableStyle)
{
    ::tools::Rectangle aRect;

    SdrObject* pPickObj = pView->GetEmptyPresentationObject( PresObjKind::Table );
    if( pPickObj )
    {
        aRect = pPickObj->GetLogicRect();
        aRect.setHeight( 200 );
    }
    else
    {
        Size aSize(14100, 200);

        Point aPos;
        ::tools::Rectangle aWinRect(aPos, pShell->GetActiveWindow()->GetOutputSizePixel());
        aWinRect = pShell->GetActiveWindow()->PixelToLogic(aWinRect);

        // make sure that the default size of the table fits on the paper and is inside the viewing area.
        // if zoomed in close, don't make the table bigger than the viewing window.
        Size aMaxSize = pShell->getCurrentPage()->GetSize();

        if (comphelper::LibreOfficeKit::isActive())
        {
            // aWinRect is nonsensical in the LOK case
            aWinRect = ::tools::Rectangle(aPos, aMaxSize);
        }
        else
        {
            if( aMaxSize.Height() > aWinRect.getOpenHeight() )
                aMaxSize.setHeight( aWinRect.getOpenHeight() );
            if( aMaxSize.Width() > aWinRect.getOpenWidth() )
                aMaxSize.setWidth( aWinRect.getOpenWidth() );
        }

        if( aSize.Width() > aMaxSize.getWidth() )
            aSize.setWidth( aMaxSize.getWidth() );

        // adjust height based on # of rows.
        if( nRows > 0 )
        {
            aSize.setHeight( aSize.Height() * nRows );
            if( aSize.Height() > aMaxSize.getHeight() )
                aSize.setHeight( aMaxSize.getHeight() );
        }

        aPos = aWinRect.Center();
        aPos.AdjustX( -(aSize.Width() / 2) );
        aPos.AdjustY( -(aSize.Height() / 2) );
        aRect = ::tools::Rectangle(aPos, aSize);
    }

    rtl::Reference<sdr::table::SdrTableObj> pObj = new sdr::table::SdrTableObj(
        *pShell->GetDoc(), // TTTT should be reference
        aRect,
        nColumns,
        nRows);
    pObj->NbcSetStyleSheet( pShell->GetDoc()->GetDefaultStyleSheet(), true );
    apply_table_style( pObj.get(), pShell->GetDoc(), sTableStyle );
    SdrPageView* pPV = pView->GetSdrPageView();

    // #i123359# if an object is to be replaced/manipulated it may be that it is in text edit mode,
    // so to be on the safe side call SdrEndTextEdit here
    SdrTextObj* pCheckForTextEdit = DynCastSdrTextObj(pPickObj);

    if(pCheckForTextEdit && pCheckForTextEdit->IsInEditMode())
    {
        pView->SdrEndTextEdit();
    }

    // if we have a pick obj we need to make this new ole a pres obj replacing the current pick obj
    if( pPickObj )
    {
        SdPage* pPage = static_cast< SdPage* >(pPickObj->getSdrPageFromSdrObject());
        if(pPage && pPage->IsPresObj(pPickObj))
        {
            pObj->SetUserCall( pPickObj->GetUserCall() );
            pPage->InsertPresObj( pObj.get(), PresObjKind::Table );
        }
    }

    pShell->GetParentWindow()->GrabFocus();
    if( pPickObj )
        pView->ReplaceObjectAtView(pPickObj, *pPV, pObj.get() );
    else
        pView->InsertObjectAtView(pObj.get(), *pPV, SdrInsertFlags::SETDEFLAYER);
}

void DrawViewShell::FuTable(SfxRequest& rReq)
{
    switch( rReq.GetSlot() )
    {
    case SID_INSERT_TABLE:
    {
        sal_Int32 nColumns = 0;
        sal_Int32 nRows = 0;
        OUString sTableStyle;
        DrawViewShell* pShell = this;
        ::sd::View* pView = mpView;

        const SfxUInt16Item* pCols = rReq.GetArg<SfxUInt16Item>(SID_ATTR_TABLE_COLUMN);
        const SfxUInt16Item* pRows = rReq.GetArg<SfxUInt16Item>(SID_ATTR_TABLE_ROW);
        const SfxStringItem* pStyle = rReq.GetArg<SfxStringItem>(SID_TABLE_STYLE);

        if( pCols )
            nColumns = pCols->GetValue();

        if( pRows )
            nRows = pRows->GetValue();

        if( pStyle )
            sTableStyle = pStyle->GetValue();

        if( (nColumns == 0) || (nRows == 0) )
        {
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            std::shared_ptr<SvxAbstractNewTableDialog> pDlg( pFact->CreateSvxNewTableDialog(rReq.GetFrameWeld()) );

            weld::DialogController::runAsync(pDlg->getDialogController(),
                [pDlg, pShell, pView, sTableStyle] (sal_Int32 nResult) {
                    if (nResult == RET_OK)
                    {
                        sal_Int32 nColumnsIn = pDlg->getColumns();
                        sal_Int32 nRowsIn = pDlg->getRows();

                        InsertTableImpl(pShell, pView, nColumnsIn, nRowsIn, sTableStyle);
                    }
                });
        }
        else
        {
            InsertTableImpl(pShell, pView, nColumns, nRows, sTableStyle);
        }

        rReq.Ignore();
        SfxViewShell* pViewShell = GetViewShell();
        assert(pViewShell!=nullptr);
        SfxBindings& rBindings = pViewShell->GetViewFrame().GetBindings();
        rBindings.Invalidate( SID_INSERT_TABLE, true );
        break;
    }
    case SID_TABLEDESIGN:
    {
        // First make sure that the sidebar is visible
        if (SfxViewFrame* pViewFrame = GetViewFrame())
        {
            pViewFrame->ShowChildWindow(SID_SIDEBAR);
            ::sfx2::sidebar::Sidebar::TogglePanel(
                u"SdTableDesignPanel",
                pViewFrame->GetFrame().GetFrameInterface());

            Cancel();
            rReq.Done ();
        }
        break;
    }
    default:
        break;
    }
}

void DrawViewShell::GetTableMenuState( SfxItemSet &rSet )
{
    OUString aActiveLayer = mpDrawView->GetActiveLayer();
    SdrPageView* pPV = mpDrawView->GetSdrPageView();

    if(
        ( !aActiveLayer.isEmpty() && pPV && ( pPV->IsLayerLocked(aActiveLayer) ||
        !pPV->IsLayerVisible(aActiveLayer) ) ) ||
        SD_MOD()->GetWaterCan() )
    {
        rSet.DisableItem( SID_INSERT_TABLE );
    }
}

void CreateTableFromRTF( SvStream& rStream, SdDrawDocument* pModel )
{
    rStream.Seek( 0 );

    if( !pModel )
        return;

    SdrPage* pPage = pModel->GetPage(0);
    if( !pPage )
        return;

    Size aSize( 200, 200 );
    ::tools::Rectangle aRect (Point(), aSize);
    rtl::Reference<sdr::table::SdrTableObj> pObj = new sdr::table::SdrTableObj(
        *pModel,
        aRect,
        1,
        1);
    pObj->NbcSetStyleSheet( pModel->GetDefaultStyleSheet(), true );
    apply_table_style( pObj.get(), pModel, OUString() );

    pPage->NbcInsertObject( pObj.get() );

    sdr::table::ImportAsRTF( rStream, *pObj );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
