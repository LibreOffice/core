/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chartuno.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:30:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <sch/schdll.hxx>
#include <sch/memchrt.hxx>
#include <sfx2/app.hxx>
#include <svtools/moduleoptions.hxx>
#include <sot/clsids.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include "chartuno.hxx"
#include "miscuno.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"
#include "undodat.hxx"
#include "chartarr.hxx"
#include "chartlis.hxx"
#include "unoguard.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScChartObj, "ScChartObj", "com.sun.star.table.TableChart" )
SC_SIMPLE_SERVICE_INFO( ScChartsObj, "ScChartsObj", "com.sun.star.table.TableCharts" )

//------------------------------------------------------------------------

SdrOle2Obj* lcl_FindChartObj( ScDocShell* pDocShell, SCTAB nTab, const String& rName )
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
        if (pDrawLayer)
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            DBG_ASSERT(pPage, "Page nicht gefunden");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( pObject->GetObjIdentifier() == OBJ_OLE2 && pDoc->IsChart(pObject) )
                    {
                        uno::Reference < embed::XEmbeddedObject > xObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                        if ( xObj.is() )
                        {
                            String aObjName = pDocShell->GetEmbeddedObjectContainer().GetEmbeddedObjectName( xObj );
                            if ( aObjName == rName )
                                return (SdrOle2Obj*)pObject;
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

ScChartsObj::ScChartsObj(ScDocShell* pDocSh, SCTAB nT) :
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
    //! Referenz-Update

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
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
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
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
                            uno::Reference < embed::XEmbeddedObject > xObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                            if ( xObj.is() )
                                aName = pDocShell->GetEmbeddedObjectContainer().GetEmbeddedObjectName( xObj );
                            break;  // nicht weitersuchen
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

ScChartObj* ScChartsObj::GetObjectByName_Impl(const rtl::OUString& aName) const
{
    String aNameString(aName);
    if ( lcl_FindChartObj( pDocShell, nTab, aNameString ) )
        return new ScChartObj( pDocShell, nTab, aNameString );
    return NULL;
}

// XTableCharts

void SAL_CALL ScChartsObj::addNewByName( const rtl::OUString& aName,
                                        const awt::Rectangle& aRect,
                                        const uno::Sequence<table::CellRangeAddress>& aRanges,
                                        sal_Bool bColumnHeaders, sal_Bool bRowHeaders )
                                    throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (!pDocShell)
        return;

    ScDocument* pDoc = pDocShell->GetDocument();
    ScDrawLayer* pModel = pDocShell->MakeDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"addChart: keine Page");
    if (!pPage)
        return;

    //  chart can't be inserted if any ole object with that name exists on any table
    //  (empty string: generate valid name)

    String aNameString(aName);
    SCTAB nDummy;
    if ( aNameString.Len() && pModel->GetNamedObject( aNameString, OBJ_OLE2, nDummy ) )
    {
        //  object exists - only RuntimeException is specified
        throw uno::RuntimeException();
    }

    ScRangeList* pList = new ScRangeList;
    USHORT nRangeCount = (USHORT)aRanges.getLength();
    if (nRangeCount)
    {
        const table::CellRangeAddress* pAry = aRanges.getConstArray();
        for (USHORT i=0; i<nRangeCount; i++)
        {
            ScRange aRange( static_cast<SCCOL>(pAry[i].StartColumn), pAry[i].StartRow, pAry[i].Sheet,
                            static_cast<SCCOL>(pAry[i].EndColumn),   pAry[i].EndRow,   pAry[i].Sheet );
            pList->Append( aRange );
        }
    }
    ScRangeListRef xNewRanges( pList );

    uno::Reference < embed::XEmbeddedObject > xObj;
    ::rtl::OUString aTmp( aNameString );
    if ( SvtModuleOptions().IsChart() )
        xObj = pDocShell->GetEmbeddedObjectContainer().CreateEmbeddedObject( SvGlobalName( SO3_SCH_CLASSID ).GetByteSequence(), aTmp );
    if ( xObj.is() )
    {
            String aObjName = aNameString;       // wirklich eingefuegter Name...

            //  Rechteck anpassen
            //! Fehler/Exception, wenn leer/ungueltig ???
            Point aRectPos( aRect.X, aRect.Y );
            if (aRectPos.X() < 0) aRectPos.X() = 0;
            if (aRectPos.Y() < 0) aRectPos.Y() = 0;
            Size aRectSize( aRect.Width, aRect.Height );
            if (aRectSize.Width() <= 0) aRectSize.Width() = 5000;   // Default-Groesse
            if (aRectSize.Height() <= 0) aRectSize.Height() = 5000;
            Rectangle aInsRect( aRectPos, aRectSize );

            sal_Int64 nAspect(embed::Aspects::MSOLE_CONTENT);
            MapUnit aMapUnit(VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) ));
            Size aSize(aInsRect.GetSize());
            aSize = Window::LogicToLogic( aSize, MapMode( MAP_100TH_MM ), MapMode( aMapUnit ) );
            awt::Size aSz;
            aSz.Width = aSize.Width();
            aSz.Height = aSize.Height();

            Window* pWin = NULL;

            ScChartArray aParam( pDoc, xNewRanges, String() );
            aParam.SetHeaders( bColumnHeaders, bRowHeaders );
            SchMemChart* pMemChart = aParam.CreateMemChart();
            // TODO/LATER: looks like there is no need here to update the replacement. But it should be checked.
            SchDLL::Update( xObj, pMemChart, pWin );
            delete pMemChart;

            ScChartListener* pChartListener =
                new ScChartListener( aObjName, pDoc, xNewRanges );
            pDoc->GetChartListenerCollection()->Insert( pChartListener );
            pChartListener->StartListeningTo();

            SdrOle2Obj* pObj = new SdrOle2Obj( xObj, aObjName, aInsRect );

            pPage->InsertObject( pObj );
            pModel->AddUndo( new SdrUndoInsertObj( *pObj ) );       //! Undo-Kommentar?

            // Dies veranlaesst Chart zum sofortigen Update
            //SvData aEmpty;
            //aIPObj->SendDataChanged( aEmpty );
    }
}

