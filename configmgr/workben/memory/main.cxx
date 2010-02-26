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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"
#include <iostream>
using namespace std;

#define ENABLE_MEMORYMEASURE
#define ENABLE_LOGMECHANISM

// If you wish to enable this memory measure macros ... you need "windows.h"
// But it's not agood idea to include it in your header!!! Because it's not compatible to VCL header .-(
// So you must include it here ... in cxx, where you whish to use it.
#ifdef ENABLE_MEMORYMEASURE
    #define VCL_NEED_BASETSD
    #include <tools/presys.h>
    #include <windows.h>
    #include <tools/postsys.h>
    #undef  VCL_NEED_BASETSD
#endif
#include "memorymeasure.hxx"

#include "logmechanism.hxx"
// -----------------------------------------------------------------------------
// ---------------------------------- M A I N ----------------------------------
// -----------------------------------------------------------------------------

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{

    START_MEMORYMEASURE( aMemoryInfo );


    MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "first start" );

    MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "1" );

    sal_Char* pTest = new sal_Char[1000 * 1000 * 50];
    sal_Char* pTest1 = new sal_Char[1000 * 1000 * 50];
    sal_Char* pTest2 = new sal_Char[1000 * 1000 * 50];

    MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "2" );

    LOG_MEMORYMEASURE( "FirstTest_of_memusage", "Values of memory access for standard filters.", aMemoryInfo );

    return 0;
}

