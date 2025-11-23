/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <xedbdata.hxx>
#include <excrecds.hxx>
#include <dbdata.hxx>
#include <document.hxx>
#include <tablestyle.hxx>
#include <oox/export/utils.hxx>
#include <oox/token/namespaces.hxx>
#include <sax/fastattribs.hxx>

using namespace oox;

namespace {

/** (So far) dummy implementation of table export for BIFF5/BIFF7. */
class XclExpTablesImpl5 : public XclExpTables
{
public:
    explicit            XclExpTablesImpl5( const XclExpRoot& rRoot );

    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;
};

/** Implementation of table export for OOXML, so far dummy for BIFF8. */
class XclExpTablesImpl8 : public XclExpTables
{
public:
    explicit            XclExpTablesImpl8( const XclExpRoot& rRoot );

    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;
};

}

XclExpTablesImpl5::XclExpTablesImpl5( const XclExpRoot& rRoot ) :
    XclExpTables( rRoot )
{
}

void XclExpTablesImpl5::Save( XclExpStream& /*rStrm*/ )
{
    // not implemented
}

void XclExpTablesImpl5::SaveXml( XclExpXmlStream& /*rStrm*/ )
{
    // not applicable
}


XclExpTablesImpl8::XclExpTablesImpl8( const XclExpRoot& rRoot ) :
    XclExpTables( rRoot )
{
}

void XclExpTablesImpl8::Save( XclExpStream& /*rStrm*/ )
{
    // not implemented
}

void XclExpTablesImpl8::SaveXml( XclExpXmlStream& rStrm )
{

    sax_fastparser::FSHelperPtr& pWorksheetStrm = rStrm.GetCurrentStream();
    pWorksheetStrm->startElement(XML_tableParts);
    for (auto const& it : maTables)
    {
        OUString aRelId;
        sax_fastparser::FSHelperPtr pTableStrm = rStrm.CreateOutputStream(
                XclXmlUtils::GetStreamName("xl/tables/", "table", it.mnTableId),
                XclXmlUtils::GetStreamName("../tables/", "table", it.mnTableId),
                pWorksheetStrm->getOutputStream(),
                CREATE_XL_CONTENT_TYPE("table"),
                CREATE_OFFICEDOC_RELATION_TYPE("table"),
                &aRelId);

        pWorksheetStrm->singleElement(XML_tablePart, FSNS(XML_r, XML_id), aRelId.toUtf8());

        rStrm.PushStream( pTableStrm);
        SaveTableXml( rStrm, it);
        rStrm.PopStream();
    }
    pWorksheetStrm->endElement( XML_tableParts);
}


XclExpTablesManager::XclExpTablesManager( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
}

XclExpTablesManager::~XclExpTablesManager()
{
}

void XclExpTablesManager::Initialize()
{
    // All non-const to be able to call RefreshTableColumnNames().
    ScDocument& rDoc = GetDoc();
    ScDBCollection* pDBColl = rDoc.GetDBCollection();
    if (!pDBColl)
        return;

    ScDBCollection::NamedDBs& rDBs = pDBColl->getNamedDBs();
    if (rDBs.empty())
        return;

    sal_Int32 nTableId = 0;
    for (const auto& rxDB : rDBs)
    {
        ScDBData* pDBData = rxDB.get();
        pDBData->RefreshTableColumnNames( &rDoc);   // currently not in sync, so refresh
        ScRange aRange( ScAddress::UNINITIALIZED);
        pDBData->GetArea( aRange);
        SCTAB nTab = aRange.aStart.Tab();
        TablesMapType::iterator it = maTablesMap.find( nTab);
        if (it == maTablesMap.end())
        {
            rtl::Reference< XclExpTables > pNew;
            switch( GetBiff() )
            {
                case EXC_BIFF5:
                    pNew = new XclExpTablesImpl5( GetRoot());
                    break;
                case EXC_BIFF8:
                    pNew = new XclExpTablesImpl8( GetRoot());
                    break;
                default:
                    assert(!"Unknown BIFF type!");
                    continue;   // for
            }
            ::std::pair< TablesMapType::iterator, bool > ins( maTablesMap.insert( ::std::make_pair( nTab, pNew)));
            if (!ins.second)
            {
                assert(!"XclExpTablesManager::Initialize - XclExpTables insert failed");
                continue;   // for
            }
            it = ins.first;
        }
        it->second->AppendTable( pDBData, ++nTableId);
    }
}

