/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "PivotTableDataProvider.hxx"
#include "PivotTableDataSource.hxx"
#include "PivotTableDataSequence.hxx"

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

#include "hints.hxx"

#include <com/sun/star/chart/ChartDataChangeEvent.hpp>

#include <unordered_map>

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

OUString lcl_identifierForData(sal_Int32 index)
{
    return "Data " + OUString::number(index + 1);
}

OUString lcl_identifierForLabel(sal_Int32 index)
{
    return "Label " + OUString::number(index + 1);
}

} // end anonymous namespace

SC_SIMPLE_SERVICE_INFO(PivotTableDataProvider, "PivotTableDataProvider", SC_SERVICENAME_CHART_PIVOTTABLE_DATAPROVIDER)

// DataProvider ==============================================================

PivotTableDataProvider::PivotTableDataProvider(ScDocument* pDoc)
    : m_pDocument(pDoc)
    , m_aPropSet(lcl_GetDataProviderPropertyMap())
    , m_bIncludeHiddenCells(true)
    , m_bNeedsUpdate(true)
    , m_xContext(comphelper::getProcessComponentContext())
{
    if (m_pDocument)
        m_pDocument->AddUnoObject(*this);
}

PivotTableDataProvider::~PivotTableDataProvider()
{
    SolarMutexGuard g;

    if (m_pDocument)
        m_pDocument->RemoveUnoObject( *this);
}

void PivotTableDataProvider::Notify(SfxBroadcaster& /*rBroadcaster*/, const SfxHint& rHint)
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
                m_bNeedsUpdate = true;
                for (uno::Reference<util::XModifyListener> const & xListener : m_aValueListeners)
                {
                    css::chart::ChartDataChangeEvent aEvent(static_cast<cppu::OWeakObject*>(this),
                                                            css::chart::ChartDataChangeType_ALL,
                                                            0, 0, 0, 0);
                    xListener->modified(aEvent);
                }
            }
        }
    }
}

sal_Bool SAL_CALL PivotTableDataProvider::createDataSourcePossible(const uno::Sequence<beans::PropertyValue>& /*aArguments*/)
{
    SolarMutexGuard aGuard;
    if (!m_pDocument)
        return false;

    if (m_sPivotTableName.isEmpty())
        return false;

    ScDPCollection* pDPCollection = m_pDocument->GetDPCollection();
    return bool(pDPCollection->GetByName(m_sPivotTableName));
}

uno::Reference<chart2::data::XDataSource> SAL_CALL
    PivotTableDataProvider::createDataSource(const uno::Sequence<beans::PropertyValue>& aArguments)
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
        xResult = createCategoriesDataSource(bOrientCol);
    else
        xResult = createValuesDataSource();

    return xResult;
}

uno::Reference<chart2::data::XLabeledDataSequence>
    PivotTableDataProvider::newLabeledDataSequence()
{
    uno::Reference<chart2::data::XLabeledDataSequence> xResult;
    if (!m_xContext.is())
        return xResult;
    xResult.set(chart2::data::LabeledDataSequence::create(m_xContext), uno::UNO_QUERY_THROW);
    return xResult;
}

void PivotTableDataProvider::setLabeledDataSequenceValues(uno::Reference<chart2::data::XLabeledDataSequence> & xResult,
                                                          OUString const & sRoleValues, OUString const & sIdValues,
                                                          std::vector<ValueAndFormat> const & rValues)
{
    std::unique_ptr<PivotTableDataSequence> pSequence(
        new PivotTableDataSequence(m_pDocument, m_sPivotTableName, sIdValues, rValues));
    pSequence->setRole(sRoleValues);
    xResult->setValues(uno::Reference<chart2::data::XDataSequence>(pSequence.release()));
}

void PivotTableDataProvider::setLabeledDataSequence(uno::Reference<chart2::data::XLabeledDataSequence> & xResult,
                                                    OUString const & sRoleValues, OUString const & sIdValues,
                                                    std::vector<ValueAndFormat> const & rValues,
                                                    OUString const & sRoleLabel,  OUString const & sIdLabel,
                                                    std::vector<ValueAndFormat> const & rLabel)
{
    setLabeledDataSequenceValues(xResult, sRoleValues, sIdValues, rValues);

    std::unique_ptr<PivotTableDataSequence> pLabelSequence(
        new PivotTableDataSequence(m_pDocument, m_sPivotTableName, sIdLabel, rLabel));
    pLabelSequence->setRole(sRoleLabel);
    xResult->setLabel(uno::Reference<chart2::data::XDataSequence>(pLabelSequence.release()));
}

