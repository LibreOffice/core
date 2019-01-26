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

#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/app.hxx>
#include <unotools/moduleoptions.hxx>
#include <comphelper/classids.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/globname.hxx>
#include <svx/charthelper.hxx>
#include <svtools/embedhlp.hxx>

#include <ChartTools.hxx>
#include <chartuno.hxx>
#include <miscuno.hxx>
#include <docsh.hxx>
#include <drwlayer.hxx>
#include <undodat.hxx>
#include <chartarr.hxx>
#include <chartlis.hxx>
#include <chart2uno.hxx>
#include <convuno.hxx>

using namespace css;

#define PROP_HANDLE_RELATED_CELLRANGES  1

SC_SIMPLE_SERVICE_INFO( ScChartObj, "ScChartObj", "com.sun.star.table.TableChart" )
SC_SIMPLE_SERVICE_INFO( ScChartsObj, "ScChartsObj", "com.sun.star.table.TableCharts" )

ScChartsObj::ScChartsObj(ScDocShell* pDocSh, SCTAB nT) :
    pDocShell( pDocSh ),
    nTab( nT )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScChartsObj::~ScChartsObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScChartsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //! update reference

    if ( rHint.GetId() == SfxHintId::Dying )
    {
        pDocShell = nullptr;
    }
}

ScChartObj* ScChartsObj::GetObjectByIndex_Impl(long nIndex) const
{
    OUString aName;
    if ( pDocShell )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
        if (pDrawLayer)
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            OSL_ENSURE(pPage, "Page not found");
            if (pPage)
            {
                long nPos = 0;
                SdrObjListIter aIter( pPage, SdrIterMode::DeepNoGroups );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( pObject->GetObjIdentifier() == OBJ_OLE2 && ScDocument::IsChart(pObject) )
                    {
                        if ( nPos == nIndex )
                        {
                            uno::Reference < embed::XEmbeddedObject > xObj = static_cast<SdrOle2Obj*>(pObject)->GetObjRef();
                            if ( xObj.is() )
                                aName = pDocShell->GetEmbeddedObjectContainer().GetEmbeddedObjectName( xObj );
                            break;  // stop searching
                        }
                        ++nPos;
                    }
                    pObject = aIter.Next();
                }
            }
        }
    }

    if (!aName.isEmpty())
        return new ScChartObj( pDocShell, nTab, aName );
    return nullptr;
}

ScChartObj* ScChartsObj::GetObjectByName_Impl(const OUString& aName) const
{
    if (sc::tools::findChartsByName(pDocShell, nTab, aName, sc::tools::ChartSourceType::CELL_RANGE))
        return new ScChartObj( pDocShell, nTab, aName );
    return nullptr;
}

// XTableCharts

