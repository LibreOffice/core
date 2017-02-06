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

#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/drawing/ShapeCollection.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/util/VetoException.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>

#include <editeng/outliner.hxx>
#include <svx/fmdpage.hxx>
#include <svx/svditer.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/unoshape.hxx>
#include <svx/fmshell.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/svapp.hxx>

#include "drawsh.hxx"
#include "drtxtob.hxx"
#include "transobj.hxx"
#include "editsh.hxx"
#include "viewuno.hxx"
#include "cellsuno.hxx"
#include "miscuno.hxx"
#include "tabvwsh.hxx"
#include "prevwsh.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"
#include "drawview.hxx"
#include "fupoor.hxx"
#include "sc.hrc"
#include "unonames.hxx"
#include "scmod.hxx"
#include "appoptio.hxx"
#include "gridwin.hxx"
#include "sheetevents.hxx"
#include "markdata.hxx"
#include "AccessibilityHints.hxx"
#include "scextopt.hxx"
#include "preview.hxx"
#include <svx/sdrhittesthelper.hxx>
#include "formatsh.hxx"
#include <sfx2/app.hxx>

using namespace com::sun::star;

//! Clipping-Markierungen

//  alles ohne Which-ID, Map nur fuer PropertySetInfo

static const SfxItemPropertyMapEntry* lcl_GetViewOptPropertyMap()
{
    static const SfxItemPropertyMapEntry aViewOptPropertyMap_Impl[] =
    {
        {OUString(OLD_UNO_COLROWHDR),   0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(SC_UNO_GRIDCOLOR),    0,  cppu::UnoType<sal_Int32>::get(),    0, 0},
        {OUString(SC_UNO_COLROWHDR),    0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(SC_UNO_HORSCROLL),    0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(SC_UNO_SHEETTABS),    0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(SC_UNO_VERTSCROLL),   0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(SC_UNO_HIDESPELL),    0,  cppu::UnoType<bool>::get(),          0, 0},  /* deprecated #i91949 */
        {OUString(OLD_UNO_HORSCROLL),   0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(SC_UNO_OUTLSYMB),     0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(SC_UNO_VALUEHIGH),    0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(OLD_UNO_OUTLSYMB),    0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(OLD_UNO_SHEETTABS),   0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(SC_UNO_SHOWANCHOR),   0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(SC_UNO_SHOWCHARTS),   0,  cppu::UnoType<sal_Int16>::get(),    0, 0},
        {OUString(SC_UNO_SHOWDRAW),     0,  cppu::UnoType<sal_Int16>::get(),    0, 0},
        {OUString(SC_UNO_SHOWFORM),     0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(SC_UNO_SHOWGRID),     0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(SC_UNO_SHOWHELP),     0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(SC_UNO_SHOWNOTES),    0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(SC_UNO_SHOWOBJ),      0,  cppu::UnoType<sal_Int16>::get(),    0, 0},
        {OUString(SC_UNO_SHOWPAGEBR),   0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(SC_UNO_SHOWZERO),     0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(OLD_UNO_VALUEHIGH),   0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(OLD_UNO_VERTSCROLL),  0,  cppu::UnoType<bool>::get(),          0, 0},
        {OUString(SC_UNO_VISAREA),      0,  cppu::UnoType<awt::Rectangle>::get(), 0, 0},
        {OUString(SC_UNO_ZOOMTYPE),     0,  cppu::UnoType<sal_Int16>::get(),    0, 0},
        {OUString(SC_UNO_ZOOMVALUE),    0,  cppu::UnoType<sal_Int16>::get(),    0, 0},
        {OUString(SC_UNO_VISAREASCREEN),0,  cppu::UnoType<awt::Rectangle>::get(), 0, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aViewOptPropertyMap_Impl;
}

#define SCTABVIEWOBJ_SERVICE        "com.sun.star.sheet.SpreadsheetView"
#define SCVIEWSETTINGS_SERVICE      "com.sun.star.sheet.SpreadsheetViewSettings"

SC_SIMPLE_SERVICE_INFO( ScViewPaneBase, "ScViewPaneObj", "com.sun.star.sheet.SpreadsheetViewPane" )

ScViewPaneBase::ScViewPaneBase(ScTabViewShell* pViewSh, sal_uInt16 nP) :
    pViewShell( pViewSh ),
    nPane( nP )
{
    if (pViewShell)
        StartListening(*pViewShell);
}

ScViewPaneBase::~ScViewPaneBase()
{
    SolarMutexGuard g;

    if (pViewShell)
        EndListening(*pViewShell);
}

void ScViewPaneBase::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.GetId() == SfxHintId::Dying )
        pViewShell = nullptr;
}

uno::Any SAL_CALL ScViewPaneBase::queryInterface( const uno::Type& rType )
{
    SC_QUERYINTERFACE( sheet::XViewPane )
    SC_QUERYINTERFACE( sheet::XCellRangeReferrer )
    SC_QUERYINTERFACE( view::XFormLayerAccess )
    SC_QUERYINTERFACE( view::XControlAccess )
    SC_QUERYINTERFACE( lang::XServiceInfo )
    SC_QUERYINTERFACE( lang::XTypeProvider )

    return uno::Any();          // OWeakObject is in derived objects
}

uno::Sequence<uno::Type> SAL_CALL ScViewPaneBase::getTypes()
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        aTypes.realloc(5);
        uno::Type* pPtr = aTypes.getArray();
        pPtr[0] = cppu::UnoType<sheet::XViewPane>::get();
        pPtr[1] = cppu::UnoType<sheet::XCellRangeReferrer>::get();
        pPtr[2] = cppu::UnoType<view::XFormLayerAccess>::get();
        pPtr[3] = cppu::UnoType<lang::XServiceInfo>::get();
        pPtr[4] = cppu::UnoType<lang::XTypeProvider>::get();
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScViewPaneBase::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XViewPane

sal_Int32 SAL_CALL ScViewPaneBase::getFirstVisibleColumn()
{
    SolarMutexGuard aGuard;
    if (pViewShell)
    {
        ScViewData& rViewData = pViewShell->GetViewData();
        ScSplitPos eWhich = ( nPane == SC_VIEWPANE_ACTIVE ) ?
                                rViewData.GetActivePart() :
                                (ScSplitPos) nPane;
        ScHSplitPos eWhichH = WhichH( eWhich );

        return rViewData.GetPosX( eWhichH );
    }
    OSL_FAIL("keine View ?!?"); //! Exception?
    return 0;
}

void SAL_CALL ScViewPaneBase::setFirstVisibleColumn(sal_Int32 nFirstVisibleColumn)
{
    SolarMutexGuard aGuard;
    if (pViewShell)
    {
        ScViewData& rViewData = pViewShell->GetViewData();
        ScSplitPos eWhich = ( nPane == SC_VIEWPANE_ACTIVE ) ?
                                rViewData.GetActivePart() :
                                (ScSplitPos) nPane;
        ScHSplitPos eWhichH = WhichH( eWhich );

        long nDeltaX = ((long)nFirstVisibleColumn) - rViewData.GetPosX( eWhichH );
        pViewShell->ScrollX( nDeltaX, eWhichH );
    }
}

sal_Int32 SAL_CALL ScViewPaneBase::getFirstVisibleRow()
{
    SolarMutexGuard aGuard;
    if (pViewShell)
    {
        ScViewData& rViewData = pViewShell->GetViewData();
        ScSplitPos eWhich = ( nPane == SC_VIEWPANE_ACTIVE ) ?
                                rViewData.GetActivePart() :
                                (ScSplitPos) nPane;
        ScVSplitPos eWhichV = WhichV( eWhich );

        return rViewData.GetPosY( eWhichV );
    }
    OSL_FAIL("keine View ?!?"); //! Exception?
    return 0;
}

void SAL_CALL ScViewPaneBase::setFirstVisibleRow( sal_Int32 nFirstVisibleRow )
{
    SolarMutexGuard aGuard;
    if (pViewShell)
    {
        ScViewData& rViewData = pViewShell->GetViewData();
        ScSplitPos eWhich = ( nPane == SC_VIEWPANE_ACTIVE ) ?
                                rViewData.GetActivePart() :
                                (ScSplitPos) nPane;
        ScVSplitPos eWhichV = WhichV( eWhich );

        long nDeltaY = ((long)nFirstVisibleRow) - rViewData.GetPosY( eWhichV );
        pViewShell->ScrollY( nDeltaY, eWhichV );
    }
}

table::CellRangeAddress SAL_CALL ScViewPaneBase::getVisibleRange()
{
    SolarMutexGuard aGuard;
    table::CellRangeAddress aAdr;
    if (pViewShell)
    {
        ScViewData& rViewData = pViewShell->GetViewData();
        ScSplitPos eWhich = ( nPane == SC_VIEWPANE_ACTIVE ) ?
                                rViewData.GetActivePart() :
                                (ScSplitPos) nPane;
        ScHSplitPos eWhichH = WhichH( eWhich );
        ScVSplitPos eWhichV = WhichV( eWhich );

        //  VisibleCellsX gibt nur komplett sichtbare Zellen,
        //  VisibleRange in Excel auch teilweise sichtbare.
        //! anpassen ???

        SCCOL nVisX = rViewData.VisibleCellsX( eWhichH );
        SCROW nVisY = rViewData.VisibleCellsY( eWhichV );
        if (!nVisX) nVisX = 1;  // irgendwas muss ja im Range sein
        if (!nVisY) nVisY = 1;
        aAdr.Sheet       = rViewData.GetTabNo();
        aAdr.StartColumn = rViewData.GetPosX( eWhichH );
        aAdr.StartRow    = rViewData.GetPosY( eWhichV );
        aAdr.EndColumn   = aAdr.StartColumn + nVisX - 1;
        aAdr.EndRow      = aAdr.StartRow    + nVisY - 1;
    }
    return aAdr;
}

// XCellRangeSource

uno::Reference<table::XCellRange> SAL_CALL ScViewPaneBase::getReferredCells()
{
    SolarMutexGuard aGuard;
    if (pViewShell)
    {
        ScDocShell* pDocSh = pViewShell->GetViewData().GetDocShell();

        table::CellRangeAddress aAdr(getVisibleRange());        //! Hilfsfunktion mit ScRange?
        ScRange aRange( (SCCOL)aAdr.StartColumn, (SCROW)aAdr.StartRow, aAdr.Sheet,
                        (SCCOL)aAdr.EndColumn, (SCROW)aAdr.EndRow, aAdr.Sheet );
        if ( aRange.aStart == aRange.aEnd )
            return new ScCellObj( pDocSh, aRange.aStart );
        else
            return new ScCellRangeObj( pDocSh, aRange );
    }

    return nullptr;
}

