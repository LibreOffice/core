/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include "PivotChartDataProvider.hxx"
#include "PivotChartDataSource.hxx"
#include "PivotChartDataSequence.hxx"

#include <vcl/svapp.hxx>

#include "miscuno.hxx"
#include "document.hxx"
#include "unonames.hxx"
#include "docsh.hxx"

#include <sfx2/objsh.hxx>
#include <comphelper/sequence.hxx>

#include <com/sun/star/chart2/data/LabeledDataSequence.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>

#include <com/sun/star/sheet/XDataPilotResults.hpp>
#include <com/sun/star/sheet/DataResultFlags.hpp>

#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#include <com/sun/star/sheet/XLevelsSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotMemberResults.hpp>
#include <com/sun/star/sheet/MemberResultFlags.hpp>

#include "dpobject.hxx"
#include "dpsave.hxx"
#include "pivot.hxx"

#include "hints.hxx"

#include <com/sun/star/chart/ChartDataChangeEvent.hpp>

using namespace css;

namespace sc
{
namespace
{

const SfxItemPropertyMapEntry* lcl_GetDataProviderPropertyMap()
{
    static const SfxItemPropertyMapEntry aDataProviderPropertyMap_Impl[] =
    {
        { OUString(SC_UNONAME_INCLUDEHIDDENCELLS), 0, cppu::UnoType<bool>::get(), 0, 0 },
        { OUString(SC_UNONAME_USE_INTERNAL_DATA_PROVIDER), 0, cppu::UnoType<bool>::get(), 0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aDataProviderPropertyMap_Impl;
}

uno::Reference<frame::XModel> lcl_GetXModel(ScDocument * pDoc)
{
    uno::Reference<frame::XModel> xModel;
    SfxObjectShell* pObjSh(pDoc ? pDoc->GetDocumentShell() : nullptr);
    if (pObjSh)
        xModel.set(pObjSh->GetModel());
    return xModel;
}

} // end anonymous namespace

SC_SIMPLE_SERVICE_INFO( PivotChartDataProvider, "PivotChartDataProvider", "com.sun.star.chart2.data.DataProvider")

// DataProvider ==============================================================

PivotChartDataProvider::PivotChartDataProvider(ScDocument* pDoc, OUString const& sPivotTableName)
    : m_pDocument(pDoc)
    , m_sPivotTableName(sPivotTableName)
    , m_aPropSet(lcl_GetDataProviderPropertyMap())
    , m_bIncludeHiddenCells(true)
{
    if (m_pDocument)
        m_pDocument->AddUnoObject(*this);
}

PivotChartDataProvider::~PivotChartDataProvider()
{
    SolarMutexGuard g;

    if (m_pDocument)
        m_pDocument->RemoveUnoObject( *this);
}

void PivotChartDataProvider::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::Dying)
    {
        m_pDocument = nullptr;
    }
    else if (dynamic_cast<const ScDataPilotModifiedHint*>(&rHint))
    {
        if (m_pDocument)
        {
            OUString sPivotTableName = static_cast<const ScDataPilotModifiedHint&>(rHint).GetName();
            if (sPivotTableName == m_sPivotTableName)
            {
                for (uno::Reference<util::XModifyListener> & xListener : m_aValueListeners)
                {
                    css::chart::ChartDataChangeEvent aEvent(static_cast<cppu::OWeakObject*>(this), css::chart::ChartDataChangeType_ALL, 0, 0, 0, 0);
                    xListener->modified(aEvent);
                }
            }
        }
    }
}

sal_Bool SAL_CALL PivotChartDataProvider::createDataSourcePossible(const uno::Sequence<beans::PropertyValue>& /*aArguments*/)
{
    SolarMutexGuard aGuard;
    if (!m_pDocument)
        return false;
    return true;
}

uno::Reference<chart2::data::XDataSource> SAL_CALL
    PivotChartDataProvider::createDataSource(const uno::Sequence<beans::PropertyValue>& aArguments)
{
    SolarMutexGuard aGuard;
    if (!m_pDocument)
        throw uno::RuntimeException();

    bool bLabel = true;
    bool bCategories = false;
    bool bOrientCol = true;
    OUString aRangeRepresentation;
    OUString sPivotTable;
    uno::Sequence<sal_Int32> aSequenceMapping;
    bool bTimeBased = false;

    for (beans::PropertyValue const & rProperty : aArguments)
    {
        if (rProperty.Name == "DataRowSource")
        {
            chart::ChartDataRowSource eSource = chart::ChartDataRowSource_COLUMNS;
            if (!(rProperty.Value >>= eSource))
            {
                sal_Int32 nSource(0);
                if (rProperty.Value >>= nSource)
                    eSource = chart::ChartDataRowSource(nSource);
            }
            bOrientCol = (eSource == chart::ChartDataRowSource_COLUMNS);
        }
        else if (rProperty.Name == "FirstCellAsLabel")
            rProperty.Value >>= bLabel;
        else if (rProperty.Name == "HasCategories")
            rProperty.Value >>= bCategories;
        else if (rProperty.Name == "CellRangeRepresentation")
            rProperty.Value >>= aRangeRepresentation;
        else if (rProperty.Name == "SequenceMapping")
            rProperty.Value >>= aSequenceMapping;
        else if (rProperty.Name == "TimeBased")
            rProperty.Value >>= bTimeBased;
        else if (rProperty.Name == "ConnectedPivotTable")
            rProperty.Value >>= sPivotTable;
    }

    uno::Reference<chart2::data::XDataSource> xResult;

    if (aRangeRepresentation == "Categories")
        xResult = createPivotChartCategoriesDataSource(aRangeRepresentation, bOrientCol);
    else
        xResult = createPivotChartDataSource(aRangeRepresentation);

    return xResult;
}

uno::Reference<chart2::data::XLabeledDataSequence>
PivotChartDataProvider::createLabeledDataSequence(uno::Reference<uno::XComponentContext>& rContext)
{
    uno::Reference<chart2::data::XLabeledDataSequence> xResult;
    xResult.set(chart2::data::LabeledDataSequence::create(rContext), uno::UNO_QUERY_THROW);
    return xResult;
}

void PivotChartDataProvider::setLabeledDataSequenceValues(uno::Reference<chart2::data::XLabeledDataSequence> & xResult,
                                                    OUString const & sRoleValues, OUString const & sIdValues,
                                                    std::vector<PivotChartItem> const & rValues)
{
    std::unique_ptr<PivotChartDataSequence> pSequence(new PivotChartDataSequence(
            m_pDocument, m_sPivotTableName, sIdValues, rValues));
    pSequence->setRole(sRoleValues);
    xResult->setValues(uno::Reference<chart2::data::XDataSequence>(pSequence.release()));
}

void PivotChartDataProvider::setLabeledDataSequence(uno::Reference<chart2::data::XLabeledDataSequence> & xResult,
                                                    OUString const & sRoleValues, OUString const & sIdValues,
                                                    std::vector<PivotChartItem> const & rValues,
                                                    OUString const & sRoleLabel,  OUString const & sIdLabel,
                                                    std::vector<PivotChartItem> const & rLabel)
{
    setLabeledDataSequenceValues(xResult, sRoleValues, sIdValues, rValues);

    std::unique_ptr<PivotChartDataSequence> pLabelSequence(new PivotChartDataSequence(
            m_pDocument, m_sPivotTableName, sIdLabel, rLabel));
    pLabelSequence->setRole(sRoleLabel);
    xResult->setLabel(uno::Reference<chart2::data::XDataSequence>(pLabelSequence.release()));
}


enum class ArrangeDirection
{
    COLUMNS,
    ROWS
};

void lclArrange(std::vector<std::vector<PivotChartItem>>& rResult,
                std::vector<std::vector<OUString>>& rInput,
                std::vector<OUString>& rPath,
                size_t index, ArrangeDirection eDirection)
{
    if (index >= rInput.size() - 1)
    {
        if (eDirection == ArrangeDirection::COLUMNS)
        {
            for (OUString const & rLabel : rInput[index])
            {
                rPath[index] = rLabel;

                size_t i = 0;
                rResult.resize(rPath.size());
                for (auto it = rPath.begin(); it != rPath.end(); ++it)
                {
                    OUString const & rEach = *it;
                    rResult[i].push_back(PivotChartItem(rEach));
                    i++;
                }
            }
        }
        else if (eDirection == ArrangeDirection::ROWS)
        {
            size_t i = 0;
            for (OUString const & rLabel : rInput[index])
            {
                rResult.resize(rInput[index].size());

                rPath[index] = rLabel;

                for (auto it = rPath.begin(); it != rPath.end(); ++it)
                {
                    OUString const & rEach = *it;
                    rResult[i].push_back(PivotChartItem(rEach));
                }
                i++;
            }
        }
    }
    else
    {
        for (OUString const & rLabel : rInput[index])
        {
            rPath[index] = rLabel;
            lclArrange(rResult, rInput, rPath, index + 1, eDirection);
        }
    }
}

void PivotChartDataProvider::createCategories(
                    ScDPSaveData* pSaveData, bool bOrientCol,
                    uno::Reference<uno::XComponentContext>& xContext,
                    std::vector<uno::Reference<chart2::data::XLabeledDataSequence>>& rOutLabeledSequences)
{
    ArrangeDirection eDirection = bOrientCol ? ArrangeDirection::COLUMNS
                                             : ArrangeDirection::ROWS;
    if (bOrientCol)
    {
        for (std::vector<PivotChartItem> const & rCategories : m_aCategoriesColumnOrientation)
        {
            uno::Reference<chart2::data::XLabeledDataSequence> xResult = createLabeledDataSequence(xContext);
            setLabeledDataSequenceValues(xResult, "categories", "Categories", rCategories);
            rOutLabeledSequences.push_back(xResult);
        }
    }
    else
    {
        for (std::vector<PivotChartItem> const & rCategories : m_aCategoriesRowOrientation)
        {
            uno::Reference<chart2::data::XLabeledDataSequence> xResult = createLabeledDataSequence(xContext);
            setLabeledDataSequenceValues(xResult, "categories", "Categories", rCategories);
            rOutLabeledSequences.push_back(xResult);
        }
    }
}

uno::Reference<chart2::data::XDataSource> PivotChartDataProvider::createPivotChartCategoriesDataSource(OUString const & aRangeRepresentation, bool bOrientCol)
{
    uno::Reference<chart2::data::XDataSource> xDataSource;
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    if (!xContext.is())
        return xDataSource;

    std::vector<uno::Reference<chart2::data::XLabeledDataSequence>> aLabeledSequences;

    ScDPCollection* pDPs = m_pDocument->GetDPCollection();
    ScDPObject* pDPObject = pDPs->GetByName(m_sPivotTableName);
    ScDPSaveData* pSaveData = pDPObject->GetSaveData();

    createCategories(pSaveData, bOrientCol, xContext, aLabeledSequences);

    xDataSource.set(new PivotChartDataSource(aRangeRepresentation, aLabeledSequences));
    return xDataSource;
}

uno::Reference<chart2::data::XDataSource> PivotChartDataProvider::createPivotChartDataSource(OUString const & aRangeRepresentation)
{
    m_aCategoriesColumnOrientation.clear();
    m_aCategoriesRowOrientation.clear();

    uno::Reference<chart2::data::XDataSource> xDataSource;
    std::vector<uno::Reference<chart2::data::XLabeledDataSequence>> aLabeledSequences;

    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    if (!xContext.is())
        return xDataSource;

    ScDPCollection* pDPCollection = m_pDocument->GetDPCollection();
    ScDPObject* pDPObject = pDPCollection->GetByName(m_sPivotTableName);

    uno::Reference<sheet::XDimensionsSupplier> xDimensionsSupplier(pDPObject->GetSource());
    uno::Reference<sheet::XDataPilotResults> xDPResults(xDimensionsSupplier, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess(xDimensionsSupplier->getDimensions());

    std::vector<std::vector<PivotChartItem>> m_aLabels;

    for (long nDim = 0; nDim < xDims->getCount(); nDim++)
    {
        uno::Reference<uno::XInterface> xDim = ScUnoHelpFunctions::AnyToInterface(xDims->getByIndex(nDim));
        uno::Reference<beans::XPropertySet> xDimProp(xDim, uno::UNO_QUERY);
        uno::Reference<container::XNamed> xDimName(xDim, uno::UNO_QUERY);
        uno::Reference<sheet::XHierarchiesSupplier> xDimSupp(xDim, uno::UNO_QUERY);

        if (xDimProp.is() && xDimSupp.is())
        {
            sheet::DataPilotFieldOrientation eDimOrient = sheet::DataPilotFieldOrientation(
                ScUnoHelpFunctions::GetEnumProperty(xDimProp, SC_UNO_DP_ORIENTATION,
                                                    sheet::DataPilotFieldOrientation_HIDDEN));

            long nDimPos = ScUnoHelpFunctions::GetLongProperty(xDimProp, SC_UNO_DP_POSITION);
            bool bIsDataLayout = ScUnoHelpFunctions::GetBoolProperty(xDimProp, SC_UNO_DP_ISDATALAYOUT);
            bool bHasHiddenMember = ScUnoHelpFunctions::GetBoolProperty(xDimProp, SC_UNO_DP_HAS_HIDDEN_MEMBER);
            sal_Int32 nNumberFormat = ScUnoHelpFunctions::GetLongProperty(xDimProp, SC_UNO_DP_NUMBERFO);

            if (eDimOrient != sheet::DataPilotFieldOrientation_HIDDEN)
            {
                uno::Reference<container::XIndexAccess> xHiers = new ScNameToIndexAccess(xDimSupp->getHierarchies());
                long nHierarchy = ScUnoHelpFunctions::GetLongProperty(xDimProp, SC_UNO_DP_USEDHIERARCHY);
                if (nHierarchy >= xHiers->getCount())
                    nHierarchy = 0;

                uno::Reference<uno::XInterface> xHier = ScUnoHelpFunctions::AnyToInterface(xHiers->getByIndex(nHierarchy));

                uno::Reference<sheet::XLevelsSupplier> xHierSupp(xHier, uno::UNO_QUERY);
                if (xHierSupp.is())
                {
                    uno::Reference<container::XIndexAccess> xLevels = new ScNameToIndexAccess(xHierSupp->getLevels());
                    long nLevCount = xLevels->getCount();
                    for (long nLev = 0; nLev < nLevCount; nLev++)
                    {

                        uno::Reference<uno::XInterface> xLevel = ScUnoHelpFunctions::AnyToInterface(xLevels->getByIndex(nLev));
                        uno::Reference<container::XNamed> xLevNam(xLevel, uno::UNO_QUERY);
                        uno::Reference<sheet::XDataPilotMemberResults> xLevRes(xLevel, uno::UNO_QUERY );
                        if (xLevNam.is() && xLevRes.is())
                        {
                            OUString aName = xLevNam->getName();

                            switch (eDimOrient)
                            {
                                case sheet::DataPilotFieldOrientation_COLUMN:
                                {
                                    uno::Sequence<sheet::MemberResult> aSeq = xLevRes->getResults();
                                    size_t i = 0;
                                    OUString sValue;
                                    m_aLabels.resize(aSeq.getLength());
                                    for (sheet::MemberResult & rMember : aSeq)
                                    {
                                        if (rMember.Flags & sheet::MemberResultFlags::HASMEMBER ||
                                            rMember.Flags & sheet::MemberResultFlags::CONTINUE)
                                        {
                                            if (!(rMember.Flags & sheet::MemberResultFlags::CONTINUE))
                                                sValue = rMember.Caption;

                                            if (nDimPos >= m_aLabels[i].size())
                                                m_aLabels[i].resize(nDimPos + 1);
                                            m_aLabels[i][nDimPos] = PivotChartItem(sValue);

                                            i++;
                                        }
                                    }
                                    break;
                                }
                                case sheet::DataPilotFieldOrientation_ROW:
                                {
                                    uno::Sequence<sheet::MemberResult> aSeq = xLevRes->getResults();
                                    m_aCategoriesRowOrientation.resize(aSeq.getLength());
                                    size_t i = 0;
                                    for (sheet::MemberResult & rMember : aSeq)
                                    {
                                        if (rMember.Flags & sheet::MemberResultFlags::HASMEMBER ||
                                            rMember.Flags & sheet::MemberResultFlags::CONTINUE)
                                        {
                                            OUString sValue;
                                            if (!(rMember.Flags & sheet::MemberResultFlags::CONTINUE))
                                                sValue = rMember.Caption;

                                            if (nDimPos >= m_aCategoriesColumnOrientation.size())
                                                m_aCategoriesColumnOrientation.resize(nDimPos + 1);
                                            m_aCategoriesColumnOrientation[nDimPos].push_back(PivotChartItem(sValue));

                                            if (nDimPos >= m_aCategoriesRowOrientation[i].size())
                                                m_aCategoriesRowOrientation[i].resize(nDimPos + 1);
                                            m_aCategoriesRowOrientation[i][nDimPos] = PivotChartItem(sValue);

                                            i++;
                                        }
                                    }
                                    break;
                                }
                                default:
                                    break;
                            }
                        }
                    }
                }
            }
        }
    }

    {
        std::vector<PivotChartItem> aFirstCategories;
        std::copy (m_aCategoriesColumnOrientation[0].begin(),
                   m_aCategoriesColumnOrientation[0].end(),
                   std::back_inserter(aFirstCategories));

        uno::Reference<chart2::data::XLabeledDataSequence> xResult = createLabeledDataSequence(xContext);
        setLabeledDataSequenceValues(xResult, "categories", "Categories", aFirstCategories);
        aLabeledSequences.push_back(xResult);
    }

    {
        uno::Sequence<uno::Sequence<sheet::DataResult>> xDataResultsSequence = xDPResults->getResults();

        std::vector<std::vector<PivotChartItem>> aDataRowVector;

        double fNan;
        rtl::math::setNan(&fNan);

        for (uno::Sequence<sheet::DataResult> const & xDataResults : xDataResultsSequence)
        {

            size_t nIndex = 0;
            for (sheet::DataResult const & rDataResult : xDataResults)
            {
                if (rDataResult.Flags == 0 || rDataResult.Flags & css::sheet::DataResultFlags::HASDATA)
                {

                    if (nIndex >= aDataRowVector.size())
                        aDataRowVector.resize(nIndex + 1);
                    aDataRowVector[nIndex].push_back(PivotChartItem(rDataResult.Flags ? rDataResult.Value : fNan));
                }
                nIndex++;
            }
        }

        int i = 0;

        for (std::vector<PivotChartItem> const & rDataRow : aDataRowVector)
        {
            OUString aValuesId = "Data " + OUString::number(i);
            OUString aLabelsId = "Label " + OUString::number(i);

            OUString aLabel;
            bool bFirst = true;
            for (PivotChartItem const & rItem : m_aLabels[i])
            {
                if (bFirst)
                {
                    aLabel += rItem.m_aString;
                    bFirst = false;
                }
                else
                {
                    aLabel += " - " + rItem.m_aString;
                }
            }

            std::vector<PivotChartItem> aLabelVector { PivotChartItem(aLabel) };

            uno::Reference<chart2::data::XLabeledDataSequence> xResult = createLabeledDataSequence(xContext);
            setLabeledDataSequence(xResult, "values-y", aValuesId, rDataRow,
                                            "values-y", aLabelsId, aLabelVector);
            aLabeledSequences.push_back(xResult);
            i++;
        }
    }

    xDataSource.set(new PivotChartDataSource(aRangeRepresentation, aLabeledSequences));
    return xDataSource;
}


uno::Sequence<beans::PropertyValue> SAL_CALL PivotChartDataProvider::detectArguments(
            const uno::Reference< chart2::data::XDataSource >& xDataSource)
{
    uno::Sequence<beans::PropertyValue> aArguments;

    if (!m_pDocument ||!xDataSource.is())
        return aArguments;

    aArguments.realloc(4);

    aArguments[0] = beans::PropertyValue("CellRangeRepresentation", -1, uno::Any(OUString("PivotChart")),
                    beans::PropertyState_DIRECT_VALUE);

    aArguments[1] = beans::PropertyValue("DataRowSource", -1, uno::Any(chart::ChartDataRowSource_COLUMNS),
                    beans::PropertyState_DIRECT_VALUE);

    aArguments[2] = beans::PropertyValue("FirstCellAsLabel", -1, uno::Any(false),
                    beans::PropertyState_DIRECT_VALUE);

    aArguments[3] = beans::PropertyValue("HasCategories", -1, uno::Any(true),
                    beans::PropertyState_DIRECT_VALUE);

    return aArguments;
}

sal_Bool SAL_CALL PivotChartDataProvider::createDataSequenceByRangeRepresentationPossible(const OUString& aRangeRepresentation)
{
    SolarMutexGuard aGuard;
    if (!m_pDocument)
        return false;
    return true;
}

uno::Reference< chart2::data::XDataSequence > SAL_CALL
    PivotChartDataProvider::createDataSequenceByRangeRepresentation(const OUString& aRangeRepresentation)
{
    SolarMutexGuard aGuard;
    uno::Reference<chart2::data::XDataSequence> xResult;

    return xResult;
}

uno::Reference<chart2::data::XDataSequence> SAL_CALL
    PivotChartDataProvider::createDataSequenceByValueArray(const OUString& aRole,
                                                           const OUString& aRangeRepresentation)
{
    return uno::Reference<chart2::data::XDataSequence>();
}

uno::Reference<sheet::XRangeSelection> SAL_CALL PivotChartDataProvider::getRangeSelection()
{
    uno::Reference<sheet::XRangeSelection> xResult;

    uno::Reference<frame::XModel> xModel(lcl_GetXModel(m_pDocument));
    if (xModel.is())
        xResult.set(xModel->getCurrentController(), uno::UNO_QUERY);

    return xResult;
}

// XModifyBroadcaster ========================================================

void SAL_CALL PivotChartDataProvider::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    SolarMutexGuard aGuard;

    m_aValueListeners.push_back(uno::Reference<util::XModifyListener>(aListener));
}

void SAL_CALL PivotChartDataProvider::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    SolarMutexGuard aGuard;