void SAL_CALL ScChartsObj::addNewByName( const OUString& rName,
                                        const awt::Rectangle& aRect,
                                        const uno::Sequence<table::CellRangeAddress>& aRanges,
                                        sal_Bool bColumnHeaders, sal_Bool bRowHeaders )
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
        return;

    ScDocument& rDoc = pDocShell->GetDocument();
    ScDrawLayer* pModel = pDocShell->MakeDrawLayer();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"addChart: no Page");
    if (!pPage)
        return;

    //  chart can't be inserted if any ole object with that name exists on any table
    //  (empty string: generate valid name)

    OUString aName = rName;
    SCTAB nDummy;
    if ( !aName.isEmpty() && pModel->GetNamedObject( aName, OBJ_OLE2, nDummy ) )
    {
        //  object exists - only RuntimeException is specified
        throw uno::RuntimeException();
    }

    ScRangeList* pList = new ScRangeList;
    sal_Int32 nRangeCount = aRanges.getLength();
    if (nRangeCount)
    {
        const table::CellRangeAddress* pAry = aRanges.getConstArray();
        for (sal_Int32 i=0; i<nRangeCount; i++)
        {
            ScRange aRange( static_cast<SCCOL>(pAry[i].StartColumn), pAry[i].StartRow, pAry[i].Sheet,
                            static_cast<SCCOL>(pAry[i].EndColumn),   pAry[i].EndRow,   pAry[i].Sheet );
            pList->push_back( aRange );
        }
    }
    ScRangeListRef xNewRanges( pList );

    uno::Reference < embed::XEmbeddedObject > xObj;
    if ( SvtModuleOptions().IsChart() )
        xObj = pDocShell->GetEmbeddedObjectContainer().CreateEmbeddedObject( SvGlobalName( SO3_SCH_CLASSID ).GetByteSequence(), aName );
    if ( xObj.is() )
    {
            //  adjust rectangle
            //! error/exception, if empty/invalid ???
            Point aRectPos( aRect.X, aRect.Y );
            bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );
            if ( ( aRectPos.X() < 0 && !bLayoutRTL ) || ( aRectPos.X() > 0 && bLayoutRTL ) )
                aRectPos.setX( 0 );

            if (aRectPos.Y() < 0)
                aRectPos.setY( 0 );

            Size aRectSize( aRect.Width, aRect.Height );
            if (aRectSize.Width() <= 0)
                aRectSize.setWidth( 5000 );   // default size

            if (aRectSize.Height() <= 0)
                aRectSize.setHeight( 5000 );
            tools::Rectangle aInsRect( aRectPos, aRectSize );

            sal_Int64 nAspect(embed::Aspects::MSOLE_CONTENT);
            MapUnit aMapUnit(VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) ));
            Size aSize(aInsRect.GetSize());
            aSize = vcl::Window::LogicToLogic( aSize, MapMode( MapUnit::Map100thMM ), MapMode( aMapUnit ) );
            awt::Size aSz;
            aSz.Width = aSize.Width();
            aSz.Height = aSize.Height();

            // Calc -> DataProvider
            uno::Reference< chart2::data::XDataProvider > xDataProvider = new
                ScChart2DataProvider( &rDoc );
            // Chart -> DataReceiver
            uno::Reference< chart2::data::XDataReceiver > xReceiver;
            uno::Reference< embed::XComponentSupplier > xCompSupp( xObj, uno::UNO_QUERY );
            if( xCompSupp.is())
                xReceiver.set( xCompSupp->getComponent(), uno::UNO_QUERY );
            if( xReceiver.is())
            {
                OUString sRangeStr;
                xNewRanges->Format(sRangeStr, ScRefFlags::RANGE_ABS_3D, &rDoc);

                // connect
                if( !sRangeStr.isEmpty() )
                    xReceiver->attachDataProvider( xDataProvider );
                else
                    sRangeStr = "all";

                uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( pDocShell->GetModel(), uno::UNO_QUERY );
                xReceiver->attachNumberFormatsSupplier( xNumberFormatsSupplier );

                // set arguments
                uno::Sequence< beans::PropertyValue > aArgs( 4 );
                aArgs[0] = beans::PropertyValue(
                    "CellRangeRepresentation", -1,
                    uno::makeAny( sRangeStr ), beans::PropertyState_DIRECT_VALUE );
                aArgs[1] = beans::PropertyValue(
                    "HasCategories", -1,
                    uno::makeAny( bRowHeaders ), beans::PropertyState_DIRECT_VALUE );
                aArgs[2] = beans::PropertyValue(
                    "FirstCellAsLabel", -1,
                    uno::makeAny( bColumnHeaders ), beans::PropertyState_DIRECT_VALUE );
                aArgs[3] = beans::PropertyValue(
                    "DataRowSource", -1,
                    uno::makeAny( chart::ChartDataRowSource_COLUMNS ), beans::PropertyState_DIRECT_VALUE );
                xReceiver->setArguments( aArgs );
            }

            ScChartListener* pChartListener =
                new ScChartListener( aName, &rDoc, xNewRanges );
            rDoc.GetChartListenerCollection()->insert( pChartListener );
            pChartListener->StartListeningTo();

            SdrOle2Obj* pObj = new SdrOle2Obj(
                *pModel,
                ::svt::EmbeddedObjectRef(xObj, embed::Aspects::MSOLE_CONTENT),
                aName,
                aInsRect);

            // set VisArea
            if( xObj.is())
                xObj->setVisualAreaSize( nAspect, aSz );

            // #i121334# This call will change the chart's default background fill from white to transparent.
            // Add here again if this is wanted (see task description for details)
            // ChartHelper::AdaptDefaultsForChart( xObj );

            pPage->InsertObject( pObj );
            pModel->AddUndo( std::make_unique<SdrUndoInsertObj>( *pObj ) );
    }
}