namespace
{
    bool lcl_prepareFormShellCall( ScTabViewShell* _pViewShell, sal_uInt16 _nPane, FmFormShell*& _rpFormShell, vcl::Window*& _rpWindow, SdrView*& _rpSdrView )
    {
        if ( !_pViewShell )
            return false;

        ScViewData& rViewData = _pViewShell->GetViewData();
        ScSplitPos eWhich = ( _nPane == SC_VIEWPANE_ACTIVE ) ?
                                rViewData.GetActivePart() :
                                (ScSplitPos) _nPane;
        _rpWindow = _pViewShell->GetWindowByPos( eWhich );
        _rpSdrView = _pViewShell->GetSdrView();
        _rpFormShell = _pViewShell->GetFormShell();
        return ( _rpFormShell != nullptr ) && ( _rpSdrView != nullptr )&& ( _rpWindow != nullptr );
    }
}

// XFormLayerAccess
uno::Reference< form::runtime::XFormController > SAL_CALL ScViewPaneBase::getFormController( const uno::Reference< form::XForm >& Form )
{
    SolarMutexGuard aGuard;

    uno::Reference< form::runtime::XFormController > xController;

    vcl::Window* pWindow( nullptr );
    SdrView* pSdrView( nullptr );
    FmFormShell* pFormShell( nullptr );
    if ( lcl_prepareFormShellCall( pViewShell, nPane, pFormShell, pWindow, pSdrView ) )
        xController = FmFormShell::GetFormController( Form, *pSdrView, *pWindow );

    return xController;
}

sal_Bool SAL_CALL ScViewPaneBase::isFormDesignMode(  )
{
    SolarMutexGuard aGuard;

    bool bIsFormDesignMode( true );

    FmFormShell* pFormShell( pViewShell ? pViewShell->GetFormShell() : nullptr );
    if ( pFormShell )
        bIsFormDesignMode = pFormShell->IsDesignMode();

    return bIsFormDesignMode;
}

void SAL_CALL ScViewPaneBase::setFormDesignMode( sal_Bool DesignMode )
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow( nullptr );
    SdrView* pSdrView( nullptr );
    FmFormShell* pFormShell( nullptr );
    if ( lcl_prepareFormShellCall( pViewShell, nPane, pFormShell, pWindow, pSdrView ) )
        pFormShell->SetDesignMode( DesignMode );
}

// XControlAccess

uno::Reference<awt::XControl> SAL_CALL ScViewPaneBase::getControl(
                            const uno::Reference<awt::XControlModel>& xModel )
{
    SolarMutexGuard aGuard;

    uno::Reference<awt::XControl> xRet;

    vcl::Window* pWindow( nullptr );
    SdrView* pSdrView( nullptr );
    FmFormShell* pFormShell( nullptr );
    if ( lcl_prepareFormShellCall( pViewShell, nPane, pFormShell, pWindow, pSdrView ) )
        pFormShell->GetFormControl( xModel, *pSdrView, *pWindow, xRet );

    if ( !xRet.is() )
        throw container::NoSuchElementException();      // no control found

    return xRet;
}

awt::Rectangle ScViewPaneBase::GetVisArea() const
{
    awt::Rectangle aVisArea;
    if (pViewShell)
    {
        ScSplitPos eWhich = ( nPane == SC_VIEWPANE_ACTIVE ) ?
                                pViewShell->GetViewData().GetActivePart() :
                                (ScSplitPos) nPane;
        ScGridWindow* pWindow = static_cast<ScGridWindow*>(pViewShell->GetWindowByPos(eWhich));
        ScDocument* pDoc = pViewShell->GetViewData().GetDocument();
        if (pWindow && pDoc)
        {
            ScHSplitPos eWhichH = ((eWhich == SC_SPLIT_TOPLEFT) || (eWhich == SC_SPLIT_BOTTOMLEFT)) ?
                                    SC_SPLIT_LEFT : SC_SPLIT_RIGHT;
            ScVSplitPos eWhichV = ((eWhich == SC_SPLIT_TOPLEFT) || (eWhich == SC_SPLIT_TOPRIGHT)) ?
                                    SC_SPLIT_TOP : SC_SPLIT_BOTTOM;
            ScAddress aCell(pViewShell->GetViewData().GetPosX(eWhichH),
                pViewShell->GetViewData().GetPosY(eWhichV),
                pViewShell->GetViewData().GetTabNo());
            Rectangle aCellRect( pDoc->GetMMRect( aCell.Col(), aCell.Row(), aCell.Col(), aCell.Row(), aCell.Tab() ) );
            Size aVisSize( pWindow->PixelToLogic( pWindow->GetSizePixel(), pWindow->GetDrawMapMode( true ) ) );
            Point aVisPos( aCellRect.TopLeft() );
            if ( pDoc->IsLayoutRTL( aCell.Tab() ) )
            {
                aVisPos = aCellRect.TopRight();
                aVisPos.X() -= aVisSize.Width();
            }
            Rectangle aVisRect( aVisPos, aVisSize );
            aVisArea = AWTRectangle(aVisRect);
        }
    }
    return aVisArea;
}

ScViewPaneObj::ScViewPaneObj(ScTabViewShell* pViewSh, sal_uInt16 nP) :
    ScViewPaneBase( pViewSh, nP )
{
}

ScViewPaneObj::~ScViewPaneObj()
{
}

uno::Any SAL_CALL ScViewPaneObj::queryInterface( const uno::Type& rType )
{
    //  ScViewPaneBase has everything except OWeakObject

    uno::Any aRet(ScViewPaneBase::queryInterface( rType ));
    if (!aRet.hasValue())
        aRet = OWeakObject::queryInterface( rType );
    return aRet;
}

void SAL_CALL ScViewPaneObj::acquire() throw()
{
    OWeakObject::acquire();
}

void SAL_CALL ScViewPaneObj::release() throw()
{
    OWeakObject::release();
}

//  Default-ctor wird fuer SMART_REFLECTION_IMPLEMENTATION gebraucht

ScTabViewObj::ScTabViewObj( ScTabViewShell* pViewSh ) :
    ScViewPaneBase( pViewSh, SC_VIEWPANE_ACTIVE ),
    SfxBaseController( pViewSh ),
    aPropSet( lcl_GetViewOptPropertyMap() ),
    aMouseClickHandlers( 0 ),
    aActivationListeners( 0 ),
    nPreviousTab( 0 ),
    bDrawSelModeSet(false),
    bFilteredRangeSelection(false),
    mbLeftMousePressed(false),
    mbPendingSelectionChanged(false)
{
    if (pViewSh)
        nPreviousTab = pViewSh->GetViewData().GetTabNo();
}

ScTabViewObj::~ScTabViewObj()
{
    //! Listening oder so
    if (!aMouseClickHandlers.empty())
    {
        acquire();
        EndMouseListening();
    }
    if (!aActivationListeners.empty())
    {
        acquire();
        EndActivationListening();
    }
}

uno::Any SAL_CALL ScTabViewObj::queryInterface( const uno::Type& rType )
{
    SC_QUERYINTERFACE( sheet::XSpreadsheetView )
    SC_QUERYINTERFACE( sheet::XEnhancedMouseClickBroadcaster )
    SC_QUERYINTERFACE( sheet::XActivationBroadcaster )
    SC_QUERYINTERFACE( container::XEnumerationAccess )
    SC_QUERYINTERFACE( container::XIndexAccess )
    SC_QUERY_MULTIPLE( container::XElementAccess, container::XIndexAccess )
    SC_QUERYINTERFACE( view::XSelectionSupplier )
    SC_QUERYINTERFACE( beans::XPropertySet )
    SC_QUERYINTERFACE( sheet::XViewSplitable )
    SC_QUERYINTERFACE( sheet::XViewFreezable )
    SC_QUERYINTERFACE( sheet::XRangeSelection )
    SC_QUERYINTERFACE( lang::XUnoTunnel )
    SC_QUERYINTERFACE( datatransfer::XTransferableSupplier )
    SC_QUERYINTERFACE( sheet::XSelectedSheetsSupplier )

    uno::Any aRet(ScViewPaneBase::queryInterface( rType ));
    if (!aRet.hasValue())
        aRet = SfxBaseController::queryInterface( rType );
    return aRet;
}

void SAL_CALL ScTabViewObj::acquire() throw()
{
    SfxBaseController::acquire();
}

void SAL_CALL ScTabViewObj::release() throw()
{
    SfxBaseController::release();
}

static void lcl_CallActivate( ScDocShell* pDocSh, SCTAB nTab, ScSheetEventId nEvent )
{
    ScDocument& rDoc = pDocSh->GetDocument();
    // when deleting a sheet, nPreviousTab can be invalid
    // (could be handled with reference updates)
    if (!rDoc.HasTable(nTab))
        return;

    const ScSheetEvents* pEvents = rDoc.GetSheetEvents(nTab);
    if (pEvents)
    {
        const OUString* pScript = pEvents->GetScript(nEvent);
        if (pScript)
        {
            uno::Any aRet;
            uno::Sequence<uno::Any> aParams;
            uno::Sequence<sal_Int16> aOutArgsIndex;
            uno::Sequence<uno::Any> aOutArgs;
            /*ErrCode eRet =*/ pDocSh->CallXScript( *pScript, aParams, aRet, aOutArgsIndex, aOutArgs );
        }
    }

    // execute VBA event handlers
    try
    {
        uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents( rDoc.GetVbaEventProcessor(), uno::UNO_SET_THROW );
        // the parameter is the clicked object, as in the mousePressed call above
        uno::Sequence< uno::Any > aArgs( 1 );
        aArgs[ 0 ] <<= nTab;
        xVbaEvents->processVbaEvent( ScSheetEvents::GetVbaSheetEventId( nEvent ), aArgs );
    }
    catch( uno::Exception& )
    {
    }
}

