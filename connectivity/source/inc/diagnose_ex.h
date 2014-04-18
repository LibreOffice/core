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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_DIAGNOSE_EX_H
#define INCLUDED_CONNECTIVITY_SOURCE_INC_DIAGNOSE_EX_H

#if OSL_DEBUG_LEVEL > 0
    #define OSL_VERIFY_RES( expression, fail_message ) \
        OSL_ENSURE( expression, fail_message )
    #define OSL_VERIFY_EQUALS( expression, compare, fail_message ) \
        OSL_ENSURE( expression == compare, fail_message )
#else
    #define OSL_VERIFY_RES( expression, fail_message ) \
        (void)(expression)
    #define OSL_VERIFY_EQUALS( expression, compare, fail_message ) \
        (void)(expression)
#endif

#define OSL_UNUSED( expression ) \
    (void)(expression)

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_DIAGNOSE_EX_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
