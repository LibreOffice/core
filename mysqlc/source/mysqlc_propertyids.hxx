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

#ifndef INCLUDED_MYSQLC_SOURCE_MYSQLC_PROPERTYIDS_HXX
#define INCLUDED_MYSQLC_SOURCE_MYSQLC_PROPERTYIDS_HXX

// this define has to be set to split the names into different dll's or so's
// every dll has his own set of property names
#include <rtl/ustring.hxx>
#include <map>

namespace connectivity
{
namespace mysqlc
{
enum
{
    PROPERTY_ID_QUERYTIMEOUT = 1,
    PROPERTY_ID_MAXFIELDSIZE,
    PROPERTY_ID_MAXROWS,
    PROPERTY_ID_CURSORNAME,
    PROPERTY_ID_RESULTSETCONCURRENCY,
    PROPERTY_ID_RESULTSETTYPE,
    PROPERTY_ID_FETCHDIRECTION,
    PROPERTY_ID_FETCHSIZE,
    PROPERTY_ID_ESCAPEPROCESSING,
    PROPERTY_ID_USEBOOKMARKS,
    PROPERTY_ID_ISBOOKMARKABLE
};
}/* mysqlc */
}/* connectivity */

#endif // INCLUDED_MYSQLC_SOURCE_MYSQLC_PROPERTYIDS_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