void ScTabViewObj::SheetChanged( bool bSameTabButMoved )
{
    if ( !GetViewShell() )
        return;

    ScViewData& rViewData = GetViewShell()->GetViewData();
    ScDocShell* pDocSh = rViewData.GetDocShell();
    if (!aActivationListeners.empty())
    {
        sheet::ActivationEvent aEvent;
        uno::Reference< sheet::XSpreadsheetView > xView(this);
        uno::Reference< uno::XInterface > xSource(xView, uno::UNO_QUERY);
        aEvent.Source = xSource;
        aEvent.ActiveSheet = new ScTableSheetObj(pDocSh, rViewData.GetTabNo());
        for (XActivationEventListenerVector::iterator it = aActivationListeners.begin(); it != aActivationListeners.end(); )
        {
            try
            {
                (*it)->activeSpreadsheetChanged( aEvent );
                ++it;
            }
            catch( uno::Exception& )
            {
                it = aActivationListeners.erase( it);
            }
        }
    }

    /*  Handle sheet events, but do not trigger event handlers, if the old
        active sheet gets re-activated after inserting/deleting/moving a sheet. */
    SCTAB nNewTab = rViewData.GetTabNo();
    if ( !bSameTabButMoved && (nNewTab != nPreviousTab) )
    {
        lcl_CallActivate( pDocSh, nPreviousTab, ScSheetEventId::UNFOCUS );
        lcl_CallActivate( pDocSh, nNewTab, ScSheetEventId::FOCUS );
    }
    nPreviousTab = nNewTab;
}

uno::Sequence<uno::Type> SAL_CALL ScTabViewObj::getTypes()
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aViewPaneTypes(ScViewPaneBase::getTypes());
        long nViewPaneLen = aViewPaneTypes.getLength();
        const uno::Type* pViewPanePtr = aViewPaneTypes.getConstArray();

        uno::Sequence<uno::Type> aControllerTypes(SfxBaseController::getTypes());
        long nControllerLen = aControllerTypes.getLength();
        const uno::Type* pControllerPtr = aControllerTypes.getConstArray();

        long nParentLen = nViewPaneLen + nControllerLen;

        aTypes.realloc( nParentLen + 12 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = cppu::UnoType<sheet::XSpreadsheetView>::get();
        pPtr[nParentLen + 1] = cppu::UnoType<container::XEnumerationAccess>::get();
        pPtr[nParentLen + 2] = cppu::UnoType<container::XIndexAccess>::get();
        pPtr[nParentLen + 3] = cppu::UnoType<view::XSelectionSupplier>::get();
        pPtr[nParentLen + 4] = cppu::UnoType<beans::XPropertySet>::get();
        pPtr[nParentLen + 5] = cppu::UnoType<sheet::XViewSplitable>::get();
        pPtr[nParentLen + 6] = cppu::UnoType<sheet::XViewFreezable>::get();
        pPtr[nParentLen + 7] = cppu::UnoType<sheet::XRangeSelection>::get();
        pPtr[nParentLen + 8] = cppu::UnoType<lang::XUnoTunnel>::get();
        pPtr[nParentLen + 9] = cppu::UnoType<sheet::XEnhancedMouseClickBroadcaster>::get();
        pPtr[nParentLen + 10] = cppu::UnoType<sheet::XActivationBroadcaster>::get();
        pPtr[nParentLen + 11] = cppu::UnoType<datatransfer::XTransferableSupplier>::get();

        long i;
        for (i=0; i<nViewPaneLen; i++)
            pPtr[i] = pViewPanePtr[i];              // parent types first
        for (i=0; i<nControllerLen; i++)
            pPtr[nViewPaneLen+i] = pControllerPtr[i];
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScTabViewObj::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XDocumentView

static bool lcl_TabInRanges( SCTAB nTab, const ScRangeList& rRanges )
{
    for (size_t i = 0, nCount = rRanges.size(); i < nCount; ++i)
    {
        const ScRange* pRange = rRanges[ i ];
        if ( nTab >= pRange->aStart.Tab() && nTab <= pRange->aEnd.Tab() )
            return true;
    }
    return false;
}

static void lcl_ShowObject( ScTabViewShell& rViewSh, ScDrawView& rDrawView, SdrObject* pSelObj )
{
    bool bFound = false;
    SCTAB nObjectTab = 0;

    SdrModel* pModel = rDrawView.GetModel();
    sal_uInt16 nPageCount = pModel->GetPageCount();
    for (sal_uInt16 i=0; i<nPageCount && !bFound; i++)
    {
        SdrPage* pPage = pModel->GetPage(i);
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, SdrIterMode::DeepWithGroups );
            SdrObject* pObject = aIter.Next();
            while (pObject && !bFound)
            {
                if ( pObject == pSelObj )
                {
                    bFound = true;
                    nObjectTab = static_cast<SCTAB>(i);
                }
                pObject = aIter.Next();
            }
        }
    }

    if (bFound)
    {
        rViewSh.SetTabNo( nObjectTab );
        rViewSh.ScrollToObject( pSelObj );
    }
}

sal_Bool SAL_CALL ScTabViewObj::select( const uno::Any& aSelection )
{
    SolarMutexGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();

    if ( !pViewSh )
        return false;

    //! Type of aSelection can be some specific interface instead of XInterface

    bool bRet = false;
    uno::Reference<uno::XInterface> xInterface(aSelection, uno::UNO_QUERY);
    if ( !xInterface.is() )  //clear all selections
    {
        ScDrawView* pDrawView = pViewSh->GetScDrawView();
        if (pDrawView)
        {
            pDrawView->ScEndTextEdit();
            pDrawView->UnmarkAll();
        }
        else //#102232#; if there is  no DrawView remove range selection
            pViewSh->Unmark();
        bRet = true;
    }

    if (bDrawSelModeSet) // remove DrawSelMode if set by API; if necessary it will be set again later
    {
        pViewSh->SetDrawSelMode(false);
        pViewSh->UpdateLayerLocks();
        bDrawSelModeSet = false;
    }

    if (bRet)
        return bRet;

    ScCellRangesBase* pRangesImp = ScCellRangesBase::getImplementation( xInterface );
    uno::Reference<drawing::XShapes> xShapeColl( xInterface, uno::UNO_QUERY );
    uno::Reference<drawing::XShape> xShapeSel( xInterface, uno::UNO_QUERY );
    SvxShape* pShapeImp = SvxShape::getImplementation( xShapeSel );

    if (pRangesImp)                                     // Zell-Ranges
    {
        ScViewData& rViewData = pViewSh->GetViewData();
        if ( rViewData.GetDocShell() == pRangesImp->GetDocShell() )
        {
            //  Zuerst evtl. Drawing-Selektion aufheben
            //  (MarkListHasChanged hebt Tabellen-Selektion auf)

            ScDrawView* pDrawView = pViewSh->GetScDrawView();
            if (pDrawView)
            {
                pDrawView->ScEndTextEdit();
                pDrawView->UnmarkAll();
            }
            FuPoor* pFunc = pViewSh->GetDrawFuncPtr();
            if ( pFunc && pFunc->GetSlotID() != SID_OBJECT_SELECT )
            {
                //  Slot der Zeichenfunktion nochmal ausfuehren -> abschalten
                SfxDispatcher* pDisp = pViewSh->GetDispatcher();
                if (pDisp)
                    pDisp->Execute( pFunc->GetSlotID(), SfxCallMode::SYNCHRON );
            }
            pViewSh->SetDrawShell(false);
            pViewSh->SetDrawSelMode(false); // nach dem Dispatcher-Execute

            //  Ranges selektieren

            const ScRangeList& rRanges = pRangesImp->GetRangeList();
            size_t nRangeCount = rRanges.size();
            // for empty range list, remove selection (cursor remains where it was)
            if ( nRangeCount == 0 )
                pViewSh->Unmark();
            else if ( nRangeCount == 1 )
                pViewSh->MarkRange( *rRanges[ 0 ] );
            else
            {
                //  Mehrfachselektion

                const ScRange* pFirst = rRanges[ 0 ];
                if ( pFirst && !lcl_TabInRanges( rViewData.GetTabNo(), rRanges ) )
                    pViewSh->SetTabNo( pFirst->aStart.Tab() );
                pViewSh->DoneBlockMode();
                pViewSh->InitOwnBlockMode();
                rViewData.GetMarkData().MarkFromRangeList( rRanges, true );
                pViewSh->MarkDataChanged();
                rViewData.GetDocShell()->PostPaintGridAll();   // Markierung (alt&neu)
                if ( pFirst )
                {
                    pViewSh->AlignToCursor( pFirst->aStart.Col(), pFirst->aStart.Row(),
                                                SC_FOLLOW_JUMP );
                    pViewSh->SetCursor( pFirst->aStart.Col(), pFirst->aStart.Row() );
                }

                //! Methode an der View, um RangeList zu selektieren
            }
            bRet = true;
        }
    }
    else if ( pShapeImp || xShapeColl.is() )            // Drawing-Layer
    {
        ScDrawView* pDrawView = pViewSh->GetScDrawView();
        if (pDrawView)
        {
            pDrawView->ScEndTextEdit();
            pDrawView->UnmarkAll();

            if (pShapeImp)      // einzelnes Shape
            {
                SdrObject *pObj = pShapeImp->GetSdrObject();
                if (pObj)
                {
                    lcl_ShowObject( *pViewSh, *pDrawView, pObj );
                    SdrPageView* pPV = pDrawView->GetSdrPageView();
                    if ( pPV && pObj->GetPage() == pPV->GetPage() )
                    {
                        pDrawView->MarkObj( pObj, pPV );
                        bRet = true;
                    }
                }
            }
            else                // Shape-Collection (xShapeColl ist nicht 0)
            {
                //  Es wird auf die Tabelle des ersten Objekts umgeschaltet,
                //  und alle Objekte selektiert, die auf dieser Tabelle liegen
                //! Exception, wenn Objekte auf verschiedenen Tabellen?

                long nCount = xShapeColl->getCount();
                if (nCount)
                {
                    SdrPageView* pPV = nullptr;
                    bool bAllMarked(true);
                    for ( long i = 0; i < nCount; i++ )
                    {
                        uno::Reference<drawing::XShape> xShapeInt(xShapeColl->getByIndex(i), uno::UNO_QUERY);
                        if (xShapeInt.is())
                        {
                            SvxShape* pShape = SvxShape::getImplementation( xShapeInt );
                            if (pShape)
                            {
                                SdrObject *pObj = pShape->GetSdrObject();
                                if (pObj)
                                {
                                    if (!bDrawSelModeSet && (pObj->GetLayer() == SC_LAYER_BACK))
                                    {
                                        pViewSh->SetDrawSelMode(true);
                                        pViewSh->UpdateLayerLocks();
                                        bDrawSelModeSet = true;
                                    }
                                    if (!pPV)               // erstes Objekt
                                    {
                                        lcl_ShowObject( *pViewSh, *pDrawView, pObj );
                                        pPV = pDrawView->GetSdrPageView();
                                    }
                                    if ( pPV && pObj->GetPage() == pPV->GetPage() )
                                    {
                                        if (pDrawView->IsObjMarkable( pObj, pPV ))
                                            pDrawView->MarkObj( pObj, pPV );
                                        else
                                            bAllMarked = false;
                                    }
                                }
                            }
                        }
                    }
                    if (bAllMarked)
                        bRet = true;
                }
                else
                    bRet = true; // empty XShapes (all shapes are deselected)
            }

            if (bRet)
                pViewSh->SetDrawShell(true);
        }
    }

    if (!bRet)
        throw lang::IllegalArgumentException();

    return bRet;
}

