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

#ifndef OOX_XLS_CHARTSHEETFRAGMENT_HXX
#define OOX_XLS_CHARTSHEETFRAGMENT_HXX

#include "oox/xls/excelhandlers.hxx"

namespace oox {
namespace xls {

// ============================================================================

class ChartsheetFragment : public WorksheetFragmentBase
{
public:
    explicit            ChartsheetFragment(
                            const WorksheetHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const ::rtl::OUString& rChars );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );

    virtual const ::oox::core::RecordInfo* getRecordInfos() const;
    virtual void        initializeImport();
    virtual void        finalizeImport();

private:
    /** Imports the the relation identifier for the DrawingML part. */
    void                importDrawing( const AttributeList& rAttribs );
    /** Imports the DRAWING record containing the relation identifier for the DrawingML part. */
    void                importDrawing( SequenceInputStream& rStrm );
};

// ============================================================================

class BiffChartsheetFragment : public BiffWorksheetFragmentBase
{
public:
    explicit            BiffChartsheetFragment(
                            const WorksheetHelper& rHelper,
                            const BiffWorkbookFragmentBase& rParent );

    /** Imports the entire sheet fragment, returns true, if EOF record has been reached. */
    virtual bool        importFragment();
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
