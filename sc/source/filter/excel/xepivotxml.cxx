/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <xepivotxml.hxx>
#include <dpcache.hxx>
#include <pivot/PivotTableFormats.hxx>
#include <dpdimsave.hxx>
#include <dpitemdata.hxx>
#include <dpobject.hxx>
#include <dpsave.hxx>
#include <dputil.hxx>
#include <document.hxx>
#include <generalfunction.hxx>
#include <unonames.hxx>
#include <xestyle.hxx>
#include <xeroot.hxx>

#include <o3tl/temporary.hxx>
#include <o3tl/safeint.hxx>
#include <oox/export/utils.hxx>
#include <oox/token/namespaces.hxx>
#include <sal/log.hxx>
#include <sax/tools/converter.hxx>
#include <sax/fastattribs.hxx>
#include <sax/fshelper.hxx>
#include <svl/numformat.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutMode.hpp>
#include <com/sun/star/sheet/DataPilotOutputRangeType.hpp>
#include <com/sun/star/sheet/XDimensionsSupplier.hpp>

#include <vector>

using namespace oox;
using namespace com::sun::star;

namespace {

void savePivotCacheRecordsXml( XclExpXmlStream& rStrm, const ScDPCache& rCache )
{
    SCROW nCount = rCache.GetDataSize();
    size_t nFieldCount = rCache.GetFieldCount();

    sax_fastparser::FSHelperPtr& pRecStrm = rStrm.GetCurrentStream();
    pRecStrm->startElement(XML_pivotCacheRecords,
        XML_xmlns, rStrm.getNamespaceURL(OOX_NS(xls)).toUtf8(),
        FSNS(XML_xmlns, XML_r), rStrm.getNamespaceURL(OOX_NS(officeRel)).toUtf8(),
        XML_count, OString::number(static_cast<tools::Long>(nCount)));

    for (SCROW i = 0; i < nCount; ++i)
    {
        pRecStrm->startElement(XML_r);
        for (size_t nField = 0; nField < nFieldCount; ++nField)
        {
            const ScDPCache::IndexArrayType* pArray = rCache.GetFieldIndexArray(nField);
            assert(pArray);
            assert(o3tl::make_unsigned(i) < pArray->size());

            // We are using XML_x reference (like: <x v="0"/>), instead of values here (eg: <s v="No Discount"/>).
            // That's why in SavePivotCacheXml method, we need to list all items.
            pRecStrm->singleElement(XML_x, XML_v, OString::number((*pArray)[i]));
        }
        pRecStrm->endElement(XML_r);
    }

    pRecStrm->endElement(XML_pivotCacheRecords);
}

const char* toOOXMLAxisType( sheet::DataPilotFieldOrientation eOrient )
{
    switch (eOrient)
    {
        case sheet::DataPilotFieldOrientation_COLUMN:
            return "axisCol";
        case sheet::DataPilotFieldOrientation_ROW:
            return "axisRow";
        case sheet::DataPilotFieldOrientation_PAGE:
            return "axisPage";
        case sheet::DataPilotFieldOrientation_DATA:
            return "axisValues";
        case sheet::DataPilotFieldOrientation_HIDDEN:
        default:
            ;
    }

    return "";
}

const char* toOOXMLSubtotalType(ScGeneralFunction eFunc)
{
    switch (eFunc)
    {
        case ScGeneralFunction::SUM:
            return "sum";
        case ScGeneralFunction::COUNT:
            return "count";
        case ScGeneralFunction::AVERAGE:
            return "average";
        case ScGeneralFunction::MAX:
            return "max";
        case ScGeneralFunction::MIN:
            return "min";
        case ScGeneralFunction::PRODUCT:
            return "product";
        case ScGeneralFunction::COUNTNUMS:
            return "countNums";
        case ScGeneralFunction::STDEV:
            return "stdDev";
        case ScGeneralFunction::STDEVP:
            return "stdDevp";
        case ScGeneralFunction::VAR:
            return "var";
        case ScGeneralFunction::VARP:
            return "varp";
        default:
            ;
    }
    return nullptr;
}

}

XclExpXmlPivotCaches::XclExpXmlPivotCaches( const XclExpRoot& rRoot ) :
    XclExpRoot(rRoot) {}

void XclExpXmlPivotCaches::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& pWorkbookStrm = rStrm.GetCurrentStream();
    pWorkbookStrm->startElement(XML_pivotCaches);

    for (size_t i = 0, n = maCaches.size(); i < n; ++i)
    {
        const Entry& rEntry = maCaches[i];

        sal_Int32 nCacheId = i + 1;
        OUString aRelId;
        sax_fastparser::FSHelperPtr pPCStrm = rStrm.CreateOutputStream(
            XclXmlUtils::GetStreamName("xl/pivotCache/", "pivotCacheDefinition", nCacheId),
            XclXmlUtils::GetStreamName(nullptr, "pivotCache/pivotCacheDefinition", nCacheId),
            rStrm.GetCurrentStream()->getOutputStream(),
            CREATE_XL_CONTENT_TYPE("pivotCacheDefinition"),
            CREATE_OFFICEDOC_RELATION_TYPE("pivotCacheDefinition"),
            &aRelId);

        pWorkbookStrm->singleElement(XML_pivotCache,
            XML_cacheId, OString::number(nCacheId),
            FSNS(XML_r, XML_id), aRelId.toUtf8());

        rStrm.PushStream(pPCStrm);
        SavePivotCacheXml(rStrm, rEntry, nCacheId);
        rStrm.PopStream();
    }

    pWorkbookStrm->endElement(XML_pivotCaches);
}

void XclExpXmlPivotCaches::SetCaches( std::vector<Entry>&& rCaches )
{
    maCaches = std::move(rCaches);
}

bool XclExpXmlPivotCaches::HasCaches() const
{
    return !maCaches.empty();
}

const XclExpXmlPivotCaches::Entry* XclExpXmlPivotCaches::GetCache( sal_Int32 nCacheId ) const
{
    if (nCacheId <= 0)
        // cache ID is 1-based.
        return nullptr;

    size_t nPos = nCacheId - 1;
    if (nPos >= maCaches.size())
        return nullptr;

    return &maCaches[nPos];
}