uno::Any SAL_CALL ScTabViewObj::getSelection()
{
    SolarMutexGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    ScCellRangesBase* pObj = nullptr;
    if (pViewSh)
    {
        //  Ist auf dem Drawing-Layer etwas selektiert?

        SdrView* pDrawView = pViewSh->GetSdrView();
        if (pDrawView)
        {
            const SdrMarkList& rMarkList = pDrawView->GetMarkedObjectList();
            const size_t nMarkCount = rMarkList.GetMarkCount();
            if (nMarkCount)
            {
                //  ShapeCollection erzeugen (wie in SdXImpressView::getSelection im Draw)
                //  Zurueckgegeben wird XInterfaceRef, das muss das UsrObject-XInterface sein

                uno::Reference< drawing::XShapes > xShapes = drawing::ShapeCollection::create(
                        comphelper::getProcessComponentContext());

                uno::Reference<uno::XInterface> xRet(xShapes);

                for (size_t i=0; i<nMarkCount; ++i)
                {
                    SdrObject* pDrawObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
                    if (pDrawObj)
                    {
                        uno::Reference<drawing::XShape> xShape( pDrawObj->getUnoShape(), uno::UNO_QUERY );
                        if (xShape.is())
                            xShapes->add(xShape);
                    }
                }
                return uno::makeAny(xRet);
            }
        }

        //  sonst Tabellen-(Zellen-)Selektion

        ScViewData& rViewData = pViewSh->GetViewData();
        ScDocShell* pDocSh = rViewData.GetDocShell();

        const ScMarkData& rMark = rViewData.GetMarkData();
        SCTAB nTabs = rMark.GetSelectCount();

        ScRange aRange;
        ScMarkType eMarkType = rViewData.GetSimpleArea(aRange);
        if ( nTabs == 1 && (eMarkType == SC_MARK_SIMPLE) )
        {
            if (aRange.aStart == aRange.aEnd)
                pObj = new ScCellObj( pDocSh, aRange.aStart );
            else
                pObj = new ScCellRangeObj( pDocSh, aRange );
        }
        else if ( nTabs == 1 && (eMarkType == SC_MARK_SIMPLE_FILTERED) )
        {
            ScMarkData aFilteredMark( rMark );
            ScViewUtil::UnmarkFiltered( aFilteredMark, &pDocSh->GetDocument());
            ScRangeList aRangeList;
            aFilteredMark.FillRangeListWithMarks( &aRangeList, false);
            // Theoretically a selection may start and end on a filtered row.
            switch ( aRangeList.size() )
            {
                case 0:
                    // No unfiltered row, we have to return some object, so
                    // here is one with no ranges.
                    pObj = new ScCellRangesObj( pDocSh, aRangeList );
                    break;
                case 1:
                    {
                        const ScRange& rRange = *(aRangeList[ 0 ]);
                        if (rRange.aStart == rRange.aEnd)
                            pObj = new ScCellObj( pDocSh, rRange.aStart );
                        else
                            pObj = new ScCellRangeObj( pDocSh, rRange );
                    }
                    break;
                default:
                    pObj = new ScCellRangesObj( pDocSh, aRangeList );
            }
        }
        else            //  Mehrfachselektion
        {
            ScRangeListRef xRanges;
            rViewData.GetMultiArea( xRanges );

            //  bei mehreren Tabellen Ranges kopieren
            //! sollte eigentlich schon in ScMarkData::FillRangeListWithMarks passieren?
            if ( nTabs > 1 )
                rMark.ExtendRangeListTables( xRanges.get() );

            pObj = new ScCellRangesObj( pDocSh, *xRanges );
        }

        if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
        {
            //  remember if the selection was from the cursor position without anything selected
            //  (used when rendering the selection)

            pObj->SetCursorOnly( true );
        }
    }

    return uno::makeAny(uno::Reference<uno::XInterface>(static_cast<cppu::OWeakObject*>(pObj)));
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScTabViewObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, "com.sun.star.sheet.SpreadsheetViewPanesEnumeration");
}

// XIndexAccess

sal_Int32 SAL_CALL ScTabViewObj::getCount()
{
    SolarMutexGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    sal_uInt16 nPanes = 0;
    if (pViewSh)
    {
        nPanes = 1;
        ScViewData& rViewData = pViewSh->GetViewData();
        if ( rViewData.GetHSplitMode() != SC_SPLIT_NONE )
            nPanes *= 2;
        if ( rViewData.GetVSplitMode() != SC_SPLIT_NONE )
            nPanes *= 2;
    }
    return nPanes;
}

uno::Any SAL_CALL ScTabViewObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XViewPane> xPane(GetObjectByIndex_Impl((sal_uInt16)nIndex));
    if (xPane.is())
        return uno::makeAny(xPane);
    else
        throw lang::IndexOutOfBoundsException();
//    return uno::Any();
}

uno::Type SAL_CALL ScTabViewObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<sheet::XViewPane>::get();
}

sal_Bool SAL_CALL ScTabViewObj::hasElements()
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

// XSpreadsheetView

ScViewPaneObj* ScTabViewObj::GetObjectByIndex_Impl(sal_uInt16 nIndex) const
{
    static const ScSplitPos ePosHV[4] =
        { SC_SPLIT_TOPLEFT, SC_SPLIT_BOTTOMLEFT, SC_SPLIT_TOPRIGHT, SC_SPLIT_BOTTOMRIGHT };

    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScSplitPos eWhich = SC_SPLIT_BOTTOMLEFT;    // default Position
        bool bError = false;
        ScViewData& rViewData = pViewSh->GetViewData();
        bool bHor = ( rViewData.GetHSplitMode() != SC_SPLIT_NONE );
        bool bVer = ( rViewData.GetVSplitMode() != SC_SPLIT_NONE );
        if ( bHor && bVer )
        {
            //  links oben, links unten, rechts oben, rechts unten - wie in Excel
            if ( nIndex < 4 )
                eWhich = ePosHV[nIndex];
            else
                bError = true;
        }
        else if ( bHor )
        {
            if ( nIndex > 1 )
                bError = true;
            else if ( nIndex == 1 )
                eWhich = SC_SPLIT_BOTTOMRIGHT;
            // sonst SC_SPLIT_BOTTOMLEFT
        }
        else if ( bVer )
        {
            if ( nIndex > 1 )
                bError = true;
            else if ( nIndex == 0 )
                eWhich = SC_SPLIT_TOPLEFT;
            // sonst SC_SPLIT_BOTTOMLEFT
        }
        else if ( nIndex > 0 )
            bError = true;          // nicht geteilt: nur 0 gueltig

        if (!bError)
            return new ScViewPaneObj( pViewSh, sal::static_int_cast<sal_uInt16>(eWhich) );
    }

    return nullptr;
}

uno::Reference<sheet::XSpreadsheet> SAL_CALL ScTabViewObj::getActiveSheet()
{
    SolarMutexGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScViewData& rViewData = pViewSh->GetViewData();
        SCTAB nTab = rViewData.GetTabNo();
        return new ScTableSheetObj( rViewData.GetDocShell(), nTab );
    }
    return nullptr;
}

// support expand (but not replace) the active sheet
void SAL_CALL ScTabViewObj::setActiveSheet( const uno::Reference<sheet::XSpreadsheet>& xActiveSheet )
{
    SolarMutexGuard aGuard;

    ScTabViewShell* pViewSh = GetViewShell();
    if ( pViewSh && xActiveSheet.is() )
    {
        //  XSpreadsheet und ScCellRangesBase -> muss ein Sheet sein

        ScCellRangesBase* pRangesImp = ScCellRangesBase::getImplementation( xActiveSheet );
        if ( pRangesImp && pViewSh->GetViewData().GetDocShell() == pRangesImp->GetDocShell() )
        {
            const ScRangeList& rRanges = pRangesImp->GetRangeList();
            if ( rRanges.size() == 1 )
            {
                SCTAB nNewTab = rRanges[ 0 ]->aStart.Tab();
                if ( pViewSh->GetViewData().GetDocument()->HasTable(nNewTab) )
                    pViewSh->SetTabNo( nNewTab );
            }
        }
    }
}

uno::Reference< uno::XInterface > ScTabViewObj::GetClickedObject(const Point& rPoint) const
{
    uno::Reference< uno::XInterface > xTarget;
    if (GetViewShell())
    {
        SCsCOL nX;
        SCsROW nY;
        ScViewData& rData = GetViewShell()->GetViewData();
        ScSplitPos eSplitMode = rData.GetActivePart();
        SCTAB nTab(rData.GetTabNo());
        rData.GetPosFromPixel( rPoint.X(), rPoint.Y(), eSplitMode, nX, nY);

        ScAddress aCellPos (nX, nY, nTab);
        ScCellObj* pCellObj = new ScCellObj(rData.GetDocShell(), aCellPos);

        xTarget.set(uno::Reference<table::XCell>(pCellObj), uno::UNO_QUERY);

        ScDocument* pDoc = rData.GetDocument();
        if (pDoc && pDoc->GetDrawLayer())
        {
            SdrPage* pDrawPage = nullptr;
            ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
            if (pDrawLayer->HasObjects() && (pDrawLayer->GetPageCount() > nTab))
                pDrawPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));

            SdrView* pDrawView = GetViewShell()->GetSdrView();

            if (pDrawPage && pDrawView && pDrawView->GetSdrPageView())
            {
                vcl::Window* pActiveWin = rData.GetActiveWin();
                Point aPos = pActiveWin->PixelToLogic(rPoint);

                sal_uInt16 nHitLog = (sal_uInt16) pActiveWin->PixelToLogic(
                                 Size(pDrawView->GetHitTolerancePixel(),0)).Width();

                const size_t nCount(pDrawPage->GetObjCount());
                bool bFound(false);
                for (size_t i = 0; i < nCount && !bFound; ++i)
                {
                    SdrObject* pObj = pDrawPage->GetObj(i);
                    if (pObj && SdrObjectPrimitiveHit(*pObj, aPos, nHitLog, *pDrawView->GetSdrPageView(), nullptr, false))
                    {
                        xTarget.set(pObj->getUnoShape(), uno::UNO_QUERY);
                        bFound = true;
                    }
                }
            }
        }
    }
    return xTarget;
}

