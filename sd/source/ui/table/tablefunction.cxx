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
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>

#include <vcl/svapp.hxx>

#include <svx/svdotable.hxx>
#include <svx/sdr/overlay/overlayobjectcell.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/svxids.hrc>
#include <editeng/outlobj.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdetc.hxx>
#include <editeng/editstat.hxx>
#include <editeng/unolingu.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdr/table/tabledesign.hxx>
#include <svx/svxdlg.hxx>
#include <vcl/msgbox.hxx>

#include <svl/itempool.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <svl/style.hxx>

#include "framework/FrameworkHelper.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "DrawViewShell.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "Window.hxx"
#include "drawview.hxx"
#include "sdresid.hxx"
#include "undo/undoobjects.hxx"

using namespace ::sd;
using namespace ::sdr::table;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::linguistic2;

namespace sd
{
extern void showTableDesignDialog( ::Window*, ViewShellBase& );

static void apply_table_style( SdrTableObj* pObj, SdrModel* pModel, const OUString& sTableStyle )
{
    if( pModel && pObj )
    {
        Reference< XNameAccess > xPool( dynamic_cast< XNameAccess* >( pModel->GetStyleSheetPool() ) );
        if( xPool.is() ) try
        {
            const OUString sFamilyName("table" );
            Reference< XNameContainer > xTableFamily( xPool->getByName( sFamilyName ), UNO_QUERY_THROW );
            OUString aStdName( "default" );
            if( !sTableStyle.isEmpty() )
                aStdName = sTableStyle;
            Reference< XIndexAccess > xStyle( xTableFamily->getByName( aStdName ), UNO_QUERY_THROW );
            pObj->setTableStyle( xStyle );
        }
        catch( Exception& )
        {
            OSL_FAIL("sd::apply_default_table_style(), exception caught!");
        }
    }
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

        SFX_REQUEST_ARG( rReq, pCols, SfxUInt16Item, SID_ATTR_TABLE_COLUMN, sal_False );
        SFX_REQUEST_ARG( rReq, pRows, SfxUInt16Item, SID_ATTR_TABLE_ROW, sal_False );
        SFX_REQUEST_ARG( rReq, pStyle, SfxStringItem, SID_TABLE_STYLE, sal_False );

        if( pCols )
            nColumns = pCols->GetValue();

        if( pRows )
            nRows = pRows->GetValue();

        if( pStyle )
            sTableStyle = pStyle->GetValue();

        if( (nColumns == 0) || (nRows == 0) )
        {
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            ::std::auto_ptr<SvxAbstractNewTableDialog> pDlg( pFact ? pFact->CreateSvxNewTableDialog( NULL ) : 0);

            if( !pDlg.get() || (pDlg->Execute() != RET_OK) )
                break;

            nColumns = pDlg->getColumns();
            nRows = pDlg->getRows();
        }

        Rectangle aRect;

        SdrObject* pPickObj = mpView->GetEmptyPresentationObject( PRESOBJ_TABLE );
        if( pPickObj )
        {
            aRect = pPickObj->GetLogicRect();
            aRect.setHeight( 200 );
        }
        else
        {
            Size aSize( 14100, 200 );

            Point aPos;
            Rectangle aWinRect(aPos, GetActiveWindow()->GetOutputSizePixel() );
            aPos = aWinRect.Center();
            aPos = GetActiveWindow()->PixelToLogic(aPos);
            aPos.X() -= aSize.Width() / 2;
            aPos.Y() -= aSize.Height() / 2;
            aRect = Rectangle(aPos, aSize);
        }

        ::sdr::table::SdrTableObj* pObj = new ::sdr::table::SdrTableObj( GetDoc(), aRect, nColumns, nRows );
        pObj->NbcSetStyleSheet( GetDoc()->GetDefaultStyleSheet(), sal_True );
        apply_table_style( pObj, GetDoc(), sTableStyle );
        SdrPageView* pPV = mpView->GetSdrPageView();

        // if we have a pick obj we need to make this new ole a pres obj replacing the current pick obj
        if( pPickObj )
        {
            SdPage* pPage = static_cast< SdPage* >(pPickObj->GetPage());
            if(pPage && pPage->IsPresObj(pPickObj))
            {
                pObj->SetUserCall( pPickObj->GetUserCall() );
                pPage->InsertPresObj( pObj, PRESOBJ_TABLE );
            }
        }

        GetParentWindow()->GrabFocus();
        if( pPickObj )
            mpView->ReplaceObjectAtView(pPickObj, *pPV, pObj, sal_True );
        else
            mpView->InsertObjectAtView(pObj, *pPV, SDRINSERT_SETDEFLAYER);

        Invalidate(SID_DRAWTBX_INSERT);
        rReq.Ignore();
        SfxViewShell* pViewShell = GetViewShell();
        OSL_ASSERT (pViewShell!=NULL);
        SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
        rBindings.Invalidate( SID_INSERT_TABLE, sal_True, sal_False );
        break;
    }
    case SID_TABLEDESIGN:
    {
        if( GetDoc() && (GetDoc()->GetDocumentType() == DOCUMENT_TYPE_DRAW) )
        {
            // in draw open a modal dialog since we have no tool pane yet
            showTableDesignDialog( GetActiveWindow(), GetViewShellBase() );
        }
        else
        {
            // Make the table design panel visible (expand it) in the
            // sidebar.
            ::sfx2::sidebar::Sidebar::ShowPanel(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ImpressTableDesignPanel")),
                GetViewFrame()->GetFrame().GetFrameInterface());
        }

        Cancel();
        rReq.Done ();
    }
    default:
        break;
    }
}

// --------------------------------------------------------------------

void DrawViewShell::GetTableMenuState( SfxItemSet &rSet )
{
    bool bIsUIActive = GetDocSh()->IsUIActive();
    if( bIsUIActive )
    {
        rSet.DisableItem( SID_INSERT_TABLE );
    }
    else
    {
        OUString aActiveLayer = mpDrawView->GetActiveLayer();
        SdrPageView* pPV = mpDrawView->GetSdrPageView();

        if( bIsUIActive ||
            ( !aActiveLayer.isEmpty() && pPV && ( pPV->IsLayerLocked(aActiveLayer) ||
            !pPV->IsLayerVisible(aActiveLayer) ) ) ||
            SD_MOD()->GetWaterCan() )
        {
            rSet.DisableItem( SID_INSERT_TABLE );
        }
    }
}

// --------------------------------------------------------------------

void CreateTableFromRTF( SvStream& rStream, SdDrawDocument* pModel )
{
    rStream.Seek( 0 );

    if( pModel )
    {
        SdrPage* pPage = pModel->GetPage(0);
        if( pPage )
        {
            Size aSize( 200, 200 );
            Point aPos;
            Rectangle aRect (aPos, aSize);
            ::sdr::table::SdrTableObj* pObj = new ::sdr::table::SdrTableObj( pModel, aRect, 1, 1 );
            pObj->NbcSetStyleSheet( pModel->GetDefaultStyleSheet(), sal_True );
            OUString sTableStyle;
            apply_table_style( pObj, pModel, sTableStyle );

            pPage->NbcInsertObject( pObj );

            sdr::table::SdrTableObj::ImportAsRTF( rStream, *pObj );
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
