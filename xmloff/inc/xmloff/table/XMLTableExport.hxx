/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _XMLOFF_XMLTABLEEXPORT_HXX
#define _XMLOFF_XMLTABLEEXPORT_HXX

#include "sal/config.h"

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>

#include <com/sun/star/table/XTableColumns.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

#include <rtl/ref.hxx>

#include "xmloff/dllapi.h"
#include "xmloff/uniref.hxx"
#include "xmloff/xmlprmap.hxx"
#include "xmloff/xmlexppr.hxx"


//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SvXMLExport;
class SvXMLExportPropertyMapper;

typedef ::std::map< const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >, rtl::OUString > TableStyleMap;

struct XMLTableInfo
{
    TableStyleMap   maColumnStyleMap;
    TableStyleMap   maRowStyleMap;
    TableStyleMap   maCellStyleMap;
    std::vector< ::rtl::OUString > maDefaultRowCellStyles;
};

typedef ::std::map< const ::com::sun::star::uno::Reference< com::sun::star::table::XColumnRowRange >, boost::shared_ptr< XMLTableInfo > > TableInfoMap;

class XMLOFF_DLLPUBLIC XMLTableExport : public UniRefBase
{
public:
    XMLTableExport(SvXMLExport& rExp, const rtl::Reference< SvXMLExportPropertyMapper >& xCellExportPropertySetMapper, const rtl::Reference< XMLPropertyHandlerFactory >& xFactoryRef );
    virtual ~XMLTableExport();

    // This method collects all automatic styles for the given table
    void collectTableAutoStyles(const com::sun::star::uno::Reference < com::sun::star::table::XColumnRowRange >& xColumnRowRange);

    // This method exports the given table
    void exportTable(const com::sun::star::uno::Reference < com::sun::star::table::XColumnRowRange >& xColumnRowRange);

    // export the styles from the cell style family
    void exportTableStyles();

    // Export the collected automatic styles
    void exportAutoStyles();

private:
    void exportTableTemplates();

    SvXMLExport&                                    mrExport;
    rtl::Reference< SvXMLExportPropertyMapper >     mxCellExportPropertySetMapper;
    rtl::Reference< SvXMLExportPropertyMapper >     mxRowExportPropertySetMapper;
    rtl::Reference< SvXMLExportPropertyMapper >     mxColumnExportPropertySetMapper;
    TableInfoMap                                    maTableInfoMap;
    bool                                            mbExportTables;

protected:
    SvXMLExport& GetExport() { return mrExport; }
    const SvXMLExport& GetExport() const  { return mrExport; }
private:

    SAL_DLLPRIVATE void ImpExportText( const com::sun::star::uno::Reference < com::sun::star::table::XCell >& xCell );

    void ExportCell( const com::sun::star::uno::Reference < com::sun::star::table::XCell >& xCell, const boost::shared_ptr< XMLTableInfo >& pTableInfo, const ::rtl::OUString& sDefaultCellStyle  );
    void ExportTableColumns( const com::sun::star::uno::Reference < com::sun::star::container::XIndexAccess >& xtableColumns, const boost::shared_ptr< XMLTableInfo >& pTableInfo );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
