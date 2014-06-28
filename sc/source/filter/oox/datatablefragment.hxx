/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_OOX_DATATABLEFRAGMENT_HXX
#define INCLUDED_SC_OOX_DATATABLEFRAGMENT_HXX

#include "excelhandlers.hxx"
#include "worksheethelper.hxx"
#include "tabledata.hxx"

namespace oox {
namespace xls {

class DataTableContext : public WorksheetContextBase
{
public:
    explicit DataTableContext(WorksheetFragmentBase& rFragment, ScTableData& rDataTable);

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) SAL_OVERRIDE;
    virtual void        onStartElement( const AttributeList& rAttribs ) SAL_OVERRIDE;
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) SAL_OVERRIDE;
    virtual void        onStartRecord( SequenceInputStream& rStrm ) SAL_OVERRIDE;

private:
    ScTableData& mrDataTable;

    void importTableStyleInfo(const AttributeList& rAttribs);
    void importTable(const AttributeList& rAttribs);
    void importTableColumn(const AttributeList& rAttribs);
};

class DataTableFragment : public WorksheetFragmentBase
{
public:
    explicit DataTableFragment(const WorksheetHelper& rHelper,
                            const OUString& rFragmentPath, const OUString& rId);

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) SAL_OVERRIDE;
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) SAL_OVERRIDE;
    virtual const ::oox::core::RecordInfo* getRecordInfos() const SAL_OVERRIDE;

private:

    ScTableData& mrDataTable;
};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
