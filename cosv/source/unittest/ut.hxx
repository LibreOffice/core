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



