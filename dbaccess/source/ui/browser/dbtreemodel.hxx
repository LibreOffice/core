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



#ifndef DBAUI_DBTREEMODEL_HXX
#define DBAUI_DBTREEMODEL_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _SVLBOX_HXX
#include <svtools/svlbox.hxx>
#endif
#ifndef _SVLBOXITM_HXX
#include <svtools/svlbitm.hxx>
#endif
#ifndef _SBA_UNODATBR_HXX_
#include "unodatbr.hxx"
#endif
#ifndef _DBAUI_COMMON_TYPES_HXX_
#include "commontypes.hxx"
#endif

// syntax of the tree           userdata
// datasource                   holds the connection
//   queries                    holds the nameaccess for the queries
//     query                    holds the query
//   tables                     holds the nameaccess for the tables
//     table                    holds the table

#define CONTAINER_QUERIES       sal_uLong( 0 )
#define CONTAINER_TABLES        sal_uLong( 1 )

namespace com { namespace sun { namespace star { namespace lang { class XMultiServiceFactory; } } } }

namespace dbaui
{
    struct DBTreeListUserData
    {
        /// if the entry denotes a table or query, this is the respective UNO object
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                        xObjectProperties;
        /// if the entry denotes a object container, this is the UNO interface for this container
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                                        xContainer;
        /// if the entry denotes a data source, this is the connection for this data source (if already connection)
        SharedConnection                xConnection;
        SbaTableQueryBrowser::EntryType eType;
        String                          sAccessor;

        DBTreeListUserData();
        ~DBTreeListUserData();
    };
}

#endif // DBAUI_DBTREEMODEL_HXX