namespace {
/**
 * Create combined date and time string according the requirements of Excel.
 * A single point in time can be represented by concatenating a complete date expression,
 * the letter T as a delimiter, and a valid time expression. For example, "2007-04-05T14:30".
 *
 * fSerialDateTime - a number representing the number of days since 1900-Jan-0 (integer portion of the number),
 * plus a fractional portion of a 24 hour day (fractional portion of the number).
 */
OUString GetExcelFormattedDate( double fSerialDateTime, const SvNumberFormatter& rFormatter )
{
    // tdf#125055: properly round the value to seconds when truncating nanoseconds below
    constexpr double fHalfSecond = 1 / 86400.0 * 0.5;
    css::util::DateTime aUDateTime
        = (DateTime(rFormatter.GetNullDate()) + fSerialDateTime + fHalfSecond).GetUNODateTime();
    // We need to reset nanoseconds, to avoid string like: "1982-02-18T16:04:47.999999849"
    aUDateTime.NanoSeconds = 0;
    OUStringBuffer sBuf;
    ::sax::Converter::convertDateTime(sBuf, aUDateTime, nullptr, true);
    return sBuf.makeStringAndClear();
}

// Excel seems to expect different order of group item values; we need to rearrange elements
// to output "<date1" first, then elements, then ">date2" last.
// Since ScDPItemData::DateFirst is -1, ScDPItemData::DateLast is 10000, and other date group
// items would fit between those in order (like 0 = Jan, 1 = Feb, etc.), we can simply sort
// the items by value.
std::vector<OUString> SortGroupItems(const ScDPCache& rCache, tools::Long nDim)
{
    struct ItemData
    {
        sal_Int32 nVal;
        const ScDPItemData* pData;
    };
    std::vector<ItemData> aDataToSort;
    ScfInt32Vec aGIIds;
    rCache.GetGroupDimMemberIds(nDim, aGIIds);
    for (sal_Int32 id : aGIIds)
    {
        const ScDPItemData* pGIData = rCache.GetItemDataById(nDim, id);
        if (pGIData->GetType() == ScDPItemData::GroupValue)
        {
            auto aGroupVal = pGIData->GetGroupValue();
            aDataToSort.push_back({ aGroupVal.mnValue, pGIData });
        }
    }
    std::sort(aDataToSort.begin(), aDataToSort.end(),
              [](const ItemData& a, const ItemData& b) { return a.nVal < b.nVal; });

    std::vector<OUString> aSortedResult;
    for (const auto& el : aDataToSort)
    {
        aSortedResult.push_back(rCache.GetFormattedString(nDim, *el.pData, false));
    }
    return aSortedResult;
}
} // namespace

