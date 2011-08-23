/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SOLTOOLS_TESTSHL_TSTMGR_HXX__
#define _SOLTOOLS_TESTSHL_TSTMGR_HXX__

#include    <sal/types.h>

#include <vector>

using namespace std;

// <namespace_tstutl>
namespace tstutl {

// <class_tstMgr>
class tstMgr {

    // <private_members>
    struct tstMgr_Impl;
    tstMgr_Impl* pImpl;
    // </private_members>

    // <private_methods>
    void cleanup();
    // </private_methods>

public:

    // <dtor>
    ~tstMgr(){
        cleanup();
    } // </dtor>


    // <public_methods>
    sal_Bool initialize( sal_Char* moduleName, sal_Bool boom = sal_False );
    sal_Bool test_Entry( sal_Char* entry, sal_Char* logName = 0 );
    sal_Bool test_Entries( vector< sal_Char* > entries, sal_Char* logName = 0 );
    sal_Bool test_EntriesFromFile( sal_Char* fName, sal_Char* logName = 0 );
    // </public_methods>

}; // </class_tstMgr>

} // </namespace_tstutl>

#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