    sal_uInt16 nCount = m_aValueListeners.size();
    for (sal_uInt16 n = nCount; n--; )
    {
        uno::Reference<util::XModifyListener>& rObj = m_aValueListeners[n];
        if (rObj == aListener)
        {
            m_aValueListeners.erase(m_aValueListeners.begin() + n);
        }
    }
}

// DataProvider XPropertySet -------------------------------------------------

uno::Reference< beans::XPropertySetInfo> SAL_CALL
PivotChartDataProvider::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( m_aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL PivotChartDataProvider::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
{
    if ( rPropertyName == SC_UNONAME_INCLUDEHIDDENCELLS )
    {
        if ( !(rValue >>= m_bIncludeHiddenCells))
            throw lang::IllegalArgumentException();
    }
    else
        throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL PivotChartDataProvider::getPropertyValue(
        const OUString& rPropertyName)
{
    uno::Any aRet;
    if ( rPropertyName == SC_UNONAME_INCLUDEHIDDENCELLS )
        aRet <<= m_bIncludeHiddenCells;
    else if (rPropertyName == SC_UNONAME_USE_INTERNAL_DATA_PROVIDER)
    {
        // This is a read-only property.
        aRet <<= m_pDocument->PastingDrawFromOtherDoc();
    }
    else
        throw beans::UnknownPropertyException();
    return aRet;
}

void SAL_CALL PivotChartDataProvider::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*xListener*/)
{
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL PivotChartDataProvider::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*rListener*/)
{
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL PivotChartDataProvider::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
{
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL PivotChartDataProvider::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/ )
{
    OSL_FAIL( "Not yet implemented" );
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
