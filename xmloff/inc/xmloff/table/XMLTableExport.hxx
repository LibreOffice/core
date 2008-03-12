/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTableExport.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:24:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    XMLTableExport(SvXMLExport& rExp, const rtl::Reference< XMLPropertySetMapper >& xCellPropertySetMapper, const rtl::Reference< XMLPropertyHandlerFactory >& xFactoryRef );
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

protected:
    SvXMLExport& GetExport() { return mrExport; }
    const SvXMLExport& GetExport() const  { return mrExport; }
private:

    SAL_DLLPRIVATE void ImpExportText( const com::sun::star::uno::Reference < com::sun::star::table::XCell >& xCell );

    void ExportCell( const com::sun::star::uno::Reference < com::sun::star::table::XCell >& xCell, const boost::shared_ptr< XMLTableInfo >& pTableInfo, const ::rtl::OUString& sDefaultCellStyle  );
    void ExportTableColumns( const com::sun::star::uno::Reference < com::sun::star::container::XIndexAccess >& xtableColumns, const boost::shared_ptr< XMLTableInfo >& pTableInfo );

};

#endif
