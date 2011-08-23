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

#ifndef OOX_XLS_TABLEFRAGMENT_HXX
#define OOX_XLS_TABLEFRAGMENT_HXX

#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/tablebuffer.hxx"

namespace oox {
namespace xls {

// ============================================================================

class OoxTableFragment : public OoxWorksheetFragmentBase
{
public:
    explicit            OoxTableFragment(
                            const WorksheetHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );

    // oox.core.FragmentHandler2 interface ------------------------------------

    virtual const ::oox::core::RecordInfo* getRecordInfos() const;

private:
    TableRef            mxTable;        /// Current table.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

