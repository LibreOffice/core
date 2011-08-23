/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <bf_svx/svditer.hxx>
#include <bf_svx/svdoole2.hxx>
#include <bf_svx/svdpage.hxx>
#include <bf_svx/svdundo.hxx>
#include <bf_sch/schdll.hxx>
#include <bf_sch/memchrt.hxx>
#include <bf_sch/schdll0.hxx>
#include <bf_svtools/moduleoptions.hxx>

#include "chartuno.hxx"
#include "miscuno.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"
#include "undodat.hxx"
#include "chartarr.hxx"
#include "chartlis.hxx"
#include <vcl/svapp.hxx>
namespace binfilter {

using namespace ::com::sun::star;

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScChartObj, "ScChartObj", "com.sun.star.table.TableChart" )
SC_SIMPLE_SERVICE_INFO( ScChartsObj, "ScChartsObj", "com.sun.star.table.TableCharts" )

//------------------------------------------------------------------------

SdrOle2Obj* lcl_FindChartObj( ScDocShell* pDocShell, USHORT nTab, const String& rName )
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
        if (pDrawLayer)
        {
            SdrPage* pPage = pDrawLayer->GetPage(nTab);
            DBG_ASSERT(pPage, "Page nicht gefunden");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( pObject->GetObjIdentifier() == OBJ_OLE2 && pDoc->IsChart(pObject) )
                    {
                        SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                        if (aIPObj.Is())
                        {
                            SvInfoObject* pInfoObj = pDocShell->Find( aIPObj );
                            if (pInfoObj)
                            {
                                if ( pInfoObj->GetObjName() == rName )
                                    return (SdrOle2Obj*)pObject;
                            }
                        }
                    }
                    pObject = aIter.Next();
                }
            }
        }
    }
    return NULL;
}

//------------------------------------------------------------------------

ScChartsObj::ScChartsObj(ScDocShell* pDocSh, USHORT nT) :
    pDocShell( pDocSh ),
    nTab( nT )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScChartsObj::~ScChartsObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScChartsObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //!	Referenz-Update

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;		// ungueltig geworden
    }
}

ScChartObj* ScChartsObj::GetObjectByIndex_Impl(long nIndex) const
{
    String aName;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
        if (pDrawLayer)
        {
            SdrPage* pPage = pDrawLayer->GetPage(nTab);
            DBG_ASSERT(pPage, "Page nicht gefunden");
            if (pPage)
            {
                long nPos = 0;
                SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( pObject->GetObjIdentifier() == OBJ_OLE2 && pDoc->IsChart(pObject) )
                    {
                        if ( nPos == nIndex )
                        {
                            SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                            if (aIPObj.Is())
                            {
                                SvInfoObject* pInfoObj = pDocShell->Find( aIPObj );
                                if (pInfoObj)
                                    aName = pInfoObj->GetObjName();
                            }
                            break;	// nicht weitersuchen
                        }
                        ++nPos;
                    }
                    pObject = aIter.Next();
                }
            }
        }
    }

    if (aName.Len())
        return new ScChartObj( pDocShell, nTab, aName );
    return NULL;
}

ScChartObj* ScChartsObj::GetObjectByName_Impl(const ::rtl::OUString& aName) const
{
    String aNameString = aName;
    if ( lcl_FindChartObj( pDocShell, nTab, aNameString ) )
        return new ScChartObj( pDocShell, nTab, aNameString );
    return NULL;
}

// XTableCharts