void XclExpXmlPivotCaches::SavePivotCacheXml( XclExpXmlStream& rStrm, const Entry& rEntry, sal_Int32 nCounter )
{
    assert(rEntry.mpCache);
    const ScDPCache& rCache = *rEntry.mpCache;

    sax_fastparser::FSHelperPtr& pDefStrm = rStrm.GetCurrentStream();

    OUString aRelId;
    sax_fastparser::FSHelperPtr pRecStrm = rStrm.CreateOutputStream(
        XclXmlUtils::GetStreamName("xl/pivotCache/", "pivotCacheRecords", nCounter),
        XclXmlUtils::GetStreamName(nullptr, "pivotCacheRecords", nCounter),
        pDefStrm->getOutputStream(),
        CREATE_XL_CONTENT_TYPE("pivotCacheRecords"),
        CREATE_OFFICEDOC_RELATION_TYPE("pivotCacheRecords"),
        &aRelId);

    rStrm.PushStream(pRecStrm);
    savePivotCacheRecordsXml(rStrm, rCache);
    rStrm.PopStream();

    pDefStrm->startElement(XML_pivotCacheDefinition,
        XML_xmlns, rStrm.getNamespaceURL(OOX_NS(xls)).toUtf8(),
        FSNS(XML_xmlns, XML_r), rStrm.getNamespaceURL(OOX_NS(officeRel)).toUtf8(),
        FSNS(XML_r, XML_id), aRelId.toUtf8(),
        XML_recordCount, OString::number(rEntry.mpCache->GetDataSize()),
        XML_createdVersion, "3"); // MS Excel 2007, tdf#112936: setting version number makes MSO to handle the pivot table differently

    pDefStrm->startElement(XML_cacheSource, XML_type, "worksheet");

    OUString aSheetName;
    GetDoc().GetName(rEntry.maSrcRange.aStart.Tab(), aSheetName);
    pDefStrm->singleElement(XML_worksheetSource,
        XML_ref, XclXmlUtils::ToOString(rStrm.GetRoot().GetDoc(), rEntry.maSrcRange),
        XML_sheet, aSheetName.toUtf8());

    pDefStrm->endElement(XML_cacheSource);

    size_t nCount = rCache.GetFieldCount();
    const size_t nGroupFieldCount = rCache.GetGroupFieldCount();
    pDefStrm->startElement(XML_cacheFields,
        XML_count, OString::number(static_cast<tools::Long>(nCount + nGroupFieldCount)));

    auto WriteFieldGroup = [this, &rCache, pDefStrm](size_t i, size_t base) {
        const sal_Int32 nDatePart = rCache.GetGroupType(i);
        if (!nDatePart)
            return;
        OString sGroupBy;
        switch (nDatePart)
        {
        case sheet::DataPilotFieldGroupBy::SECONDS:
            sGroupBy = "seconds"_ostr;
            break;
        case sheet::DataPilotFieldGroupBy::MINUTES:
            sGroupBy = "minutes"_ostr;
            break;
        case sheet::DataPilotFieldGroupBy::HOURS:
            sGroupBy = "hours"_ostr;
            break;
        case sheet::DataPilotFieldGroupBy::DAYS:
            sGroupBy = "days"_ostr;
            break;
        case sheet::DataPilotFieldGroupBy::MONTHS:
            sGroupBy = "months"_ostr;
            break;
        case sheet::DataPilotFieldGroupBy::QUARTERS:
            sGroupBy = "quarters"_ostr;
            break;
        case sheet::DataPilotFieldGroupBy::YEARS:
            sGroupBy = "years"_ostr;
            break;
        }

        // fieldGroup element
        pDefStrm->startElement(XML_fieldGroup, XML_base, OString::number(base));

        SvNumberFormatter& rFormatter = GetFormatter();

        // rangePr element
        const ScDPNumGroupInfo* pGI = rCache.GetNumGroupInfo(i);
        auto pGroupAttList = sax_fastparser::FastSerializerHelper::createAttrList();
        pGroupAttList->add(XML_groupBy, sGroupBy);
        // Possible TODO: find out when to write autoStart attribute for years grouping
        pGroupAttList->add(XML_startDate, GetExcelFormattedDate(pGI->mfStart, rFormatter).toUtf8());
        pGroupAttList->add(XML_endDate, GetExcelFormattedDate(pGI->mfEnd, rFormatter).toUtf8());
        if (pGI->mfStep)
            pGroupAttList->add(XML_groupInterval, OString::number(pGI->mfStep));
        pDefStrm->singleElement(XML_rangePr, pGroupAttList);

        // groupItems element
        auto aElemVec = SortGroupItems(rCache, i);
        pDefStrm->startElement(XML_groupItems, XML_count, OString::number(aElemVec.size()));
        for (const auto& sElem : aElemVec)
        {
            pDefStrm->singleElement(XML_s, XML_v, sElem.toUtf8());
        }
        pDefStrm->endElement(XML_groupItems);
        pDefStrm->endElement(XML_fieldGroup);
    };

    for (size_t i = 0; i < nCount; ++i)
    {
        OUString aName = rCache.GetDimensionName(i);

        pDefStrm->startElement(XML_cacheField,
            XML_name, aName.toUtf8(),
            XML_numFmtId, OString::number(0));

        const ScDPCache::ScDPItemDataVec& rFieldItems = rCache.GetDimMemberValues(i);

        std::set<ScDPItemData::Type> aDPTypes;
        double fMin = std::numeric_limits<double>::infinity(), fMax = -std::numeric_limits<double>::infinity();
        bool isValueInteger = true;
        bool isContainsDate = rCache.IsDateDimension(i);
        bool isLongText = false;
        for (const auto& rFieldItem : rFieldItems)
        {
            ScDPItemData::Type eType = rFieldItem.GetType();
            // tdf#123939 : error and string are same for cache; if both are present, keep only one
            if (eType == ScDPItemData::Error)
                eType = ScDPItemData::String;
            aDPTypes.insert(eType);
            if (eType == ScDPItemData::Value)
            {
                double fVal = rFieldItem.GetValue();
                fMin = std::min(fMin, fVal);
                fMax = std::max(fMax, fVal);

                // Check if all values are integers
                if (isValueInteger && (modf(fVal, &o3tl::temporary(double())) != 0.0))
                {
                    isValueInteger = false;
                }
            }
            else if (eType == ScDPItemData::String && !isLongText)
            {
                isLongText = rFieldItem.GetString().getLength() > 255;
            }
        }

        auto pAttList = sax_fastparser::FastSerializerHelper::createAttrList();
        // TODO In same cases, disable listing of items, as it is done in MS Excel.
        // Exporting savePivotCacheRecordsXml method needs to be updated accordingly
        //bool bListItems = true;

        std::set<ScDPItemData::Type> aDPTypesWithoutBlank = aDPTypes;
        aDPTypesWithoutBlank.erase(ScDPItemData::Empty);

        const bool isContainsString = aDPTypesWithoutBlank.count(ScDPItemData::String) > 0;
        const bool isContainsBlank = aDPTypes.count(ScDPItemData::Empty) > 0;
        const bool isContainsNumber
            = !isContainsDate && aDPTypesWithoutBlank.count(ScDPItemData::Value) > 0;
        bool isContainsNonDate = !(isContainsDate && aDPTypesWithoutBlank.size() <= 1);

        // XML_containsSemiMixedTypes possible values:
        // 1 - (Default) at least one text value, or can also contain a mix of other data types and blank values,
        //     or blank values only
        // 0 - the field does not have a mix of text and other values
        if (!(isContainsString || (aDPTypes.size() > 1) || (isContainsBlank && aDPTypesWithoutBlank.empty())))
            pAttList->add(XML_containsSemiMixedTypes, ToPsz10(false));

        if (!isContainsNonDate)
            pAttList->add(XML_containsNonDate, ToPsz10(false));

        if (isContainsDate)
            pAttList->add(XML_containsDate, ToPsz10(true));

        // default for containsString field is true, so we are writing only when is false
        if (!isContainsString)
            pAttList->add(XML_containsString, ToPsz10(false));

        if (isContainsBlank)
            pAttList->add(XML_containsBlank, ToPsz10(true));

        // XML_containsMixedType possible values:
        // 1 - field contains more than one data type
        // 0 - (Default) only one data type. The field can still contain blank values (that's why we are using aDPTypesWithoutBlank)
        if (aDPTypesWithoutBlank.size() > 1)
            pAttList->add(XML_containsMixedTypes, ToPsz10(true));

        // If field contain mixed types (Date and Numbers), MS Excel is saving only "minDate" and "maxDate" and not "minValue" and "maxValue"
        // Example how Excel is saving mixed Date and Numbers:
        // <sharedItems containsSemiMixedTypes="0" containsDate="1" containsString="0" containsMixedTypes="1" minDate="1900-01-03T22:26:04" maxDate="1900-01-07T14:02:04" />
        // Example how Excel is saving Dates only:
        // <sharedItems containsSemiMixedTypes="0" containsNonDate="0" containsDate="1" containsString="0" minDate="1903-08-24T07:40:48" maxDate="2024-05-23T07:12:00"/>
        if (isContainsNumber)
            pAttList->add(XML_containsNumber, ToPsz10(true));

        if (isValueInteger && isContainsNumber)
            pAttList->add(XML_containsInteger, ToPsz10(true));


        // Number type fields could be mixed with blank types, and it shouldn't be treated as listed items.
        // Example:
        //    <cacheField name="employeeID" numFmtId="0">
        //        <sharedItems containsString="0" containsBlank="1" containsNumber="1" containsInteger="1" minValue="35" maxValue="89"/>
        //    </cacheField>
        if (isContainsNumber)
        {
            pAttList->add(XML_minValue, OString::number(fMin));
            pAttList->add(XML_maxValue, OString::number(fMax));
        }

        if (isContainsDate)
        {
            pAttList->add(XML_minDate, GetExcelFormattedDate(fMin, GetFormatter()).toUtf8());
            pAttList->add(XML_maxDate, GetExcelFormattedDate(fMax, GetFormatter()).toUtf8());
        }

        //if (bListItems) // see TODO above
        {
            pAttList->add(XML_count, OString::number(static_cast<tools::Long>(rFieldItems.size())));
        }

        if (isLongText)
        {
            pAttList->add(XML_longText, ToPsz10(true));
        }

        pDefStrm->startElement(XML_sharedItems, pAttList);

        //if (bListItems) // see TODO above
        {
            for (const ScDPItemData& rItem : rFieldItems)
            {
                switch (rItem.GetType())
                {
                    case ScDPItemData::String:
                        pDefStrm->singleElement(XML_s, XML_v, rItem.GetString().toUtf8());
                    break;
                    case ScDPItemData::Value:
                        if (isContainsDate)
                        {
                            pDefStrm->singleElement(XML_d,
                                XML_v, GetExcelFormattedDate(rItem.GetValue(), GetFormatter()).toUtf8());
                        }
                        else
                            pDefStrm->singleElement(XML_n,
                                XML_v, OString::number(rItem.GetValue()));
                    break;
                    case ScDPItemData::Empty:
                        pDefStrm->singleElement(XML_m);
                    break;
                    case ScDPItemData::Error:
                        pDefStrm->singleElement(XML_e,
                            XML_v, rItem.GetString().toUtf8());
                    break;
                    case ScDPItemData::GroupValue: // Should not happen here!
                    case ScDPItemData::RangeStart:
                        // TODO : What do we do with these types?
                        pDefStrm->singleElement(XML_m);
                    break;
                    default:
                        ;
                }
            }
        }

        pDefStrm->endElement(XML_sharedItems);

        WriteFieldGroup(i, i);

        pDefStrm->endElement(XML_cacheField);
    }

    ScDPObject* pDPObject
        = rCache.GetAllReferences().empty() ? nullptr : *rCache.GetAllReferences().begin();

    for (size_t i = nCount; pDPObject && i < nCount + nGroupFieldCount; ++i)
    {
        const OUString aName = pDPObject->GetDimName(i, o3tl::temporary(bool()));
        // tdf#126748: DPObject might not reference all group fields, when there are several
        // DPObjects referencing this cache. Trying to get a dimension data for a field not used
        // in a given DPObject will give nullptr, and dereferencing it then will crash. To avoid
        // the crash, until there's a correct method to find the names of group fields in cache,
        // just skip the fields, creating bad cache data, which is of course a temporary hack.
        // TODO: reimplement the whole block to get the names from another source, not from first
        // cache reference.
        if (aName.isEmpty())
            break;

        ScDPSaveData* pSaveData = pDPObject->GetSaveData();
        assert(pSaveData);

        const ScDPSaveGroupDimension* pDim = pSaveData->GetDimensionData()->GetNamedGroupDim(aName);
        assert(pDim);

        const SCCOL nBase = rCache.GetDimensionIndex(pDim->GetSourceDimName());
        assert(nBase >= 0);

        pDefStrm->startElement(XML_cacheField, XML_name, aName.toUtf8(), XML_numFmtId,
                               OString::number(0), XML_databaseField, ToPsz10(false));
        WriteFieldGroup(i, nBase);
        pDefStrm->endElement(XML_cacheField);
    }

    pDefStrm->endElement(XML_cacheFields);

    pDefStrm->endElement(XML_pivotCacheDefinition);
}

