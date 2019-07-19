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
#ifndef INCLUDED_SVX_OPTGENRL_HXX
#define INCLUDED_SVX_OPTGENRL_HXX

// Defines for setting the focus of an Edit via a slot from external.
enum class EditPosition
{
    UNKNOWN    = 0,
    COMPANY    = 1,
    FIRSTNAME  = 2,
    LASTNAME   = 3,
    STREET     = 4,
    COUNTRY    = 5,
    PLZ        = 6,
    CITY       = 7,
    STATE      = 8,
    TITLE      = 9,
    POSITION   = 10,
    SHORTNAME  = 11,
    TELPRIV    = 12,
    TELCOMPANY = 13,
    FAX        = 14,
    EMAIL      = 15,
};

#endif // INCLUDED_SVX_OPTGENRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
