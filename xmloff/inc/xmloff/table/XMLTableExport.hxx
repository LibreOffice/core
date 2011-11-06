/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