void SAL_CALL ScChartsObj::addNewByName( const ::rtl::OUString& aName,
                                        const awt::Rectangle& aRect,
                                        const uno::Sequence<table::CellRangeAddress>& aRanges,
                                        sal_Bool bColumnHeaders, sal_Bool bRowHeaders )
                                    throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
        return;

    ScDocument* pDoc = pDocShell->GetDocument();
    ScDrawLayer* pModel = pDocShell->MakeDrawLayer();
    SdrPage* pPage = pModel->GetPage(nTab);
    DBG_ASSERT(pPage,"addChart: keine Page");
    if (!pPage)
        return;

    //	chart can't be inserted if any ole object with that name exists on any table
    //	(empty string: generate valid name)

    String aNameString = aName;
    USHORT nDummy;
    if ( aNameString.Len() && pModel->GetNamedObject( aNameString, OBJ_OLE2, nDummy ) )
    {
        //	object exists - only RuntimeException is specified
        throw uno::RuntimeException();
    }

    ScRangeList* pList = new ScRangeList;
    USHORT nRangeCount = (USHORT)aRanges.getLength();
    if (nRangeCount)
    {
        const table::CellRangeAddress* pAry = aRanges.getConstArray();
        for (USHORT i=0; i<nRangeCount; i++)
        {
            ScRange aRange( (USHORT)pAry[i].StartColumn, (USHORT)pAry[i].StartRow, pAry[i].Sheet,
                            (USHORT)pAry[i].EndColumn,   (USHORT)pAry[i].EndRow,   pAry[i].Sheet );
            pList->Append( aRange );
        }
    }
    ScRangeListRef xNewRanges( pList );

    SvStorageRef aStor = new SvStorage( String() );
    SvInPlaceObjectRef aIPObj;
    if ( SvtModuleOptions().IsChart() )
        aIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                                        *SCH_MOD()->pSchChartDocShellFactory,
                                        aStor );
    if ( aIPObj.Is() )
    {
        SvEmbeddedInfoObject* pInfoObj = pDocShell->InsertObject( aIPObj, aNameString );
        if ( pInfoObj )
        {
            String aObjName = pInfoObj->GetObjName();		// wirklich eingefuegter Name...

            //	Rechteck anpassen
            //!	Fehler/Exception, wenn leer/ungueltig ???
            Point aRectPos( aRect.X, aRect.Y );
            if (aRectPos.X() < 0) aRectPos.X() = 0;
            if (aRectPos.Y() < 0) aRectPos.Y() = 0;
            Size aRectSize( aRect.Width, aRect.Height );
            if (aRectSize.Width() <= 0) aRectSize.Width() = 5000;	// Default-Groesse
            if (aRectSize.Height() <= 0) aRectSize.Height() = 5000;
            Rectangle aInsRect( aRectPos, aRectSize );

            Size aSize = aInsRect.GetSize();
            aSize = Window::LogicToLogic( aSize, MapMode( MAP_100TH_MM ), MapMode( aIPObj->GetMapUnit() ) );
            aIPObj->SetVisAreaSize(aSize);

            Window* pWin = NULL;

            ScChartArray aParam( pDoc, xNewRanges, String() );
            aParam.SetHeaders( bColumnHeaders, bRowHeaders );
            SchMemChart* pMemChart = aParam.CreateMemChart();
            SchDLL::Update( aIPObj, pMemChart, pWin );
            delete pMemChart;

            ScChartListener* pChartListener =
                new ScChartListener( aObjName, pDoc, xNewRanges );
            pDoc->GetChartListenerCollection()->Insert( pChartListener );
            pChartListener->StartListeningTo();

            SdrOle2Obj* pObj = new SdrOle2Obj( aIPObj, aObjName, aInsRect );

            pPage->InsertObject( pObj );
            pModel->AddUndo( new SdrUndoInsertObj( *pObj ) );		//! Undo-Kommentar?

            // Dies veranlaesst Chart zum sofortigen Update
            //SvData aEmpty;
            //aIPObj->SendDataChanged( aEmpty );
            aIPObj->SendViewChanged();
        }
    }
}

void SAL_CALL ScChartsObj::removeByName( const ::rtl::OUString& aName )
                                            throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aNameString = aName;
    SdrOle2Obj* pObj = lcl_FindChartObj( pDocShell, nTab, aNameString );
    if (pObj)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDrawLayer* pModel = pDoc->GetDrawLayer();		// ist nicht 0
        SdrPage* pPage = pModel->GetPage(nTab);			// ist nicht 0

        pModel->AddUndo( new SdrUndoRemoveObj( *pObj ) );	//!	Undo-Kommentar?
        pPage->RemoveObject( pObj->GetOrdNum() );

        //!	Notify etc.???
    }
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScChartsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.table.TableChartsEnumeration")));
}

// XIndexAccess

sal_Int32 SAL_CALL ScChartsObj::getCount() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    INT32 nCount = 0;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
        if (pDrawLayer)
        {
            SdrPage* pPage = pDrawLayer->GetPage(nTab);
            DBG_ASSERT(pPage, "Page nicht gefunden");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( pObject->GetObjIdentifier() == OBJ_OLE2 && pDoc->IsChart(pObject) )
                        ++nCount;
                    pObject = aIter.Next();
                }
            }
        }
    }
    return nCount;
}

