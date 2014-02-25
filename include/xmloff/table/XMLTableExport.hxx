/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_XMLOFF_TABLE_XMLTABLEEXPORT_HXX
#define INCLUDED_XMLOFF_TABLE_XMLTABLEEXPORT_HXX

#include <sal/config.h>

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

#include <xmloff/dllapi.h>
#include <xmloff/uniref.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlexppr.hxx>



// predeclarations

class SvXMLExport;
class SvXMLExportPropertyMapper;

typedef ::std::map< const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >, OUString > TableStyleMap;

struct XMLTableInfo
{
    TableStyleMap   maColumnStyleMap;
    TableStyleMap   maRowStyleMap;
    TableStyleMap   maCellStyleMap;
    std::vector< OUString > maDefaultRowCellStyles;
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

    void ExportCell( const com::sun::star::uno::Reference < com::sun::star::table::XCell >& xCell, const boost::shared_ptr< XMLTableInfo >& pTableInfo, const OUString& sDefaultCellStyle  );
    void ExportTableColumns( const com::sun::star::uno::Reference < com::sun::star::container::XIndexAccess >& xtableColumns, const boost::shared_ptr< XMLTableInfo >& pTableInfo );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
