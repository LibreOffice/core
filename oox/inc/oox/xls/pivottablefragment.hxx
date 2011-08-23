/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef OOX_XLS_PIVOTTABLEFRAGMENT_HXX
#define OOX_XLS_PIVOTTABLEFRAGMENT_HXX

#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/worksheethelper.hxx"

namespace oox {
namespace xls {

class PivotTable;
class PivotTableField;
class PivotTableFilter;

// ============================================================================

class OoxPivotTableFieldContext : public OoxWorksheetContextBase
{
public:
    explicit            OoxPivotTableFieldContext(
                            OoxWorksheetFragmentBase& rFragment,
                            PivotTableField& rTableField );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        onStartRecord( RecordInputStream& rStrm );

private:
    PivotTableField&    mrTableField;
};

// ============================================================================

class OoxPivotTableFilterContext : public OoxWorksheetContextBase
{
public:
    explicit            OoxPivotTableFilterContext(
                            OoxWorksheetFragmentBase& rFragment,
                            PivotTableFilter& rTableFilter );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        onStartRecord( RecordInputStream& rStrm );

private:
    PivotTableFilter&   mrTableFilter;
};

// ============================================================================

class OoxPivotTableFragment : public OoxWorksheetFragmentBase
{
public:
    explicit            OoxPivotTableFragment(
                            const WorksheetHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual const ::oox::core::RecordInfo* getRecordInfos() const;

private:
    PivotTable&         mrPivotTable;
};

// ============================================================================
// ============================================================================

class BiffPivotTableContext : public BiffWorksheetContextBase
{
public:
    explicit            BiffPivotTableContext( const BiffWorksheetFragmentBase& rFragment, PivotTable& rPivotTable );

    /** Imports all records related to the current pivot table. */
    virtual void        importRecord();

private:
    PivotTable&         mrPivotTable;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