rtl::Reference< XclExpTables > XclExpTablesManager::GetTablesBySheet( SCTAB nTab )
{
    TablesMapType::iterator it = maTablesMap.find(nTab);
    return it == maTablesMap.end() ? nullptr : it->second;
}

XclExpTables::Entry::Entry( const ScDBData* pData, sal_Int32 nTableId ) :
    mpData(pData), mnTableId(nTableId)
{
}

XclExpTables::XclExpTables( const XclExpRoot& rRoot ) :
    XclExpRoot(rRoot)
{
}

XclExpTables::~XclExpTables()
{
}

void XclExpTables::AppendTable( const ScDBData* pData, sal_Int32 nTableId )
{
    maTables.emplace_back( pData, nTableId);
}

void XclExpTables::SaveTableXml( XclExpXmlStream& rStrm, const Entry& rEntry )
{
    const ScDBData& rData = *rEntry.mpData;
    ScRange aRange( ScAddress::UNINITIALIZED);
    rData.GetArea( aRange);
    sax_fastparser::FSHelperPtr& pTableStrm = rStrm.GetCurrentStream();

    bool hasTableTypeAttr = !rData.GetTableType().isEmpty();
    std::optional<OUString> tableType = std::nullopt;

    if (hasTableTypeAttr)
        tableType = rData.GetTableType();

    const std::vector<TableColumnAttributes> aTotalValues
        = rData.GetTotalRowAttributes(formula::FormulaGrammar::GRAM_OOXML);

    // if the Total row have ever been showed it will be true
    bool hasAnySetValue = std::any_of(aTotalValues.begin(), aTotalValues.end(),
                                      [](const TableColumnAttributes& attr)
                                      {
                                          return attr.maTotalsRowLabel.has_value()
                                                 || attr.maTotalsFunction.has_value()
                                                 || attr.maCustomFunction.has_value();
                                      });

    pTableStrm->startElement( XML_table,
        XML_xmlns, rStrm.getNamespaceURL(OOX_NS(xls)).toUtf8(),
        XML_id, OString::number( rEntry.mnTableId),
        XML_name, rData.GetName().toUtf8(),
        XML_displayName, rData.GetName().toUtf8(),
        XML_ref, XclXmlUtils::ToOString(rStrm.GetRoot().GetDoc(), aRange),
        XML_tableType, tableType,
        XML_headerRowCount, ToPsz10(rData.HasHeader()),
        XML_totalsRowCount, ToPsz10(rData.HasTotals()),
        XML_totalsRowShown, ToPsz10(hasAnySetValue)
        // OOXTODO: XML_comment, ...,
        // OOXTODO: XML_connectionId, ...,
        // OOXTODO: XML_dataCellStyle, ...,
        // OOXTODO: XML_dataDxfId, ...,
        // OOXTODO: XML_headerRowBorderDxfId, ...,
        // OOXTODO: XML_headerRowCellStyle, ...,
        // OOXTODO: XML_headerRowDxfId, ...,
        // OOXTODO: XML_insertRow, ...,
        // OOXTODO: XML_insertRowShift, ...,
        // OOXTODO: XML_published, ...,
        // OOXTODO: XML_tableBorderDxfId, ...,
        // OOXTODO: XML_totalsRowBorderDxfId, ...,
        // OOXTODO: XML_totalsRowCellStyle, ...,
        // OOXTODO: XML_totalsRowDxfId, ...
    );

    if (rData.HasAutoFilter())
    {
        /* TODO: does this need to exclude totals row? */

        /* TODO: in OOXML  12.3.21 Table Definition Part  has information
         * that an applied autoFilter has child elements
         * <af:filterColumn><af:filters><af:filter>.
         * When not applied but buttons hidden, Excel writes, for example,
         * <filterColumn colId="0" hiddenButton="1"/> */

        ExcAutoFilterRecs aAutoFilter( rStrm.GetRoot(), aRange.aStart.Tab(), &rData);
        aAutoFilter.SaveXml( rStrm);
    }

    const std::vector< OUString >& rColNames = rData.GetTableColumnNames();
    const std::vector< TableColumnModel >& rTableColumnModel = rData.GetTableColumnModel();
    if (!rColNames.empty())
    {
        // rColNames and aTotalValues size should always be equal
        assert((rColNames.size() == aTotalValues.size()) &&
               "XclExpTables::SaveTableXml - mismatch between column names and total values");

        pTableStrm->startElement(XML_tableColumns,
                XML_count, OString::number(aRange.aEnd.Col() - aRange.aStart.Col() + 1));

        for (size_t i=0, n=rColNames.size(); i < n; ++i)
        {
            // OOXTODO: write <calculatedColumnFormula> once we support it, in
            // which case we'd need start/endElement XML_tableColumn for such
            // column.

            std::optional<OUString> uniqueName = std::nullopt;

            // uniqueName should only be used when this table's tableType is queryTable or xml.
            // in this implementation: if tableType attribute exists, it has either queryTable or xml value.
            if (hasTableTypeAttr)
            {
                if (i < rTableColumnModel.size() && !rTableColumnModel[i].maUniqueName.isEmpty())
                    uniqueName = rTableColumnModel[i].maUniqueName;
                else
                    uniqueName = rColNames[i]; // fallback to column name if no unique name.
            }

            pTableStrm->startElement( XML_tableColumn,
                    XML_id, OString::number(i+1),
                    XML_uniqueName, uniqueName,
                    XML_name, rColNames[i].toUtf8(),
                    XML_totalsRowLabel, (i < aTotalValues.size() ? aTotalValues[i].maTotalsRowLabel : std::nullopt),
                    XML_totalsRowFunction, (i < aTotalValues.size() ? aTotalValues[i].maTotalsFunction : std::nullopt)
                    // OOXTODO: XML_dataCellStyle, ...,
                    // OOXTODO: XML_dataDxfId, ...,
                    // OOXTODO: XML_headerRowCellStyle, ...,
                    // OOXTODO: XML_headerRowDxfId, ...,
                    // OOXTODO: XML_queryTableFieldId, ...,
                    // OOXTODO: XML_totalsRowCellStyle, ...,
                    // OOXTODO: XML_totalsRowDxfId, ...,
                    // OOXTODO: XML_uniqueName, ...
            );

            if (i < rTableColumnModel.size() && rTableColumnModel[i].mxXmlColumnPr)
            {
                // export <xmlColumnPr>
                rtl::Reference<sax_fastparser::FastAttributeList> pXmlColumnPrAttrList
                    = sax_fastparser::FastSerializerHelper::createAttrList();

                XmlColumnPrModel* rXmlColRef = rTableColumnModel[i].mxXmlColumnPr.get();

                pXmlColumnPrAttrList->add(XML_mapId, OUString::number(rXmlColRef->mnMapId));
                pXmlColumnPrAttrList->add(XML_xpath, rXmlColRef->msXpath);
                pXmlColumnPrAttrList->add(XML_xmlDataType, rXmlColRef->msXmlDataType);
                pXmlColumnPrAttrList->add(XML_denormalized, ToPsz10(rXmlColRef->mbDenormalized));

                pTableStrm->singleElement(XML_xmlColumnPr, pXmlColumnPrAttrList);
            }

            if (i < aTotalValues.size() && aTotalValues[i].maTotalsFunction.has_value()
                && aTotalValues[i].maTotalsFunction.value() == u"custom"_ustr)
            {
                // write custom functions
                pTableStrm->startElement(XML_totalsRowFormula);
                pTableStrm->writeEscaped(aTotalValues[i].maCustomFunction.value());
                pTableStrm->endElement(XML_totalsRowFormula);
            }

            // put </tableColumn>
            pTableStrm->endElement(XML_tableColumn);
        }

        pTableStrm->endElement( XML_tableColumns);
    }

    if (const ScTableStyleParam* pParam = rData.GetTableStyleInfo())
    {
        const OUString& rStyleName = pParam->maStyleID;
        if (const ScTableStyles* pTableStyles = rStrm.GetRoot().GetDoc().GetTableStyles())
        {
            if (pTableStyles->GetTableStyle(rStyleName))
            {
                pTableStrm->singleElement(XML_tableStyleInfo, XML_name, rStyleName.toUtf8(),
                                          XML_showFirstColumn, ToPsz10(pParam->mbFirstColumn),
                                          XML_showLastColumn, ToPsz10(pParam->mbLastColumn),
                                          XML_showRowStripes, ToPsz10(pParam->mbRowStripes),
                                          XML_showColumnStripes, ToPsz10(pParam->mbColumnStripes));
            }
        }
    }

    pTableStrm->endElement( XML_table);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