void SAL_CALL ScChartsObj::removeByName( const rtl::OUString& aName )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString(aName);
    SdrOle2Obj* pObj = lcl_FindChartObj( pDocShell, nTab, aNameString );
    if (pObj)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDrawLayer* pModel = pDoc->GetDrawLayer();     // ist nicht 0
        SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));    // ist nicht 0

        pModel->AddUndo( new SdrUndoRemoveObj( *pObj ) );   //! Undo-Kommentar?
        pPage->RemoveObject( pObj->GetOrdNum() );

        //! Notify etc.???
    }
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScChartsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.table.TableChartsEnumeration")));
}

// XIndexAccess

sal_Int32 SAL_CALL ScChartsObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    INT32 nCount = 0;
    if ( pDocShell )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
        if (pDrawLayer)
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
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
    ScUnoGuard aGuard;
    uno::Reference<table::XTableChart> xChart(GetObjectByIndex_Impl(nIndex));
    if (xChart.is())
        return uno::makeAny(xChart);
    else
        throw lang::IndexOutOfBoundsException();
    return uno::Any();
}

uno::Type SAL_CALL ScChartsObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<table::XTableChart>*)0);
}

sal_Bool SAL_CALL ScChartsObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCount() != 0;
}

uno::Any SAL_CALL ScChartsObj::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<table::XTableChart> xChart(GetObjectByName_Impl(aName));
    if (xChart.is())
        return uno::makeAny(xChart);
    else
        throw container::NoSuchElementException();
    return uno::Any();
}

uno::Sequence<rtl::OUString> SAL_CALL ScChartsObj::getElementNames() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();

        long nCount = getCount();
        uno::Sequence<rtl::OUString> aSeq(nCount);
        rtl::OUString* pAry = aSeq.getArray();

        long nPos = 0;
        ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
        if (pDrawLayer)
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
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
                        uno::Reference < embed::XEmbeddedObject > xObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                        if ( xObj.is() )
                            aName = pDocShell->GetEmbeddedObjectContainer().GetEmbeddedObjectName( xObj );

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
    return uno::Sequence<rtl::OUString>(0);
}

