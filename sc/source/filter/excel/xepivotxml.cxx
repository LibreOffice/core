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
#include <generalfunction.hxx>

#include <oox/export/utils.hxx>
#include <oox/token/namespaces.hxx>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotOutputRangeType.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>

#include <o3tl/make_unique.hxx>

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

        const ScDPCache::ScDPItemDataVec& rFieldItems = rCache.GetDimMemberValues(i);

        ScDPCache::ScDPItemDataVec::const_iterator it = rFieldItems.begin(), itEnd = rFieldItems.end();

        std::set<ScDPItemData::Type> aDPTypes;
        double fMin = std::numeric_limits<double>::infinity(), fMax = -std::numeric_limits<double>::infinity();
        for (; it != itEnd; ++it)
        {
            ScDPItemData::Type eType = it->GetType();
            aDPTypes.insert(eType);
            if (eType == ScDPItemData::Value)
            {
                double fVal = it->GetValue();
                fMin = std::min(fMin, fVal);
                fMax = std::max(fMax, fVal);
            }
        }

        auto aDPTypeEnd = aDPTypes.cend();

        auto pAttList = sax_fastparser::FastSerializerHelper::createAttrList();
        // tdf#89139: Only create item list for string-only fields.
        // Using containsXXX attributes in this case makes Excel think the file is corrupted.
        // OTOH listing items for e.g. number fields also triggers "corrupted" warning in Excel.
        bool bListItems = aDPTypes.size() == 1 && aDPTypes.find(ScDPItemData::String) != aDPTypeEnd;
        if (bListItems)
        {
            pAttList->add(XML_count, OString::number(static_cast<long>(rFieldItems.size())));
        }
        else
        {
            pAttList->add(XML_containsMixedTypes, XclXmlUtils::ToPsz10(aDPTypes.size() > 1));
            pAttList->add(XML_containsSemiMixedTypes, XclXmlUtils::ToPsz10(aDPTypes.size() > 1));
            pAttList->add(XML_containsString, XclXmlUtils::ToPsz10(aDPTypes.find(ScDPItemData::String) != aDPTypeEnd));
            if (aDPTypes.find(ScDPItemData::Value) != aDPTypeEnd)
            {
                pAttList->add(XML_containsNumber, XclXmlUtils::ToPsz10(true));
                pAttList->add(XML_minValue, OString::number(fMin));
                pAttList->add(XML_maxValue, OString::number(fMax));
            }
        }
        sax_fastparser::XFastAttributeListRef xAttributeList(pAttList);

        pDefStrm->startElement(XML_sharedItems, xAttributeList);

        if (bListItems)
        {
            it = rFieldItems.begin();
            for (; it != itEnd; ++it)
            {
                const ScDPItemData& rItem = *it;
                switch (rItem.GetType())
                {
                    case ScDPItemData::String:
                        pDefStrm->singleElement(XML_s,
                            XML_v, XclXmlUtils::ToOString(rItem.GetString()),
                            FSEND);
                    break;
                    case ScDPItemData::Value:
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
    for (const auto & rRange : rRanges)
    {
        const ScDPCache* pCache = rSheetCaches.getExistingCache(rRange);
        if (!pCache)
            continue;

        // Get all pivot objects that reference this cache, and set up an
        // object to cache ID mapping.
        const ScDPCache::ScDPObjectSet& rRefs = pCache->GetAllReferences();
        ScDPCache::ScDPObjectSet::const_iterator it = rRefs.begin(), itEnd = rRefs.end();
        for (; it != itEnd; ++it)
            maCacheIdMap.insert(CacheIdMapType::value_type(*it, aCaches.size()+1));

        XclExpXmlPivotCaches::Entry aEntry;
        aEntry.meType = XclExpXmlPivotCaches::Worksheet;
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
                m_Tables.insert(std::make_pair(nTab, o3tl::make_unique<XclExpXmlPivotTables>(GetRoot(), maCaches)));
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
                aDataFields.push_back(DataField(nPos, &rDim));
            break;
            case sheet::DataPilotFieldOrientation_HIDDEN:
            default:
                ;
        }
    }

    sax_fastparser::FSHelperPtr& pPivotStrm = rStrm.GetCurrentStream();
    pPivotStrm->startElement(XML_pivotTableDefinition,
        XML_xmlns, XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(xls))).getStr(),
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

    for (size_t i = 0; i < nFieldCount; ++i)
    {
        const ScDPSaveDimension* pDim = aCachedDims[i];
        if (!pDim)
        {
            pPivotStrm->singleElement(XML_pivotField,
                XML_showAll, BS(false),
                FSEND);
            continue;
        }

        sheet::DataPilotFieldOrientation eOrient = pDim->GetOrientation();

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

        // Dump field items.
        css::uno::Sequence<OUString> aMemberNames;
        {
            // We need to get the members in actual order, getting which requires non-const reference here
            auto& dpo = const_cast<ScDPObject&>(rDPObj);
            dpo.GetMemberNames(i, aMemberNames);
        }

        const ScDPCache::ScDPItemDataVec& rCacheFieldItems = rCache.GetDimMemberValues(i);
        std::vector<size_t> aMemberSequence;
        for (const OUString& sMemberName : aMemberNames)
        {
            auto it = std::find_if(rCacheFieldItems.begin(), rCacheFieldItems.end(),
                [&sMemberName](const ScDPItemData& arg) -> bool { return arg.GetString() == sMemberName; });
            if (it != rCacheFieldItems.end())
            {
                aMemberSequence.push_back(it - rCacheFieldItems.begin());
            }
        }

        auto pAttList = sax_fastparser::FastSerializerHelper::createAttrList();
        pAttList->add(XML_axis, toOOXMLAxisType(eOrient));
        pAttList->add(XML_showAll, BS(false));

        long nSubTotalCount = pDim->GetSubTotalsCount();
        std::vector<OString> aSubtotalSequence;
        for (long nSubTotal = 0; nSubTotal < nSubTotalCount; ++nSubTotal)
        {
            ScGeneralFunction eFunc = pDim->GetSubTotalFunc(nSubTotal);
            aSubtotalSequence.push_back(GetSubtotalFuncName(eFunc));
            sal_Int32 nAttToken = GetSubtotalAttrToken(eFunc);
            if (!pAttList->hasAttribute(nAttToken))
                pAttList->add(nAttToken, BS(true));
        }

        sax_fastparser::XFastAttributeListRef xAttributeList(pAttList);
        pPivotStrm->startElement(XML_pivotField, xAttributeList);

        pPivotStrm->startElement(XML_items,
            XML_count, OString::number(static_cast<long>(aMemberSequence.size() + aSubtotalSequence.size())),
            FSEND);

        for (size_t nMember : aMemberSequence)
        {
            pPivotStrm->singleElement(XML_item,
                XML_x, OString::number(static_cast<long>(nMember)),
                FSEND);
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
    maTables.push_back(Entry(pTable, nCacheId, nPivotId));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