void SAL_CALL ScChartsObj::removeByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    SdrOle2Obj* pObj = sc::tools::findChartsByName(pDocShell, nTab, aName, sc::tools::ChartSourceType::CELL_RANGE);
    if (pObj)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        rDoc.GetChartListenerCollection()->removeByName(aName);
        ScDrawLayer* pModel = rDoc.GetDrawLayer();     // is not zero
        SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nTab));    // is not zero

        pModel->AddUndo( std::make_unique<SdrUndoDelObj>( *pObj ) );
        pPage->RemoveObject( pObj->GetOrdNum() );

        //! Notify etc.???
    }
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScChartsObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, "com.sun.star.table.TableChartsEnumeration");
}

// XIndexAccess

sal_Int32 SAL_CALL ScChartsObj::getCount()
{
    SolarMutexGuard aGuard;
    sal_Int32 nCount = 0;
    if ( pDocShell )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
        if (pDrawLayer)
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            OSL_ENSURE(pPage, "Page not found");
            if (pPage)
            {
                SdrObjListIter aIter( pPage, SdrIterMode::DeepNoGroups );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( pObject->GetObjIdentifier() == OBJ_OLE2 && ScDocument::IsChart(pObject) )
                        ++nCount;
                    pObject = aIter.Next();
                }
            }
        }
    }
    return nCount;
}

uno::Any SAL_CALL ScChartsObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XTableChart> xChart(GetObjectByIndex_Impl(nIndex));
    if (!xChart.is())
        throw lang::IndexOutOfBoundsException();

    return uno::makeAny(xChart);
}

uno::Type SAL_CALL ScChartsObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<table::XTableChart>::get();
}

sal_Bool SAL_CALL ScChartsObj::hasElements()
{
    SolarMutexGuard aGuard;
    return getCount() != 0;
}

uno::Any SAL_CALL ScChartsObj::getByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XTableChart> xChart(GetObjectByName_Impl(aName));
    if (!xChart.is())
        throw container::NoSuchElementException();

    return uno::makeAny(xChart);
}

uno::Sequence<OUString> SAL_CALL ScChartsObj::getElementNames()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();

        long nCount = getCount();
        uno::Sequence<OUString> aSeq(nCount);
        OUString* pAry = aSeq.getArray();

        long nPos = 0;
        ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
        if (pDrawLayer)
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            OSL_ENSURE(pPage, "Page not found");
            if (pPage)
            {
                SdrObjListIter aIter( pPage, SdrIterMode::DeepNoGroups );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( pObject->GetObjIdentifier() == OBJ_OLE2 && ScDocument::IsChart(pObject) )
                    {
                        OUString aName;
                        uno::Reference < embed::XEmbeddedObject > xObj = static_cast<SdrOle2Obj*>(pObject)->GetObjRef();
                        if ( xObj.is() )
                            aName = pDocShell->GetEmbeddedObjectContainer().GetEmbeddedObjectName( xObj );

                        OSL_ENSURE(nPos<nCount, "oops, miscounted?");
                        pAry[nPos++] = aName;
                    }
                    pObject = aIter.Next();
                }
            }
        }
        OSL_ENSURE(nPos==nCount, "hey, miscounted?");

        return aSeq;
    }
    return uno::Sequence<OUString>(0);
}

sal_Bool SAL_CALL ScChartsObj::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    SdrOle2Obj* aOle2Obj = sc::tools::findChartsByName(pDocShell, nTab, aName,
                                                       sc::tools::ChartSourceType::CELL_RANGE);
    return aOle2Obj != nullptr;
}

