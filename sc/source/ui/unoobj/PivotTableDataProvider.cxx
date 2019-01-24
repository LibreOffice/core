/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <sal/config.h>

#include <PivotTableDataProvider.hxx>
#include <PivotTableDataSource.hxx>
#include <PivotTableDataSequence.hxx>

#include <miscuno.hxx>
#include <document.hxx>
#include <unonames.hxx>
#include <docsh.hxx>
#include <scresid.hxx>
#include <globstr.hrc>
#include <strings.hrc>
#include <dpobject.hxx>
#include <hints.hxx>

#include <vcl/svapp.hxx>
#include <sfx2/objsh.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/chart2/data/LabeledDataSequence.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>

#include <com/sun/star/sheet/XDataPilotResults.hpp>
#include <com/sun/star/sheet/DataResultFlags.hpp>

#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#include <com/sun/star/sheet/XLevelsSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotMemberResults.hpp>
#include <com/sun/star/sheet/MemberResultFlags.hpp>
#include <com/sun/star/sheet/XMembersSupplier.hpp>

#include <com/sun/star/chart/ChartDataChangeEvent.hpp>

#include <unordered_map>

using namespace css;

namespace sc
{
namespace
{
const OUString constIdCategories("categories");
const OUString constIdLabel("label");
const OUString constIdData("data");

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

uno::Reference<frame::XModel> lcl_GetXModel(const ScDocument * pDoc)
{
    uno::Reference<frame::XModel> xModel;
    SfxObjectShell* pObjSh(pDoc ? pDoc->GetDocumentShell() : nullptr);
    if (pObjSh)
        xModel.set(pObjSh->GetModel());
    return xModel;
}

OUString lcl_identifierForData(sal_Int32 index)
{
    return "PT@" + constIdData + " " + OUString::number(index);
}

OUString lcl_identifierForLabel(sal_Int32 index)
{
    return "PT@" + constIdLabel + " " + OUString::number(index);
}

OUString lcl_identifierForCategories()
{
    return "PT@" + constIdCategories;
}

std::vector<OUString> lcl_getVisiblePageMembers(const uno::Reference<uno::XInterface> & xLevel)
{
    std::vector<OUString> aResult;
    if (!xLevel.is())
        return aResult;

    uno::Reference<sheet::XMembersSupplier> xMembersSupplier(xLevel, uno::UNO_QUERY);
    if (!xMembersSupplier.is())
        return aResult;

    uno::Reference<sheet::XMembersAccess> xMembersAccess = xMembersSupplier->getMembers();
    if (!xMembersAccess.is())
        return aResult;

    for (OUString const & rMemberNames : xMembersAccess->getElementNames())
    {
        uno::Reference<beans::XPropertySet> xProperties(xMembersAccess->getByName(rMemberNames), uno::UNO_QUERY);
        if (!xProperties.is())
            continue;

        OUString aCaption = ScUnoHelpFunctions::GetStringProperty(xProperties, SC_UNO_DP_LAYOUTNAME, OUString());
        if (aCaption.isEmpty())
            aCaption = rMemberNames;

        bool bVisible = ScUnoHelpFunctions::GetBoolProperty(xProperties, SC_UNO_DP_ISVISIBLE);

        if (bVisible)
            aResult.push_back(aCaption);
    }

    return aResult;
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

    bool bOrientCol = true;
    OUString aRangeRepresentation;

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
        else if (rProperty.Name == "CellRangeRepresentation")
            rProperty.Value >>= aRangeRepresentation;
    }

    uno::Reference<chart2::data::XDataSource> xResult;