XclExpXmlPivotTableManager::XclExpXmlPivotTableManager( const XclExpRoot& rRoot ) :
    XclExpRoot(rRoot), maCaches(rRoot) {}

void XclExpXmlPivotTableManager::Initialize()
{
    ScDocument& rDoc = GetDoc();
    if (!rDoc.HasPivotTable())
        // No pivot table to export.
        return;

    ScDPCollection* pDPColl = rDoc.GetDPCollection();
    if (!pDPColl)
        return;

    // Update caches from DPObject
    for (size_t i = 0; i < pDPColl->GetCount(); ++i)
    {
        ScDPObject& rDPObj = (*pDPColl)[i];
        rDPObj.SyncAllDimensionMembers();
        (void)rDPObj.GetOutputRangeByType(sheet::DataPilotOutputRangeType::TABLE);
    }

    // Go through the caches first.

    std::vector<XclExpXmlPivotCaches::Entry> aCaches;
    const ScDPCollection::SheetCaches& rSheetCaches = pDPColl->GetSheetCaches();
    const std::vector<ScRange>& rRanges = rSheetCaches.getAllRanges();
    for (const auto & rRange : rRanges)
    {
        const ScDPCache* pCache = rSheetCaches.getExistingCache(rRange);
        if (!pCache)
            continue;

        // Get all pivot objects that reference this cache, and set up an
        // object to cache ID mapping.
        const ScDPCache::ScDPObjectSet& rRefs = pCache->GetAllReferences();
        for (const auto& rRef : rRefs)
            maCacheIdMap.emplace(rRef, aCaches.size()+1);

        XclExpXmlPivotCaches::Entry aEntry;
        aEntry.mpCache = pCache;
        aEntry.maSrcRange = rRange;
        aCaches.push_back(aEntry); // Cache ID equals position + 1.
    }

    // TODO : Handle name and database caches as well.

    for (size_t i = 0, n = pDPColl->GetCount(); i < n; ++i)
    {
        const ScDPObject& rDPObj = (*pDPColl)[i];

        // Get the cache ID for this pivot table.
        CacheIdMapType::iterator itCache = maCacheIdMap.find(&rDPObj);
        if (itCache == maCacheIdMap.end())
            // No cache ID found.  Something is wrong here...
            continue;

        sal_Int32 nCacheId = itCache->second;
        SCTAB nTab = rDPObj.GetOutRange().aStart.Tab();

        TablesType::iterator it = m_Tables.find(nTab);
        if (it == m_Tables.end())
        {
            // Insert a new instance for this sheet index.
            std::pair<TablesType::iterator, bool> r =
                m_Tables.insert(std::make_pair(nTab, std::make_unique<XclExpXmlPivotTables>(GetRoot(), maCaches)));
            it = r.first;
        }

        XclExpXmlPivotTables *const p = it->second.get();
        p->AppendTable(&rDPObj, nCacheId, i+1);
    }

    maCaches.SetCaches(std::move(aCaches));
}

XclExpXmlPivotCaches& XclExpXmlPivotTableManager::GetCaches()
{
    return maCaches;
}

XclExpXmlPivotTables* XclExpXmlPivotTableManager::GetTablesBySheet( SCTAB nTab )
{
    TablesType::iterator const it = m_Tables.find(nTab);
    return it == m_Tables.end() ? nullptr : it->second.get();
}

XclExpXmlPivotTables::Entry::Entry( const ScDPObject* pTable, sal_Int32 nCacheId, sal_Int32 nPivotId ) :
    mpTable(pTable), mnCacheId(nCacheId), mnPivotId(nPivotId) {}

XclExpXmlPivotTables::XclExpXmlPivotTables( const XclExpRoot& rRoot, const XclExpXmlPivotCaches& rCaches ) :
    XclExpRoot(rRoot), mrCaches(rCaches) {}

