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

#ifndef COSV_UNITTEST_UT_HXX
#define COSV_UNITTEST_UT_HXX


#define UT_CHECK( fname, cond ) \
    if ( NOT (cond) ) { std::cerr << "ftest_" << #fname << " " << #cond << endl; \
        ret = false; }

#define CUT_DECL( nsp, cname ) \
    bool classtest_##cname()
#define FUT_DECL( cname, fname ) \
    bool ftest_##fname( cname & r##cname )

#define UT_RESULT( result ) \
    if (result ) std::cout << "All unit tests passed successfully." << std::endl; \
    else std::cout << "Errors in unit tests.\n" << std::endl



CUT_DECL( csv, File );
CUT_DECL( csv, String );

#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