uno::Any SAL_CALL ScChartsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XTableChart> xChart = GetObjectByIndex_Impl(nIndex);
    uno::Any aAny;
    if (xChart.is())
        aAny <<= xChart;
    else
        throw lang::IndexOutOfBoundsException();
    return aAny;
}

uno::Type SAL_CALL ScChartsObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((uno::Reference<table::XTableChart>*)0);
}

sal_Bool SAL_CALL ScChartsObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCount() != 0;
}

uno::Any SAL_CALL ScChartsObj::getByName( const ::rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XTableChart> xChart = GetObjectByName_Impl(aName);
    uno::Any aAny;
    if (xChart.is())
        aAny <<= xChart;
    else
        throw container::NoSuchElementException();
    return aAny;
}

uno::Sequence< ::rtl::OUString> SAL_CALL ScChartsObj::getElementNames() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();

        long nCount = getCount();
        uno::Sequence< ::rtl::OUString> aSeq(nCount);
        ::rtl::OUString* pAry = aSeq.getArray();

        long nPos = 0;
        ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
        if (pDrawLayer)
        {
            SdrPage* pPage = pDrawLayer->GetPage(nTab);
            DBG_ASSERT(pPage, "Page nicht gefunden");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( pObject->GetObjIdentifier() == OBJ_OLE2 && pDoc->IsChart(pObject) )
                    {
                        String aName;
                        SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                        if (aIPObj.Is())
                        {
                            SvInfoObject* pInfoObj = pDocShell->Find( aIPObj );
                            if (pInfoObj)
                                aName = pInfoObj->GetObjName();
                        }
                        DBG_ASSERT(nPos<nCount, "huch, verzaehlt?");
                        pAry[nPos++] = aName;
                    }
                    pObject = aIter.Next();
                }
            }
        }
        DBG_ASSERT(nPos==nCount, "nanu, verzaehlt?");

        return aSeq;
    }
    return uno::Sequence< ::rtl::OUString>(0);
}

sal_Bool SAL_CALL ScChartsObj::hasByName( const ::rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aNameString = aName;
    return ( lcl_FindChartObj( pDocShell, nTab, aNameString ) != NULL );
}

//------------------------------------------------------------------------

ScChartObj::ScChartObj(ScDocShell* pDocSh, USHORT nT, const String& rN) :
    pDocShell( pDocSh ),
    nTab( nT ),
    aChartName( rN )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScChartObj::~ScChartObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScChartObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //!	Referenz-Update

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;		// ungueltig geworden
    }
}

void ScChartObj::GetData_Impl( ScRangeListRef& rRanges, BOOL& rColHeaders, BOOL& rRowHeaders ) const
{
    BOOL bFound = FALSE;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        SchMemChart* pOld = pDoc->FindChartData(aChartName);
        if (pOld)
        {
            ScChartArray aData(pDoc,*pOld);
            rRanges = aData.GetRangeList();
            rColHeaders = aData.HasColHeaders();
            rRowHeaders = aData.HasRowHeaders();
            bFound = TRUE;
        }
    }
    if (!bFound)		// Default
    {
        rRanges = NULL;
        rColHeaders = FALSE;
        rRowHeaders = FALSE;
    }
}

void ScChartObj::Update_Impl( const ScRangeListRef& rRanges, BOOL bColHeaders, BOOL bRowHeaders )
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        Window* pWin = NULL;
        pDoc->UpdateChartArea( aChartName, rRanges, bColHeaders, bRowHeaders, FALSE, pWin );
    }
}

// XTableChart

sal_Bool SAL_CALL ScChartObj::getHasColumnHeaders() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScRangeListRef xRanges;
    BOOL bColHeaders, bRowHeaders;
    GetData_Impl( xRanges, bColHeaders, bRowHeaders );
    return bColHeaders;
}

void SAL_CALL ScChartObj::setHasColumnHeaders( sal_Bool bHasColumnHeaders )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScRangeListRef xRanges;
    BOOL bOldColHeaders, bOldRowHeaders;
    GetData_Impl( xRanges, bOldColHeaders, bOldRowHeaders );
    if ( bOldColHeaders != bHasColumnHeaders )
        Update_Impl( xRanges, bHasColumnHeaders, bOldRowHeaders );
}

