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