bool ScTabViewObj::IsMouseListening() const
{
    if ( !aMouseClickHandlers.empty() )
        return true;

    // also include sheet events, because MousePressed must be called for them
    ScViewData& rViewData = GetViewShell()->GetViewData();
    ScDocument* pDoc = rViewData.GetDocument();
    SCTAB nTab = rViewData.GetTabNo();
    return
        pDoc->HasSheetEventScript( nTab, ScSheetEventId::RIGHTCLICK, true ) ||
        pDoc->HasSheetEventScript( nTab, ScSheetEventId::DOUBLECLICK, true ) ||
        pDoc->HasSheetEventScript( nTab, ScSheetEventId::SELECT, true );

}

bool ScTabViewObj::MousePressed( const awt::MouseEvent& e )
{
    bool bReturn(false);
    if ( e.Buttons == css::awt::MouseButton::LEFT )
        mbLeftMousePressed = true;

    uno::Reference< uno::XInterface > xTarget = GetClickedObject(Point(e.X, e.Y));
    if (!aMouseClickHandlers.empty() && xTarget.is())
    {
        awt::EnhancedMouseEvent aMouseEvent;

        aMouseEvent.Buttons = e.Buttons;
        aMouseEvent.X = e.X;
        aMouseEvent.Y = e.Y;
        aMouseEvent.ClickCount = e.ClickCount;
        aMouseEvent.PopupTrigger = e.PopupTrigger;
        aMouseEvent.Target = xTarget;
        aMouseEvent.Modifiers = e.Modifiers;

        for (XMouseClickHandlerVector::iterator it = aMouseClickHandlers.begin(); it != aMouseClickHandlers.end(); )
        {
            try
            {
                if (!(*it)->mousePressed( aMouseEvent ))
                    bReturn = true;
                ++it;
            }
            catch ( uno::Exception& )
            {
                it = aMouseClickHandlers.erase(it);
            }
        }
    }

    // handle sheet events
    bool bDoubleClick = ( e.Buttons == awt::MouseButton::LEFT && e.ClickCount == 2 );
    bool bRightClick = ( e.Buttons == awt::MouseButton::RIGHT && e.ClickCount == 1 );
    if ( ( bDoubleClick || bRightClick ) && !bReturn && xTarget.is())
    {
        ScSheetEventId nEvent = bDoubleClick ? ScSheetEventId::DOUBLECLICK : ScSheetEventId::RIGHTCLICK;

        ScTabViewShell* pViewSh = GetViewShell();
        ScViewData& rViewData = pViewSh->GetViewData();
        ScDocShell* pDocSh = rViewData.GetDocShell();
        ScDocument& rDoc = pDocSh->GetDocument();
        SCTAB nTab = rViewData.GetTabNo();
        const ScSheetEvents* pEvents = rDoc.GetSheetEvents(nTab);
        if (pEvents)
        {
            const OUString* pScript = pEvents->GetScript(nEvent);
            if (pScript)
            {
                // the macro parameter is the clicked object, as in the mousePressed call above
                uno::Sequence<uno::Any> aParams(1);
                aParams[0] <<= xTarget;

                uno::Any aRet;
                uno::Sequence<sal_Int16> aOutArgsIndex;
                uno::Sequence<uno::Any> aOutArgs;

                /*ErrCode eRet =*/ pDocSh->CallXScript( *pScript, aParams, aRet, aOutArgsIndex, aOutArgs );

                // look for a boolean return value of true
                bool bRetValue = false;
                if (aRet >>= bRetValue)
                {
                    if (bRetValue)
                        bReturn = true;
                }
            }
        }

        // execute VBA event handler
        if (!bReturn && xTarget.is()) try
        {
            uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents( rDoc.GetVbaEventProcessor(), uno::UNO_SET_THROW );
            // the parameter is the clicked object, as in the mousePressed call above
            uno::Sequence< uno::Any > aArgs( 1 );
            aArgs[ 0 ] <<= xTarget;
            xVbaEvents->processVbaEvent( ScSheetEvents::GetVbaSheetEventId( nEvent ), aArgs );
        }
        catch( util::VetoException& )
        {
            bReturn = true;
        }
        catch( uno::Exception& )
        {
        }
    }

    return bReturn;
}

bool ScTabViewObj::MouseReleased( const awt::MouseEvent& e )
{
    if ( e.Buttons == css::awt::MouseButton::LEFT )
    {
        try
        {
            mbPendingSelectionChanged = false;
            ScTabViewShell* pViewSh = GetViewShell();
            ScViewData& rViewData = pViewSh->GetViewData();
            ScDocShell* pDocSh = rViewData.GetDocShell();
            ScDocument& rDoc = pDocSh->GetDocument();
            uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents( rDoc.GetVbaEventProcessor(), uno::UNO_SET_THROW );
            uno::Sequence< uno::Any > aArgs( 1 );
            aArgs[ 0 ] = getSelection();
            xVbaEvents->processVbaEvent( ScSheetEvents::GetVbaSheetEventId( ScSheetEventId::SELECT ), aArgs );
        }
        catch( uno::Exception& )
        {
        }
        mbLeftMousePressed = false;
    }

    bool bReturn(false);

    if (!aMouseClickHandlers.empty())
    {
        uno::Reference< uno::XInterface > xTarget = GetClickedObject(Point(e.X, e.Y));

        if (xTarget.is())
        {
            awt::EnhancedMouseEvent aMouseEvent;

            aMouseEvent.Buttons = e.Buttons;
            aMouseEvent.X = e.X;
            aMouseEvent.Y = e.Y;
            aMouseEvent.ClickCount = e.ClickCount;
            aMouseEvent.PopupTrigger = e.PopupTrigger;
            aMouseEvent.Target = xTarget;
            aMouseEvent.Modifiers = e.Modifiers;

            for (XMouseClickHandlerVector::iterator it = aMouseClickHandlers.begin(); it != aMouseClickHandlers.end(); )
            {
                try
                {
                    if (!(*it)->mouseReleased( aMouseEvent ))
                        bReturn = true;
                    ++it;
                }
                catch ( uno::Exception& )
                {
                    it = aMouseClickHandlers.erase(it);
                }
            }
        }
    }
    return bReturn;
}

// XEnhancedMouseClickBroadcaster

void ScTabViewObj::EndMouseListening()
{
    lang::EventObject aEvent;
    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
    for (const auto& rListener : aMouseClickHandlers)
    {
        try
        {
            rListener->disposing(aEvent);
        }
        catch ( uno::Exception& )
        {
        }
    }
    aMouseClickHandlers.clear();
}

void ScTabViewObj::EndActivationListening()
{
    lang::EventObject aEvent;
    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
    for (const auto& rListener : aActivationListeners)
    {
        try
        {
            rListener->disposing(aEvent);
        }
        catch ( uno::Exception& )
        {
        }
    }
    aActivationListeners.clear();
}

void SAL_CALL ScTabViewObj::addEnhancedMouseClickHandler( const uno::Reference< awt::XEnhancedMouseClickHandler >& aListener )
{
    SolarMutexGuard aGuard;

    if (aListener.is())
    {
        aMouseClickHandlers.push_back( aListener );
    }
}

void SAL_CALL ScTabViewObj::removeEnhancedMouseClickHandler( const uno::Reference< awt::XEnhancedMouseClickHandler >& aListener )
{
    SolarMutexGuard aGuard;
    sal_uInt16 nCount = aMouseClickHandlers.size();
    for (XMouseClickHandlerVector::iterator it = aMouseClickHandlers.begin(); it != aMouseClickHandlers.end(); )
    {
        if ( *it == aListener )
            it = aMouseClickHandlers.erase(it);
        else
            ++it;
    }
    if (aMouseClickHandlers.empty() && (nCount > 0)) // only if last listener removed
        EndMouseListening();
}

// XActivationBroadcaster

void SAL_CALL ScTabViewObj::addActivationEventListener( const uno::Reference< sheet::XActivationEventListener >& aListener )
{
    SolarMutexGuard aGuard;

    if (aListener.is())
    {
        aActivationListeners.push_back( aListener );
    }
}

void SAL_CALL ScTabViewObj::removeActivationEventListener( const uno::Reference< sheet::XActivationEventListener >& aListener )
{
    SolarMutexGuard aGuard;
    sal_uInt16 nCount = aActivationListeners.size();
    for (XActivationEventListenerVector::iterator it = aActivationListeners.begin(); it != aActivationListeners.end(); )
    {
        if ( *it == aListener )
            it = aActivationListeners.erase(it);
        else
            ++it;
    }
    if (aActivationListeners.empty() && (nCount > 0)) // only if last listener removed
        EndActivationListening();
}

sal_Int16 ScTabViewObj::GetZoom() const
{
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        const Fraction& rZoomY = pViewSh->GetViewData().GetZoomY();    // Y wird angezeigt
        return (sal_Int16)(( rZoomY.GetNumerator() * 100 ) / rZoomY.GetDenominator());
    }
    return 0;
}

void ScTabViewObj::SetZoom(sal_Int16 nZoom)
{
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        if ( nZoom != GetZoom() && nZoom != 0 )
        {
            if (!pViewSh->GetViewData().IsPagebreakMode())
            {
                ScModule* pScMod = SC_MOD();
                ScAppOptions aNewOpt(pScMod->GetAppOptions());
                aNewOpt.SetZoom( nZoom );
                aNewOpt.SetZoomType( pViewSh->GetViewData().GetView()->GetZoomType() );
                pScMod->SetAppOptions( aNewOpt );
            }
        }
        Fraction aFract( nZoom, 100 );
        pViewSh->SetZoom( aFract, aFract, true );
        pViewSh->PaintGrid();
        pViewSh->PaintTop();
        pViewSh->PaintLeft();
        pViewSh->GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
        pViewSh->GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOMSLIDER );
    }
}