ScChartObj::ScChartObj(ScDocShell* pDocSh, SCTAB nT, const OUString& rN)
    :ScChartObj_Base( m_aMutex )
    ,ScChartObj_PBase( ScChartObj_Base::rBHelper )
    ,pDocShell( pDocSh )
    ,nTab( nT )
    ,aChartName( rN )
{
    pDocShell->GetDocument().AddUnoObject(*this);

    registerPropertyNoMember( "RelatedCellRanges",
        PROP_HANDLE_RELATED_CELLRANGES, beans::PropertyAttribute::MAYBEVOID,
        cppu::UnoType<uno::Sequence<table::CellRangeAddress>>::get(),
        css::uno::Any(uno::Sequence<table::CellRangeAddress>()) );
}

ScChartObj::~ScChartObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScChartObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //! update reference

    if ( rHint.GetId() == SfxHintId::Dying )
    {
        pDocShell = nullptr;
    }
}

void ScChartObj::GetData_Impl( ScRangeListRef& rRanges, bool& rColHeaders, bool& rRowHeaders ) const
{
    bool bFound = false;

    if( pDocShell )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        uno::Reference< chart2::XChartDocument > xChartDoc( rDoc.GetChartByName( aChartName ) );
        if( xChartDoc.is() )
        {
            uno::Reference< chart2::data::XDataReceiver > xReceiver( xChartDoc, uno::UNO_QUERY );
            uno::Reference< chart2::data::XDataProvider > xProvider = xChartDoc->getDataProvider();
            if( xReceiver.is() && xProvider.is() )
            {
                uno::Sequence< beans::PropertyValue > aArgs( xProvider->detectArguments( xReceiver->getUsedData() ) );

                OUString aRanges;
                chart::ChartDataRowSource eDataRowSource = chart::ChartDataRowSource_COLUMNS;
                bool bHasCategories=false;
                bool bFirstCellAsLabel=false;
                const beans::PropertyValue* pPropArray = aArgs.getConstArray();
                long nPropCount = aArgs.getLength();
                for (long i = 0; i < nPropCount; i++)
                {
                    const beans::PropertyValue& rProp = pPropArray[i];
                    OUString aPropName(rProp.Name);

                    if (aPropName == "CellRangeRepresentation")
                        rProp.Value >>= aRanges;
                    else if (aPropName == "DataRowSource")
                        eDataRowSource = static_cast<chart::ChartDataRowSource>(ScUnoHelpFunctions::GetEnumFromAny( rProp.Value ));
                    else if (aPropName == "HasCategories")
                        bHasCategories = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
                    else if (aPropName == "FirstCellAsLabel")
                        bFirstCellAsLabel = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
                }

                if( chart::ChartDataRowSource_COLUMNS == eDataRowSource )
                {
                    rColHeaders=bFirstCellAsLabel;
                    rRowHeaders=bHasCategories;
                }
                else
                {
                    rColHeaders=bHasCategories;
                    rRowHeaders=bFirstCellAsLabel;
                }
                rRanges->Parse( aRanges, &rDoc);
            }
            bFound = true;
        }
     }
    if( !bFound )
    {
        rRanges = nullptr;
        rColHeaders = false;
        rRowHeaders = false;
    }
}

void ScChartObj::Update_Impl( const ScRangeListRef& rRanges, bool bColHeaders, bool bRowHeaders )
{
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        bool bUndo(rDoc.IsUndoEnabled());

        if (bUndo)
        {
            pDocShell->GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoChartData>( pDocShell, aChartName, rRanges, bColHeaders, bRowHeaders, false ) );
        }
        rDoc.UpdateChartArea( aChartName, rRanges, bColHeaders, bRowHeaders, false );
    }
}

// ::comphelper::OPropertySetHelper

::cppu::IPropertyArrayHelper& ScChartObj::getInfoHelper()
{
    return *ScChartObj_PABase::getArrayHelper();
}

