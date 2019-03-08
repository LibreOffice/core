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
#include <dpitemdata.hxx>
#include <dpobject.hxx>
#include <dpsave.hxx>
#include <dputil.hxx>
#include <document.hxx>
#include <generalfunction.hxx>

#include <o3tl/temporary.hxx>
#include <oox/export/utils.hxx>
#include <oox/token/namespaces.hxx>
#include <sax/tools/converter.hxx>
#include <sax/fastattribs.hxx>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutMode.hpp>
#include <com/sun/star/sheet/DataPilotOutputRangeType.hpp>

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
        XML_xmlns, XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(xls))).getStr(),
        FSNS(XML_xmlns, XML_r), XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(officeRel))).getStr(),
        XML_count, OString::number(static_cast<long>(nCount)).getStr(),
        FSEND);

    for (SCROW i = 0; i < nCount; ++i)
    {
        pRecStrm->startElement(XML_r, FSEND);
        for (size_t nField = 0; nField < nFieldCount; ++nField)
        {
            const ScDPCache::IndexArrayType* pArray = rCache.GetFieldIndexArray(nField);
            assert(pArray);
            assert(static_cast<size_t>(i) < pArray->size());

            // We are using XML_x reference (like: <x v="0"/>), instead of values here (eg: <s v="No Discount"/>).
            // That's why in SavePivotCacheXml method, we need to list all items.
            pRecStrm->singleElement(XML_x, XML_v, OString::number((*pArray)[i]), FSEND);
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
    pWorkbookStrm->startElement(XML_pivotCaches, FSEND);

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
            XML_cacheId, OString::number(nCacheId).getStr(),
            FSNS(XML_r, XML_id), XclXmlUtils::ToOString(aRelId).getStr(),
            FSEND);

        rStrm.PushStream(pPCStrm);
        SavePivotCacheXml(rStrm, rEntry, nCacheId);
        rStrm.PopStream();
    }

    pWorkbookStrm->endElement(XML_pivotCaches);
}