void XclExpXmlPivotTables::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& pWSStrm = rStrm.GetCurrentStream(); // worksheet stream

    for (const auto& rTable : maTables)
    {
        const ScDPObject& rObj = *rTable.mpTable;
        sal_Int32 nCacheId = rTable.mnCacheId;
        sal_Int32 nPivotId = rTable.mnPivotId;

        sax_fastparser::FSHelperPtr pPivotStrm = rStrm.CreateOutputStream(
            XclXmlUtils::GetStreamName("xl/pivotTables/", "pivotTable", nPivotId),
            XclXmlUtils::GetStreamName(nullptr, "../pivotTables/pivotTable", nPivotId),
            pWSStrm->getOutputStream(),
            CREATE_XL_CONTENT_TYPE("pivotTable"),
            CREATE_OFFICEDOC_RELATION_TYPE("pivotTable"));

        rStrm.PushStream(pPivotStrm);
        SavePivotTableXml(rStrm, rObj, nCacheId);
        rStrm.PopStream();
    }
}

namespace {

struct DataField
{
    tools::Long mnPos; // field index in pivot cache.
    const ScDPSaveDimension* mpDim;

    DataField( tools::Long nPos, const ScDPSaveDimension* pDim ) : mnPos(nPos), mpDim(pDim) {}
};

/** Returns an OOXML subtotal function name string. See ECMA-376-1:2016 18.18.43 */
OString GetSubtotalFuncName(ScGeneralFunction eFunc)
{
    switch (eFunc)
    {
        case ScGeneralFunction::SUM:       return "sum"_ostr;
        case ScGeneralFunction::COUNT:     return "count"_ostr;
        case ScGeneralFunction::AVERAGE:   return "avg"_ostr;
        case ScGeneralFunction::MAX:       return "max"_ostr;
        case ScGeneralFunction::MIN:       return "min"_ostr;
        case ScGeneralFunction::PRODUCT:   return "product"_ostr;
        case ScGeneralFunction::COUNTNUMS: return "countA"_ostr;
        case ScGeneralFunction::STDEV:     return "stdDev"_ostr;
        case ScGeneralFunction::STDEVP:    return "stdDevP"_ostr;
        case ScGeneralFunction::VAR:       return "var"_ostr;
        case ScGeneralFunction::VARP:      return "varP"_ostr;
        default:;
    }
    return "default"_ostr;
}

sal_Int32 GetSubtotalAttrToken(ScGeneralFunction eFunc)
{
    switch (eFunc)
    {
        case ScGeneralFunction::SUM:       return XML_sumSubtotal;
        case ScGeneralFunction::COUNT:     return XML_countSubtotal;
        case ScGeneralFunction::AVERAGE:   return XML_avgSubtotal;
        case ScGeneralFunction::MAX:       return XML_maxSubtotal;
        case ScGeneralFunction::MIN:       return XML_minSubtotal;
        case ScGeneralFunction::PRODUCT:   return XML_productSubtotal;
        case ScGeneralFunction::COUNTNUMS: return XML_countASubtotal;
        case ScGeneralFunction::STDEV:     return XML_stdDevSubtotal;
        case ScGeneralFunction::STDEVP:    return XML_stdDevPSubtotal;
        case ScGeneralFunction::VAR:       return XML_varSubtotal;
        case ScGeneralFunction::VARP:      return XML_varPSubtotal;
        default:;
    }
    return XML_defaultSubtotal;
}

// An item is expected to contain sequences of css::xml::FastAttribute and css::xml::Attribute
void WriteGrabBagItemToStream(XclExpXmlStream& rStrm, sal_Int32 tokenId, const css::uno::Any& rItem)
{
    css::uno::Sequence<css::uno::Any> aSeqs;
    if(!(rItem >>= aSeqs))
        return;

    auto& pStrm = rStrm.GetCurrentStream();
    pStrm->write("<")->writeId(tokenId);

    css::uno::Sequence<css::xml::FastAttribute> aFastSeq;
    css::uno::Sequence<css::xml::Attribute> aUnkSeq;
    for (const auto& a : aSeqs)
    {
        if (a >>= aFastSeq)
        {
            for (const auto& rAttr : aFastSeq)
                rStrm.WriteAttributes(rAttr.Token, rAttr.Value);
        }
        else if (a >>= aUnkSeq)
        {
            for (const auto& rAttr : aUnkSeq)
                pStrm->write(" ")
                    ->write(rAttr.Name)
                    ->write("=\"")
                    ->writeEscaped(rAttr.Value)
                    ->write("\"");
        }
    }

    pStrm->write("/>");
}
}

