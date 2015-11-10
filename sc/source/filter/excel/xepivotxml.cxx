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
#include <dpobject.hxx>
#include <dpsave.hxx>
#include <dputil.hxx>
#include <document.hxx>

#include <oox/export/utils.hxx>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotOutputRangeType.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>

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
        XML_xmlns, "http://schemas.openxmlformats.org/spreadsheetml/2006/main",
        FSNS(XML_xmlns, XML_r), "http://schemas.openxmlformats.org/officeDocument/2006/relationships",
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

const char* toOOXMLSubtotalType( sheet::GeneralFunction eFunc )
{
    switch (eFunc)
    {
        case sheet::GeneralFunction_SUM:
            return "sum";
        case sheet::GeneralFunction_COUNT:
            return "count";
        case sheet::GeneralFunction_AVERAGE:
            return "average";
        case sheet::GeneralFunction_MAX:
            return "max";
        case sheet::GeneralFunction_MIN:
            return "min";
        case sheet::GeneralFunction_PRODUCT:
            return "product";
        case sheet::GeneralFunction_COUNTNUMS:
            return "countNums";
        case sheet::GeneralFunction_STDEV:
            return "stdDev";
        case sheet::GeneralFunction_STDEVP:
            return "stdDevp";
        case sheet::GeneralFunction_VAR:
            return "var";
        case sheet::GeneralFunction_VARP:
            return "varp";
        case sheet::GeneralFunction_NONE:
        case sheet::GeneralFunction_AUTO:
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
        XML_xmlns, "http://schemas.openxmlformats.org/spreadsheetml/2006/main",
        FSNS(XML_xmlns, XML_r), "http://schemas.openxmlformats.org/officeDocument/2006/relationships",
        FSNS(XML_r, XML_id), XclXmlUtils::ToOString(aRelId).getStr(),
        XML_recordCount, OString::number(rEntry.mpCache->GetDataSize()).getStr(),
        FSEND);

    if (rEntry.meType == Worksheet)
    {
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
    }

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

        const ScDPCache::ItemsType& rFieldItems = rCache.GetDimMemberValues(i);

        ScDPCache::ItemsType::const_iterator it = rFieldItems.begin(), itEnd = rFieldItems.end();

        std::set<ScDPItemData::Type> aDPTypes;
        for (; it != itEnd; ++it)
        {
            aDPTypes.insert(it->GetType());
        }

        auto aDPTypeEnd = aDPTypes.cend();

        pDefStrm->startElement(XML_sharedItems,
            XML_count, OString::number(static_cast<long>(rFieldItems.size())).getStr(),
            XML_containsMixedTypes, XclXmlUtils::ToPsz10(aDPTypes.size() > 1),
            XML_containsSemiMixedTypes, XclXmlUtils::ToPsz10(aDPTypes.size() > 1),
            XML_containsString, XclXmlUtils::ToPsz10(aDPTypes.find(ScDPItemData::String) != aDPTypeEnd),
            XML_containsNumber, XclXmlUtils::ToPsz10(aDPTypes.find(ScDPItemData::Value) != aDPTypeEnd),
            FSEND);

        it = rFieldItems.begin();
        for (; it != itEnd; ++it)
        {
            const ScDPItemData& rItem = *it;
            switch (rItem.GetType())
            {
                case ScDPItemData::String:
                    pDefStrm->singleElement(XML_s,
                        XML_v, XclXmlUtils::ToOString(rItem.GetString()).getStr(),
                        FSEND);
                break;
                case ScDPItemData::Value:
                    pDefStrm->singleElement(XML_n,
                        XML_v, OString::number(rItem.GetValue()).getStr(),
                        FSEND);
                break;
                case ScDPItemData::Empty:
                    pDefStrm->singleElement(XML_m, FSEND);
                break;
                case ScDPItemData::Error:
                    pDefStrm->singleElement(XML_e,
                        XML_v, XclXmlUtils::ToOString(rItem.GetString()).getStr(),
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
    const ScDocument& rDoc = GetDoc();
    if (!rDoc.HasPivotTable())
        // No pivot table to export.
        return;

    const ScDPCollection* pDPColl = rDoc.GetDPCollection();
    if (!pDPColl)
        return;

    // Go through the caches first.

    std::vector<XclExpXmlPivotCaches::Entry> aCaches;
    const ScDPCollection::SheetCaches& rSheetCaches = pDPColl->GetSheetCaches();
    const std::vector<ScRange>& rRanges = rSheetCaches.getAllRanges();
    for (size_t i = 0, n = rRanges.size(); i < n; ++i)
    {
        const ScDPCache* pCache = rSheetCaches.getExistingCache(rRanges[i]);
        if (!pCache)
            continue;

        // Get all pivot objects that reference this cache, and set up an
        // object to cache ID mapping.
        const ScDPCache::ObjectSetType& rRefs = pCache->GetAllReferences();
        ScDPCache::ObjectSetType::const_iterator it = rRefs.begin(), itEnd = rRefs.end();
        for (; it != itEnd; ++it)
            maCacheIdMap.insert(CacheIdMapType::value_type(*it, aCaches.size()+1));

        XclExpXmlPivotCaches::Entry aEntry;
        aEntry.meType = XclExpXmlPivotCaches::Worksheet;
        aEntry.mpCache = pCache;
        aEntry.maSrcRange = rRanges[i];
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

        TablesType::iterator it = maTables.find(nTab);
        if (it == maTables.end())
        {
            // Insert a new instance for this sheet index.
            std::pair<TablesType::iterator, bool> r =
                maTables.insert(nTab, new XclExpXmlPivotTables(GetRoot(), maCaches));
            it = r.first;
        }

        XclExpXmlPivotTables* p = it->second;
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
    TablesType::iterator it = maTables.find(nTab);
    return it == maTables.end() ? nullptr : it->second;
}

XclExpXmlPivotTables::Entry::Entry( const ScDPObject* pTable, sal_Int32 nCacheId, sal_Int32 nPivotId ) :
    mpTable(pTable), mnCacheId(nCacheId), mnPivotId(nPivotId) {}

XclExpXmlPivotTables::XclExpXmlPivotTables( const XclExpRoot& rRoot, const XclExpXmlPivotCaches& rCaches ) :
    XclExpRoot(rRoot), mrCaches(rCaches) {}

void XclExpXmlPivotTables::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& pWSStrm = rStrm.GetCurrentStream(); // worksheet stream

    sal_Int32 nCounter = 1; // 1-based
    TablesType::iterator it = maTables.begin(), itEnd = maTables.end();
    for (; it != itEnd; ++it, ++nCounter)
    {
        const ScDPObject& rObj = *it->mpTable;
        sal_Int32 nCacheId = it->mnCacheId;
        sal_Int32 nPivotId = it->mnPivotId;

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
    long mnPos; // field index in pivot cache.
    const ScDPSaveDimension* mpDim;

    DataField( long nPos, const ScDPSaveDimension* pDim ) : mnPos(nPos), mpDim(pDim) {}
};

}

void XclExpXmlPivotTables::SavePivotTableXml( XclExpXmlStream& rStrm, const ScDPObject& rDPObj, sal_Int32 nCacheId )
{
    typedef std::unordered_map<OUString, long, OUStringHash> NameToIdMapType;

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
        aNameToIdMap.insert(NameToIdMapType::value_type(aName, aCachedDims.size()));
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

    for (size_t i = 0, n = rDims.size(); i < n; ++i)
    {
        const ScDPSaveDimension& rDim = *rDims[i];

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

        sheet::DataPilotFieldOrientation eOrient =
            static_cast<sheet::DataPilotFieldOrientation>(rDim.GetOrientation());

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
                aDataFields.push_back(DataField(nPos, &rDim));
            break;
            case sheet::DataPilotFieldOrientation_HIDDEN:
            default:
                ;
        }
    }

    sax_fastparser::FSHelperPtr& pPivotStrm = rStrm.GetCurrentStream();
    pPivotStrm->startElement(XML_pivotTableDefinition,
        XML_xmlns, "http://schemas.openxmlformats.org/spreadsheetml/2006/main",
        XML_name, XclXmlUtils::ToOString(rDPObj.GetName()).getStr(),
        XML_cacheId, OString::number(nCacheId).getStr(),
        XML_applyNumberFormats, BS(false),
        XML_applyBorderFormats, BS(false),
        XML_applyFontFormats, BS(false),
        XML_applyPatternFormats, BS(false),
        XML_applyAlignmentFormats, BS(false),
        XML_applyWidthHeightFormats, BS(false),
        XML_dataCaption, "Values",
        XML_useAutoFormatting, BS(false),
        XML_itemPrintTitles, BS(true),
        XML_indent, BS(false),
        XML_outline, BS(true),
        XML_outlineData, BS(true),
        FSEND);

    // NB: Excel's range does not include page field area (if any).
    ScRange aOutRange = rDPObj.GetOutputRangeByType(sheet::DataPilotOutputRangeType::TABLE);

    sal_Int32 nFirstHeaderRow = aColFields.size();
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

    for (size_t i = 0, n = aCachedDims.size(); i < n; ++i)
    {
        const ScDPSaveDimension* pDim = aCachedDims[i];
        if (!pDim)
        {
            pPivotStrm->singleElement(XML_pivotField,
                XML_showAll, BS(false),
                FSEND);
            continue;
        }

        sheet::DataPilotFieldOrientation eOrient =
            static_cast<sheet::DataPilotFieldOrientation>(pDim->GetOrientation());

        if (eOrient == sheet::DataPilotFieldOrientation_HIDDEN)
        {
            pPivotStrm->singleElement(XML_pivotField,
                XML_showAll, BS(false),
                FSEND);
            continue;
        }

        if (eOrient == sheet::DataPilotFieldOrientation_DATA)
        {
            pPivotStrm->singleElement(XML_pivotField,
                XML_dataField, BS(true),
                XML_showAll, BS(false),
                FSEND);

            continue;
        }

        pPivotStrm->startElement(XML_pivotField,
            XML_axis, toOOXMLAxisType(eOrient),
            XML_showAll, BS(false),
            FSEND);

        // TODO : Dump field items.

        pPivotStrm->endElement(XML_pivotField);
    }

    pPivotStrm->endElement(XML_pivotFields);

    // <rowFields>

    if (!aRowFields.empty())
    {
        pPivotStrm->startElement(XML_rowFields,
            XML_count, OString::number(static_cast<long>(aRowFields.size())),
            FSEND);

        std::vector<long>::iterator it = aRowFields.begin(), itEnd = aRowFields.end();
        for (; it != itEnd; ++it)
        {
            pPivotStrm->singleElement(XML_field,
                XML_x, OString::number(*it),
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

        std::vector<long>::iterator it = aColFields.begin(), itEnd = aColFields.end();
        for (; it != itEnd; ++it)
        {
            pPivotStrm->singleElement(XML_field,
                XML_x, OString::number(*it),
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

        std::vector<long>::iterator it = aPageFields.begin(), itEnd = aPageFields.end();
        for (; it != itEnd; ++it)
        {
            pPivotStrm->singleElement(XML_pageField,
                XML_fld, OString::number(*it),
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

        std::vector<DataField>::iterator it = aDataFields.begin(), itEnd = aDataFields.end();
        for (; it != itEnd; ++it)
        {
            long nDimIdx = it->mnPos;
            assert(aCachedDims[nDimIdx]); // the loop above should have screened for NULL's.
            const ScDPSaveDimension& rDim = *it->mpDim;
            const OUString* pName = rDim.GetLayoutName();
            pPivotStrm->write("<")->writeId(XML_dataField);
            if (pName)
                rStrm.WriteAttributes(XML_name, XclXmlUtils::ToOString(*pName), FSEND);

            rStrm.WriteAttributes(XML_fld, OString::number(nDimIdx).getStr(), FSEND);

            sheet::GeneralFunction eFunc = static_cast<sheet::GeneralFunction>(rDim.GetFunction());
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
    maTables.push_back(Entry(pTable, nCacheId, nPivotId));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
