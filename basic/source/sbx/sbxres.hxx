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

#ifndef INCLUDED_BASIC_SOURCE_SBX_SBXRES_HXX
#define INCLUDED_BASIC_SOURCE_SBX_SBXRES_HXX

#include <rtl/ustring.hxx>

// Currently there are no resources provided in the SVTOOLS-Project.
// Because it is non-critical resources (BASIC-Keywords),
// we can work with dummies.

enum class StringId {
    Types      =  0,
    Any        = 13,
    As         = 32,
    Optional   = 33,
    ByRef      = 34,

    NameProp   = 35,
    ParentProp = 36,
    CountProp  = 38,
    AddMeth    = 39,
    ItemMeth   = 40,
    RemoveMeth = 41,

    ErrorMsg   = 42,
    False      = 43,
    True       = 44,

    LastValue  = 44
};

OUString GetSbxRes( StringId );


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
