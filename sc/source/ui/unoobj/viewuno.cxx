/*************************************************************************
 *
 *  $RCSfile: viewuno.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:30:01 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <svx/fmdpage.hxx>
#include <svx/svditer.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/unoshape.hxx>
#include <svx/unoshcol.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/request.hxx>
#include <rtl/uuid.h>
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif

#include "viewuno.hxx"
#include "cellsuno.hxx"
#include "miscuno.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"
#include "drawview.hxx"
#include "fupoor.hxx"
#include "sc.hrc"
#include "unoguard.hxx"
#include "unonames.hxx"
#include "scmod.hxx"
#include "appoptio.hxx"
#ifndef SC_GRIDWIN_HXX
#include "gridwin.hxx"
#endif

#ifndef _COM_SUN_STAR_VIEW_DOCUMENTZOOMTYPE_HPP_
#include <com/sun/star/view/DocumentZoomType.hpp>
#endif
#ifndef SC_ACCESSIBILITYHINTS_HXX
#include "AccessibilityHints.hxx"
#endif

using namespace com::sun::star;

//------------------------------------------------------------------------

//! Clipping-Markierungen

//  alles ohne Which-ID, Map nur fuer PropertySetInfo

const SfxItemPropertyMap* lcl_GetViewOptPropertyMap()
{
    static SfxItemPropertyMap aViewOptPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(OLD_UNO_COLROWHDR),   0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(SC_UNO_GRIDCOLOR),    0,  &getCppuType((sal_Int32*)0),    0},
        {MAP_CHAR_LEN(SC_UNO_COLROWHDR),    0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(SC_UNO_HORSCROLL),    0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(SC_UNO_SHEETTABS),    0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(SC_UNO_VERTSCROLL),   0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(SC_UNO_HIDESPELL),    0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(OLD_UNO_HORSCROLL),   0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(SC_UNO_OUTLSYMB),     0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(SC_UNO_VALUEHIGH),    0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(OLD_UNO_OUTLSYMB),    0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(OLD_UNO_SHEETTABS),   0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(SC_UNO_SHOWANCHOR),   0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(SC_UNO_SHOWCHARTS),   0,  &getCppuType((sal_Int16*)0),    0},
        {MAP_CHAR_LEN(SC_UNO_SHOWDRAW),     0,  &getCppuType((sal_Int16*)0),    0},
        {MAP_CHAR_LEN(SC_UNO_SHOWFORM),     0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(SC_UNO_SHOWGRID),     0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(SC_UNO_SHOWHELP),     0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(SC_UNO_SHOWNOTES),    0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(SC_UNO_SHOWOBJ),      0,  &getCppuType((sal_Int16*)0),    0},
        {MAP_CHAR_LEN(SC_UNO_SHOWPAGEBR),   0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(SC_UNO_SHOWZERO),     0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(SC_UNO_SHOWSOLID),    0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(OLD_UNO_VALUEHIGH),   0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(OLD_UNO_VERTSCROLL),  0,  &getBooleanCppuType(),          0},
        {MAP_CHAR_LEN(SC_UNO_VISAREA),      0,  &getCppuType((awt::Rectangle*)0),   0},
        {MAP_CHAR_LEN(SC_UNO_ZOOMTYPE),     0,  &getCppuType((sal_Int16*)0),    0},
        {MAP_CHAR_LEN(SC_UNO_ZOOMVALUE),    0,  &getCppuType((sal_Int16*)0),    0},
        {0,0,0,0}
    };
    return aViewOptPropertyMap_Impl;
}

//------------------------------------------------------------------------

SV_IMPL_PTRARR( XRangeSelectionListenerArr_Impl, XRangeSelectionListenerPtr );
SV_IMPL_PTRARR( XRangeSelectionChangeListenerArr_Impl, XRangeSelectionChangeListenerPtr );
SV_IMPL_PTRARR( XSelectionChangeListenerArr_Impl, XSelectionChangeListenerPtr );
SV_IMPL_PTRARR( XViewPropertyChangeListenerArr_Impl, XViewPropertyChangeListenerPtr );
SV_IMPL_PTRARR( XMouseClickHandlerArr_Impl, XMouseClickHandlerPtr );
SV_IMPL_PTRARR( XActivationEventListenerArr_Impl, XActivationEventListenerPtr );

#define SCTABVIEWOBJ_SERVICE        "com.sun.star.sheet.SpreadsheetView"
#define SCVIEWSETTINGS_SERVICE      "com.sun.star.sheet.SpreadsheetViewSettings"

SC_SIMPLE_SERVICE_INFO( ScViewPaneBase, "ScViewPaneObj", "com.sun.star.sheet.SpreadsheetViewPane" )

//------------------------------------------------------------------------

ScViewPaneBase::ScViewPaneBase(ScTabViewShell* pViewSh, USHORT nP) :
    pViewShell( pViewSh ),
    nPane( nP )
{
    if (pViewShell)
        StartListening(*pViewShell);
}

ScViewPaneBase::~ScViewPaneBase()
{
    if (pViewShell)
        EndListening(*pViewShell);
}

void ScViewPaneBase::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
        pViewShell = NULL;
}

uno::Any SAL_CALL ScViewPaneBase::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( sheet::XViewPane )
    SC_QUERYINTERFACE( sheet::XCellRangeReferrer )
    SC_QUERYINTERFACE( view::XControlAccess )
    SC_QUERYINTERFACE( lang::XServiceInfo )
    SC_QUERYINTERFACE( lang::XTypeProvider )

    return uno::Any();          // OWeakObject is in derived objects
}

uno::Sequence<uno::Type> SAL_CALL ScViewPaneBase::getTypes() throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        aTypes.realloc(5);
        uno::Type* pPtr = aTypes.getArray();
        pPtr[0] = getCppuType((const uno::Reference<sheet::XViewPane>*)0);
        pPtr[1] = getCppuType((const uno::Reference<sheet::XCellRangeReferrer>*)0);
        pPtr[2] = getCppuType((const uno::Reference<view::XControlAccess>*)0);
        pPtr[3] = getCppuType((const uno::Reference<lang::XServiceInfo>*)0);
        pPtr[4] = getCppuType((const uno::Reference<lang::XTypeProvider>*)0);
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScViewPaneBase::getImplementationId()
                                                    throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XViewPane

sal_Int32 SAL_CALL ScViewPaneBase::getFirstVisibleColumn() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pViewShell)
    {
        ScViewData* pViewData = pViewShell->GetViewData();
        ScSplitPos eWhich = ( nPane == SC_VIEWPANE_ACTIVE ) ?
                                pViewData->GetActivePart() :
                                (ScSplitPos) nPane;
        ScHSplitPos eWhichH = WhichH( eWhich );

        return pViewData->GetPosX( eWhichH );
    }
    DBG_ERROR("keine View ?!?");    //! Exception?
    return 0;
}

void SAL_CALL ScViewPaneBase::setFirstVisibleColumn( sal_Int32 nFirstVisibleColumn )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pViewShell)
    {
        ScViewData* pViewData = pViewShell->GetViewData();
        ScSplitPos eWhich = ( nPane == SC_VIEWPANE_ACTIVE ) ?
                                pViewData->GetActivePart() :
                                (ScSplitPos) nPane;
        ScHSplitPos eWhichH = WhichH( eWhich );

        long nDeltaX = ((long)nFirstVisibleColumn) - pViewData->GetPosX( eWhichH );
        pViewShell->ScrollX( nDeltaX, eWhichH );
    }
}

sal_Int32 SAL_CALL ScViewPaneBase::getFirstVisibleRow() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pViewShell)
    {
        ScViewData* pViewData = pViewShell->GetViewData();
        ScSplitPos eWhich = ( nPane == SC_VIEWPANE_ACTIVE ) ?
                                pViewData->GetActivePart() :
                                (ScSplitPos) nPane;
        ScVSplitPos eWhichV = WhichV( eWhich );

        return pViewData->GetPosY( eWhichV );
    }
    DBG_ERROR("keine View ?!?");    //! Exception?
    return 0;
}

void SAL_CALL ScViewPaneBase::setFirstVisibleRow( sal_Int32 nFirstVisibleRow )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pViewShell)
    {
        ScViewData* pViewData = pViewShell->GetViewData();
        ScSplitPos eWhich = ( nPane == SC_VIEWPANE_ACTIVE ) ?
                                pViewData->GetActivePart() :
                                (ScSplitPos) nPane;
        ScVSplitPos eWhichV = WhichV( eWhich );

        long nDeltaY = ((long)nFirstVisibleRow) - pViewData->GetPosY( eWhichV );
        pViewShell->ScrollY( nDeltaY, eWhichV );
    }
}

table::CellRangeAddress SAL_CALL ScViewPaneBase::getVisibleRange() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    table::CellRangeAddress aAdr;
    if (pViewShell)
    {
        ScViewData* pViewData = pViewShell->GetViewData();
        ScSplitPos eWhich = ( nPane == SC_VIEWPANE_ACTIVE ) ?
                                pViewData->GetActivePart() :
                                (ScSplitPos) nPane;
        ScHSplitPos eWhichH = WhichH( eWhich );
        ScVSplitPos eWhichV = WhichV( eWhich );

        //  VisibleCellsX gibt nur komplett sichtbare Zellen,
        //  VisibleRange in Excel auch teilweise sichtbare.
        //! anpassen ???

        SCCOL nVisX = pViewData->VisibleCellsX( eWhichH );
        SCROW nVisY = pViewData->VisibleCellsY( eWhichV );
        if (!nVisX) nVisX = 1;  // irgendwas muss ja im Range sein
        if (!nVisY) nVisY = 1;
        aAdr.Sheet       = pViewData->GetTabNo();
        aAdr.StartColumn = pViewData->GetPosX( eWhichH );
        aAdr.StartRow    = pViewData->GetPosY( eWhichV );
        aAdr.EndColumn   = aAdr.StartColumn + nVisX - 1;
        aAdr.EndRow      = aAdr.StartRow    + nVisY - 1;
    }
    return aAdr;
}

// XCellRangeSource

uno::Reference<table::XCellRange> SAL_CALL ScViewPaneBase::getReferredCells()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pViewShell)
    {
        ScDocShell* pDocSh = pViewShell->GetViewData()->GetDocShell();

        table::CellRangeAddress aAdr = getVisibleRange();       //! Hilfsfunktion mit ScRange?
        ScRange aRange( (SCCOL)aAdr.StartColumn, (SCROW)aAdr.StartRow, aAdr.Sheet,
                        (SCCOL)aAdr.EndColumn, (SCROW)aAdr.EndRow, aAdr.Sheet );
        if ( aRange.aStart == aRange.aEnd )
            return new ScCellObj( pDocSh, aRange.aStart );
        else
            return new ScCellRangeObj( pDocSh, aRange );
    }

    return NULL;
}

// XControlAccess

uno::Reference<awt::XControl> SAL_CALL ScViewPaneBase::getControl(
                            const uno::Reference<awt::XControlModel>& xModel )
                throw(container::NoSuchElementException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<awt::XControl> xRet;
    if ( pViewShell )
    {
        ScViewData* pViewData = pViewShell->GetViewData();
        ScSplitPos eWhich = ( nPane == SC_VIEWPANE_ACTIVE ) ?
                                pViewData->GetActivePart() :
                                (ScSplitPos) nPane;
        Window* pWin = pViewShell->GetWindowByPos( eWhich );
        SdrModel* pModel = pViewData->GetDocument()->GetDrawLayer();
        if ( pWin && pModel )
        {
            SdrPage* pPage = pModel->GetPage( static_cast<sal_uInt16>(pViewData->GetTabNo()) );
            if ( pPage )
            {
                ULONG nCount = pPage->GetObjCount();
                for ( ULONG i=0; i<nCount; i++ )
                {
                    SdrUnoObj* pFormObj = PTR_CAST( SdrUnoObj, pPage->GetObj(i) );
                    if ( pFormObj )
                    {
                        uno::Reference<awt::XControlModel> xCM = pFormObj->GetUnoControlModel();
                        if ( xCM.is() && xModel == xCM )
                        {
                            xRet = pFormObj->GetUnoControl( pWin );
                            break;
                        }
                    }
                }
            }
        }
    }

    if (!xRet.is())
        throw container::NoSuchElementException();      // no control found

    return xRet;
}

awt::Rectangle ScViewPaneBase::GetVisArea() const
{
    awt::Rectangle aVisArea;
    if (pViewShell)
    {
        ScSplitPos eWhich = ( nPane == SC_VIEWPANE_ACTIVE ) ?
                                pViewShell->GetViewData()->GetActivePart() :
                                (ScSplitPos) nPane;
        ScGridWindow* pWindow = (ScGridWindow*)pViewShell->GetWindowByPos(eWhich);
        ScDocument* pDoc = pViewShell->GetViewData()->GetDocument();
        if (pWindow && pDoc)
        {
            ScHSplitPos eWhichH = ((eWhich == SC_SPLIT_TOPLEFT) || (eWhich == SC_SPLIT_BOTTOMLEFT)) ?
                                    SC_SPLIT_LEFT : SC_SPLIT_RIGHT;
            ScVSplitPos eWhichV = ((eWhich == SC_SPLIT_TOPLEFT) || (eWhich == SC_SPLIT_TOPRIGHT)) ?
                                    SC_SPLIT_TOP : SC_SPLIT_BOTTOM;
            ScAddress aCell(pViewShell->GetViewData()->GetPosX(eWhichH),
                pViewShell->GetViewData()->GetPosY(eWhichV),
                pViewShell->GetViewData()->GetTabNo());
            Rectangle aVisRect(pDoc->GetMMRect(aCell.Col(), aCell.Row(), aCell.Col(), aCell.Row(), aCell.Tab()));

            aVisRect.SetSize(pWindow->PixelToLogic(pWindow->GetSizePixel(), pWindow->GetDrawMapMode(sal_True)));

            aVisArea = AWTRectangle(aVisRect);
        }
    }
    return aVisArea;
}

//------------------------------------------------------------------------

ScViewPaneObj::ScViewPaneObj(ScTabViewShell* pViewSh, USHORT nP) :
    ScViewPaneBase( pViewSh, nP )
{
}

ScViewPaneObj::~ScViewPaneObj()
{
}

uno::Any SAL_CALL ScViewPaneObj::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    //  ScViewPaneBase has everything except OWeakObject

    uno::Any aRet = ScViewPaneBase::queryInterface( rType );
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

//------------------------------------------------------------------------

//  Default-ctor wird fuer SMART_REFLECTION_IMPLEMENTATION gebraucht

ScTabViewObj::ScTabViewObj() :
    SfxBaseController( NULL ),
    ScViewPaneBase( NULL, SC_VIEWPANE_ACTIVE ),
    aPropSet( lcl_GetViewOptPropertyMap() ),
    bDrawSelModeSet(sal_False),
    aMouseClickHandlers( 0 ),
    aActivationListeners( 0 )
{
}

ScTabViewObj::ScTabViewObj( ScTabViewShell* pViewSh ) :
    SfxBaseController( pViewSh ),
    ScViewPaneBase( pViewSh, SC_VIEWPANE_ACTIVE ),
    aPropSet( lcl_GetViewOptPropertyMap() ),
    bDrawSelModeSet(sal_False),
    aMouseClickHandlers( 0 ),
    aActivationListeners( 0 )
{
    //! Listening oder so
}

ScTabViewObj::~ScTabViewObj()
{
    //! Listening oder so
    if (aMouseClickHandlers.Count())
    {
        acquire();
        EndMouseListening();
    }
    if (aActivationListeners.Count())
    {
        acquire();
        EndActivationListening();
    }
}

uno::Any SAL_CALL ScTabViewObj::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
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

    uno::Any aRet = ScViewPaneBase::queryInterface( rType );
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

void ScTabViewObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SC_HINT_TABLECHANGED )
    {
        if (aActivationListeners.Count() > 0)
        {
            sheet::ActivationEvent aEvent;
            uno::Reference< sheet::XSpreadsheetView > xView(this);
            uno::Reference< uno::XInterface > xSource(xView, uno::UNO_QUERY);
            aEvent.Source = xSource;
            aEvent.ActiveSheet = new ScTableSheetObj(GetViewShell()->GetViewData()->GetDocShell(), GetViewShell()->GetViewData()->GetTabNo());
            for ( USHORT n=0; n<aActivationListeners.Count(); n++ )
            {
                try
                {
                    (*aActivationListeners[n])->activeSpreadsheetChanged( aEvent );
                }
                catch( uno::Exception e )
                {
                    aActivationListeners.DeleteAndDestroy( n );
                    --n; // because it will be increased again in the loop
                }
            }
        }
    }
    ScViewPaneBase::Notify(rBC, rHint);
}

uno::Sequence<uno::Type> SAL_CALL ScTabViewObj::getTypes() throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aViewPaneTypes = ScViewPaneBase::getTypes();
        long nViewPaneLen = aViewPaneTypes.getLength();
        const uno::Type* pViewPanePtr = aViewPaneTypes.getConstArray();

        uno::Sequence<uno::Type> aControllerTypes = SfxBaseController::getTypes();
        long nControllerLen = aControllerTypes.getLength();
        const uno::Type* pControllerPtr = aControllerTypes.getConstArray();

        long nParentLen = nViewPaneLen + nControllerLen;

        aTypes.realloc( nParentLen + 11 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = getCppuType((const uno::Reference<sheet::XSpreadsheetView>*)0);
        pPtr[nParentLen + 1] = getCppuType((const uno::Reference<container::XEnumerationAccess>*)0);
        pPtr[nParentLen + 2] = getCppuType((const uno::Reference<container::XIndexAccess>*)0);
        pPtr[nParentLen + 3] = getCppuType((const uno::Reference<view::XSelectionSupplier>*)0);
        pPtr[nParentLen + 4] = getCppuType((const uno::Reference<beans::XPropertySet>*)0);
        pPtr[nParentLen + 5] = getCppuType((const uno::Reference<sheet::XViewSplitable>*)0);
        pPtr[nParentLen + 6] = getCppuType((const uno::Reference<sheet::XViewFreezable>*)0);
        pPtr[nParentLen + 7] = getCppuType((const uno::Reference<sheet::XRangeSelection>*)0);
        pPtr[nParentLen + 8] = getCppuType((const uno::Reference<lang::XUnoTunnel>*)0);
        pPtr[nParentLen + 9] = getCppuType((const uno::Reference<sheet::XEnhancedMouseClickBroadcaster>*)0);
        pPtr[nParentLen + 10] = getCppuType((const uno::Reference<sheet::XActivationBroadcaster>*)0);

        long i;
        for (i=0; i<nViewPaneLen; i++)
            pPtr[i] = pViewPanePtr[i];              // parent types first
        for (i=0; i<nControllerLen; i++)
            pPtr[nViewPaneLen+i] = pControllerPtr[i];
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScTabViewObj::getImplementationId()
                                                throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XDocumentView

BOOL lcl_TabInRanges( SCTAB nTab, const ScRangeList& rRanges )
{
    ULONG nCount = rRanges.Count();
    for (ULONG i=0; i<nCount; i++)
    {
        const ScRange* pRange = rRanges.GetObject(i);
        if ( nTab >= pRange->aStart.Tab() && nTab <= pRange->aEnd.Tab() )
            return TRUE;
    }
    return FALSE;
}

void lcl_ShowObject( ScTabViewShell& rViewSh, ScDrawView& rDrawView, SdrObject* pSelObj )
{
    BOOL bFound = FALSE;
    SCTAB nObjectTab = 0;

    SdrModel* pModel = rDrawView.GetModel();
    USHORT nPageCount = pModel->GetPageCount();
    for (USHORT i=0; i<nPageCount && !bFound; i++)
    {
        SdrPage* pPage = pModel->GetPage(i);
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject && !bFound)
            {
                if ( pObject == pSelObj )
                {
                    bFound = TRUE;
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
                        throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();

    if ( !pViewSh )
        return FALSE;

    //! Type of aSelection can be some specific interface instead of XInterface

    BOOL bRet = FALSE;
    uno::Reference<uno::XInterface> xInterface;
    aSelection >>= xInterface;
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
        bRet = TRUE;
    }

    if (bDrawSelModeSet) // remove DrawSelMode if set by API; if necessary it will be set again later
    {
        pViewSh->SetDrawSelMode(sal_False);
        pViewSh->UpdateLayerLocks();
        bDrawSelModeSet = sal_False;
    }

    if (bRet)
        return bRet;


    ScCellRangesBase* pRangesImp = ScCellRangesBase::getImplementation( xInterface );
    uno::Reference<drawing::XShapes> xShapeColl( xInterface, uno::UNO_QUERY );
    uno::Reference<drawing::XShape> xShapeSel( xInterface, uno::UNO_QUERY );
    SvxShape* pShapeImp = SvxShape::getImplementation( xShapeSel );

    if (pRangesImp)                                     // Zell-Ranges
    {
        ScViewData* pViewData = pViewSh->GetViewData();
        if ( pViewData->GetDocShell() == pRangesImp->GetDocShell() )
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
                    pDisp->Execute( pFunc->GetSlotID(), SFX_CALLMODE_SYNCHRON );
            }
            pViewSh->SetDrawShell(FALSE);
            pViewSh->SetDrawSelMode(FALSE); // nach dem Dispatcher-Execute

            //  Ranges selektieren

            const ScRangeList& rRanges = pRangesImp->GetRangeList();
            ULONG nRangeCount = rRanges.Count();
            // for empty range list, remove selection (cursor remains where it was)
            if ( nRangeCount == 0 )
                pViewSh->Unmark();
            else if ( nRangeCount == 1 )
                pViewSh->MarkRange( *rRanges.GetObject(0) );
            else
            {
                //  Mehrfachselektion

                const ScRange* pFirst = rRanges.GetObject(0);
                if ( pFirst && !lcl_TabInRanges( pViewData->GetTabNo(), rRanges ) )
                    pViewSh->SetTabNo( pFirst->aStart.Tab() );
                pViewSh->DoneBlockMode();
                pViewSh->InitOwnBlockMode();
                pViewData->GetMarkData().MarkFromRangeList( rRanges, TRUE );
                pViewData->GetDocShell()->PostPaintGridAll();   // Markierung (alt&neu)
                if ( pFirst )
                {
                    pViewSh->AlignToCursor( pFirst->aStart.Col(), pFirst->aStart.Row(),
                                                SC_FOLLOW_JUMP );
                    pViewSh->SetCursor( pFirst->aStart.Col(), pFirst->aStart.Row() );
                }

                //! Methode an der View, um RangeList zu selektieren
            }
            bRet = TRUE;
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
                    SdrPageView* pPV = pDrawView->GetPageViewPvNum(0);
                    if ( pPV && pObj->GetPage() == pPV->GetPage() )
                    {
                        pDrawView->MarkObj( pObj, pPV );
                        bRet = TRUE;
                    }
                }
            }
            else                // Shape-Collection (xShapeColl ist nicht 0)
            {
                //  Es wird auf die Tabelle des ersten Objekts umgeschaltet,
                //  und alle Objekte selektiert, die auf dieser Tabelle liegen
                //! Exception, wenn Objekte auf verschiedenen Tabellen?

                SdrPageView* pPV = NULL;
                long nCount = xShapeColl->getCount();
                if (nCount)
                {
                    sal_Bool bAllMarked(sal_True);
                    for ( long i = 0; i < nCount; i++ )
                    {
                        uno::Reference<drawing::XShape> xShapeInt;
                        uno::Any aAny = xShapeColl->getByIndex(i);
                        aAny >>= xShapeInt;
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
                                        pViewSh->SetDrawSelMode(sal_True);
                                        pViewSh->UpdateLayerLocks();
                                        bDrawSelModeSet = sal_True;
                                    }
                                    if (!pPV)               // erstes Objekt
                                    {
                                        lcl_ShowObject( *pViewSh, *pDrawView, pObj );
                                        pPV = pDrawView->GetPageViewPvNum(0);
                                    }
                                    if ( pPV && pObj->GetPage() == pPV->GetPage() )
                                    {
                                        if (pDrawView->IsObjMarkable( pObj, pPV ))
                                            pDrawView->MarkObj( pObj, pPV );
                                        else
                                            bAllMarked = sal_False;
                                    }
                                }
                            }
                        }
                    }
                    if (bAllMarked)
                        bRet = TRUE;
                }
                else
                    bRet = TRUE; // empty XShapes (all shapes are deselected)
            }

            if (bRet)
                pViewSh->SetDrawShell(TRUE);
        }
    }

    if (!bRet)
        throw lang::IllegalArgumentException();

    return bRet;
}

uno::Any SAL_CALL ScTabViewObj::getSelection() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    ScCellRangesBase* pObj = NULL;
    if (pViewSh)
    {
        //  Ist auf dem Drawing-Layer etwas selektiert?

        SdrView* pDrawView = pViewSh->GetSdrView();
        if (pDrawView)
        {
            const SdrMarkList& rMarkList = pDrawView->GetMarkedObjectList();
            ULONG nMarkCount = rMarkList.GetMarkCount();
            if (nMarkCount)
            {
                //  ShapeCollection erzeugen (wie in SdXImpressView::getSelection im Draw)
                //  Zurueckgegeben wird XInterfaceRef, das muss das UsrObject-XInterface sein

                SvxShapeCollection* pShapes = new SvxShapeCollection();
                uno::Reference<uno::XInterface> xRet = static_cast<cppu::OWeakObject*>(pShapes);

                for (ULONG i=0; i<nMarkCount; i++)
                {
                    SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
                    if (pObj)
                    {
                        uno::Reference<drawing::XShape> xShape( pObj->getUnoShape(), uno::UNO_QUERY );
                        if (xShape.is())
                            pShapes->add(xShape);
                    }
                }
                uno::Any aAny;
                aAny <<= xRet;
                return aAny;
            }
        }

        //  sonst Tabellen-(Zellen-)Selektion

        ScViewData* pViewData = pViewSh->GetViewData();
        ScDocShell* pDocSh = pViewData->GetDocShell();

        const ScMarkData& rMark = pViewData->GetMarkData();
        SCTAB nTabs = rMark.GetSelectCount();

        ScRange aRange;
        if ( nTabs == 1 && pViewData->GetSimpleArea(aRange) )
        {
            if (aRange.aStart == aRange.aEnd)
                pObj = new ScCellObj( pDocSh, aRange.aStart );
            else
                pObj = new ScCellRangeObj( pDocSh, aRange );
        }
        else            //  Mehrfachselektion
        {
            ScRangeListRef xRanges;
            pViewData->GetMultiArea( xRanges );

            //  bei mehreren Tabellen Ranges kopieren
            //! sollte eigentlich schon in ScMarkData::FillRangeListWithMarks passieren?
            if ( nTabs > 1 )
                rMark.ExtendRangeListTables( xRanges );

            pObj = new ScCellRangesObj( pDocSh, *xRanges );
        }

        if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
        {
            //  remember if the selection was from the cursor position without anything selected
            //  (used when rendering the selection)

            pObj->SetCursorOnly( TRUE );
        }
    }

    uno::Reference<uno::XInterface> xRet = static_cast<cppu::OWeakObject*>(pObj);
    uno::Any aAny;
    aAny <<= xRet;
    return aAny;
}


#if 0
// XPrintable

rtl::OUString ScTabViewObj::getPrinterName(void) const
{
    ScUnoGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        SfxPrinter* pPrinter = pViewSh->GetPrinter(TRUE);
        if (pPrinter)
            return pPrinter->GetName();
    }

    DBG_ERROR("getPrinterName: keine View oder kein Printer");
    return rtl::OUString();
}

void ScTabViewObj::setPrinterName(const rtl::OUString& PrinterName)
{
    ScUnoGuard aGuard;
    //  Drucker setzen - wie in SfxViewShell::ExecPrint_Impl

    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        SfxPrinter* pPrinter = pViewSh->GetPrinter(TRUE);
        if (pPrinter)
        {
            String aString = PrinterName;
            SfxPrinter* pNewPrinter = new SfxPrinter( pPrinter->GetOptions().Clone(), aString );
            if (pNewPrinter->IsKnown())
                pViewSh->SetPrinter( pNewPrinter, SFX_PRINTER_PRINTER );
            else
                delete pNewPrinter;
        }
    }
}

XPropertySetRef ScTabViewObj::createPrintOptions(void)
{
    ScUnoGuard aGuard;
    return new ScPrintSettingsObj;      //! ScPrintSettingsObj implementieren!
}

void ScTabViewObj::print(const XPropertySetRef& xOptions)
{
    ScUnoGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        //! xOptions auswerten (wie denn?)

        SfxRequest aReq( SID_PRINTDOCDIRECT, SFX_CALLMODE_SYNCHRON, pViewSh->GetPool() );
        pViewSh->ExecuteSlot( aReq );
    }
}
#endif

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScTabViewObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SpreadsheetViewPanesEnumeration")));
}

// XIndexAccess

sal_Int32 SAL_CALL ScTabViewObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    USHORT nPanes = 0;
    if (pViewSh)
    {
        nPanes = 1;
        ScViewData* pViewData = pViewSh->GetViewData();
        if ( pViewData->GetHSplitMode() != SC_SPLIT_NONE )
            nPanes *= 2;
        if ( pViewData->GetVSplitMode() != SC_SPLIT_NONE )
            nPanes *= 2;
    }
    return nPanes;
}

uno::Any SAL_CALL ScTabViewObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<sheet::XViewPane> xPane = GetObjectByIndex_Impl((USHORT)nIndex);
    uno::Any aAny;
    if (xPane.is())
        aAny <<= xPane;
    else
        throw lang::IndexOutOfBoundsException();
    return aAny;
}

uno::Type SAL_CALL ScTabViewObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<sheet::XViewPane>*)0);
}

sal_Bool SAL_CALL ScTabViewObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

// XSpreadsheetView

ScViewPaneObj* ScTabViewObj::GetObjectByIndex_Impl(USHORT nIndex) const
{
    static ScSplitPos ePosHV[4] =
        { SC_SPLIT_TOPLEFT, SC_SPLIT_BOTTOMLEFT, SC_SPLIT_TOPRIGHT, SC_SPLIT_BOTTOMRIGHT };

    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScSplitPos eWhich = SC_SPLIT_BOTTOMLEFT;    // default Position
        BOOL bError = FALSE;
        ScViewData* pViewData = pViewSh->GetViewData();
        BOOL bHor = ( pViewData->GetHSplitMode() != SC_SPLIT_NONE );
        BOOL bVer = ( pViewData->GetVSplitMode() != SC_SPLIT_NONE );
        if ( bHor && bVer )
        {
            //  links oben, links unten, rechts oben, rechts unten - wie in Excel
            if ( nIndex < 4 )
                eWhich = ePosHV[nIndex];
            else
                bError = TRUE;
        }
        else if ( bHor )
        {
            if ( nIndex > 1 )
                bError = TRUE;
            else if ( nIndex == 1 )
                eWhich = SC_SPLIT_BOTTOMRIGHT;
            // sonst SC_SPLIT_BOTTOMLEFT
        }
        else if ( bVer )
        {
            if ( nIndex > 1 )
                bError = TRUE;
            else if ( nIndex == 0 )
                eWhich = SC_SPLIT_TOPLEFT;
            // sonst SC_SPLIT_BOTTOMLEFT
        }
        else if ( nIndex > 0 )
            bError = TRUE;          // nicht geteilt: nur 0 gueltig

        if (!bError)
            return new ScViewPaneObj( pViewSh, eWhich );
    }

    return NULL;
}

uno::Reference<sheet::XSpreadsheet> SAL_CALL ScTabViewObj::getActiveSheet()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScViewData* pData = pViewSh->GetViewData();
        SCTAB nTab = pData->GetTabNo();
        return new ScTableSheetObj( pData->GetDocShell(), nTab );
    }
    return NULL;
}

void SAL_CALL ScTabViewObj::setActiveSheet( const uno::Reference<sheet::XSpreadsheet>& xActiveSheet )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    BOOL bDone = FALSE;
    ScTabViewShell* pViewSh = GetViewShell();
    if ( pViewSh && xActiveSheet.is() )
    {
        //  XSpreadsheet und ScCellRangesBase -> muss ein Sheet sein

        ScCellRangesBase* pRangesImp = ScCellRangesBase::getImplementation( xActiveSheet );
        if ( pRangesImp && pViewSh->GetViewData()->GetDocShell() == pRangesImp->GetDocShell() )
        {
            const ScRangeList& rRanges = pRangesImp->GetRangeList();
            if ( rRanges.Count() == 1 )
            {
                SCTAB nNewTab = rRanges.GetObject(0)->aStart.Tab();
                if ( pViewSh->GetViewData()->GetDocument()->HasTable(nNewTab) )
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
        ScViewData* pData = GetViewShell()->GetViewData();
        ScSplitPos eSplitMode = pData->GetActivePart();
        SCTAB nTab(pData->GetTabNo());
        pData->GetPosFromPixel( rPoint.X(), rPoint.Y(), eSplitMode, nX, nY);

        ScAddress aCellPos (nX, nY, nTab);
        ScCellObj* pCellObj = new ScCellObj(pData->GetDocShell(), aCellPos);

        xTarget.set(uno::Reference<table::XCell>(pCellObj), uno::UNO_QUERY);

        ScDocument* pDoc = pData->GetDocument();
        if (pDoc && pDoc->GetDrawLayer())
        {
            SdrPage* pDrawPage = NULL;
            ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
            if (pDrawLayer->HasObjects() && (pDrawLayer->GetPageCount() > nTab))
                pDrawPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));

            if (pDrawPage)
            {
                Point aPos = pData->GetActiveWin()->PixelToLogic(rPoint);

                sal_uInt32 nCount(pDrawPage->GetObjCount());
                sal_Bool bFound(sal_False);
                sal_uInt32 i(0);
                while (i < nCount && !bFound)
                {
                    SdrObject* pObj = pDrawPage->GetObj(i);
                    if (pObj && pObj->IsHit(aPos, -2))
                    {
                        xTarget.set(pObj->getUnoShape(), uno::UNO_QUERY);
                        bFound = sal_True;
                    }
                    ++i;
                }
            }
        }
    }
    return xTarget;
}

sal_Bool ScTabViewObj::MousePressed( const awt::MouseEvent& e )
                                    throw (::uno::RuntimeException)
{
    sal_Bool bReturn(sal_False);

    if (aMouseClickHandlers.Count())
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

            for ( USHORT n=0; n<aMouseClickHandlers.Count(); n++ )
            {
                try
                {
                    if (!(*aMouseClickHandlers[n])->mousePressed( aMouseEvent ))
                        bReturn = sal_True;
                }
                catch ( uno::Exception e )
                {
                    aMouseClickHandlers.DeleteAndDestroy(n);
                    --n; // because it will be increased again in the loop
                }
            }
        }
    }
    return bReturn;
}

sal_Bool ScTabViewObj::MouseReleased( const awt::MouseEvent& e )
                                    throw (uno::RuntimeException)
{
    sal_Bool bReturn(sal_False);

    if (aMouseClickHandlers.Count())
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

            for ( USHORT n=0; n<aMouseClickHandlers.Count(); n++ )
            {
                try
                {
                    if (!(*aMouseClickHandlers[n])->mouseReleased( aMouseEvent ))
                        bReturn = sal_True;
                }
                catch ( uno::Exception e )
                {
                    aMouseClickHandlers.DeleteAndDestroy(n);
                    --n; // because it will be increased again in the loop
                }
            }
        }
    }
    return bReturn;
}

// XEnhancedMouseClickBroadcaster

void ScTabViewObj::StartMouseListening()
{
}

void ScTabViewObj::EndMouseListening()
{
    USHORT nCount(aMouseClickHandlers.Count());
    lang::EventObject aEvent;
    aEvent.Source = (cppu::OWeakObject*)this;
    for ( USHORT n=0; n<nCount; n++ )
    {
        try
        {
            (*aMouseClickHandlers[n])->disposing(aEvent);
        }
        catch ( uno::Exception e )
        {
        }
    }
    aMouseClickHandlers.DeleteAndDestroy(0, nCount);
}

void ScTabViewObj::StartActivationListening()
{
    if (GetViewShell() && GetViewShell()->GetViewData()->GetDocument())
        GetViewShell()->GetViewData()->GetDocument()->AddUnoObject(*this);
}

void ScTabViewObj::EndActivationListening()
{
    USHORT nCount = aActivationListeners.Count();
    lang::EventObject aEvent;
    aEvent.Source = (cppu::OWeakObject*)this;
    for ( USHORT n=0; n<nCount; n++ )
    {
        try
        {
            (*aActivationListeners[n])->disposing(aEvent);
        }
        catch ( uno::Exception e )
        {
        }
    }
    aActivationListeners.DeleteAndDestroy(0, nCount);

    if (GetViewShell() && GetViewShell()->GetViewData()->GetDocument())
        GetViewShell()->GetViewData()->GetDocument()->RemoveUnoObject(*this);
}

void SAL_CALL ScTabViewObj::addEnhancedMouseClickHandler( const uno::Reference< awt::XEnhancedMouseClickHandler >& aListener )
                                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if (aListener.is())
    {
        USHORT nCount = aMouseClickHandlers.Count();
        uno::Reference<awt::XEnhancedMouseClickHandler> *pObj =
                new uno::Reference<awt::XEnhancedMouseClickHandler>( aListener );
        aMouseClickHandlers.Insert( pObj, nCount );

        if (aMouseClickHandlers.Count() == 1 && nCount == 0) // only if a listener added
            StartMouseListening();
    }
}

void SAL_CALL ScTabViewObj::removeEnhancedMouseClickHandler( const uno::Reference< awt::XEnhancedMouseClickHandler >& aListener )
                                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    USHORT nCount = aMouseClickHandlers.Count();
    for ( USHORT n=nCount; n--; )
    {
        uno::Reference<awt::XEnhancedMouseClickHandler> *pObj = aMouseClickHandlers[n];
        if ( *pObj == aListener )
            aMouseClickHandlers.DeleteAndDestroy( n );
    }
    if ((aMouseClickHandlers.Count() == 0) && (nCount > 0)) // only if last listener removed
        EndMouseListening();
}

// XActivationBroadcaster

void SAL_CALL ScTabViewObj::addActivationEventListener( const uno::Reference< sheet::XActivationEventListener >& aListener )
                                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if (aListener.is())
    {
        USHORT nCount = aActivationListeners.Count();
        uno::Reference<sheet::XActivationEventListener> *pObj =
                new uno::Reference<sheet::XActivationEventListener>( aListener );
        aActivationListeners.Insert( pObj, nCount );

        if (aActivationListeners.Count() == 1 && nCount == 0) // only if a listener added
            StartActivationListening();
    }
}

void SAL_CALL ScTabViewObj::removeActivationEventListener( const uno::Reference< sheet::XActivationEventListener >& aListener )
                                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    USHORT nCount = aActivationListeners.Count();
    for ( USHORT n=nCount; n--; )
    {
        uno::Reference<sheet::XActivationEventListener> *pObj = aActivationListeners[n];
        if ( *pObj == aListener )
            aActivationListeners.DeleteAndDestroy( n );
    }
    if ((aActivationListeners.Count() == 0) && (nCount > 0)) // only if last listener removed
        EndActivationListening();
}

//  PageBreakMode / Zoom sind Properties

#if 0

BOOL ScTabViewObj::getPagebreakMode(void) const
{
    ScUnoGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
        return pViewSh->GetViewData()->IsPagebreakMode();
    return FALSE;
}

void ScTabViewObj::setPagebreakMode(BOOL PagebreakMode)
{
    ScUnoGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
        pViewSh->SetPagebreakMode(PagebreakMode);
}

#endif

INT16 ScTabViewObj::GetZoom(void) const
{
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        const Fraction& rZoomY = pViewSh->GetViewData()->GetZoomY();    // Y wird angezeigt
        return (INT16)(( rZoomY.GetNumerator() * 100 ) / rZoomY.GetDenominator());
    }
    return 0;
}

void ScTabViewObj::SetZoom(INT16 nZoom)
{
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        if ( nZoom != GetZoom() && nZoom != 0 )
        {
            if (!pViewSh->GetViewData()->IsPagebreakMode())
            {
                ScModule* pScMod = SC_MOD();
                ScAppOptions aNewOpt = pScMod->GetAppOptions();
                aNewOpt.SetZoom( nZoom );
                aNewOpt.SetZoomType( pViewSh->GetViewData()->GetView()->GetZoomType() );
                pScMod->SetAppOptions( aNewOpt );
            }
        }
        Fraction aFract( nZoom, 100 );
        pViewSh->SetZoom( aFract, aFract );
        pViewSh->PaintGrid();
        pViewSh->PaintTop();
        pViewSh->PaintLeft();
        pViewSh->GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
    }
}

INT16 ScTabViewObj::GetZoomType(void) const
{
    INT16 aZoomType = view::DocumentZoomType::OPTIMAL;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        SvxZoomType eZoomType = pViewSh->GetViewData()->GetView()->GetZoomType();
        switch (eZoomType)
        {
        case SVX_ZOOM_PERCENT:
            aZoomType = view::DocumentZoomType::BY_VALUE;
            break;
        case SVX_ZOOM_OPTIMAL:
            aZoomType = view::DocumentZoomType::OPTIMAL;
            break;
        case SVX_ZOOM_WHOLEPAGE:
            aZoomType = view::DocumentZoomType::ENTIRE_PAGE;
            break;
        case SVX_ZOOM_PAGEWIDTH:
            aZoomType = view::DocumentZoomType::PAGE_WIDTH;
            break;
        case SVX_ZOOM_PAGEWIDTH_NOBORDER:
            aZoomType = view::DocumentZoomType::PAGE_WIDTH_EXACT;
            break;
        }
    }
    return aZoomType;
}

void ScTabViewObj::SetZoomType(INT16 aZoomType)
{
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScDBFunc* pView = pViewSh->GetViewData()->GetView();
        if (pView)
        {
            SvxZoomType eZoomType;
            switch (aZoomType)
            {
            case view::DocumentZoomType::BY_VALUE:
                eZoomType = SVX_ZOOM_PERCENT;
                break;
            case view::DocumentZoomType::OPTIMAL:
                eZoomType = SVX_ZOOM_OPTIMAL;
                break;
            case view::DocumentZoomType::ENTIRE_PAGE:
                eZoomType = SVX_ZOOM_WHOLEPAGE;
                break;
            case view::DocumentZoomType::PAGE_WIDTH:
                eZoomType = SVX_ZOOM_PAGEWIDTH;
                break;
            case view::DocumentZoomType::PAGE_WIDTH_EXACT:
                eZoomType = SVX_ZOOM_PAGEWIDTH_NOBORDER;
                break;
            default:
                eZoomType = SVX_ZOOM_OPTIMAL;
            }
            sal_Int16 nZoom(GetZoom());
            sal_Int16 nOldZoom(nZoom);
            if ( eZoomType == SVX_ZOOM_PERCENT )
            {
                if ( nZoom < MINZOOM )  nZoom = MINZOOM;
                if ( nZoom > MAXZOOM )  nZoom = MAXZOOM;
            }
            else
                nZoom = pView->CalcZoom( eZoomType, nOldZoom );

            switch ( eZoomType )
            {
                case SVX_ZOOM_WHOLEPAGE:
                case SVX_ZOOM_PAGEWIDTH:
                    pView->SetZoomType( eZoomType );
                    break;

                default:
                    pView->SetZoomType( SVX_ZOOM_PERCENT );
            }
            SetZoom( nZoom );
        }
    }
}

sal_Bool SAL_CALL ScTabViewObj::getIsWindowSplit() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //  wie Menue-Slot SID_WINDOW_SPLIT

    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScViewData* pViewData = pViewSh->GetViewData();
        return ( pViewData->GetHSplitMode() == SC_SPLIT_NORMAL ||
                 pViewData->GetVSplitMode() == SC_SPLIT_NORMAL );
    }

    return FALSE;
}

sal_Bool SAL_CALL ScTabViewObj::hasFrozenPanes() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //  wie Menue-Slot SID_WINDOW_FIX

    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScViewData* pViewData = pViewSh->GetViewData();
        return ( pViewData->GetHSplitMode() == SC_SPLIT_FIX ||
                 pViewData->GetVSplitMode() == SC_SPLIT_FIX );
    }

    return FALSE;
}

sal_Int32 SAL_CALL ScTabViewObj::getSplitHorizontal() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScViewData* pViewData = pViewSh->GetViewData();
        if ( pViewData->GetHSplitMode() != SC_SPLIT_NONE )
            return pViewData->GetHSplitPos();
    }
    return 0;
}

sal_Int32 SAL_CALL ScTabViewObj::getSplitVertical() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScViewData* pViewData = pViewSh->GetViewData();
        if ( pViewData->GetVSplitMode() != SC_SPLIT_NONE )
            return pViewData->GetVSplitPos();
    }
    return 0;
}

sal_Int32 SAL_CALL ScTabViewObj::getSplitColumn() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScViewData* pViewData = pViewSh->GetViewData();
        if ( pViewData->GetHSplitMode() != SC_SPLIT_NONE )
        {
            long nSplit = pViewData->GetHSplitPos();

            ScSplitPos ePos = SC_SPLIT_BOTTOMLEFT;
            if ( pViewData->GetVSplitMode() != SC_SPLIT_NONE )
                ePos = SC_SPLIT_TOPLEFT;

            SCsCOL nCol;
            SCsROW nRow;
            pViewData->GetPosFromPixel( nSplit, 0, ePos, nCol, nRow, FALSE );
            if ( nCol > 0 )
                return nCol;
        }
    }
    return 0;
}

sal_Int32 SAL_CALL ScTabViewObj::getSplitRow() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScViewData* pViewData = pViewSh->GetViewData();
        if ( pViewData->GetVSplitMode() != SC_SPLIT_NONE )
        {
            long nSplit = pViewData->GetVSplitPos();

            ScSplitPos ePos = SC_SPLIT_TOPLEFT;     // es ist vertikal geteilt
            SCsCOL nCol;
            SCsROW nRow;
            pViewData->GetPosFromPixel( 0, nSplit, ePos, nCol, nRow, FALSE );
            if ( nRow > 0 )
                return nRow;
        }
    }
    return 0;
}

void SAL_CALL ScTabViewObj::splitAtPosition( sal_Int32 nPixelX, sal_Int32 nPixelY )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        pViewSh->SplitAtPixel( Point( nPixelX, nPixelY ), TRUE, TRUE );
        pViewSh->FreezeSplitters( FALSE );
        pViewSh->InvalidateSplit();
    }
}

void SAL_CALL ScTabViewObj::freezeAtPosition( sal_Int32 nColumns, sal_Int32 nRows )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        //  erst alles aufheben -> kein Stress mit Scrolling zwischendurch o.ae.

        pViewSh->RemoveSplit();

        Point aWinStart;
        Window* pWin = pViewSh->GetWindowByPos( SC_SPLIT_BOTTOMLEFT );
        if (pWin)
            aWinStart = pWin->GetPosPixel();

        ScViewData* pViewData = pViewSh->GetViewData();
        Point aSplit = pViewData->GetScrPos( (SCCOL)nColumns, (SCROW)nRows, SC_SPLIT_BOTTOMLEFT, TRUE );
        aSplit += aWinStart;

        pViewSh->SplitAtPixel( aSplit, TRUE, TRUE );
        pViewSh->FreezeSplitters( TRUE );
        pViewSh->InvalidateSplit();
    }
}

void SAL_CALL ScTabViewObj::addSelectionChangeListener(
                const uno::Reference<view::XSelectionChangeListener>& xListener )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<view::XSelectionChangeListener>* pObj =
            new uno::Reference<view::XSelectionChangeListener>( xListener );
    aSelectionListeners.Insert( pObj, aSelectionListeners.Count() );
}

void SAL_CALL ScTabViewObj::removeSelectionChangeListener(
                const uno::Reference< view::XSelectionChangeListener >& xListener )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    USHORT nCount = aSelectionListeners.Count();
    for ( USHORT n=nCount; n--; )
    {
        uno::Reference<view::XSelectionChangeListener> *pObj = aSelectionListeners[n];
        if ( *pObj == xListener )       //! wozu der Mumpitz mit queryInterface?
        {
            aSelectionListeners.DeleteAndDestroy( n );
            break;
        }
    }
}

void ScTabViewObj::SelectionChanged()
{
    lang::EventObject aEvent;
    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
    for ( USHORT n=0; n<aSelectionListeners.Count(); n++ )
        (*aSelectionListeners[n])->selectionChanged( aEvent );
}


//  XPropertySet (View-Optionen)
//! auch an der Applikation anbieten?

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTabViewObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScTabViewObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aString = aPropertyName;

    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        ScViewData* pViewData = pViewSh->GetViewData();
        const ScViewOptions& rOldOpt = pViewSh->GetViewData()->GetOptions();
        ScViewOptions aNewOpt = rOldOpt;

        if ( aString.EqualsAscii( SC_UNO_COLROWHDR ) || aString.EqualsAscii( OLD_UNO_COLROWHDR ) )
            aNewOpt.SetOption( VOPT_HEADER, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNO_HORSCROLL ) || aString.EqualsAscii( OLD_UNO_HORSCROLL ) )
            aNewOpt.SetOption( VOPT_HSCROLL, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNO_OUTLSYMB ) || aString.EqualsAscii( OLD_UNO_OUTLSYMB ) )
            aNewOpt.SetOption( VOPT_OUTLINER, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNO_SHEETTABS ) || aString.EqualsAscii( OLD_UNO_SHEETTABS ) )
            aNewOpt.SetOption( VOPT_TABCONTROLS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWANCHOR ) )
            aNewOpt.SetOption( VOPT_ANCHOR, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWFORM ) )
            aNewOpt.SetOption( VOPT_FORMULAS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWGRID ) )
            aNewOpt.SetOption( VOPT_GRID, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWHELP ) )
            aNewOpt.SetOption( VOPT_HELPLINES, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWNOTES ) )
            aNewOpt.SetOption( VOPT_NOTES, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWPAGEBR ) )
            aNewOpt.SetOption( VOPT_PAGEBREAKS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWZERO ) )
            aNewOpt.SetOption( VOPT_NULLVALS, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWSOLID ) )
            aNewOpt.SetOption( VOPT_SOLIDHANDLES, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNO_VALUEHIGH ) || aString.EqualsAscii( OLD_UNO_VALUEHIGH ) )
            aNewOpt.SetOption( VOPT_SYNTAX, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNO_VERTSCROLL ) || aString.EqualsAscii( OLD_UNO_VERTSCROLL ) )
            aNewOpt.SetOption( VOPT_VSCROLL, ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWOBJ ) )
        {
            sal_Int16 nIntVal;
            if ( aValue >>= nIntVal )
                aNewOpt.SetObjMode( VOBJ_TYPE_OLE, (ScVObjMode) nIntVal );
        }
        else if ( aString.EqualsAscii( SC_UNO_SHOWCHARTS ) )
        {
            sal_Int16 nIntVal;
            if ( aValue >>= nIntVal )
                aNewOpt.SetObjMode( VOBJ_TYPE_CHART, (ScVObjMode) nIntVal );
        }
        else if ( aString.EqualsAscii( SC_UNO_SHOWDRAW ) )
        {
            sal_Int16 nIntVal;
            if ( aValue >>= nIntVal )
                aNewOpt.SetObjMode( VOBJ_TYPE_DRAW, (ScVObjMode) nIntVal );
        }
        else if ( aString.EqualsAscii( SC_UNO_GRIDCOLOR ) )
        {
            sal_Int32 nIntVal;
            if ( aValue >>= nIntVal )
                aNewOpt.SetGridColor( nIntVal, String() );
        }
        else if ( aString.EqualsAscii( SC_UNO_HIDESPELL ) )
            aNewOpt.SetHideAutoSpell( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        else if ( aString.EqualsAscii( SC_UNO_ZOOMTYPE ) )
        {
            sal_Int16 nIntVal;
            if ( aValue >>= nIntVal )
                SetZoomType(nIntVal);
        }
        else if ( aString.EqualsAscii( SC_UNO_ZOOMVALUE ) )
        {
            sal_Int16 nIntVal;
            if ( aValue >>= nIntVal )
                SetZoom(nIntVal);
        }

        //  Optionen werden an der View und am Dokument (fuer neue Views) gesetzt,
        //  damit sie beim Speichern erhalten bleiben.
        //! An der App (Module) braeuchte man noch eine Extra-Moeglichkeit,
        //! das einzustellen (fuer neue Dokumente)

        if ( aNewOpt != rOldOpt )
        {
            pViewData->SetOptions( aNewOpt );
            pViewData->GetDocument()->SetViewOptions( aNewOpt );
            pViewData->GetDocShell()->SetDocumentModified();    //! wirklich?

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

uno::Any SAL_CALL ScTabViewObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aString = aPropertyName;
    uno::Any aRet;

    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        const ScViewOptions& rOpt = pViewSh->GetViewData()->GetOptions();

        if ( aString.EqualsAscii( SC_UNO_COLROWHDR ) || aString.EqualsAscii( OLD_UNO_COLROWHDR ) )
            ScUnoHelpFunctions::SetBoolInAny( aRet, rOpt.GetOption( VOPT_HEADER ) );
        else if ( aString.EqualsAscii( SC_UNO_HORSCROLL ) || aString.EqualsAscii( OLD_UNO_HORSCROLL ) )
            ScUnoHelpFunctions::SetBoolInAny( aRet, rOpt.GetOption( VOPT_HSCROLL ) );
        else if ( aString.EqualsAscii( SC_UNO_OUTLSYMB ) || aString.EqualsAscii( OLD_UNO_OUTLSYMB ) )
            ScUnoHelpFunctions::SetBoolInAny( aRet, rOpt.GetOption( VOPT_OUTLINER ) );
        else if ( aString.EqualsAscii( SC_UNO_SHEETTABS ) || aString.EqualsAscii( OLD_UNO_SHEETTABS ) )
            ScUnoHelpFunctions::SetBoolInAny( aRet, rOpt.GetOption( VOPT_TABCONTROLS ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWANCHOR ) ) ScUnoHelpFunctions::SetBoolInAny( aRet, rOpt.GetOption( VOPT_ANCHOR ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWFORM ) )   ScUnoHelpFunctions::SetBoolInAny( aRet, rOpt.GetOption( VOPT_FORMULAS ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWGRID ) )   ScUnoHelpFunctions::SetBoolInAny( aRet, rOpt.GetOption( VOPT_GRID ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWHELP ) )   ScUnoHelpFunctions::SetBoolInAny( aRet, rOpt.GetOption( VOPT_HELPLINES ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWNOTES ) )  ScUnoHelpFunctions::SetBoolInAny( aRet, rOpt.GetOption( VOPT_NOTES ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWPAGEBR ) ) ScUnoHelpFunctions::SetBoolInAny( aRet, rOpt.GetOption( VOPT_PAGEBREAKS ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWZERO ) )   ScUnoHelpFunctions::SetBoolInAny( aRet, rOpt.GetOption( VOPT_NULLVALS ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWSOLID ) )  ScUnoHelpFunctions::SetBoolInAny( aRet, rOpt.GetOption( VOPT_SOLIDHANDLES ) );
        else if ( aString.EqualsAscii( SC_UNO_VALUEHIGH ) || aString.EqualsAscii( OLD_UNO_VALUEHIGH ) )
            ScUnoHelpFunctions::SetBoolInAny( aRet, rOpt.GetOption( VOPT_SYNTAX ) );
        else if ( aString.EqualsAscii( SC_UNO_VERTSCROLL ) || aString.EqualsAscii( OLD_UNO_VERTSCROLL ) )
            ScUnoHelpFunctions::SetBoolInAny( aRet, rOpt.GetOption( VOPT_VSCROLL ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWOBJ ) )    aRet <<= (sal_Int16)( rOpt.GetObjMode( VOBJ_TYPE_OLE ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWCHARTS ) ) aRet <<= (sal_Int16)( rOpt.GetObjMode( VOBJ_TYPE_CHART ) );
        else if ( aString.EqualsAscii( SC_UNO_SHOWDRAW ) )   aRet <<= (sal_Int16)( rOpt.GetObjMode( VOBJ_TYPE_DRAW ) );
        else if ( aString.EqualsAscii( SC_UNO_GRIDCOLOR ) )  aRet <<= (sal_Int32)( rOpt.GetGridColor().GetColor() );
        else if ( aString.EqualsAscii( SC_UNO_HIDESPELL ) )  ScUnoHelpFunctions::SetBoolInAny( aRet, rOpt.IsHideAutoSpell() );
        else if ( aString.EqualsAscii( SC_UNO_VISAREA ) ) aRet <<= GetVisArea();
        else if ( aString.EqualsAscii( SC_UNO_ZOOMTYPE ) ) aRet <<= GetZoomType();
        else if ( aString.EqualsAscii( SC_UNO_ZOOMVALUE ) ) aRet <<= GetZoom();
    }

    return aRet;
}

void SAL_CALL ScTabViewObj::addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const uno::Reference<beans::XPropertyChangeListener >& xListener )
                                throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException,
                                    uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<beans::XPropertyChangeListener>* pObj =
            new uno::Reference<beans::XPropertyChangeListener>( xListener );
    aPropertyChgListeners.Insert( pObj, aPropertyChgListeners.Count() );
}

void SAL_CALL ScTabViewObj::removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const uno::Reference<beans::XPropertyChangeListener >& xListener )
                                throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException,
                                    uno::RuntimeException)
{
    ScUnoGuard aGuard;
    USHORT nCount = aPropertyChgListeners.Count();
    for ( USHORT n=nCount; n--; )
    {
        uno::Reference<beans::XPropertyChangeListener> *pObj = aPropertyChgListeners[n];
        if ( *pObj == xListener )       //! wozu der Mumpitz mit queryInterface?
        {
            aPropertyChgListeners.DeleteAndDestroy( n );
            break;
        }
    }
}

void SAL_CALL ScTabViewObj::addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const uno::Reference<beans::XVetoableChangeListener >& aListener )
                                throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException,
                                    uno::RuntimeException)
{
}

void SAL_CALL ScTabViewObj::removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const uno::Reference<beans::XVetoableChangeListener >& aListener )
                                throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException,
                                    uno::RuntimeException)
{
}

void ScTabViewObj::VisAreaChanged()
{
    beans::PropertyChangeEvent aEvent;
    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
    for ( USHORT n=0; n<aPropertyChgListeners.Count(); n++ )
        (*aPropertyChgListeners[n])->propertyChange( aEvent );
}

// XRangeSelection

void SAL_CALL ScTabViewObj::startRangeSelection(
                                const uno::Sequence<beans::PropertyValue>& aArguments )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
    {
        String aInitVal, aTitle;
        BOOL bCloseOnButtonUp = FALSE;

        rtl::OUString aStrVal;
        const beans::PropertyValue* pPropArray = aArguments.getConstArray();
        long nPropCount = aArguments.getLength();
        for (long i = 0; i < nPropCount; i++)
        {
            const beans::PropertyValue& rProp = pPropArray[i];
            String aPropName = rProp.Name;

            if (aPropName.EqualsAscii( SC_UNONAME_CLOSEONUP ))
                bCloseOnButtonUp = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
            else if (aPropName.EqualsAscii( SC_UNONAME_TITLE ))
            {
                if ( rProp.Value >>= aStrVal )
                    aTitle = String( aStrVal );
            }
            else if (aPropName.EqualsAscii( SC_UNONAME_INITVAL ))
            {
                if ( rProp.Value >>= aStrVal )
                    aInitVal = String( aStrVal );
            }
        }

        pViewSh->StartSimpleRefDialog( aTitle, aInitVal, bCloseOnButtonUp );
    }
}

void SAL_CALL ScTabViewObj::abortRangeSelection() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScTabViewShell* pViewSh = GetViewShell();
    if (pViewSh)
        pViewSh->StopSimpleRefDialog();
}

void SAL_CALL ScTabViewObj::addRangeSelectionListener(
                                const uno::Reference<sheet::XRangeSelectionListener>& xListener )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<sheet::XRangeSelectionListener>* pObj =
            new uno::Reference<sheet::XRangeSelectionListener>( xListener );
    aRangeSelListeners.Insert( pObj, aRangeSelListeners.Count() );
}

void SAL_CALL ScTabViewObj::removeRangeSelectionListener(
                                const uno::Reference<sheet::XRangeSelectionListener>& xListener )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    USHORT nCount = aRangeSelListeners.Count();
    for ( USHORT n=nCount; n--; )
    {
        uno::Reference<sheet::XRangeSelectionListener> *pObj = aRangeSelListeners[n];
        if ( *pObj == xListener )
        {
            aRangeSelListeners.DeleteAndDestroy( n );
            break;
        }
    }
}

void SAL_CALL ScTabViewObj::addRangeSelectionChangeListener(
                                const uno::Reference<sheet::XRangeSelectionChangeListener>& xListener )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<sheet::XRangeSelectionChangeListener>* pObj =
            new uno::Reference<sheet::XRangeSelectionChangeListener>( xListener );
    aRangeChgListeners.Insert( pObj, aRangeChgListeners.Count() );
}

void SAL_CALL ScTabViewObj::removeRangeSelectionChangeListener(
                                const uno::Reference<sheet::XRangeSelectionChangeListener>& xListener )
                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    USHORT nCount = aRangeChgListeners.Count();
    for ( USHORT n=nCount; n--; )
    {
        uno::Reference<sheet::XRangeSelectionChangeListener> *pObj = aRangeChgListeners[n];
        if ( *pObj == xListener )
        {
            aRangeChgListeners.DeleteAndDestroy( n );
            break;
        }
    }
}

void ScTabViewObj::RangeSelDone( const String& rText )
{
    sheet::RangeSelectionEvent aEvent;
    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
    aEvent.RangeDescriptor = rtl::OUString( rText );

    for ( USHORT n=0; n<aRangeSelListeners.Count(); n++ )
        (*aRangeSelListeners[n])->done( aEvent );
}

void ScTabViewObj::RangeSelAborted( const String& rText )
{
    sheet::RangeSelectionEvent aEvent;
    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
    aEvent.RangeDescriptor = rtl::OUString( rText );

    for ( USHORT n=0; n<aRangeSelListeners.Count(); n++ )
        (*aRangeSelListeners[n])->aborted( aEvent );
}

void ScTabViewObj::RangeSelChanged( const String& rText )
{
    sheet::RangeSelectionEvent aEvent;
    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
    aEvent.RangeDescriptor = rtl::OUString( rText );

    for ( USHORT n=0; n<aRangeChgListeners.Count(); n++ )
        (*aRangeChgListeners[n])->descriptorChanged( aEvent );
}

// XServiceInfo

rtl::OUString SAL_CALL ScTabViewObj::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString::createFromAscii( "ScTabViewObj" );
}

sal_Bool SAL_CALL ScTabViewObj::supportsService( const rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr( rServiceName );
    return aServiceStr.EqualsAscii( SCTABVIEWOBJ_SERVICE ) ||
           aServiceStr.EqualsAscii( SCVIEWSETTINGS_SERVICE );
}

uno::Sequence<rtl::OUString> SAL_CALL ScTabViewObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(2);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString::createFromAscii( SCTABVIEWOBJ_SERVICE );
    pArray[1] = rtl::OUString::createFromAscii( SCVIEWSETTINGS_SERVICE );
    return aRet;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScTabViewObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

// static
const uno::Sequence<sal_Int8>& ScTabViewObj::getUnoTunnelId()
{
    static uno::Sequence<sal_Int8> * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

// static
ScTabViewObj* ScTabViewObj::getImplementation( const uno::Reference<uno::XInterface> xObj )
{
    ScTabViewObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScTabViewObj*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}


//------------------------------------------------------------------------




