/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: webquerybuffer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:49 $
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

#ifndef OOX_XLS_WEBQUERYBUFFER_HXX
#define OOX_XLS_WEBQUERYBUFFER_HXX

#include <vector>
#include <hash_map>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

struct QueryTable
{
    sal_Int32   mnConnectionId;
};

// struct WebQuery
// {
//     enum WebQueryMode
//     {
//         xlWQUnknown,                /// Not specified.
//         xlWQDocument,               /// Entire document.
//         xlWQAllTables,              /// All tables.
//         xlWQSpecTables              /// Specific tables.
//     };
//
//     ::rtl::OUString maURL;                          /// Source document URL.
//     ::rtl::OUString maTables;                       /// List of source range names.
//     ::com::sun::star::table::CellRangeAddress maDestRange; /// Destination range.
//     WebQueryMode meMode;                            /// Current mode of web query.
//     sal_uInt16 mnRefresh;                           /// Refresh time in minutes.
// };

struct BaseProperties
{
};

/** Stores web query properties from webPr element. */
struct WebProperties : public BaseProperties
{
    ::rtl::OUString     maURL;
};

/** Stores text data source properties from textPr element. */
struct TextProperties : public BaseProperties
{
    ::rtl::OUString     maSourceFile;
};

struct Connection
{
    static const sal_Int32 CONNECTION_ODBC_SOURCE;
    static const sal_Int32 CONNECTION_DAO_SOURCE;
    static const sal_Int32 CONNECTION_FILE_SOURCE;
    static const sal_Int32 CONNECTION_WEBQUERY;
    static const sal_Int32 CONNECTION_OLEDB_SOURCE;
    static const sal_Int32 CONNECTION_TEXT_SOURCE;
    static const sal_Int32 CONNECTION_ADO_RECORD_SET;
    static const sal_Int32 CONNECTION_DSP;

    ::rtl::OUString     maName;                     /// Connection name (must be unique).
    sal_Int32           mnType;
    ::boost::shared_ptr< BaseProperties > mpProperties;
};

// ============================================================================

typedef ::std::hash_map< ::rtl::OUString, QueryTable, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > QueryTableHashMap;

class WebQueryBuffer : public WorkbookHelper
{
public:
    explicit            WebQueryBuffer( const WorkbookHelper& rHelper );

    void                importQueryTable( const AttributeList& rAttribs );

    void                importConnection( const AttributeList& rAttribs );

    void                importWebPr( const AttributeList& rAttribs );

    /** Dumps content for debug purposes. */
    void                dump() const;

private:
    QueryTableHashMap           maQueryTableMap;
    ::std::vector< Connection > maConnections;

    sal_Int32                   mnCurConnId;        /// Current connection ID.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