sal_Int16 ScTabViewObj::GetZoomType() const
{
    sal_Int16 aZoomType = view::DocumentZoomType::OPTIMAL;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        SvxZoomType eZoomType = pViewSh->GetViewData().GetView()->GetZoomType();
        switch (eZoomType)
        {
        case SvxZoomType::PERCENT:
            aZoomType = view::DocumentZoomType::BY_VALUE;
            break;
        case SvxZoomType::OPTIMAL:
            aZoomType = view::DocumentZoomType::OPTIMAL;
            break;
        case SvxZoomType::WHOLEPAGE:
            aZoomType = view::DocumentZoomType::ENTIRE_PAGE;
            break;
        case SvxZoomType::PAGEWIDTH:
            aZoomType = view::DocumentZoomType::PAGE_WIDTH;
            break;
        case SvxZoomType::PAGEWIDTH_NOBORDER:
            aZoomType = view::DocumentZoomType::PAGE_WIDTH_EXACT;
            break;
        }
    }
    return aZoomType;
}

void ScTabViewObj::SetZoomType(sal_Int16 aZoomType)
{
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScDBFunc* pView = pViewSh->GetViewData().GetView();
        if (pView)
        {
            SvxZoomType eZoomType;
            switch (aZoomType)
            {
            case view::DocumentZoomType::BY_VALUE:
                eZoomType = SvxZoomType::PERCENT;
                break;
            case view::DocumentZoomType::OPTIMAL:
                eZoomType = SvxZoomType::OPTIMAL;
                break;
            case view::DocumentZoomType::ENTIRE_PAGE:
                eZoomType = SvxZoomType::WHOLEPAGE;
                break;
            case view::DocumentZoomType::PAGE_WIDTH:
                eZoomType = SvxZoomType::PAGEWIDTH;
                break;
            case view::DocumentZoomType::PAGE_WIDTH_EXACT:
                eZoomType = SvxZoomType::PAGEWIDTH_NOBORDER;
                break;
            default:
                eZoomType = SvxZoomType::OPTIMAL;
            }
            sal_Int16 nZoom(GetZoom());
            sal_Int16 nOldZoom(nZoom);
            if ( eZoomType == SvxZoomType::PERCENT )
            {
                if ( nZoom < MINZOOM )  nZoom = MINZOOM;
                if ( nZoom > MAXZOOM )  nZoom = MAXZOOM;
            }
            else
                nZoom = pView->CalcZoom( eZoomType, nOldZoom );

            switch ( eZoomType )
            {
                case SvxZoomType::WHOLEPAGE:
                case SvxZoomType::PAGEWIDTH:
                    pView->SetZoomType( eZoomType, true );
                    break;

                default:
                    pView->SetZoomType( SvxZoomType::PERCENT, true );
            }
            SetZoom( nZoom );
        }
    }
}

sal_Bool SAL_CALL ScTabViewObj::getIsWindowSplit()
{
    SolarMutexGuard aGuard;
    //  wie Menue-Slot SID_WINDOW_SPLIT

    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScViewData& rViewData = pViewSh->GetViewData();
        return ( rViewData.GetHSplitMode() == SC_SPLIT_NORMAL ||
                 rViewData.GetVSplitMode() == SC_SPLIT_NORMAL );
    }

    return false;
}

sal_Bool SAL_CALL ScTabViewObj::hasFrozenPanes()
{
    SolarMutexGuard aGuard;
    //  wie Menue-Slot SID_WINDOW_FIX

    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScViewData& rViewData = pViewSh->GetViewData();
        return ( rViewData.GetHSplitMode() == SC_SPLIT_FIX ||
                 rViewData.GetVSplitMode() == SC_SPLIT_FIX );
    }

    return false;
}

sal_Int32 SAL_CALL ScTabViewObj::getSplitHorizontal()
{
    SolarMutexGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScViewData& rViewData = pViewSh->GetViewData();
        if ( rViewData.GetHSplitMode() != SC_SPLIT_NONE )
            return rViewData.GetHSplitPos();
    }
    return 0;
}

sal_Int32 SAL_CALL ScTabViewObj::getSplitVertical()
{
    SolarMutexGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScViewData& rViewData = pViewSh->GetViewData();
        if ( rViewData.GetVSplitMode() != SC_SPLIT_NONE )
            return rViewData.GetVSplitPos();
    }
    return 0;
}

sal_Int32 SAL_CALL ScTabViewObj::getSplitColumn()
{
    SolarMutexGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScViewData& rViewData = pViewSh->GetViewData();
        if ( rViewData.GetHSplitMode() != SC_SPLIT_NONE )
        {
            long nSplit = rViewData.GetHSplitPos();

            ScSplitPos ePos = SC_SPLIT_BOTTOMLEFT;
            if ( rViewData.GetVSplitMode() != SC_SPLIT_NONE )
                ePos = SC_SPLIT_TOPLEFT;

            SCsCOL nCol;
            SCsROW nRow;
            rViewData.GetPosFromPixel( nSplit, 0, ePos, nCol, nRow, false );
            if ( nCol > 0 )
                return nCol;
        }
    }
    return 0;
}

sal_Int32 SAL_CALL ScTabViewObj::getSplitRow()
{
    SolarMutexGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScViewData& rViewData = pViewSh->GetViewData();
        if ( rViewData.GetVSplitMode() != SC_SPLIT_NONE )
        {
            long nSplit = rViewData.GetVSplitPos();

            ScSplitPos ePos = SC_SPLIT_TOPLEFT;     // es ist vertikal geteilt
            SCsCOL nCol;
            SCsROW nRow;
            rViewData.GetPosFromPixel( 0, nSplit, ePos, nCol, nRow, false );
            if ( nRow > 0 )
                return nRow;
        }
    }
    return 0;
}

void SAL_CALL ScTabViewObj::splitAtPosition( sal_Int32 nPixelX, sal_Int32 nPixelY )
{
    SolarMutexGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        pViewSh->SplitAtPixel( Point( nPixelX, nPixelY ) );
        pViewSh->FreezeSplitters( false );
        pViewSh->InvalidateSplit();
    }
}

void SAL_CALL ScTabViewObj::freezeAtPosition( sal_Int32 nColumns, sal_Int32 nRows )
{
    SolarMutexGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        //  erst alles aufheben -> kein Stress mit Scrolling zwischendurch o.ae.

        pViewSh->RemoveSplit();

        Point aWinStart;
        vcl::Window* pWin = pViewSh->GetWindowByPos( SC_SPLIT_BOTTOMLEFT );
        if (pWin)
            aWinStart = pWin->GetPosPixel();

        ScViewData& rViewData = pViewSh->GetViewData();
        Point aSplit(rViewData.GetScrPos( (SCCOL)nColumns, (SCROW)nRows, SC_SPLIT_BOTTOMLEFT, true ));
        aSplit += aWinStart;

        pViewSh->SplitAtPixel( aSplit );
        pViewSh->FreezeSplitters( true );
        pViewSh->InvalidateSplit();
    }
}

void SAL_CALL ScTabViewObj::addSelectionChangeListener(
    const uno::Reference<view::XSelectionChangeListener>& xListener )
{
    SolarMutexGuard aGuard;
    aSelectionChgListeners.push_back( xListener );
}

void SAL_CALL ScTabViewObj::removeSelectionChangeListener(
                const uno::Reference< view::XSelectionChangeListener >& xListener )
{
    SolarMutexGuard aGuard;
    for (XSelectionChangeListenerVector::iterator it = aSelectionChgListeners.begin();
         it != aSelectionChgListeners.end(); ++it )
    {
        if ( *it == xListener ) //! wozu der Mumpitz mit queryInterface?
        {
            aSelectionChgListeners.erase(it);
            break;
        }
    }
}

void ScTabViewObj::SelectionChanged()
{
    // Selection changed so end any style preview
    // Note: executing this slot through the dispatcher
    // will cause the style dialog to be raised so we go
    // direct here
    ScFormatShell aShell( &GetViewShell()->GetViewData() );
    SfxAllItemSet reqList( SfxGetpApp()->GetPool() );
    SfxRequest aReq( SID_STYLE_END_PREVIEW, SfxCallMode::SLOT, reqList );
    aShell.ExecuteStyle( aReq );
    lang::EventObject aEvent;
    aEvent.Source.set(static_cast<cppu::OWeakObject*>(this));
    for (const auto& rListener : aSelectionChgListeners)
        rListener->selectionChanged( aEvent );

    // handle sheet events
    ScTabViewShell* pViewSh = GetViewShell();
    ScViewData& rViewData = pViewSh->GetViewData();
    ScDocShell* pDocSh = rViewData.GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTab = rViewData.GetTabNo();
    const ScSheetEvents* pEvents = rDoc.GetSheetEvents(nTab);
    if (pEvents)
    {
        const OUString* pScript = pEvents->GetScript(ScSheetEventId::SELECT);
        if (pScript)
        {
            // the macro parameter is the selection as returned by getSelection
            uno::Sequence<uno::Any> aParams(1);
            aParams[0] = getSelection();
            uno::Any aRet;
            uno::Sequence<sal_Int16> aOutArgsIndex;
            uno::Sequence<uno::Any> aOutArgs;
            /*ErrCode eRet =*/ pDocSh->CallXScript( *pScript, aParams, aRet, aOutArgsIndex, aOutArgs );
        }
    }
    if ( !mbLeftMousePressed ) // selection still in progress
    {
        mbPendingSelectionChanged = false;
        try
        {
            uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents( rDoc.GetVbaEventProcessor(), uno::UNO_SET_THROW );
            uno::Sequence< uno::Any > aArgs( 1 );
            aArgs[ 0 ] = getSelection();
            xVbaEvents->processVbaEvent( ScSheetEvents::GetVbaSheetEventId( ScSheetEventId::SELECT ), aArgs );
        }
        catch( uno::Exception& )
        {
        }
    }
    else
    {
        mbPendingSelectionChanged = true;
    }
}