void ScChartObj::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const uno::Any& rValue )
{
    switch ( nHandle )
    {
        case PROP_HANDLE_RELATED_CELLRANGES:
            {
                uno::Sequence< table::CellRangeAddress > aCellRanges;
                if ( rValue >>= aCellRanges )
                {
                    ScRangeListRef rRangeList = new ScRangeList();
                    for ( table::CellRangeAddress const & aCellRange : aCellRanges )
                    {
                        ScRange aRange;
                        ScUnoConversion::FillScRange( aRange, aCellRange );
                        rRangeList->push_back( aRange );
                    }
                    if ( pDocShell )
                    {
                        ScChartListenerCollection* pCollection = pDocShell->GetDocument().GetChartListenerCollection();
                        if ( pCollection )
                        {
                            pCollection->ChangeListening( aChartName, rRangeList );
                        }
                    }
                }
            }
            break;
        default:
            break;
    }
}

void ScChartObj::getFastPropertyValue( uno::Any& rValue, sal_Int32 nHandle ) const
{
    switch ( nHandle )
    {
        case PROP_HANDLE_RELATED_CELLRANGES:
        {
            if (!pDocShell)
                break;
            ScDocument& rDoc = pDocShell->GetDocument();

            ScChartListenerCollection* pCollection = rDoc.GetChartListenerCollection();
            if (!pCollection)
                break;

            ScChartListener* pListener = pCollection->findByName(aChartName);
            if (!pListener)
                break;

            const ScRangeListRef& rRangeList = pListener->GetRangeList();
            if (!rRangeList.is())
                break;

            size_t nCount = rRangeList->size();
            uno::Sequence<table::CellRangeAddress> aCellRanges(nCount);
            table::CellRangeAddress* pCellRanges = aCellRanges.getArray();
            for (size_t i = 0; i < nCount; ++i)
            {
                ScRange const & rRange = (*rRangeList)[i];
                table::CellRangeAddress aCellRange;
                ScUnoConversion::FillApiRange(aCellRange, rRange);
                pCellRanges[i] = aCellRange;
            }
            rValue <<= aCellRanges;
        }
        break;
        default:
            ;
    }
}

// ::comphelper::OPropertyArrayUsageHelper

::cppu::IPropertyArrayHelper* ScChartObj::createArrayHelper() const
{
    uno::Sequence< beans::Property > aProps;
    describeProperties( aProps );
    return new ::cppu::OPropertyArrayHelper( aProps );
}

// XInterface

IMPLEMENT_FORWARD_XINTERFACE2( ScChartObj, ScChartObj_Base, ScChartObj_PBase )

// XTypeProvider

IMPLEMENT_FORWARD_XTYPEPROVIDER2( ScChartObj, ScChartObj_Base, ScChartObj_PBase )

// XTableChart

sal_Bool SAL_CALL ScChartObj::getHasColumnHeaders()
{
    SolarMutexGuard aGuard;
    ScRangeListRef xRanges = new ScRangeList;
    bool bColHeaders, bRowHeaders;
    GetData_Impl( xRanges, bColHeaders, bRowHeaders );
    return bColHeaders;
}

void SAL_CALL ScChartObj::setHasColumnHeaders( sal_Bool bHasColumnHeaders )
{
    SolarMutexGuard aGuard;
    ScRangeListRef xRanges = new ScRangeList;
    bool bOldColHeaders, bOldRowHeaders;
    GetData_Impl( xRanges, bOldColHeaders, bOldRowHeaders );
    if ( bOldColHeaders != bool(bHasColumnHeaders) )
        Update_Impl( xRanges, bHasColumnHeaders, bOldRowHeaders );
}

sal_Bool SAL_CALL ScChartObj::getHasRowHeaders()
{
    SolarMutexGuard aGuard;
    ScRangeListRef xRanges = new ScRangeList;
    bool bColHeaders, bRowHeaders;
    GetData_Impl( xRanges, bColHeaders, bRowHeaders );
    return bRowHeaders;
}