void XclExpXmlPivotCaches::SetCaches( const std::vector<Entry>& rCaches )
{
    maCaches = rCaches;
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
    //::sax::Converter::convertDateTime(sBuf, (DateTime(rFormatter.GetNullDate()) + fSerialDateTime).GetUNODateTime(), 0, true);
    css::util::DateTime aUDateTime = (DateTime(rFormatter.GetNullDate()) + fSerialDateTime).GetUNODateTime();
    // We need to reset nanoseconds, to avoid string like: "1982-02-18T16:04:47.999999849"
    aUDateTime.NanoSeconds = 0;
    OUStringBuffer sBuf;
    ::sax::Converter::convertDateTime(sBuf, aUDateTime, nullptr, true);
    return sBuf.makeStringAndClear();
}
}

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
        XML_xmlns, XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(xls))).getStr(),
        FSNS(XML_xmlns, XML_r), XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(officeRel))).getStr(),
        FSNS(XML_r, XML_id), XclXmlUtils::ToOString(aRelId).getStr(),
        XML_recordCount, OString::number(rEntry.mpCache->GetDataSize()).getStr(),
        XML_createdVersion, "3", // MS Excel 2007, tdf#112936: setting version number makes MSO to handle the pivot table differently
        FSEND);

    pDefStrm->startElement(XML_cacheSource,
        XML_type, "worksheet",
        FSEND);

    OUString aSheetName;
    GetDoc().GetName(rEntry.maSrcRange.aStart.Tab(), aSheetName);
    pDefStrm->singleElement(XML_worksheetSource,
        XML_ref, XclXmlUtils::ToOString(rEntry.maSrcRange).getStr(),
        XML_sheet, XclXmlUtils::ToOString(aSheetName).getStr(),
        FSEND);

    pDefStrm->endElement(XML_cacheSource);

    size_t nCount = rCache.GetFieldCount();
    pDefStrm->startElement(XML_cacheFields,
        XML_count, OString::number(static_cast<long>(nCount)).getStr(),
        FSEND);

    for (size_t i = 0; i < nCount; ++i)
    {
        OUString aName = rCache.GetDimensionName(i);

        pDefStrm->startElement(XML_cacheField,
            XML_name, XclXmlUtils::ToOString(aName).getStr(),
            XML_numFmtId, OString::number(0).getStr(),
            FSEND);

        const ScDPCache::ScDPItemDataVec& rFieldItems = rCache.GetDimMemberValues(i);

        std::set<ScDPItemData::Type> aDPTypes;
        double fMin = std::numeric_limits<double>::infinity(), fMax = -std::numeric_limits<double>::infinity();
        bool isValueInteger = true;
        bool isContainsDate = rCache.IsDateDimension(i);
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
            pAttList->add(XML_minDate, XclXmlUtils::ToOString(GetExcelFormattedDate(fMin, GetFormatter())));
            pAttList->add(XML_maxDate, XclXmlUtils::ToOString(GetExcelFormattedDate(fMax, GetFormatter())));
        }

        //if (bListItems) // see TODO above
        {
            pAttList->add(XML_count, OString::number(static_cast<long>(rFieldItems.size())));
        }
        sax_fastparser::XFastAttributeListRef xAttributeList(pAttList);

        pDefStrm->startElement(XML_sharedItems, xAttributeList);

        //if (bListItems) // see TODO above
        {
            for (const ScDPItemData& rItem : rFieldItems)
            {
                switch (rItem.GetType())
                {
                    case ScDPItemData::String:
                        pDefStrm->singleElement(XML_s,
                            XML_v, XclXmlUtils::ToOString(rItem.GetString()),
                            FSEND);
                    break;
                    case ScDPItemData::Value:
                        if (isContainsDate)
                        {
                            pDefStrm->singleElement(XML_d,
                                XML_v, XclXmlUtils::ToOString(GetExcelFormattedDate(rItem.GetValue(), GetFormatter())),
                                FSEND);
                        }
                        else
                            pDefStrm->singleElement(XML_n,
                                XML_v, OString::number(rItem.GetValue()),
                                FSEND);
                    break;
                    case ScDPItemData::Empty:
                        pDefStrm->singleElement(XML_m, FSEND);
                    break;
                    case ScDPItemData::Error:
                        pDefStrm->singleElement(XML_e,
                            XML_v, XclXmlUtils::ToOString(rItem.GetString()),
                            FSEND);
                    break;
                    case ScDPItemData::GroupValue:
                    case ScDPItemData::RangeStart:
                        // TODO : What do we do with these types?
                        pDefStrm->singleElement(XML_m, FSEND);
                    break;
                    default:
                        ;
                }
            }
        }

        pDefStrm->endElement(XML_sharedItems);
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
            // No cache ID found.  Something is wrong here....
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

    maCaches.SetCaches(aCaches);
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
    long const mnPos; // field index in pivot cache.
    const ScDPSaveDimension* mpDim;

    DataField( long nPos, const ScDPSaveDimension* pDim ) : mnPos(nPos), mpDim(pDim) {}
};