//  XPropertySet (View-Optionen)
//! auch an der Applikation anbieten?

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTabViewObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScTabViewObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    if ( aPropertyName == SC_UNO_FILTERED_RANGE_SELECTION )
    {
        bFilteredRangeSelection = ScUnoHelpFunctions::GetBoolFromAny(aValue);
        return;
    }

    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScViewData& rViewData = pViewSh->GetViewData();
        const ScViewOptions& rOldOpt = pViewSh->GetViewData().GetOptions();
        ScViewOptions aNewOpt(rOldOpt);

        if ( aPropertyName == SC_UNO_COLROWHDR || aPropertyName == OLD_UNO_COLROWHDR )
            aNewOpt.SetOption( VOPT_HEADER, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aPropertyName == SC_UNO_HORSCROLL || aPropertyName == OLD_UNO_HORSCROLL )
            aNewOpt.SetOption( VOPT_HSCROLL, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aPropertyName == SC_UNO_OUTLSYMB || aPropertyName == OLD_UNO_OUTLSYMB )
            aNewOpt.SetOption( VOPT_OUTLINER, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aPropertyName == SC_UNO_SHEETTABS || aPropertyName == OLD_UNO_SHEETTABS )
            aNewOpt.SetOption( VOPT_TABCONTROLS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aPropertyName == SC_UNO_SHOWANCHOR )
            aNewOpt.SetOption( VOPT_ANCHOR, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aPropertyName == SC_UNO_SHOWFORM )
            aNewOpt.SetOption( VOPT_FORMULAS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aPropertyName == SC_UNO_SHOWGRID )
            aNewOpt.SetOption( VOPT_GRID, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aPropertyName == SC_UNO_SHOWHELP )
            aNewOpt.SetOption( VOPT_HELPLINES, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aPropertyName == SC_UNO_SHOWNOTES )
            aNewOpt.SetOption( VOPT_NOTES, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aPropertyName == SC_UNO_SHOWPAGEBR )
            aNewOpt.SetOption( VOPT_PAGEBREAKS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aPropertyName == SC_UNO_SHOWZERO )
            aNewOpt.SetOption( VOPT_NULLVALS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aPropertyName == SC_UNO_VALUEHIGH || aPropertyName == OLD_UNO_VALUEHIGH )
            aNewOpt.SetOption( VOPT_SYNTAX, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aPropertyName == SC_UNO_VERTSCROLL || aPropertyName == OLD_UNO_VERTSCROLL )
            aNewOpt.SetOption( VOPT_VSCROLL, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aPropertyName == SC_UNO_SHOWOBJ )
        {
            sal_Int16 nIntVal = 0;
            if ( aValue >>= nIntVal )
            {
                //#i80528# adapt to new range eventually
                if((sal_Int16)VOBJ_MODE_HIDE < nIntVal) nIntVal = (sal_Int16)VOBJ_MODE_SHOW;

                aNewOpt.SetObjMode( VOBJ_TYPE_OLE, (ScVObjMode)nIntVal);
            }
        }
        else if ( aPropertyName == SC_UNO_SHOWCHARTS )
        {
            sal_Int16 nIntVal = 0;
            if ( aValue >>= nIntVal )
            {
                //#i80528# adapt to new range eventually
                if((sal_Int16)VOBJ_MODE_HIDE < nIntVal) nIntVal = (sal_Int16)VOBJ_MODE_SHOW;

                aNewOpt.SetObjMode( VOBJ_TYPE_CHART, (ScVObjMode)nIntVal);
            }
        }
        else if ( aPropertyName == SC_UNO_SHOWDRAW )
        {
            sal_Int16 nIntVal = 0;
            if ( aValue >>= nIntVal )
            {
                //#i80528# adapt to new range eventually
                if((sal_Int16)VOBJ_MODE_HIDE < nIntVal) nIntVal = (sal_Int16)VOBJ_MODE_SHOW;

                aNewOpt.SetObjMode( VOBJ_TYPE_DRAW, (ScVObjMode)nIntVal);
            }
        }
        else if ( aPropertyName == SC_UNO_GRIDCOLOR )
        {
            sal_Int32 nIntVal = 0;
            if ( aValue >>= nIntVal )
                aNewOpt.SetGridColor( nIntVal, OUString() );
        }
        else if ( aPropertyName == SC_UNO_ZOOMTYPE )
        {
            sal_Int16 nIntVal = 0;
            if ( aValue >>= nIntVal )
                SetZoomType(nIntVal);
        }
        else if ( aPropertyName == SC_UNO_ZOOMVALUE )
        {
            sal_Int16 nIntVal = 0;
            if ( aValue >>= nIntVal )
                SetZoom(nIntVal);
        }

        //  Optionen werden an der View und am Dokument (fuer neue Views) gesetzt,
        //  damit sie beim Speichern erhalten bleiben.
        //! An der App (Module) braeuchte man noch eine Extra-Moeglichkeit,
        //! das einzustellen (fuer neue Dokumente)

        if ( aNewOpt != rOldOpt )
        {
            rViewData.SetOptions( aNewOpt );
            rViewData.GetDocument()->SetViewOptions( aNewOpt );
            rViewData.GetDocShell()->SetDocumentModified();    //! wirklich?

            pViewSh->UpdateFixPos();
            pViewSh->PaintGrid();
            pViewSh->PaintTop();
            pViewSh->PaintLeft();
            pViewSh->PaintExtras();
            pViewSh->InvalidateBorder();

            SfxBindings& rBindings = pViewSh->GetViewFrame()->GetBindings();
            rBindings.Invalidate( FID_TOGGLEHEADERS ); // -> Checks im Menue
            rBindings.Invalidate( FID_TOGGLESYNTAX );
        }
    }
}

uno::Any SAL_CALL ScTabViewObj::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;

    if ( aPropertyName == SC_UNO_FILTERED_RANGE_SELECTION )
    {
        aRet <<= bFilteredRangeSelection;
        return aRet;
    }

    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        const ScViewOptions& rOpt = pViewSh->GetViewData().GetOptions();

        if ( aPropertyName == SC_UNO_COLROWHDR || aPropertyName == OLD_UNO_COLROWHDR )
            aRet <<= rOpt.GetOption( VOPT_HEADER );
        else if ( aPropertyName == SC_UNO_HORSCROLL || aPropertyName == OLD_UNO_HORSCROLL )
            aRet <<= rOpt.GetOption( VOPT_HSCROLL );
        else if ( aPropertyName == SC_UNO_OUTLSYMB || aPropertyName == OLD_UNO_OUTLSYMB )
            aRet <<= rOpt.GetOption( VOPT_OUTLINER );
        else if ( aPropertyName == SC_UNO_SHEETTABS || aPropertyName == OLD_UNO_SHEETTABS )
            aRet <<= rOpt.GetOption( VOPT_TABCONTROLS );
        else if ( aPropertyName == SC_UNO_SHOWANCHOR ) aRet <<= rOpt.GetOption( VOPT_ANCHOR );
        else if ( aPropertyName == SC_UNO_SHOWFORM )   aRet <<= rOpt.GetOption( VOPT_FORMULAS );
        else if ( aPropertyName == SC_UNO_SHOWGRID )   aRet <<= rOpt.GetOption( VOPT_GRID );
        else if ( aPropertyName == SC_UNO_SHOWHELP )   aRet <<= rOpt.GetOption( VOPT_HELPLINES );
        else if ( aPropertyName == SC_UNO_SHOWNOTES )  aRet <<= rOpt.GetOption( VOPT_NOTES );
        else if ( aPropertyName == SC_UNO_SHOWPAGEBR ) aRet <<= rOpt.GetOption( VOPT_PAGEBREAKS );
        else if ( aPropertyName == SC_UNO_SHOWZERO )   aRet <<= rOpt.GetOption( VOPT_NULLVALS );
        else if ( aPropertyName == SC_UNO_VALUEHIGH || aPropertyName == OLD_UNO_VALUEHIGH )
            aRet <<= rOpt.GetOption( VOPT_SYNTAX );
        else if ( aPropertyName == SC_UNO_VERTSCROLL || aPropertyName == OLD_UNO_VERTSCROLL )
            aRet <<= rOpt.GetOption( VOPT_VSCROLL );
        else if ( aPropertyName == SC_UNO_SHOWOBJ )    aRet <<= (sal_Int16)( rOpt.GetObjMode( VOBJ_TYPE_OLE ) );
        else if ( aPropertyName == SC_UNO_SHOWCHARTS ) aRet <<= (sal_Int16)( rOpt.GetObjMode( VOBJ_TYPE_CHART ) );
        else if ( aPropertyName == SC_UNO_SHOWDRAW )   aRet <<= (sal_Int16)( rOpt.GetObjMode( VOBJ_TYPE_DRAW ) );
        else if ( aPropertyName == SC_UNO_GRIDCOLOR )  aRet <<= (sal_Int32)( rOpt.GetGridColor().GetColor() );
        else if ( aPropertyName == SC_UNO_VISAREA ) aRet <<= GetVisArea();
        else if ( aPropertyName == SC_UNO_ZOOMTYPE ) aRet <<= GetZoomType();
        else if ( aPropertyName == SC_UNO_ZOOMVALUE ) aRet <<= GetZoom();
        else if ( aPropertyName == SC_UNO_VISAREASCREEN )
        {
            ScViewData& rViewData = pViewSh->GetViewData();
            vcl::Window* pActiveWin = rViewData.GetActiveWin();
            if ( pActiveWin )
            {
                Rectangle aRect = pActiveWin->GetWindowExtentsRelative( nullptr );
                aRet <<= AWTRectangle( aRect );
            }
        }
    }

    return aRet;
}

void SAL_CALL ScTabViewObj::addPropertyChangeListener( const OUString& /* aPropertyName */,
    const uno::Reference<beans::XPropertyChangeListener >& xListener )
{
    SolarMutexGuard aGuard;
    aPropertyChgListeners.push_back( xListener );
}

void SAL_CALL ScTabViewObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                                    const uno::Reference<beans::XPropertyChangeListener >& xListener )
{
    SolarMutexGuard aGuard;
    for (XViewPropertyChangeListenerVector::iterator it = aPropertyChgListeners.begin();
         it != aPropertyChgListeners.end(); ++it )
    {
        if ( *it == xListener ) //! wozu der Mumpitz mit queryInterface?
        {
            aPropertyChgListeners.erase(it);
            break;
        }
    }
}

void SAL_CALL ScTabViewObj::addVetoableChangeListener( const OUString& /* PropertyName */,
                                    const uno::Reference<beans::XVetoableChangeListener >& /* aListener */ )
{
}

void SAL_CALL ScTabViewObj::removeVetoableChangeListener( const OUString& /* PropertyName */,
                                    const uno::Reference<beans::XVetoableChangeListener >& /* aListener */ )
{
}