void XclExpXmlPivotTables::SavePivotTableXml( XclExpXmlStream& rStrm, const ScDPObject& rDPObj, sal_Int32 nCacheId )
{
    typedef std::unordered_map<OUString, long> NameToIdMapType;

    const XclExpXmlPivotCaches::Entry* pCacheEntry = mrCaches.GetCache(nCacheId);
    if (!pCacheEntry)
        // Something is horribly wrong.  Check your logic.
        return;

    const ScDPCache& rCache = *pCacheEntry->mpCache;

    const ScDPSaveData& rSaveData = *rDPObj.GetSaveData();

    size_t nFieldCount = rCache.GetFieldCount() + rCache.GetGroupFieldCount();
    std::vector<const ScDPSaveDimension*> aCachedDims;
    NameToIdMapType aNameToIdMap;

    aCachedDims.reserve(nFieldCount);
    for (size_t i = 0; i < nFieldCount; ++i)
    {
        OUString aName = const_cast<ScDPObject&>(rDPObj).GetDimName(i, o3tl::temporary(bool()));
        aNameToIdMap.emplace(aName, aCachedDims.size());
        const ScDPSaveDimension* pDim = rSaveData.GetExistingDimensionByName(aName);
        aCachedDims.push_back(pDim);
    }

    std::vector<tools::Long> aRowFields;
    std::vector<tools::Long> aColFields;
    std::vector<tools::Long> aPageFields;
    std::vector<DataField> aDataFields;

    tools::Long nDataDimCount = rSaveData.GetDataDimensionCount();
    // Use dimensions in the save data to get their correct ordering.
    // Dimension order here is significant as they specify the order of
    // appearance in each axis.
    const ScDPSaveData::DimsType& rDims = rSaveData.GetDimensions();
    bool bTabularMode = false;
    bool bCompactMode = true;
    for (const auto & i : rDims)
    {
        const ScDPSaveDimension& rDim = *i;

        tools::Long nPos = -1; // position in cache
        if (rDim.IsDataLayout())
            nPos = -2; // Excel uses an index of -2 to indicate a data layout field.
        else
        {
            OUString aSrcName = ScDPUtil::getSourceDimensionName(rDim.GetName());
            NameToIdMapType::iterator it = aNameToIdMap.find(aSrcName);
            if (it != aNameToIdMap.end())
                nPos = it->second;

            if (nPos == -1)
                continue;

            if (!aCachedDims[nPos])
                continue;
        }

        sheet::DataPilotFieldOrientation eOrient = rDim.GetOrientation();

        switch (eOrient)
        {
            case sheet::DataPilotFieldOrientation_COLUMN:
                if (nPos == -2 && nDataDimCount <= 1)
                    break;
                aColFields.push_back(nPos);
            break;
            case sheet::DataPilotFieldOrientation_ROW:
                aRowFields.push_back(nPos);
            break;
            case sheet::DataPilotFieldOrientation_PAGE:
                aPageFields.push_back(nPos);
            break;
            case sheet::DataPilotFieldOrientation_DATA:
                aDataFields.emplace_back(nPos, &rDim);
            break;
            case sheet::DataPilotFieldOrientation_HIDDEN:
            default:
                ;
        }
        if(rDim.GetLayoutInfo())
        {
            const auto eLayoutMode = rDim.GetLayoutInfo()->LayoutMode;
            bTabularMode |= (eLayoutMode == sheet::DataPilotFieldLayoutMode::TABULAR_LAYOUT);
            bCompactMode &= (eLayoutMode == sheet::DataPilotFieldLayoutMode::COMPACT_LAYOUT);
        }
    }

    sax_fastparser::FSHelperPtr& pPivotStrm = rStrm.GetCurrentStream();
    pPivotStrm->startElement(XML_pivotTableDefinition,
        XML_xmlns, rStrm.getNamespaceURL(OOX_NS(xls)).toUtf8(),
        XML_name, rDPObj.GetName().toUtf8(),
        XML_cacheId, OString::number(nCacheId),
        XML_applyNumberFormats, ToPsz10(false),
        XML_applyBorderFormats, ToPsz10(false),
        XML_applyFontFormats, ToPsz10(false),
        XML_applyPatternFormats, ToPsz10(false),
        XML_applyAlignmentFormats, ToPsz10(false),
        XML_applyWidthHeightFormats, ToPsz10(false),
        XML_dataCaption, "Values",
        XML_showDrill, ToPsz10(rSaveData.GetExpandCollapse()),
        XML_useAutoFormatting, ToPsz10(false),
        XML_itemPrintTitles, ToPsz10(true),
        XML_indent, ToPsz10(false),
        XML_outline, ToPsz10(!bTabularMode),
        XML_outlineData, ToPsz10(!bTabularMode),
        XML_compact, ToPsz10(bCompactMode),
        XML_compactData, ToPsz10(bCompactMode));

    // NB: Excel's range does not include page field area (if any).
    ScRange aOutRange = rDPObj.GetOutputRangeByType(sheet::DataPilotOutputRangeType::TABLE);

    sal_Int32 nFirstHeaderRow = rDPObj.GetHeaderLayout() ? 2 : 1;
    sal_Int32 nFirstDataRow = 2;
    sal_Int32 nFirstDataCol = 1;
    ScRange aResRange = rDPObj.GetOutputRangeByType(sheet::DataPilotOutputRangeType::RESULT);

    if (!aOutRange.IsValid())
        aOutRange = rDPObj.GetOutRange();

    if (aOutRange.IsValid() && aResRange.IsValid())
    {
        nFirstDataRow = aResRange.aStart.Row() - aOutRange.aStart.Row();
        nFirstDataCol = aResRange.aStart.Col() - aOutRange.aStart.Col();
    }

    pPivotStrm->write("<")->writeId(XML_location);
    rStrm.WriteAttributes(XML_ref,
        XclXmlUtils::ToOString(rStrm.GetRoot().GetDoc(), aOutRange),
        XML_firstHeaderRow, OUString::number(nFirstHeaderRow),
        XML_firstDataRow, OUString::number(nFirstDataRow),
        XML_firstDataCol, OUString::number(nFirstDataCol));

    if (!aPageFields.empty())
    {
        rStrm.WriteAttributes(XML_rowPageCount, OUString::number(static_cast<tools::Long>(aPageFields.size())));
        rStrm.WriteAttributes(XML_colPageCount, OUString::number(1));
    }

    pPivotStrm->write("/>");

    // <pivotFields> - It must contain all fields in the pivot cache even if
    // only some of them are used in the pivot table.  The order must be as
    // they appear in the cache.

    pPivotStrm->startElement(XML_pivotFields,
        XML_count, OString::number(static_cast<tools::Long>(aCachedDims.size())));

    for (size_t i = 0; i < nFieldCount; ++i)
    {
        const ScDPSaveDimension* pDim = aCachedDims[i];
        if (!pDim)
        {
            pPivotStrm->singleElement(XML_pivotField,
                XML_compact, ToPsz10(false),
                XML_showAll, ToPsz10(false));
            continue;
        }

        bool bDimInTabularMode = false;
        bool bDimInCompactMode = false;
        if(pDim->GetLayoutInfo())
        {
            const auto eLayoutMode = pDim->GetLayoutInfo()->LayoutMode;
            bDimInTabularMode = (eLayoutMode == sheet::DataPilotFieldLayoutMode::TABULAR_LAYOUT);
            bDimInCompactMode = (eLayoutMode == sheet::DataPilotFieldLayoutMode::COMPACT_LAYOUT);
        }

        sheet::DataPilotFieldOrientation eOrient = pDim->GetOrientation();

        if (eOrient == sheet::DataPilotFieldOrientation_HIDDEN)
        {
            if(bDimInTabularMode)
            {
                pPivotStrm->singleElement(XML_pivotField,
                    XML_compact, ToPsz10(false),
                    XML_showAll, ToPsz10(false),
                    XML_outline, ToPsz10(false));
            }
            else
            {
                if (bDimInCompactMode)
                    pPivotStrm->singleElement(XML_pivotField,
                        XML_showAll, ToPsz10(false));
                else
                    pPivotStrm->singleElement(XML_pivotField,
                        XML_compact, ToPsz10(false),
                        XML_showAll, ToPsz10(false));
            }
            continue;
        }

        if (eOrient == sheet::DataPilotFieldOrientation_DATA)
        {
            if(bDimInTabularMode)
            {
                pPivotStrm->singleElement(XML_pivotField,
                    XML_dataField, ToPsz10(true),
                    XML_compact, ToPsz10(false),
                    XML_showAll, ToPsz10(false),
                    XML_outline, ToPsz10(false));
            }
            else
            {
                if (bDimInCompactMode)
                    pPivotStrm->singleElement(XML_pivotField,
                        XML_dataField, ToPsz10(true),
                        XML_showAll, ToPsz10(false));
                else
                    pPivotStrm->singleElement(XML_pivotField,
                        XML_dataField, ToPsz10(true),
                        XML_compact, ToPsz10(false),
                        XML_showAll, ToPsz10(false));
            }
            continue;
        }

        // Dump field items.
        std::vector<ScDPLabelData::Member> aMembers;
        {
            // We need to get the members in actual order, getting which requires non-const reference here
            auto& dpo = const_cast<ScDPObject&>(rDPObj);
            dpo.GetMembers(i, dpo.GetUsedHierarchy(i), aMembers);
        }

        std::vector<OUString> aCacheFieldItems;
        if (i < rCache.GetFieldCount() && !rCache.GetGroupType(i))
        {
            for (const auto& it : rCache.GetDimMemberValues(i))
            {
                OUString sFormattedName;
                if (it.HasStringData() || it.IsEmpty())
                    sFormattedName = it.GetString();
                else
                    sFormattedName = const_cast<ScDPObject&>(rDPObj).GetFormattedString(
                        pDim->GetName(), it.GetValue());
                aCacheFieldItems.push_back(sFormattedName);
            }
        }
        else
        {
            aCacheFieldItems = SortGroupItems(rCache, i);
        }
        // The pair contains the member index in cache and if it is hidden
        std::vector< std::pair<size_t, bool> > aMemberSequence;
        std::set<size_t> aUsedCachePositions;
        for (const auto & rMember : aMembers)
        {
            auto it = std::find(aCacheFieldItems.begin(), aCacheFieldItems.end(), rMember.maName);
            if (it != aCacheFieldItems.end())
            {
                size_t nCachePos = static_cast<size_t>(std::distance(aCacheFieldItems.begin(), it));
                auto aInserted = aUsedCachePositions.insert(nCachePos);
                if (aInserted.second)
                    aMemberSequence.emplace_back(std::make_pair(nCachePos, !rMember.mbVisible));
            }
        }
        // Now add all remaining cache items as hidden
        for (size_t nItem = 0; nItem < aCacheFieldItems.size(); ++nItem)
        {
            if (aUsedCachePositions.find(nItem) == aUsedCachePositions.end())
                aMemberSequence.emplace_back(nItem, true);
        }

        // tdf#125086: check if this field *also* appears in Data region
        bool bAppearsInData = false;
        {
            OUString aSrcName = ScDPUtil::getSourceDimensionName(pDim->GetName());
            const auto it = std::find_if(
                aDataFields.begin(), aDataFields.end(), [&aSrcName](const DataField& rDataField) {
                    OUString aThisName
                        = ScDPUtil::getSourceDimensionName(rDataField.mpDim->GetName());
                    return aThisName == aSrcName;
                });
            if (it != aDataFields.end())
                bAppearsInData = true;
        }

        auto pAttList = sax_fastparser::FastSerializerHelper::createAttrList();
        pAttList->add(XML_axis, toOOXMLAxisType(eOrient));
        if (bAppearsInData)
            pAttList->add(XML_dataField, ToPsz10(true));

        if (!bDimInCompactMode)
            pAttList->add(XML_compact, ToPsz10(false));

        pAttList->add(XML_showAll, ToPsz10(false));

        tools::Long nSubTotalCount = pDim->GetSubTotalsCount();
        std::vector<OString> aSubtotalSequence;
        bool bHasDefaultSubtotal = false;
        for (tools::Long nSubTotal = 0; nSubTotal < nSubTotalCount; ++nSubTotal)
        {
            ScGeneralFunction eFunc = pDim->GetSubTotalFunc(nSubTotal);
            aSubtotalSequence.push_back(GetSubtotalFuncName(eFunc));
            sal_Int32 nAttToken = GetSubtotalAttrToken(eFunc);
            if (nAttToken == XML_defaultSubtotal)
                bHasDefaultSubtotal = true;
            else if (!pAttList->hasAttribute(nAttToken))
                pAttList->add(nAttToken, ToPsz10(true));
        }
        // XML_defaultSubtotal is true by default; only write it if it's false
        if (!bHasDefaultSubtotal)
            pAttList->add(XML_defaultSubtotal, ToPsz10(false));

        if(bDimInTabularMode)
            pAttList->add( XML_outline, ToPsz10(false));
        pPivotStrm->startElement(XML_pivotField, pAttList);

        pPivotStrm->startElement(XML_items,
            XML_count, OString::number(static_cast<tools::Long>(aMemberSequence.size() + aSubtotalSequence.size())));

        for (const auto & nMember : aMemberSequence)
        {
            auto pItemAttList = sax_fastparser::FastSerializerHelper::createAttrList();
            if (nMember.second)
                pItemAttList->add(XML_h, ToPsz10(true));
            pItemAttList->add(XML_x, OString::number(static_cast<tools::Long>(nMember.first)));
            pPivotStrm->singleElement(XML_item, pItemAttList);
        }

        for (const OString& sSubtotal : aSubtotalSequence)
        {
            pPivotStrm->singleElement(XML_item, XML_t, sSubtotal);
        }

        pPivotStrm->endElement(XML_items);
        pPivotStrm->endElement(XML_pivotField);
    }

    pPivotStrm->endElement(XML_pivotFields);

    // <rowFields>

    if (!aRowFields.empty())
    {
        pPivotStrm->startElement(XML_rowFields,
            XML_count, OString::number(static_cast<tools::Long>(aRowFields.size())));

        for (const auto& rRowField : aRowFields)
        {
            pPivotStrm->singleElement(XML_field, XML_x, OString::number(rRowField));
        }

        pPivotStrm->endElement(XML_rowFields);
    }

    // <rowItems>

    // <colFields>

    if (!aColFields.empty())
    {
        pPivotStrm->startElement(XML_colFields,
            XML_count, OString::number(static_cast<tools::Long>(aColFields.size())));

        for (const auto& rColField : aColFields)
        {
            pPivotStrm->singleElement(XML_field, XML_x, OString::number(rColField));
        }

        pPivotStrm->endElement(XML_colFields);
    }

    // <colItems>

    // <pageFields>

    if (!aPageFields.empty())
    {
        pPivotStrm->startElement(XML_pageFields,
            XML_count, OString::number(static_cast<tools::Long>(aPageFields.size())));

        for (const auto& rPageField : aPageFields)
        {
            pPivotStrm->singleElement(XML_pageField,
                XML_fld, OString::number(rPageField),
                XML_hier, OString::number(-1)); // TODO : handle this correctly.
        }

        pPivotStrm->endElement(XML_pageFields);
    }

    // <dataFields>

    if (!aDataFields.empty())
    {
        css::uno::Reference<css::container::XNameAccess> xDimsByName;
        if (auto xDimSupplier = const_cast<ScDPObject&>(rDPObj).GetSource())
            xDimsByName = xDimSupplier->getDimensions();

        pPivotStrm->startElement(XML_dataFields,
            XML_count, OString::number(static_cast<tools::Long>(aDataFields.size())));

        for (const auto& rDataField : aDataFields)
        {
            tools::Long nDimIdx = rDataField.mnPos;
            assert(nDimIdx == -2 || aCachedDims[nDimIdx]); // the loop above should have screened for NULL's, skip check for -2 "data field"
            const ScDPSaveDimension& rDim = *rDataField.mpDim;
            std::optional<OUString> pName = rDim.GetLayoutName();
            // tdf#124651: despite being optional in CT_DataField according to ECMA-376 Part 1,
            // Excel (at least 2016) seems to insist on the presence of "name" attribute in
            // dataField element.
            // tdf#124881: try to create a meaningful name; don't use empty string.
            if (!pName)
                pName = ScDPUtil::getDisplayedMeasureName(
                    rDim.GetName(), ScDPUtil::toSubTotalFunc(rDim.GetFunction()));
            auto pItemAttList = sax_fastparser::FastSerializerHelper::createAttrList();
            pItemAttList->add(XML_name, pName->toUtf8());
            pItemAttList->add(XML_fld, OString::number(nDimIdx));
            const char* pSubtotal = toOOXMLSubtotalType(rDim.GetFunction());
            if (pSubtotal)
                pItemAttList->add(XML_subtotal, pSubtotal);
            if (xDimsByName)
            {
                try
                {
                    css::uno::Reference<css::beans::XPropertySet> xDimProps(
                        xDimsByName->getByName(rDim.GetName()), uno::UNO_QUERY_THROW);
                    css::uno::Any aVal = xDimProps->getPropertyValue(SC_UNONAME_NUMFMT);
                    sal_uInt32 nScNumFmt = aVal.get<sal_uInt32>();
                    sal_uInt16 nXclNumFmt = GetRoot().GetNumFmtBuffer().Insert(nScNumFmt);
                    pItemAttList->add(XML_numFmtId, OString::number(nXclNumFmt));
                }
                catch (uno::Exception&)
                {
                    SAL_WARN("sc.filter",
                             "Couldn't get number format for data field " << rDim.GetName());
                    // Just skip exporting number format
                }
            }
            pPivotStrm->singleElement(XML_dataField, pItemAttList);
        }

        pPivotStrm->endElement(XML_dataFields);
    }

    // <formats>
    savePivotTableFormats(rStrm, rDPObj);

    // Now add style info (use grab bag, or just a set which is default on Excel 2007 through 2016)
    if (const auto [bHas, aVal] = rDPObj.GetInteropGrabBagValue("pivotTableStyleInfo"); bHas)
        WriteGrabBagItemToStream(rStrm, XML_pivotTableStyleInfo, aVal);
    else
        pPivotStrm->singleElement(XML_pivotTableStyleInfo, XML_name, "PivotStyleLight16",
                                  XML_showRowHeaders, "1", XML_showColHeaders, "1",
                                  XML_showRowStripes, "0", XML_showColStripes, "0",
                                  XML_showLastColumn, "1");

    OUString aBuf = "../pivotCache/pivotCacheDefinition" +
        OUString::number(nCacheId) +
        ".xml";

    rStrm.addRelation(
        pPivotStrm->getOutputStream(),
        CREATE_OFFICEDOC_RELATION_TYPE("pivotCacheDefinition"),
        aBuf);

    pPivotStrm->endElement(XML_pivotTableDefinition);
}

