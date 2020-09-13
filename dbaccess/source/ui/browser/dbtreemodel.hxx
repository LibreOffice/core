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

#pragma once

#include <unodatbr.hxx>
#include <commontypes.hxx>

// syntax of the tree           userdata
// datasource                   holds the connection
//   queries                    holds the nameaccess for the queries
//     query                    holds the query
//   tables                     holds the nameaccess for the tables
//     table                    holds the table

#define CONTAINER_QUERIES       sal_uLong( 0 )
#define CONTAINER_TABLES        sal_uLong( 1 )

namespace com::sun::star::lang { class XMultiServiceFactory; }

namespace dbaui
{
    struct DBTreeListUserData
    {
        /// if the entry denotes a table or query, this is the respective UNO object
        css::uno::Reference< css::beans::XPropertySet >
                                        xObjectProperties;
        /// if the entry denotes an object container, this is the UNO interface for this container
        css::uno::Reference< css::uno::XInterface >
                                        xContainer;
        /// if the entry denotes a data source, this is the connection for this data source (if already connection)
        SharedConnection                xConnection;
        SbaTableQueryBrowser::EntryType eType;
        OUString                        sAccessor;

        DBTreeListUserData();
        ~DBTreeListUserData();
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