void ScTabViewObj::VisAreaChanged()
{
    beans::PropertyChangeEvent aEvent;
    aEvent.Source.set(static_cast<cppu::OWeakObject*>(this));
    for (const auto& rListener : aPropertyChgListeners)
        rListener->propertyChange( aEvent );
}

// XRangeSelection

void SAL_CALL ScTabViewObj::startRangeSelection(
                                const uno::Sequence<beans::PropertyValue>& aArguments )
{
    SolarMutexGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        OUString aInitVal, aTitle;
        bool bCloseOnButtonUp = false;
        bool bSingleCell = false;
        bool bMultiSelection = false;

        OUString aStrVal;
        const beans::PropertyValue* pPropArray = aArguments.getConstArray();
        long nPropCount = aArguments.getLength();
        for (long i = 0; i < nPropCount; i++)
        {
            const beans::PropertyValue& rProp = pPropArray[i];
            OUString aPropName(rProp.Name);

            if (aPropName == SC_UNONAME_CLOSEONUP )
                bCloseOnButtonUp = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
            else if (aPropName == SC_UNONAME_TITLE )
            {
                if ( rProp.Value >>= aStrVal )
                    aTitle = aStrVal;
            }
            else if (aPropName == SC_UNONAME_INITVAL )
            {
                if ( rProp.Value >>= aStrVal )
                    aInitVal = aStrVal;
            }
            else if (aPropName == SC_UNONAME_SINGLECELL )
                bSingleCell = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
            else if (aPropName == SC_UNONAME_MULTISEL )
                bMultiSelection = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
        }

        pViewSh->StartSimpleRefDialog( aTitle, aInitVal, bCloseOnButtonUp, bSingleCell, bMultiSelection );
    }
}

void SAL_CALL ScTabViewObj::abortRangeSelection()
{
    SolarMutexGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
        pViewSh->StopSimpleRefDialog();
}

void SAL_CALL ScTabViewObj::addRangeSelectionListener(
    const uno::Reference<sheet::XRangeSelectionListener>& xListener )
{
    SolarMutexGuard aGuard;
    aRangeSelListeners.push_back( xListener );
}

void SAL_CALL ScTabViewObj::removeRangeSelectionListener(
                                const uno::Reference<sheet::XRangeSelectionListener>& xListener )
{
    SolarMutexGuard aGuard;
    for (XRangeSelectionListenerVector::iterator it = aRangeSelListeners.begin();
         it != aRangeSelListeners.end(); ++it )
    {
        if ( *it == xListener )
        {
            aRangeSelListeners.erase(it);
            break;
        }
    }
}

void SAL_CALL ScTabViewObj::addRangeSelectionChangeListener(
    const uno::Reference<sheet::XRangeSelectionChangeListener>& xListener )
{
    SolarMutexGuard aGuard;
    aRangeChgListeners.push_back( xListener );
}

void SAL_CALL ScTabViewObj::removeRangeSelectionChangeListener(
                                const uno::Reference<sheet::XRangeSelectionChangeListener>& xListener )
{
    SolarMutexGuard aGuard;
    for (XRangeSelectionChangeListenerVector::iterator it = aRangeChgListeners.begin();
         it != aRangeChgListeners.end(); ++it )
    {
        if ( *it == xListener )
        {
            aRangeChgListeners.erase(it);
            break;
        }
    }
}

void ScTabViewObj::RangeSelDone( const OUString& rText )
{
    sheet::RangeSelectionEvent aEvent;
    aEvent.Source.set(static_cast<cppu::OWeakObject*>(this));
    aEvent.RangeDescriptor = rText;

    // copy on the stack because listener could remove itself
    XRangeSelectionListenerVector const listeners(aRangeSelListeners);

    for (const auto& rListener : listeners)
        rListener->done( aEvent );
}

void ScTabViewObj::RangeSelAborted( const OUString& rText )
{
    sheet::RangeSelectionEvent aEvent;
    aEvent.Source.set(static_cast<cppu::OWeakObject*>(this));
    aEvent.RangeDescriptor = rText;

    // copy on the stack because listener could remove itself
    XRangeSelectionListenerVector const listeners(aRangeSelListeners);

    for (const auto& rListener : listeners)
        rListener->aborted( aEvent );
}

void ScTabViewObj::RangeSelChanged( const OUString& rText )
{
    sheet::RangeSelectionEvent aEvent;
    aEvent.Source.set(static_cast<cppu::OWeakObject*>(this));
    aEvent.RangeDescriptor = rText;

    // copy on the stack because listener could remove itself
    XRangeSelectionChangeListenerVector const listener(aRangeChgListeners);

    for (const auto& rListener : listener)
        rListener->descriptorChanged( aEvent );
}

// XServiceInfo
OUString SAL_CALL ScTabViewObj::getImplementationName()
{
    return OUString( "ScTabViewObj" );
}

sal_Bool SAL_CALL ScTabViewObj::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScTabViewObj::getSupportedServiceNames()
{
    return {SCTABVIEWOBJ_SERVICE, SCVIEWSETTINGS_SERVICE};
}

// XUnoTunnel

sal_Int64 SAL_CALL ScTabViewObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId )
{
    if ( rId.getLength() == 16 &&
          0 == memcmp( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return 0;
}

namespace
{
    class theScTabViewObjUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScTabViewObjUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScTabViewObj::getUnoTunnelId()
{
    return theScTabViewObjUnoTunnelId::get().getSeq();
}

ScTabViewObj* ScTabViewObj::getImplementation(const uno::Reference<uno::XInterface>& rObj)
{
    ScTabViewObj* pRet = nullptr;
    uno::Reference<lang::XUnoTunnel> xUT(rObj, uno::UNO_QUERY);
    if (xUT.is())
        pRet = reinterpret_cast<ScTabViewObj*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

css::uno::Reference< css::datatransfer::XTransferable > SAL_CALL ScTabViewObj::getTransferable()
{
    SolarMutexGuard aGuard;
    ScEditShell* pShell = dynamic_cast<ScEditShell*>( GetViewShell()->GetViewFrame()->GetDispatcher()->GetShell(0)  );
    if (pShell)
        return pShell->GetEditView()->GetTransferable();

    ScDrawTextObjectBar* pTextShell = dynamic_cast<ScDrawTextObjectBar*>( GetViewShell()->GetViewFrame()->GetDispatcher()->GetShell(0)  );
    if (pTextShell)
    {
        ScViewData& rViewData = GetViewShell()->GetViewData();
        ScDrawView* pView = rViewData.GetScDrawView();
        OutlinerView* pOutView = pView->GetTextEditOutlinerView();
        if (pOutView)
            return pOutView->GetEditView().GetTransferable();
    }

    ScDrawShell* pDrawShell = dynamic_cast<ScDrawShell*>( GetViewShell()->GetViewFrame()->GetDispatcher()->GetShell(0)  );
    if (pDrawShell)
        return pDrawShell->GetDrawView()->CopyToTransferable();

    ScTransferObj* pObj = GetViewShell()->CopyToTransferable();
    uno::Reference<datatransfer::XTransferable> xTransferable( pObj );
    return xTransferable;
}

void SAL_CALL ScTabViewObj::insertTransferable( const css::uno::Reference< css::datatransfer::XTransferable >& xTrans )
{
    SolarMutexGuard aGuard;
    ScEditShell* pShell = dynamic_cast<ScEditShell*>( GetViewShell()->GetViewFrame()->GetDispatcher()->GetShell(0)  );
    if (pShell)
        pShell->GetEditView()->InsertText( xTrans, OUString(), false );
    else
    {
        ScDrawTextObjectBar* pTextShell = dynamic_cast<ScDrawTextObjectBar*>( GetViewShell()->GetViewFrame()->GetDispatcher()->GetShell(0)  );
        if (pTextShell)
        {
            ScViewData& rViewData = GetViewShell()->GetViewData();
            ScDrawView* pView = rViewData.GetScDrawView();
            OutlinerView* pOutView = pView->GetTextEditOutlinerView();
            if ( pOutView )
            {
                pOutView->GetEditView().InsertText( xTrans, OUString(), false );
                return;
            }
        }

        GetViewShell()->PasteFromTransferable( xTrans );
    }
}

namespace {

uno::Sequence<sal_Int32> toSequence(const ScMarkData::MarkedTabsType& rSelected)
{
    uno::Sequence<sal_Int32> aRet(rSelected.size());
    ScMarkData::MarkedTabsType::const_iterator itr = rSelected.begin(), itrEnd = rSelected.end();
    for (size_t i = 0; itr != itrEnd; ++itr, ++i)
        aRet[i] = static_cast<sal_Int32>(*itr);

    return aRet;
}

}

uno::Sequence<sal_Int32> ScTabViewObj::getSelectedSheets()
{
    ScTabViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return uno::Sequence<sal_Int32>();

    ScViewData& rViewData = pViewSh->GetViewData();

    // #i95280# when printing from the shell, the view is never activated,
    // so Excel view settings must also be evaluated here.
    ScExtDocOptions* pExtOpt = rViewData.GetDocument()->GetExtDocOptions();
    if (pExtOpt && pExtOpt->IsChanged())
    {
        pViewSh->GetViewData().ReadExtOptions(*pExtOpt);        // Excel view settings
        pViewSh->SetTabNo(pViewSh->GetViewData().GetTabNo(), true);
        pExtOpt->SetChanged(false);
    }

    return toSequence(rViewData.GetMarkData().GetSelectedTabs());
}

ScPreviewObj::ScPreviewObj(ScPreviewShell* pViewSh) :
    SfxBaseController(pViewSh),
    mpViewShell(pViewSh)
{
    if (mpViewShell)
        StartListening(*mpViewShell);
}

ScPreviewObj::~ScPreviewObj()
{
    if (mpViewShell)
        EndListening(*mpViewShell);
}

uno::Any ScPreviewObj::queryInterface(const uno::Type& rType)
{
    SC_QUERYINTERFACE(sheet::XSelectedSheetsSupplier)
    return SfxBaseController::queryInterface(rType);
}

void ScPreviewObj::acquire() throw()
{
    SfxBaseController::acquire();
}

void ScPreviewObj::release() throw()
{
    SfxBaseController::release();
}

void ScPreviewObj::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::Dying)
        mpViewShell = nullptr;
}

uno::Sequence<sal_Int32> ScPreviewObj::getSelectedSheets()
{
    ScPreview* p = mpViewShell ? mpViewShell->GetPreview() : nullptr;
    if (!p)
        return uno::Sequence<sal_Int32>();

    return toSequence(p->GetSelectedTabs());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
