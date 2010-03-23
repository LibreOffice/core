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

#ifndef OOX_XLS_PIVOTCACHEFRAGMENT_HXX
#define OOX_XLS_PIVOTCACHEFRAGMENT_HXX

#include "oox/xls/excelhandlers.hxx"

namespace oox {
namespace xls {

class PivotCache;
class PivotCacheField;

// ============================================================================

class OoxPivotCacheFieldContext : public OoxWorkbookContextBase
{
public:
    explicit            OoxPivotCacheFieldContext(
                            OoxWorkbookFragmentBase& rFragment,
                            PivotCacheField& rCacheField );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        onStartRecord( RecordInputStream& rStrm );

private:
    PivotCacheField&    mrCacheField;
};

// ============================================================================

class OoxPivotCacheDefinitionFragment : public OoxWorkbookFragmentBase
{
public:
    explicit            OoxPivotCacheDefinitionFragment(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath,
                            PivotCache& rPivotCache );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual const ::oox::core::RecordInfo* getRecordInfos() const;
    virtual void        finalizeImport();

private:
    PivotCache&         mrPivotCache;
};

// ============================================================================

class OoxPivotCacheRecordsFragment : public OoxWorksheetFragmentBase
{
public:
    explicit            OoxPivotCacheRecordsFragment(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath,
                            const PivotCache& rPivotCache );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual const ::oox::core::RecordInfo* getRecordInfos() const;

private:
    void                startCacheRecord();
    void                importPCRecord( RecordInputStream& rStrm );
    void                importPCRecordItem( sal_Int32 nRecId, RecordInputStream& rStrm );

private:
    const PivotCache&   mrPivotCache;
    sal_Int32           mnCol;
    sal_Int32           mnRow;
    bool                mbInRecord;
};

// ============================================================================
// ============================================================================

class BiffPivotCacheFragment : public BiffWorkbookFragmentBase
{
public:
    explicit            BiffPivotCacheFragment(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rStrmName,
                            PivotCache& rPivotCache );

    /** Imports the entire fragment, returns true, if EOF record has been reached. */
    virtual bool        importFragment();

private:
    PivotCache&         mrPivotCache;
};

// ============================================================================

class BiffPivotCacheRecordsContext : public BiffWorksheetContextBase
{
public:
    explicit            BiffPivotCacheRecordsContext(
                            const BiffWorkbookFragmentBase& rFragment,
                            const PivotCache& rPivotCache );

    /** Reads the current record from stream and tries to insert a cell into
        the source data sheet. */
    virtual void        importRecord();

private:
    void                startNextRow();

private:
    typedef ::std::vector< sal_Int32 > ColumnIndexVector;

    const PivotCache&   mrPivotCache;
    ColumnIndexVector   maUnsharedCols; /// Column indexes of all unshared cache fields.
    size_t              mnColIdx;       /// Current index into maUnsharedCols.
    sal_Int32           mnRow;          /// Current row in source data (0-based).
    bool                mbHasShared;    /// True = pivot cache contains fields with shared items.
    bool                mbInRow;        /// True = a data row has been started.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