uno::Reference<chart2::data::XDataSource>
PivotTableDataProvider::createCategoriesDataSource(bool bOrientationIsColumn)
{
    if (m_bNeedsUpdate)
        collectPivotTableData();

    uno::Reference<chart2::data::XDataSource> xDataSource;
    std::vector<uno::Reference<chart2::data::XLabeledDataSequence>> aLabeledSequences;

    if (bOrientationIsColumn)
    {
        for (std::vector<ValueAndFormat> const & rCategories : m_aCategoriesColumnOrientation)
        {
            uno::Reference<chart2::data::XLabeledDataSequence> xResult = newLabeledDataSequence();
            setLabeledDataSequenceValues(xResult, "categories", "Categories", rCategories);
            aLabeledSequences.push_back(xResult);
        }
    }
    else
    {
        for (std::vector<ValueAndFormat> const & rCategories : m_aCategoriesRowOrientation)
        {
            uno::Reference<chart2::data::XLabeledDataSequence> xResult = newLabeledDataSequence();
            setLabeledDataSequenceValues(xResult, "categories", "Categories", rCategories);
            aLabeledSequences.push_back(xResult);
        }
    }

    xDataSource.set(new PivotTableDataSource(aLabeledSequences));
    return xDataSource;
}

void PivotTableDataProvider::collectPivotTableData()
{
    ScDPCollection* pDPCollection = m_pDocument->GetDPCollection();
    ScDPObject* pDPObject = pDPCollection->GetByName(m_sPivotTableName);
    if (!pDPObject)
        return;

    m_aCategoriesColumnOrientation.clear();
    m_aCategoriesRowOrientation.clear();
    m_aLabels.clear();
    m_aDataRowVector.clear();
    m_aColumnFields.clear();
    m_aRowFields.clear();
    m_aPageFields.clear();
    m_aDataFields.clear();

    uno::Reference<sheet::XDataPilotResults> xDPResults(pDPObject->GetSource(), uno::UNO_QUERY);
    uno::Sequence<uno::Sequence<sheet::DataResult>> xDataResultsSequence = xDPResults->getResults();

    double fNan;
    rtl::math::setNan(&fNan);

    for (uno::Sequence<sheet::DataResult> const & xDataResults : xDataResultsSequence)
    {
        size_t nIndex = 0;
        for (sheet::DataResult const & rDataResult : xDataResults)
        {
            if (rDataResult.Flags & css::sheet::DataResultFlags::SUBTOTAL)
                continue;
            if (rDataResult.Flags == 0 || rDataResult.Flags & css::sheet::DataResultFlags::HASDATA)
            {
                if (nIndex >= m_aDataRowVector.size())
                    m_aDataRowVector.resize(nIndex + 1);
                m_aDataRowVector[nIndex].push_back(ValueAndFormat(rDataResult.Flags ? rDataResult.Value : fNan, 0));
            }
            nIndex++;
        }
    }

    uno::Reference<sheet::XDimensionsSupplier> xDimensionsSupplier(pDPObject->GetSource());
    uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess(xDimensionsSupplier->getDimensions());

    std::unordered_map<OUString, sal_Int32, OUStringHash> aDataFieldNumberFormatMap;
    std::vector<OUString> aDataFieldNamesVectors;

    std::unordered_map<OUString, OUString, OUStringHash> aDataFieldCaptionNames;
    std::vector<std::pair<OUString, sal_Int32>> aDataFieldPairs;

    sheet::DataPilotFieldOrientation eDataFieldOrientation = sheet::DataPilotFieldOrientation_HIDDEN;

    for (sal_Int32 nDim = 0; nDim < xDims->getCount(); nDim++)
    {
        uno::Reference<uno::XInterface> xDim = ScUnoHelpFunctions::AnyToInterface(xDims->getByIndex(nDim));
        uno::Reference<beans::XPropertySet> xDimProp(xDim, uno::UNO_QUERY);
        uno::Reference<container::XNamed> xDimName(xDim, uno::UNO_QUERY);
        uno::Reference<sheet::XHierarchiesSupplier> xDimSupp(xDim, uno::UNO_QUERY);

        if (!xDimProp.is() || !xDimSupp.is())
            continue;

        sheet::DataPilotFieldOrientation eDimOrient = sheet::DataPilotFieldOrientation(
            ScUnoHelpFunctions::GetEnumProperty(xDimProp, SC_UNO_DP_ORIENTATION,
                                                sheet::DataPilotFieldOrientation_HIDDEN));

        if (eDimOrient == sheet::DataPilotFieldOrientation_HIDDEN)
            continue;

        uno::Reference<container::XIndexAccess> xHierarchies = new ScNameToIndexAccess(xDimSupp->getHierarchies());
        sal_Int32 nHierarchy = ScUnoHelpFunctions::GetLongProperty(xDimProp, SC_UNO_DP_USEDHIERARCHY);
        if (nHierarchy >= xHierarchies->getCount())
            nHierarchy = 0;

        uno::Reference<uno::XInterface> xHierarchy = ScUnoHelpFunctions::AnyToInterface(xHierarchies->getByIndex(nHierarchy));

        uno::Reference<sheet::XLevelsSupplier> xLevelsSupplier(xHierarchy, uno::UNO_QUERY);

        if (!xLevelsSupplier.is())
            continue;

        uno::Reference<container::XIndexAccess> xLevels = new ScNameToIndexAccess(xLevelsSupplier->getLevels());

        for (long nLevel = 0; nLevel < xLevels->getCount(); nLevel++)
        {
            uno::Reference<uno::XInterface> xLevel = ScUnoHelpFunctions::AnyToInterface(xLevels->getByIndex(nLevel));
            uno::Reference<container::XNamed> xLevelName(xLevel, uno::UNO_QUERY);
            uno::Reference<sheet::XDataPilotMemberResults> xLevelResult(xLevel, uno::UNO_QUERY );

            bool bIsDataLayout = ScUnoHelpFunctions::GetBoolProperty(xDimProp, SC_UNO_DP_ISDATALAYOUT);
            long nDimPos = ScUnoHelpFunctions::GetLongProperty(xDimProp, SC_UNO_DP_POSITION);
            sal_Int32 nNumberFormat = ScUnoHelpFunctions::GetLongProperty(xDimProp, SC_UNO_DP_NUMBERFO);

            if (xLevelName.is() && xLevelResult.is())
            {
                switch (eDimOrient)
                {
                    case sheet::DataPilotFieldOrientation_COLUMN:
                    {
                        m_aColumnFields.push_back(chart2::data::PivotTableFieldEntry{xLevelName->getName(), nDim});

                        uno::Sequence<sheet::MemberResult> aSequence = xLevelResult->getResults();
                        size_t i = 0;
                        OUString sCaption;
                        OUString sName;
                        m_aLabels.resize(aSequence.getLength());
                        for (sheet::MemberResult & rMember : aSequence)
                        {
                            if (rMember.Flags & sheet::MemberResultFlags::HASMEMBER ||
                                rMember.Flags & sheet::MemberResultFlags::CONTINUE)
                            {
                                if (!(rMember.Flags & sheet::MemberResultFlags::CONTINUE))
                                {
                                    sCaption = rMember.Caption;
                                    sName = rMember.Name;
                                }

                                if (size_t(nDimPos) >= m_aLabels[i].size())
                                    m_aLabels[i].resize(nDimPos + 1);
                                m_aLabels[i][nDimPos] = ValueAndFormat(sCaption);

                                if (bIsDataLayout)
                                {
                                    // Remember data fields to determine the number format of data
                                    aDataFieldNamesVectors.push_back(sName);
                                    eDataFieldOrientation = sheet::DataPilotFieldOrientation_COLUMN;
                                    // Remember the caption name
                                    aDataFieldCaptionNames[rMember.Name] = rMember.Caption;
                                }
                                i++;
                            }
                        }
                    }
                    break;

                    case sheet::DataPilotFieldOrientation_ROW:
                    {
                        m_aRowFields.push_back(chart2::data::PivotTableFieldEntry{xLevelName->getName(), nDim});

                        uno::Sequence<sheet::MemberResult> aSequence = xLevelResult->getResults();
                        m_aCategoriesRowOrientation.resize(aSequence.getLength());
                        size_t i = 0;
                        for (sheet::MemberResult & rMember : aSequence)
                        {
                            bool bHasContinueFlag = rMember.Flags & sheet::MemberResultFlags::CONTINUE;

                            if (rMember.Flags & sheet::MemberResultFlags::HASMEMBER || bHasContinueFlag)
                            {
                                std::unique_ptr<ValueAndFormat> pItem;

                                double fValue = rMember.Value;

                                if (rtl::math::isNan(fValue))
                                {
                                    OUString sStringValue = bHasContinueFlag ? "" : rMember.Caption;
                                    pItem.reset(new ValueAndFormat(sStringValue));
                                }
                                else
                                {
                                    if (bHasContinueFlag)
                                        pItem.reset(new ValueAndFormat());
                                    else
                                        pItem.reset(new ValueAndFormat(fValue, nNumberFormat));
                                }

                                if (size_t(nDimPos) >= m_aCategoriesColumnOrientation.size())
                                    m_aCategoriesColumnOrientation.resize(nDimPos + 1);
                                m_aCategoriesColumnOrientation[nDimPos].push_back(*pItem);

                                if (size_t(nDimPos) >= m_aCategoriesRowOrientation[i].size())
                                    m_aCategoriesRowOrientation[i].resize(nDimPos + 1);
                                m_aCategoriesRowOrientation[i][nDimPos] = *pItem;

                                if (bIsDataLayout)
                                {
                                    // Remember data fields to determine the number format of data
                                    aDataFieldNamesVectors.push_back(rMember.Name);
                                    eDataFieldOrientation = sheet::DataPilotFieldOrientation_ROW;

                                    // Remember the caption name
                                    aDataFieldCaptionNames[rMember.Name] = rMember.Caption;
                                }
                                i++;
                            }
                        }
                    }
                    break;

                    case sheet::DataPilotFieldOrientation_PAGE:
                    {
                        m_aPageFields.push_back(chart2::data::PivotTableFieldEntry{xLevelName->getName(), nDim});
                    }
                    break;

                    case sheet::DataPilotFieldOrientation_DATA:
                    {
                        aDataFieldNumberFormatMap[xLevelName->getName()] = nNumberFormat;
                        aDataFieldPairs.push_back(std::pair<OUString, sal_Int32>(xLevelName->getName(), nDim));
                    }
                    break;

                    default:
                        break;
                }
            }
        }
    }

    // Fill data field entry info
    for (std::pair<OUString, sal_Int32> & rPair : aDataFieldPairs)
    {
        m_aDataFields.push_back(chart2::data::PivotTableFieldEntry{
                                    aDataFieldCaptionNames[rPair.first],
                                    rPair.second});
    }

    // Apply number format to the data
    if (eDataFieldOrientation == sheet::DataPilotFieldOrientation_ROW)
    {
        for (std::vector<ValueAndFormat> & rDataRow : m_aDataRowVector)
        {
            size_t i = 0;
            for (ValueAndFormat & rItem : rDataRow)
            {
                OUString sName = aDataFieldNamesVectors[i];
                sal_Int32 nNumberFormat = aDataFieldNumberFormatMap[sName];
                rItem.m_nNumberFormat = nNumberFormat;
                i++;
            }
        }
    }
    else if (eDataFieldOrientation == sheet::DataPilotFieldOrientation_COLUMN)
    {
        size_t i = 0;
        for (std::vector<ValueAndFormat> & rDataRow : m_aDataRowVector)
        {
            OUString sName = aDataFieldNamesVectors[i];
            sal_Int32 nNumberFormat = aDataFieldNumberFormatMap[sName];
            for (ValueAndFormat & rItem : rDataRow)
            {
                rItem.m_nNumberFormat = nNumberFormat;
            }
            i++;
        }
    }

    m_bNeedsUpdate = false;
}

