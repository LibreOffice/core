/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "datatablefragment.hxx"
#include "datatablebuffer.hxx"

namespace oox {
namespace xls {

DataTableContext::DataTableContext(WorksheetFragmentBase& rFragment,
        ScTableData& rDataTable):
    WorksheetContextBase(rFragment),
    mrDataTable(rDataTable)
{
}

oox::core::ContextHandlerRef DataTableContext::onCreateContext(sal_Int32 , const AttributeList& )
{
    return this;
}

void DataTableContext::importTableStyleInfo(const AttributeList& rAttribs)
{
    mrDataTable.mbFirstColumn = rAttribs.getBool(XML_showFirstColumn, false);
    mrDataTable.mbLastColumn = rAttribs.getBool(XML_showLastColumn, false);
    mrDataTable.mbBandedRows = rAttribs.getBool(XML_showRowStripes, false);
    mrDataTable.mbBandedColumns = rAttribs.getBool(XML_showColumnStripes, false);
    mrDataTable.maStyleName = rAttribs.getXString(XML_name, OUString());
}

void DataTableContext::importTable(const AttributeList& rAttribs)
{
    mrDataTable.maName = rAttribs.getXString(XML_name, OUString());
    mrDataTable.maRange.Parse(rAttribs.getXString(XML_ref, OUString()));
}

void DataTableContext::importTableColumn(const AttributeList& rAttribs)
{
    ScTableDataColumn aColumn;
    aColumn.maName = rAttribs.getXString(XML_name, OUString());
    aColumn.mnId = rAttribs.getInteger(XML_id, 0);
    mrDataTable.maColumnData.push_back(aColumn);
}

void DataTableContext::onStartElement(const AttributeList& rAttribs)
{
    switch(getCurrentElement())
    {
        case XLS_TOKEN(autoFilter):
            break;
        case XLS_TOKEN(tableColumns):
            break;
        case XLS_TOKEN(tableColumn):
            importTableColumn(rAttribs);
            break;
        case XLS_TOKEN(tableStyleInfo):
            importTableStyleInfo(rAttribs);
            break;
        case XLS_TOKEN(table):
            importTable(rAttribs);
            break;
        default:
            SAL_WARN("sc.oox", "unsupported table element");
    }
}

oox::core::ContextHandlerRef DataTableContext::onCreateRecordContext(sal_Int32 , SequenceInputStream& )
{
    return 0;
}

void DataTableContext::onStartRecord(SequenceInputStream& )
{
}

DataTableFragment::DataTableFragment(const WorksheetHelper& rHelper,
        const OUString& rFragmentPath, const OUString& rId):
    WorksheetFragmentBase(rHelper, rFragmentPath),
    mrDataTable(getTableDataBuffer().createDataTable(rId))
{
}

oox::core::ContextHandlerRef DataTableFragment::onCreateContext(sal_Int32 nElement, const AttributeList& )
{
    switch(getCurrentElement())
    {
        case oox::core::XML_ROOT_CONTEXT:
            if(nElement == XLS_TOKEN(table))
                return new DataTableContext(*this, mrDataTable);
            break;
    }

    return 0;
}

oox::core::ContextHandlerRef DataTableFragment::onCreateRecordContext(sal_Int32 , SequenceInputStream& )
{
    return 0;
}

const oox::core::RecordInfo* DataTableFragment::getRecordInfos() const
{
    return 0;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
