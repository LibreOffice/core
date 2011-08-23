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