void PivotTableDataProvider::assignValuesToDataSequence(uno::Reference<chart2::data::XDataSequence> & rDataSequence,
                                                        size_t nIndex)
{
    if (nIndex >= m_aDataRowVector.size())
        return;

    OUString sDataID = lcl_identifierForData(nIndex);

    std::vector<ValueAndFormat> const & rRowOfData = m_aDataRowVector[size_t(nIndex)];
    std::unique_ptr<PivotTableDataSequence> pSequence(new PivotTableDataSequence(m_pDocument, m_sPivotTableName,
                                                                                 sDataID, rRowOfData));
    pSequence->setRole("values-y");
    rDataSequence.set(uno::Reference<chart2::data::XDataSequence>(pSequence.release()));
}

void PivotTableDataProvider::assignLabelsToDataSequence(uno::Reference<chart2::data::XDataSequence> & rDataSequence,
                                                        size_t nIndex)
{
    if (nIndex >= m_aLabels.size())
        return;

    OUString sLabelID = lcl_identifierForLabel(nIndex);

    OUString aLabel;
    bool bFirst = true;
    for (ValueAndFormat const & rItem : m_aLabels[size_t(nIndex)])
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

    std::vector<ValueAndFormat> aLabelVector { ValueAndFormat(aLabel) };

    std::unique_ptr<PivotTableDataSequence> pSequence(new PivotTableDataSequence(m_pDocument, m_sPivotTableName,
                                                                                 sLabelID, aLabelVector));
    pSequence->setRole("values-y");
    rDataSequence.set(uno::Reference<chart2::data::XDataSequence>(pSequence.release()));
}