void XclExpXmlPivotTables::savePivotTableFormats(XclExpXmlStream& rStream, ScDPObject const& rDPObject)
{
    sax_fastparser::FSHelperPtr& pPivotStream = rStream.GetCurrentStream();

    ScDPSaveData* pSaveData = rDPObject.GetSaveData();
    if (pSaveData && pSaveData->hasFormats())
    {
        sc::PivotTableFormats const& rFormats = pSaveData->getFormats();
        if (rFormats.size() > 0)
        {
            pPivotStream->startElement(XML_formats, XML_count, OString::number(rFormats.size()));

            for (auto const& rFormat : rFormats.getVector())
            {
                if (!rFormat.pPattern)
                    continue;

                sal_Int32 nDxf = GetDxfs().GetDxfIdForPattern(rFormat.pPattern.get());
                if (nDxf == -1)
                    continue;

                pPivotStream->startElement(XML_format, XML_dxfId, OString::number(nDxf));
                {
                    auto pAttributeList = sax_fastparser::FastSerializerHelper::createAttrList();
                    if (!rFormat.bDataOnly) // default is true
                        pAttributeList->add(XML_dataOnly, "0");
                    if (rFormat.bLabelOnly) // default is false
                        pAttributeList->add(XML_labelOnly, "1");
                    if (!rFormat.bOutline) // default is true
                        pAttributeList->add(XML_outline, "0");
                    if (rFormat.oFieldPosition)
                        pAttributeList->add(XML_fieldPosition, OString::number(*rFormat.oFieldPosition));
                    pPivotStream->startElement(XML_pivotArea, pAttributeList);
                }
                pPivotStream->startElement(XML_references, XML_count, OString::number(rFormat.aSelections.size()));
                for (sc::Selection const& rSelection : rFormat.getSelections())
                {
                    {
                        auto pRefAttributeList = sax_fastparser::FastSerializerHelper::createAttrList();
                        pRefAttributeList->add(XML_field, OString::number(sal_uInt32(rSelection.nField)));
                        pRefAttributeList->add(XML_count, "1");
                        if (!rSelection.bSelected) // default is true
                            pRefAttributeList->add(XML_selected, "0");
                        pPivotStream->startElement(XML_reference, pRefAttributeList);
                    }

                    pPivotStream->singleElement(XML_x, XML_v, OString::number(rSelection.nDataIndex));

                    pPivotStream->endElement(XML_reference);
                }
                pPivotStream->endElement(XML_references);
                pPivotStream->endElement(XML_pivotArea);

                pPivotStream->endElement(XML_format);
            }
            pPivotStream->endElement(XML_formats);
        }
    }
}

void XclExpXmlPivotTables::AppendTable( const ScDPObject* pTable, sal_Int32 nCacheId, sal_Int32 nPivotId )
{
    maTables.emplace_back(pTable, nCacheId, nPivotId);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