sal_Bool SAL_CALL ScChartObj::getHasRowHeaders() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScRangeListRef xRanges;
    BOOL bColHeaders, bRowHeaders;
    GetData_Impl( xRanges, bColHeaders, bRowHeaders );
    return bRowHeaders;
}

void SAL_CALL ScChartObj::setHasRowHeaders( sal_Bool bHasRowHeaders )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScRangeListRef xRanges;
    BOOL bOldColHeaders, bOldRowHeaders;
    GetData_Impl( xRanges, bOldColHeaders, bOldRowHeaders );
    if ( bOldRowHeaders != bHasRowHeaders )
        Update_Impl( xRanges, bOldColHeaders, bHasRowHeaders );
}

uno::Sequence<table::CellRangeAddress> SAL_CALL ScChartObj::getRanges() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScRangeListRef xRanges;
    BOOL bColHeaders, bRowHeaders;
    GetData_Impl( xRanges, bColHeaders, bRowHeaders );
    if ( xRanges.Is() )
    {
        ULONG nCount = xRanges->Count();

        table::CellRangeAddress aRangeAddress;
        uno::Sequence<table::CellRangeAddress> aSeq(nCount);
        table::CellRangeAddress* pAry = aSeq.getArray();
        for (USHORT i=0; i<nCount; i++)
        {
            ScRange aRange = *xRanges->GetObject(i);

            aRangeAddress.Sheet		  = aRange.aStart.Tab();
            aRangeAddress.StartColumn = aRange.aStart.Col();
            aRangeAddress.StartRow	  = aRange.aStart.Row();
            aRangeAddress.EndColumn	  = aRange.aEnd.Col();
            aRangeAddress.EndRow	  = aRange.aEnd.Row();

            pAry[i] = aRangeAddress;
        }
        return aSeq;
    }

    DBG_ERROR("ScChartObj::getRanges: keine Ranges");
    return uno::Sequence<table::CellRangeAddress>();
}

void SAL_CALL ScChartObj::setRanges( const uno::Sequence<table::CellRangeAddress>& aRanges )
                                                throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ScRangeListRef xOldRanges;
    BOOL bColHeaders, bRowHeaders;
    GetData_Impl( xOldRanges, bColHeaders, bRowHeaders );

    ScRangeList* pList = new ScRangeList;
    USHORT nRangeCount = (USHORT)aRanges.getLength();
    if (nRangeCount)
    {
        const table::CellRangeAddress* pAry = aRanges.getConstArray();
        for (USHORT i=0; i<nRangeCount; i++)
        {
            ScRange aRange( (USHORT)pAry[i].StartColumn, (USHORT)pAry[i].StartRow, pAry[i].Sheet,
                            (USHORT)pAry[i].EndColumn,   (USHORT)pAry[i].EndRow,   pAry[i].Sheet );
            pList->Append( aRange );
        }
    }
    ScRangeListRef xNewRanges( pList );

    if ( !xOldRanges.Is() || *xOldRanges != *xNewRanges )
        Update_Impl( xNewRanges, bColHeaders, bRowHeaders );
}

// XEmbeddedObjectSupplier

uno::Reference<lang::XComponent> SAL_CALL ScChartObj::getEmbeddedObject() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SdrOle2Obj* pObject = lcl_FindChartObj( pDocShell, nTab, aChartName );
    if (pObject)
    {
        SvInPlaceObjectRef aIPObj = pObject->GetObjRef();
        if (aIPObj.Is())
        {
            SfxInPlaceObjectRef aSfxObj( aIPObj );
            if (aSfxObj.Is())
            {
                SfxObjectShell* pObjSh = aSfxObj->GetObjectShell();
                if (pObjSh)
                    return pObjSh->GetBaseModel().get();
            }
        }
    }
    return NULL;
}

// XNamed

::rtl::OUString SAL_CALL ScChartObj::getName() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return aChartName;
}

void SAL_CALL ScChartObj::setName( const ::rtl::OUString& aName ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    throw uno::RuntimeException();		// name cannot be changed
}

//------------------------------------------------------------------------



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