/** Returns a OOXML subtotal function name string. See ECMA-376-1:2016 18.18.43 */
OString GetSubtotalFuncName(ScGeneralFunction eFunc)
{
    switch (eFunc)
    {
        case ScGeneralFunction::SUM:       return "sum";
        case ScGeneralFunction::COUNT:     return "count";
        case ScGeneralFunction::AVERAGE:   return "avg";
        case ScGeneralFunction::MAX:       return "max";
        case ScGeneralFunction::MIN:       return "min";
        case ScGeneralFunction::PRODUCT:   return "product";
        case ScGeneralFunction::COUNTNUMS: return "countA";
        case ScGeneralFunction::STDEV:     return "stdDev";
        case ScGeneralFunction::STDEVP:    return "stdDevP";
        case ScGeneralFunction::VAR:       return "var";
        case ScGeneralFunction::VARP:      return "varP";
        default:;
    }
    return "default";
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

    size_t nFieldCount = rCache.GetFieldCount();
    std::vector<const ScDPSaveDimension*> aCachedDims;
    NameToIdMapType aNameToIdMap;

    aCachedDims.reserve(nFieldCount);
    for (size_t i = 0; i < nFieldCount; ++i)
    {
        OUString aName = rCache.GetDimensionName(i);
        aNameToIdMap.emplace(aName, aCachedDims.size());
        const ScDPSaveDimension* pDim = rSaveData.GetExistingDimensionByName(aName);
        aCachedDims.push_back(pDim);
    }

    std::vector<long> aRowFields;
    std::vector<long> aColFields;
    std::vector<long> aPageFields;
    std::vector<DataField> aDataFields;

    // Use dimensions in the save data to get their correct ordering.
    // Dimension order here is significant as they specify the order of
    // appearance in each axis.
    const ScDPSaveData::DimsType& rDims = rSaveData.GetDimensions();
    bool bTabularMode = false;
    for (const auto & i : rDims)
    {
        const ScDPSaveDimension& rDim = *i;

        long nPos = -1; // position in cache
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
            bTabularMode |= (rDim.GetLayoutInfo()->LayoutMode == sheet::DataPilotFieldLayoutMode::TABULAR_LAYOUT);
    }

    sax_fastparser::FSHelperPtr& pPivotStrm = rStrm.GetCurrentStream();
    pPivotStrm->startElement(XML_pivotTableDefinition,
        XML_xmlns, XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(xls))).getStr(),
        XML_name, XclXmlUtils::ToOString(rDPObj.GetName()).getStr(),
        XML_cacheId, OString::number(nCacheId).getStr(),
        XML_applyNumberFormats, ToPsz10(false),
        XML_applyBorderFormats, ToPsz10(false),
        XML_applyFontFormats, ToPsz10(false),
        XML_applyPatternFormats, ToPsz10(false),
        XML_applyAlignmentFormats, ToPsz10(false),
        XML_applyWidthHeightFormats, ToPsz10(false),
        XML_dataCaption, "Values",
        XML_useAutoFormatting, ToPsz10(false),
        XML_itemPrintTitles, ToPsz10(true),
        XML_indent, ToPsz10(false),
        XML_outline, ToPsz10(!bTabularMode),
        XML_outlineData, ToPsz10(!bTabularMode),
        XML_compact, ToPsz10(false),
        XML_compactData, ToPsz10(false),
        FSEND);

    // NB: Excel's range does not include page field area (if any).
    ScRange aOutRange = rDPObj.GetOutputRangeByType(sheet::DataPilotOutputRangeType::TABLE);

    sal_Int32 nFirstHeaderRow = rDPObj.GetHeaderLayout() ? 2 : 1;
    sal_Int32 nFirstDataRow = 2;
    sal_Int32 nFirstDataCol = 1;
    ScRange aResRange = rDPObj.GetOutputRangeByType(sheet::DataPilotOutputRangeType::RESULT);
    if (aOutRange.IsValid() && aResRange.IsValid())
    {
        nFirstDataRow = aResRange.aStart.Row() - aOutRange.aStart.Row();
        nFirstDataCol = aResRange.aStart.Col() - aOutRange.aStart.Col();
    }

    if (!aOutRange.IsValid())
        aOutRange = rDPObj.GetOutRange();

    pPivotStrm->write("<")->writeId(XML_location);
    rStrm.WriteAttributes(XML_ref,
        XclXmlUtils::ToOString(aOutRange),
        XML_firstHeaderRow, OString::number(nFirstHeaderRow).getStr(),
        XML_firstDataRow, OString::number(nFirstDataRow).getStr(),
        XML_firstDataCol, OString::number(nFirstDataCol).getStr(),
        FSEND);

    if (!aPageFields.empty())
    {
        rStrm.WriteAttributes(XML_rowPageCount, OString::number(static_cast<long>(aPageFields.size())).getStr(), FSEND);
        rStrm.WriteAttributes(XML_colPageCount, OString::number(1).getStr(), FSEND);
    }

    pPivotStrm->write("/>");

    // <pivotFields> - It must contain all fields in the pivot cache even if
    // only some of them are used in the pivot table.  The order must be as
    // they appear in the cache.

    pPivotStrm->startElement(XML_pivotFields,
        XML_count, OString::number(static_cast<long>(aCachedDims.size())).getStr(),
        FSEND);

    for (size_t i = 0; i < nFieldCount; ++i)
    {
        const ScDPSaveDimension* pDim = aCachedDims[i];
        if (!pDim)
        {
            pPivotStrm->singleElement(XML_pivotField,
                XML_showAll, ToPsz10(false),
                XML_compact, ToPsz10(false),
                FSEND);
            continue;
        }

        bool bDimInTabularMode = false;
        if(pDim->GetLayoutInfo())
            bDimInTabularMode = (pDim->GetLayoutInfo()->LayoutMode == sheet::DataPilotFieldLayoutMode::TABULAR_LAYOUT);

        sheet::DataPilotFieldOrientation eOrient = pDim->GetOrientation();

        if (eOrient == sheet::DataPilotFieldOrientation_HIDDEN)
        {
            if(bDimInTabularMode)
            {
                pPivotStrm->singleElement(XML_pivotField,
                    XML_showAll, ToPsz10(false),
                    XML_compact, ToPsz10(false),
                    XML_outline, ToPsz10(false),
                    FSEND);
            }
            else
            {
                pPivotStrm->singleElement(XML_pivotField,
                    XML_showAll, ToPsz10(false),
                    XML_compact, ToPsz10(false),
                    FSEND);
            }
            continue;
        }

        if (eOrient == sheet::DataPilotFieldOrientation_DATA)
        {
            if(bDimInTabularMode)
            {
                pPivotStrm->singleElement(XML_pivotField,
                    XML_dataField, ToPsz10(true),
                    XML_showAll, ToPsz10(false),
                    XML_compact, ToPsz10(false),
                    XML_outline, ToPsz10(false),
                    FSEND);
            }
            else
            {
                pPivotStrm->singleElement(XML_pivotField,
                    XML_dataField, ToPsz10(true),
                    XML_showAll, ToPsz10(false),
                    XML_compact, ToPsz10(false),
                    FSEND);
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

        const ScDPCache::ScDPItemDataVec& rCacheFieldItems = rCache.GetDimMemberValues(i);
        // The pair contains the member index in cache and if it is hidden
        std::vector< std::pair<size_t, bool> > aMemberSequence;
        std::set<size_t> aUsedCachePositions;
        for (const auto & rMember : aMembers)
        {
            auto it = std::find_if(rCacheFieldItems.begin(), rCacheFieldItems.end(),
                [&rDPObj, &pDim, &rMember](const ScDPItemData& rItem) {
                    OUString sFormattedName;
                    if (rItem.HasStringData() || rItem.IsEmpty())
                        sFormattedName = rItem.GetString();
                    else
                        sFormattedName = const_cast<ScDPObject&>(rDPObj).GetFormattedString(pDim->GetName(), rItem.GetValue());
                    return sFormattedName == rMember.maName;
                });
            if (it != rCacheFieldItems.end())
            {
                size_t nCachePos = static_cast<size_t>(std::distance(rCacheFieldItems.begin(), it));
                auto aInserted = aUsedCachePositions.insert(nCachePos);
                if (aInserted.second)
                    aMemberSequence.emplace_back(std::make_pair(nCachePos, !rMember.mbVisible));
            }
        }
        // Now add all remaining cache items as hidden
        for (size_t nItem = 0; nItem < rCacheFieldItems.size(); ++nItem)
        {
            if (aUsedCachePositions.find(nItem) == aUsedCachePositions.end())
                aMemberSequence.emplace_back(nItem, true);
        }

        auto pAttList = sax_fastparser::FastSerializerHelper::createAttrList();
        pAttList->add(XML_axis, toOOXMLAxisType(eOrient));
        pAttList->add(XML_showAll, ToPsz10(false));

        long nSubTotalCount = pDim->GetSubTotalsCount();
        std::vector<OString> aSubtotalSequence;
        bool bHasDefaultSubtotal = false;
        for (long nSubTotal = 0; nSubTotal < nSubTotalCount; ++nSubTotal)
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

        pAttList->add( XML_compact, ToPsz10(false));
        if(bDimInTabularMode)
            pAttList->add( XML_outline, ToPsz10(false));
        sax_fastparser::XFastAttributeListRef xAttributeList(pAttList);
        pPivotStrm->startElement(XML_pivotField, xAttributeList);

        pPivotStrm->startElement(XML_items,
            XML_count, OString::number(static_cast<long>(aMemberSequence.size() + aSubtotalSequence.size())),
            FSEND);

        for (const auto & nMember : aMemberSequence)
        {
            auto pItemAttList = sax_fastparser::FastSerializerHelper::createAttrList();
            if (nMember.second)
                pItemAttList->add(XML_h, ToPsz10(true));
            pItemAttList->add(XML_x, OString::number(static_cast<long>(nMember.first)));
            sax_fastparser::XFastAttributeListRef xItemAttributeList(pItemAttList);
            pPivotStrm->singleElement(XML_item, xItemAttributeList);
        }

        for (const OString& sSubtotal : aSubtotalSequence)
        {
            pPivotStrm->singleElement(XML_item,
                XML_t, sSubtotal,
                FSEND);
        }

        pPivotStrm->endElement(XML_items);
        pPivotStrm->endElement(XML_pivotField);
    }

    pPivotStrm->endElement(XML_pivotFields);

    // <rowFields>

    if (!aRowFields.empty())
    {
        pPivotStrm->startElement(XML_rowFields,
            XML_count, OString::number(static_cast<long>(aRowFields.size())),
            FSEND);

        for (const auto& rRowField : aRowFields)
        {
            pPivotStrm->singleElement(XML_field,
                XML_x, OString::number(rRowField),
                FSEND);
        }

        pPivotStrm->endElement(XML_rowFields);
    }

    // <rowItems>

    // <colFields>

    if (!aColFields.empty())
    {
        pPivotStrm->startElement(XML_colFields,
            XML_count, OString::number(static_cast<long>(aColFields.size())),
            FSEND);

        for (const auto& rColField : aColFields)
        {
            pPivotStrm->singleElement(XML_field,
                XML_x, OString::number(rColField),
                FSEND);
        }

        pPivotStrm->endElement(XML_colFields);
    }

    // <colItems>

    // <pageFields>

    if (!aPageFields.empty())
    {
        pPivotStrm->startElement(XML_pageFields,
            XML_count, OString::number(static_cast<long>(aPageFields.size())),
            FSEND);

        for (const auto& rPageField : aPageFields)
        {
            pPivotStrm->singleElement(XML_pageField,
                XML_fld, OString::number(rPageField),
                XML_hier, OString::number(-1), // TODO : handle this correctly.
                FSEND);
        }

        pPivotStrm->endElement(XML_pageFields);
    }

    // <dataFields>

    if (!aDataFields.empty())
    {
        pPivotStrm->startElement(XML_dataFields,
            XML_count, OString::number(static_cast<long>(aDataFields.size())),
            FSEND);

        for (const auto& rDataField : aDataFields)
        {
            long nDimIdx = rDataField.mnPos;
            assert(aCachedDims[nDimIdx]); // the loop above should have screened for NULL's.
            const ScDPSaveDimension& rDim = *rDataField.mpDim;
            const boost::optional<OUString> & pName = rDim.GetLayoutName();
            pPivotStrm->write("<")->writeId(XML_dataField);
            if (pName)
                rStrm.WriteAttributes(XML_name, XclXmlUtils::ToOString(*pName), FSEND);

            rStrm.WriteAttributes(XML_fld, OString::number(nDimIdx).getStr(), FSEND);

            ScGeneralFunction eFunc = rDim.GetFunction();
            const char* pSubtotal = toOOXMLSubtotalType(eFunc);
            if (pSubtotal)
                rStrm.WriteAttributes(XML_subtotal, pSubtotal, FSEND);

            pPivotStrm->write("/>");
        }

        pPivotStrm->endElement(XML_dataFields);
    }

    OUStringBuffer aBuf("../pivotCache/pivotCacheDefinition");
    aBuf.append(nCacheId);
    aBuf.append(".xml");

    rStrm.addRelation(
        pPivotStrm->getOutputStream(),
        CREATE_OFFICEDOC_RELATION_TYPE("pivotCacheDefinition"),
        aBuf.makeStringAndClear());

    pPivotStrm->endElement(XML_pivotTableDefinition);
}

void XclExpXmlPivotTables::AppendTable( const ScDPObject* pTable, sal_Int32 nCacheId, sal_Int32 nPivotId )
{
    maTables.emplace_back(pTable, nCacheId, nPivotId);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
