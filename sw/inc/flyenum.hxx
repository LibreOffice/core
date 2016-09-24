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

#ifndef INCLUDED_SW_INC_FLYENUM_HXX
#define INCLUDED_SW_INC_FLYENUM_HXX

enum FlyCntType
{
     FLYCNTTYPE_ALL = 0,
     FLYCNTTYPE_FRM,
     FLYCNTTYPE_GRF,
     FLYCNTTYPE_OLE

};

// Return values for chainable and chain.
enum class SwChainRet
{
    OK             = 0,
    NOT_EMPTY      = 1,       ///< Only empty frames may be connected.
    IS_IN_CHAIN    = 2,       ///< Destination already in chain.
    WRONG_AREA     = 3,       /**< Destination in section where it shouldn't be
                                 (header, footer). */
    NOT_FOUND      = 4,       ///< Destination and/or source not found.
    SOURCE_CHAINED = 5,       ///< Source already has a follow.
    SELF           = 6        ///< Self-chaining is not allowed.
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
