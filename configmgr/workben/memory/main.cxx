/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: main.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:51:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
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

#ifndef __FRAMEWORK_MACROS_DEBUG_MEMORYMEASURE_HXX_
#include "memorymeasure.hxx"
#endif

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