sal_Bool SAL_CALL ScChartsObj::hasByName( const rtl::OUString& aName )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString(aName);
    return ( lcl_FindChartObj( pDocShell, nTab, aNameString ) != NULL );
}

//------------------------------------------------------------------------

ScChartObj::ScChartObj(ScDocShell* pDocSh, SCTAB nT, const String& rN) :
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
    //! Referenz-Update

    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
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
    if (!bFound)        // Default
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
        BOOL bUndo(pDoc->IsUndoEnabled());
        Window* pWin = NULL;

        if (bUndo)
        {
            pDocShell->GetUndoManager()->AddUndoAction(
                new ScUndoChartData( pDocShell, aChartName, rRanges, bColHeaders, bRowHeaders, FALSE ) );
        }
        pDoc->UpdateChartArea( aChartName, rRanges, bColHeaders, bRowHeaders, FALSE, pWin );
    }
}

// XTableChart

sal_Bool SAL_CALL ScChartObj::getHasColumnHeaders() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScRangeListRef xRanges;
    BOOL bColHeaders, bRowHeaders;
    GetData_Impl( xRanges, bColHeaders, bRowHeaders );
    return bColHeaders;
}

void SAL_CALL ScChartObj::setHasColumnHeaders( sal_Bool bHasColumnHeaders )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScRangeListRef xRanges;
    BOOL bOldColHeaders, bOldRowHeaders;
    GetData_Impl( xRanges, bOldColHeaders, bOldRowHeaders );
    if ( bOldColHeaders != bHasColumnHeaders )
        Update_Impl( xRanges, bHasColumnHeaders, bOldRowHeaders );
}

sal_Bool SAL_CALL ScChartObj::getHasRowHeaders() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScRangeListRef xRanges;
    BOOL bColHeaders, bRowHeaders;
    GetData_Impl( xRanges, bColHeaders, bRowHeaders );
    return bRowHeaders;
}

void SAL_CALL ScChartObj::setHasRowHeaders( sal_Bool bHasRowHeaders )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScRangeListRef xRanges;
    BOOL bOldColHeaders, bOldRowHeaders;
    GetData_Impl( xRanges, bOldColHeaders, bOldRowHeaders );
    if ( bOldRowHeaders != bHasRowHeaders )
        Update_Impl( xRanges, bOldColHeaders, bHasRowHeaders );
}

uno::Sequence<table::CellRangeAddress> SAL_CALL ScChartObj::getRanges() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
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
            ScRange aRange(*xRanges->GetObject(i));

            aRangeAddress.Sheet       = aRange.aStart.Tab();
            aRangeAddress.StartColumn = aRange.aStart.Col();
            aRangeAddress.StartRow    = aRange.aStart.Row();
            aRangeAddress.EndColumn   = aRange.aEnd.Col();
            aRangeAddress.EndRow      = aRange.aEnd.Row();

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
    ScUnoGuard aGuard;
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
            ScRange aRange( static_cast<SCCOL>(pAry[i].StartColumn), pAry[i].StartRow, pAry[i].Sheet,
                            static_cast<SCCOL>(pAry[i].EndColumn),   pAry[i].EndRow,   pAry[i].Sheet );
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
    ScUnoGuard aGuard;
    SdrOle2Obj* pObject = lcl_FindChartObj( pDocShell, nTab, aChartName );
    if ( pObject && svt::EmbeddedObjectRef::TryRunningState( pObject->GetObjRef() ) )
    {
        //TODO/LATER: is it OK that something is returned for *all* objects, not only own objects?
        return uno::Reference < lang::XComponent > ( pObject->GetObjRef()->getComponent(), uno::UNO_QUERY );
    }

    return NULL;
}

// XNamed

rtl::OUString SAL_CALL ScChartObj::getName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return aChartName;
}

void SAL_CALL ScChartObj::setName( const rtl::OUString& aName ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    throw uno::RuntimeException();      // name cannot be changed
}

//------------------------------------------------------------------------