void SAL_CALL ScChartObj::setHasRowHeaders( sal_Bool bHasRowHeaders )
{
    SolarMutexGuard aGuard;
    ScRangeListRef xRanges = new ScRangeList;
    bool bOldColHeaders, bOldRowHeaders;
    GetData_Impl( xRanges, bOldColHeaders, bOldRowHeaders );
    if ( bOldRowHeaders != bool(bHasRowHeaders) )
        Update_Impl( xRanges, bOldColHeaders, bHasRowHeaders );
}

uno::Sequence<table::CellRangeAddress> SAL_CALL ScChartObj::getRanges()
{
    SolarMutexGuard aGuard;
    ScRangeListRef xRanges = new ScRangeList;
    bool bColHeaders, bRowHeaders;
    GetData_Impl( xRanges, bColHeaders, bRowHeaders );
    if ( xRanges.is() )
    {
        size_t nCount = xRanges->size();

        table::CellRangeAddress aRangeAddress;
        uno::Sequence<table::CellRangeAddress> aSeq(nCount);
        table::CellRangeAddress* pAry = aSeq.getArray();
        for (size_t i = 0; i < nCount; i++)
        {
            ScRange const & rRange = (*xRanges)[i];

            aRangeAddress.Sheet       = rRange.aStart.Tab();
            aRangeAddress.StartColumn = rRange.aStart.Col();
            aRangeAddress.StartRow    = rRange.aStart.Row();
            aRangeAddress.EndColumn   = rRange.aEnd.Col();
            aRangeAddress.EndRow      = rRange.aEnd.Row();

            pAry[i] = aRangeAddress;
        }
        return aSeq;
    }

    OSL_FAIL("ScChartObj::getRanges: no Ranges");
    return uno::Sequence<table::CellRangeAddress>();
}

void SAL_CALL ScChartObj::setRanges( const uno::Sequence<table::CellRangeAddress>& aRanges )
{
    SolarMutexGuard aGuard;
    ScRangeListRef xOldRanges = new ScRangeList;
    bool bColHeaders, bRowHeaders;
    GetData_Impl( xOldRanges, bColHeaders, bRowHeaders );

    ScRangeList* pList = new ScRangeList;
    sal_uInt16 nRangeCount = static_cast<sal_uInt16>(aRanges.getLength());
    if (nRangeCount)
    {
        const table::CellRangeAddress* pAry = aRanges.getConstArray();
        for (sal_uInt16 i=0; i<nRangeCount; i++)
        {
            ScRange aRange( static_cast<SCCOL>(pAry[i].StartColumn), pAry[i].StartRow, pAry[i].Sheet,
                            static_cast<SCCOL>(pAry[i].EndColumn),   pAry[i].EndRow,   pAry[i].Sheet );
            pList->push_back( aRange );
        }
    }
    ScRangeListRef xNewRanges( pList );

    if ( !xOldRanges.is() || *xOldRanges != *xNewRanges )
        Update_Impl( xNewRanges, bColHeaders, bRowHeaders );
}

// XEmbeddedObjectSupplier

uno::Reference<lang::XComponent> SAL_CALL ScChartObj::getEmbeddedObject()
{
    SolarMutexGuard aGuard;
    SdrOle2Obj* pObject = sc::tools::findChartsByName(pDocShell, nTab, aChartName,
                                                      sc::tools::ChartSourceType::CELL_RANGE);
    if ( pObject && svt::EmbeddedObjectRef::TryRunningState( pObject->GetObjRef() ) )
    {
        //TODO/LATER: is it OK that something is returned for *all* objects, not only own objects?
        return uno::Reference < lang::XComponent > ( pObject->GetObjRef()->getComponent(), uno::UNO_QUERY );
    }

    return nullptr;
}

// XNamed

OUString SAL_CALL ScChartObj::getName()
{
    SolarMutexGuard aGuard;
    return aChartName;
}

void SAL_CALL ScChartObj::setName( const OUString& /* aName */ )
{
    SolarMutexGuard aGuard;
    throw uno::RuntimeException();      // name cannot be changed
}

// XPropertySet

uno::Reference< beans::XPropertySetInfo > ScChartObj::getPropertySetInfo()
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