uno::Reference<chart2::data::XDataSource>
    PivotTableDataProvider::createValuesDataSource()
{
    if (m_bNeedsUpdate)
        collectPivotTableData();

    uno::Reference<chart2::data::XDataSource> xDataSource;
    std::vector<uno::Reference<chart2::data::XLabeledDataSequence>> aLabeledSequences;

    {
        std::vector<ValueAndFormat> aFirstCategories;
        if (!m_aCategoriesColumnOrientation.empty())
        {
            std::copy(m_aCategoriesColumnOrientation[0].begin(),
                      m_aCategoriesColumnOrientation[0].end(),
                      std::back_inserter(aFirstCategories));
        }
        uno::Reference<chart2::data::XLabeledDataSequence> xResult = newLabeledDataSequence();
        setLabeledDataSequenceValues(xResult, "categories", "Categories", aFirstCategories);
        aLabeledSequences.push_back(xResult);
    }

    {
        int i = 0;
        for (std::vector<ValueAndFormat> const & rRowOfData : m_aDataRowVector)
        {
            OUString aValuesId = lcl_identifierForData(i);
            OUString aLabelsId = lcl_identifierForLabel(i);

            OUString aLabel;
            bool bFirst = true;
            for (ValueAndFormat const & rItem : m_aLabels[i])
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

            std::vector<ValueAndFormat> aLabelVector { ValueAndFormat(aLabel) };

            uno::Reference<chart2::data::XLabeledDataSequence> xResult = newLabeledDataSequence();
            setLabeledDataSequence(xResult, "values-y", aValuesId, rRowOfData,
                                            "values-y", aLabelsId, aLabelVector);
            aLabeledSequences.push_back(xResult);
            i++;
        }
    }

    xDataSource.set(new PivotTableDataSource(aLabeledSequences));
    return xDataSource;
}


