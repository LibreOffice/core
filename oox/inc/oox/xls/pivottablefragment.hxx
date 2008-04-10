/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pivottablefragment.hxx,v $
 * $Revision: 1.4 $
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
#include "oox/xls/pivottablebuffer.hxx"
#include "oox/xls/worksheethelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

class OoxPivotTableFragment : public OoxWorksheetFragmentBase
{
public:
    explicit            OoxPivotTableFragment(
                            const WorksheetHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );

    // oox.core.FragmentHandler2 interface ------------------------------------

    virtual void        finalizeImport();

private:
    void                importLocation( const AttributeList& rAttribs );
    void                importPivotTableDefinition( const AttributeList& rAttribs );
    void                importPivotFields( const AttributeList& rAttribs );
    void                importPivotField( const AttributeList& rAttribs );

private:
    ::rtl::OUString maName;
    PivotTableData  maData;

    bool        mbValidRange;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