    if (aRangeRepresentation == lcl_identifierForCategories())
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

uno::Reference<chart2::data::XDataSource>
PivotTableDataProvider::createCategoriesDataSource(bool bOrientationIsColumn)
{
    if (m_bNeedsUpdate)
        collectPivotTableData();

    uno::Reference<chart2::data::XDataSource> xDataSource;
    std::vector<uno::Reference<chart2::data::XLabeledDataSequence>> aLabeledSequences;

    std::vector<std::vector<ValueAndFormat>> const & rCategoriesVector = bOrientationIsColumn ? m_aCategoriesColumnOrientation
                                                                                              : m_aCategoriesRowOrientation;

    for (std::vector<ValueAndFormat> const & rCategories : rCategoriesVector)
    {
        uno::Reference<chart2::data::XLabeledDataSequence> xResult = newLabeledDataSequence();
        std::unique_ptr<PivotTableDataSequence> pSequence;
        pSequence.reset(new PivotTableDataSequence(m_pDocument, m_sPivotTableName,
                                                   lcl_identifierForCategories(), rCategories));
        pSequence->setRole("categories");
        xResult->setValues(uno::Reference<chart2::data::XDataSequence>(pSequence.release()));

        aLabeledSequences.push_back(xResult);
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
    m_aFieldOutputDescriptionMap.clear();

    uno::Reference<sheet::XDataPilotResults> xDPResults(pDPObject->GetSource(), uno::UNO_QUERY);
    uno::Sequence<uno::Sequence<sheet::DataResult>> xDataResultsSequence = xDPResults->getResults();

    double fNan;
    rtl::math::setNan(&fNan);

    std::unordered_set<size_t> aValidRowIndex;

    size_t nRowIndex = 0;
    for (uno::Sequence<sheet::DataResult> const & xDataResults : xDataResultsSequence)
    {
        std::vector<ValueAndFormat> aRow;
        bool bRowEmpty = true;
        // First pass - collect a row of valid data and track if the row is empty
        for (sheet::DataResult const & rDataResult : xDataResults)
        {
            if (rDataResult.Flags & css::sheet::DataResultFlags::SUBTOTAL)
                continue;
            if (rDataResult.Flags == 0 || rDataResult.Flags & css::sheet::DataResultFlags::HASDATA)
            {
                aRow.emplace_back(rDataResult.Flags ? rDataResult.Value : fNan, 0);
                if (rDataResult.Flags != 0) // set as valid only if we have data
                {
                    bRowEmpty = false;
                    // We need to remember all valid (non-empty) row indices
                    aValidRowIndex.insert(nRowIndex);
                }
            }
        }
        // Second pass: add to collection only non-empty rows
        if (!bRowEmpty)
        {
            size_t nColumnIndex = 0;
            for (ValueAndFormat const & aValue : aRow)
            {
                if (nColumnIndex >= m_aDataRowVector.size())
                    m_aDataRowVector.resize(nColumnIndex + 1);
                m_aDataRowVector[nColumnIndex].push_back(aValue);
                nColumnIndex++;
            }
        }
        nRowIndex++;
    }

    uno::Reference<sheet::XDimensionsSupplier> xDimensionsSupplier(pDPObject->GetSource());
    uno::Reference<container::XIndexAccess> xDims = new ScNameToIndexAccess(xDimensionsSupplier->getDimensions());

    std::unordered_map<OUString, sal_Int32> aDataFieldNumberFormatMap;
    std::vector<OUString> aDataFieldNamesVectors;

    std::unordered_map<OUString, OUString> aDataFieldCaptionNames;

    sheet::DataPilotFieldOrientation eDataFieldOrientation = sheet::DataPilotFieldOrientation_HIDDEN;

    for (sal_Int32 nDim = 0; nDim < xDims->getCount(); nDim++)
    {
        uno::Reference<uno::XInterface> xDim = ScUnoHelpFunctions::AnyToInterface(xDims->getByIndex(nDim));
        uno::Reference<beans::XPropertySet> xDimProp(xDim, uno::UNO_QUERY);
        uno::Reference<sheet::XHierarchiesSupplier> xDimSupp(xDim, uno::UNO_QUERY);

        if (!xDimProp.is() || !xDimSupp.is())
            continue;

        sheet::DataPilotFieldOrientation eDimOrient =
            ScUnoHelpFunctions::GetEnumProperty(xDimProp, SC_UNO_DP_ORIENTATION,
                                                sheet::DataPilotFieldOrientation_HIDDEN);

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

            if (xLevelName.is() && xLevelResult.is())
            {
                bool bIsDataLayout = ScUnoHelpFunctions::GetBoolProperty(xDimProp, SC_UNO_DP_ISDATALAYOUT);
                sal_Int32 nDimPos = ScUnoHelpFunctions::GetLongProperty(xDimProp, SC_UNO_DP_POSITION);
                sal_Int32 nNumberFormat = ScUnoHelpFunctions::GetLongProperty(xDimProp, SC_UNO_DP_NUMBERFO);
                bool bHasHiddenMember = ScUnoHelpFunctions::GetBoolProperty(xDimProp, SC_UNO_DP_HAS_HIDDEN_MEMBER);

                switch (eDimOrient)
                {
                    case sheet::DataPilotFieldOrientation_COLUMN:
                    {
                        m_aColumnFields.emplace_back(xLevelName->getName(), nDim, nDimPos, bHasHiddenMember);

                        uno::Sequence<sheet::MemberResult> aSequence = xLevelResult->getResults();
                        size_t i = 0;
                        OUString sCaption;
                        OUString sName;
                        for (sheet::MemberResult const & rMember : aSequence)
                        {
                            // Skip grandtotals and subtotals
                            if (rMember.Flags & sheet::MemberResultFlags::SUBTOTAL ||
                                rMember.Flags & sheet::MemberResultFlags::GRANDTOTAL)
                                    continue;
                            if (rMember.Flags & sheet::MemberResultFlags::HASMEMBER ||
                                rMember.Flags & sheet::MemberResultFlags::CONTINUE)
                            {
                                if (!(rMember.Flags & sheet::MemberResultFlags::CONTINUE))
                                {
                                    sCaption = rMember.Caption;
                                    sName = rMember.Name;
                                }

                                if (i >= m_aLabels.size())
                                    m_aLabels.resize(i + 1);

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
                        m_aRowFields.emplace_back(xLevelName->getName(), nDim, nDimPos, bHasHiddenMember);

                        uno::Sequence<sheet::MemberResult> aSequence = xLevelResult->getResults();

                        size_t i = 0;
                        size_t nEachIndex = 0;
                        std::unique_ptr<ValueAndFormat> pItem;

                        for (sheet::MemberResult const & rMember : aSequence)
                        {
                            bool bFound = aValidRowIndex.find(nEachIndex) != aValidRowIndex.end();

                            nEachIndex++;

                            bool bHasContinueFlag = rMember.Flags & sheet::MemberResultFlags::CONTINUE;

                            if (rMember.Flags & sheet::MemberResultFlags::HASMEMBER || bHasContinueFlag)
                            {
                                if (!bHasContinueFlag)
                                {
                                    // Chart2 does not use number format for labels, so use the display string.
                                    pItem.reset(new ValueAndFormat(rMember.Caption));
                                }

                                if (bFound)
                                {
                                    if (i >= m_aCategoriesRowOrientation.size())
                                        m_aCategoriesRowOrientation.resize(i + 1);

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

                                    // Set to empty so the sub categories are set to empty when they continue
                                    pItem.reset(new ValueAndFormat);
                                    i++;
                                }
                            }
                        }
                    }
                    break;

                    case sheet::DataPilotFieldOrientation_PAGE:
                    {
                        m_aPageFields.emplace_back(xLevelName->getName(), nDim, nDimPos, bHasHiddenMember);

                        // Resolve filtering
                        OUString aFieldOutputDescription;
                        if (bHasHiddenMember)
                        {
                            std::vector<OUString> aMembers = lcl_getVisiblePageMembers(xLevel);

                            if (aMembers.size() == 1)
                                aFieldOutputDescription = aMembers[0];
                            else
                                aFieldOutputDescription = ScResId(SCSTR_MULTIPLE);
                        }
                        else
                        {
                            aFieldOutputDescription = ScResId(SCSTR_ALL);
                        }
                        m_aFieldOutputDescriptionMap[nDim] = aFieldOutputDescription;
                    }
                    break;

                    case sheet::DataPilotFieldOrientation_DATA:
                    {
                        aDataFieldNumberFormatMap[xLevelName->getName()] = nNumberFormat;
                        m_aDataFields.emplace_back(xLevelName->getName(), nDim, nDimPos, bHasHiddenMember);
                    }
                    break;

                    default:
                        break;
                }
            }
        }
    }

    // Transform the name of data fields
    for (chart2::data::PivotTableFieldEntry& rDataFields : m_aDataFields)
    {
        rDataFields.Name = aDataFieldCaptionNames[rDataFields.Name];
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

    // Sort fields so it respects the order of how it is represented in the pivot table

    auto funcDimensionPositionSortCompare = [] (chart2::data::PivotTableFieldEntry const & entry1,
                                                chart2::data::PivotTableFieldEntry const & entry2)
    {
        return entry1.DimensionPositionIndex < entry2.DimensionPositionIndex;
    };

    std::sort(m_aColumnFields.begin(), m_aColumnFields.end(), funcDimensionPositionSortCompare);
    std::sort(m_aRowFields.begin(),    m_aRowFields.end(),    funcDimensionPositionSortCompare);
    std::sort(m_aPageFields.begin(),   m_aPageFields.end(),   funcDimensionPositionSortCompare);
    std::sort(m_aDataFields.begin(),   m_aDataFields.end(),   funcDimensionPositionSortCompare);

    // Mark that we updated the data
    m_bNeedsUpdate = false;
}

uno::Reference<chart2::data::XDataSequence>
PivotTableDataProvider::assignValuesToDataSequence(size_t nIndex)
{
    uno::Reference<chart2::data::XDataSequence> xDataSequence;
    if (nIndex >= m_aDataRowVector.size())
        return xDataSequence;

    OUString sDataID = lcl_identifierForData(nIndex);

    std::vector<ValueAndFormat> const & rRowOfData = m_aDataRowVector[nIndex];
    std::unique_ptr<PivotTableDataSequence> pSequence;
    pSequence.reset(new PivotTableDataSequence(m_pDocument, m_sPivotTableName, sDataID, rRowOfData));
    pSequence->setRole("values-y");
    xDataSequence.set(pSequence.release());
    return xDataSequence;
}

uno::Reference<chart2::data::XDataSequence>
PivotTableDataProvider::assignLabelsToDataSequence(size_t nIndex)
{
    uno::Reference<chart2::data::XDataSequence> xDataSequence;

    OUString sLabelID = lcl_identifierForLabel(nIndex);

    OUStringBuffer aLabel;
    bool bFirst = true;

    if (m_aLabels.empty())
    {
        aLabel = ScResId(STR_PIVOT_TOTAL);
    }
    else if (nIndex < m_aLabels.size())
    {
        for (ValueAndFormat const & rItem : m_aLabels[nIndex])
        {
            if (bFirst)
            {
                aLabel.append(rItem.m_aString);
                bFirst = false;
            }
            else
            {
                aLabel.append(" - ").append(rItem.m_aString);
            }
        }
    }

    std::vector<ValueAndFormat> aLabelVector { ValueAndFormat(aLabel.makeStringAndClear()) };

    std::unique_ptr<PivotTableDataSequence> pSequence;
    pSequence.reset(new PivotTableDataSequence(m_pDocument, m_sPivotTableName,
                                               sLabelID, aLabelVector));
    pSequence->setRole("values-y");
    xDataSequence.set(pSequence.release());
    return xDataSequence;
}

css::uno::Reference<css::chart2::data::XDataSequence>
    PivotTableDataProvider::assignFirstCategoriesToDataSequence()
{
    uno::Reference<chart2::data::XDataSequence> xDataSequence;

    if (m_aCategoriesColumnOrientation.empty())
        return xDataSequence;

    std::vector<ValueAndFormat> const & rCategories = m_aCategoriesColumnOrientation.back();

    std::unique_ptr<PivotTableDataSequence> pSequence;
    pSequence.reset(new PivotTableDataSequence(m_pDocument, m_sPivotTableName,
                                               lcl_identifierForCategories(), rCategories));
    pSequence->setRole("categories");
    xDataSequence.set(uno::Reference<chart2::data::XDataSequence>(pSequence.release()));

    return xDataSequence;
}

uno::Reference<chart2::data::XDataSource>
    PivotTableDataProvider::createValuesDataSource()
{
    if (m_bNeedsUpdate)
        collectPivotTableData();

    uno::Reference<chart2::data::XDataSource> xDataSource;
    std::vector<uno::Reference<chart2::data::XLabeledDataSequence>> aLabeledSequences;

    // Fill first sequence of categories
    {
        uno::Reference<chart2::data::XLabeledDataSequence> xResult = newLabeledDataSequence();
        xResult->setValues(assignFirstCategoriesToDataSequence());
        aLabeledSequences.push_back(xResult);
    }

    // Fill values and labels
    {
        for (size_t i = 0; i < m_aDataRowVector.size(); ++i)
        {
            uno::Reference<chart2::data::XLabeledDataSequence> xResult = newLabeledDataSequence();
            xResult->setValues(assignValuesToDataSequence(i));
            xResult->setLabel(assignLabelsToDataSequence(i));
            aLabeledSequences.push_back(xResult);
        }
    }

    xDataSource.set(new PivotTableDataSource(aLabeledSequences));
    return xDataSource;
}


uno::Sequence<beans::PropertyValue> SAL_CALL PivotTableDataProvider::detectArguments(
            const uno::Reference<chart2::data::XDataSource> & xDataSource)
{
    if (!m_pDocument ||!xDataSource.is())
        return uno::Sequence<beans::PropertyValue>();

    return comphelper::InitPropertySequence({
        { "CellRangeRepresentation", uno::Any(OUString("PivotChart")) },
        { "DataRowSource", uno::Any(chart::ChartDataRowSource_COLUMNS) },
        { "FirstCellAsLabel", uno::Any(false) },
        { "HasCategories", uno::Any(true) }
    });
}

sal_Bool SAL_CALL PivotTableDataProvider::createDataSequenceByRangeRepresentationPossible(const OUString& /*aRangeRepresentation*/)
{
    return false;
}

uno::Reference<chart2::data::XDataSequence> SAL_CALL
    PivotTableDataProvider::createDataSequenceByRangeRepresentation(const OUString& /*rRangeRepresentation*/)
{
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

// XPivotTableDataProvider ========================================================

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

sal_Bool PivotTableDataProvider::hasPivotTable()
{
    if (m_sPivotTableName.isEmpty())
        return false;

    ScDPCollection* pDPCollection = m_pDocument->GetDPCollection();
    ScDPObject* pDPObject = pDPCollection->GetByName(m_sPivotTableName);

    if (pDPObject)
        return true;

    return false;
}

uno::Reference<chart2::data::XDataSequence>
    PivotTableDataProvider::createDataSequenceOfValuesByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;

    if (m_bNeedsUpdate)
        collectPivotTableData();

    return assignValuesToDataSequence(size_t(nIndex));
}

uno::Reference<css::chart2::data::XDataSequence>
    PivotTableDataProvider::createDataSequenceOfLabelsByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;

    if (m_bNeedsUpdate)
        collectPivotTableData();

    return assignLabelsToDataSequence(size_t(nIndex));
}

uno::Reference<css::chart2::data::XDataSequence>
    PivotTableDataProvider::createDataSequenceOfCategories()
{
    SolarMutexGuard aGuard;

    if (m_bNeedsUpdate)
        collectPivotTableData();

    return assignFirstCategoriesToDataSequence();
}

OUString PivotTableDataProvider::getFieldOutputDescription(sal_Int32 nDimensionIndex)
{
    if (nDimensionIndex < 0)
        return OUString();
    return m_aFieldOutputDescriptionMap[size_t(nDimensionIndex)];
}

// XModifyBroadcaster ========================================================

void SAL_CALL PivotTableDataProvider::addModifyListener(const uno::Reference< util::XModifyListener>& aListener)
{
    SolarMutexGuard aGuard;

    m_aValueListeners.emplace_back(aListener);
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

// DataProvider XPropertySet ========================================================

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
    if (rPropertyName != SC_UNONAME_INCLUDEHIDDENCELLS)
        throw beans::UnknownPropertyException();

    if (!(rValue >>= m_bIncludeHiddenCells))
        throw lang::IllegalArgumentException();
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