uno::Sequence<beans::PropertyValue> SAL_CALL PivotTableDataProvider::detectArguments(
            const uno::Reference<chart2::data::XDataSource> & xDataSource)
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

sal_Bool SAL_CALL PivotTableDataProvider::createDataSequenceByRangeRepresentationPossible(const OUString& /*aRangeRepresentation*/)
{
    SolarMutexGuard aGuard;
    return false;
}

uno::Reference<chart2::data::XDataSequence> SAL_CALL
    PivotTableDataProvider::createDataSequenceByRangeRepresentation(const OUString& /*rRangeRepresentation*/)
{
    SolarMutexGuard aGuard;
    uno::Reference<chart2::data::XDataSequence> xDataSequence;
    return xDataSequence;
}

uno::Reference<chart2::data::XDataSequence> SAL_CALL
    PivotTableDataProvider::createDataSequenceByValueArray(const OUString& /*aRole*/,
                                                           const OUString& /*aRangeRepresentation*/)
{
    return uno::Reference<chart2::data::XDataSequence>();
}

uno::Reference<sheet::XRangeSelection> SAL_CALL PivotTableDataProvider::getRangeSelection()
{
    uno::Reference<sheet::XRangeSelection> xResult;

    uno::Reference<frame::XModel> xModel(lcl_GetXModel(m_pDocument));
    if (xModel.is())
        xResult.set(xModel->getCurrentController(), uno::UNO_QUERY);

    return xResult;
}

uno::Sequence<chart2::data::PivotTableFieldEntry> PivotTableDataProvider::getColumnFields()
{
    return comphelper::containerToSequence(m_aColumnFields);
}

uno::Sequence<chart2::data::PivotTableFieldEntry> PivotTableDataProvider::getRowFields()
{
    return comphelper::containerToSequence(m_aRowFields);
}

uno::Sequence<chart2::data::PivotTableFieldEntry> PivotTableDataProvider::getPageFields()
{
    return comphelper::containerToSequence(m_aPageFields);
}

uno::Sequence<chart2::data::PivotTableFieldEntry> PivotTableDataProvider::getDataFields()
{
    return comphelper::containerToSequence(m_aDataFields);
}

OUString PivotTableDataProvider::getPivotTableName()
{
    return m_sPivotTableName;
}

void PivotTableDataProvider::setPivotTableName(const OUString& sPivotTableName)
{
    ScDPCollection* pDPCollection = m_pDocument->GetDPCollection();
    ScDPObject* pDPObject = pDPCollection->GetByName(sPivotTableName);
    if (pDPObject)
        m_sPivotTableName = sPivotTableName;
}

uno::Reference<chart2::data::XDataSequence>
    PivotTableDataProvider::createDataSequenceOfValuesByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;

    if (m_bNeedsUpdate)
        collectPivotTableData();

    uno::Reference<chart2::data::XDataSequence> xDataSequence;
    assignValuesToDataSequence(xDataSequence, size_t(nIndex));
    return xDataSequence;
}

uno::Reference<css::chart2::data::XDataSequence>
    PivotTableDataProvider::createDataSequenceOfLabelsByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;

    if (m_bNeedsUpdate)
        collectPivotTableData();

    uno::Reference<chart2::data::XDataSequence> xDataSequence;
    assignLabelsToDataSequence(xDataSequence, size_t(nIndex));
    return xDataSequence;
}

uno::Reference<css::chart2::data::XDataSequence>
    PivotTableDataProvider::createDataSequenceOfCategories()
{
    SolarMutexGuard aGuard;

    if (m_bNeedsUpdate)
        collectPivotTableData();

    uno::Reference<chart2::data::XDataSequence> xDataSequence;

    if (m_aCategoriesColumnOrientation.empty())
        return xDataSequence;

    std::vector<ValueAndFormat> const & rCategories = m_aCategoriesColumnOrientation[0];

    std::unique_ptr<PivotTableDataSequence> pSequence(new PivotTableDataSequence(m_pDocument, m_sPivotTableName,
                                                                                 "Categories", rCategories));
    pSequence->setRole("categories");
    xDataSequence.set(uno::Reference<chart2::data::XDataSequence>(pSequence.release()));

    return xDataSequence;
}

// XModifyBroadcaster ========================================================

void SAL_CALL PivotTableDataProvider::addModifyListener(const uno::Reference< util::XModifyListener>& aListener)
{
    SolarMutexGuard aGuard;

    m_aValueListeners.push_back(uno::Reference<util::XModifyListener>(aListener));
}

void SAL_CALL PivotTableDataProvider::removeModifyListener(const uno::Reference<util::XModifyListener>& aListener )
{
    SolarMutexGuard aGuard;

    sal_uInt16 nCount = m_aValueListeners.size();
    for (sal_uInt16 n = nCount; n--;)
    {
        uno::Reference<util::XModifyListener>& rObject = m_aValueListeners[n];
        if (rObject == aListener)
        {
            m_aValueListeners.erase(m_aValueListeners.begin() + n);
        }
    }
}

// DataProvider XPropertySet -------------------------------------------------

uno::Reference< beans::XPropertySetInfo> SAL_CALL
    PivotTableDataProvider::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( m_aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL PivotTableDataProvider::setPropertyValue(const OUString& rPropertyName, const uno::Any& rValue)
{
    if (rPropertyName == SC_UNONAME_INCLUDEHIDDENCELLS)
    {
        if (!(rValue >>= m_bIncludeHiddenCells))
            throw lang::IllegalArgumentException();
    }
    else
        throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL PivotTableDataProvider::getPropertyValue(const OUString& rPropertyName)
{
    uno::Any aRet;
    if (rPropertyName == SC_UNONAME_INCLUDEHIDDENCELLS)
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

void SAL_CALL PivotTableDataProvider::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference<beans::XPropertyChangeListener>& /*xListener*/)
{
    OSL_FAIL("Not yet implemented");
}

void SAL_CALL PivotTableDataProvider::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference<beans::XPropertyChangeListener>& /*rListener*/)
{
    OSL_FAIL("Not yet implemented");
}

void SAL_CALL PivotTableDataProvider::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference<beans::XVetoableChangeListener>& /*rListener*/)
{
    OSL_FAIL("Not yet implemented");
}

void SAL_CALL PivotTableDataProvider::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference<beans::XVetoableChangeListener>& /*rListener*/ )
{
    OSL_FAIL("Not yet implemented");
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
