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

/*
 *  setup.h:
 *      sample master include file which gets included
 *      from all helpers *.c sources.
 */

#ifndef SETUP_HEADER_INCLUDED
    #define SETUP_HEADER_INCLUDED

    // XWPEXPORT defines the standard linkage for the
    // XWorkplace helpers.
    #ifdef __EMX__
        #define XWPENTRY
    #elif defined (__IBMCPP__) || defined (__IBMC__)
        #define XWPENTRY _Optlink
    #endif

    /*************************************************************
     *                                                           *
     *   Additional stuff for EMX                                *
     *                                                           *
     *************************************************************/

    #ifdef __EMX__
        // EMX doesn't have all these 16-bit typedefs;
        // added (99-10-22) [umoeller]
        #define APIENTRY16  _Far16 _Pascal
        #define PASCAL16    _Far16 _Pascal
        #define CDECL16     _Far16 _Cdecl

        typedef unsigned short APIRET16;
        typedef unsigned long  APIRET32;

    #endif

    // the following is a VAC++-specific macro, which doesn't exist
    // with EMX, so we need to implement this... this was one of
    // the "undefined symbols" we got (99-10-23) [umoeller]
    // changed this to prefix underscore, because the STL apparently
    // redefines this V0.9.3 (2000-05-15) [umoeller]
    #define _min(a,b) ( ((a) > (b)) ? b : a )
    #define _max(a,b) ( ((a) > (b)) ? a : b )

    // Uncomment the following if you have trouble with the
    // exception handlers in helpers\except.c; WarpIN will
    // then install _no_ additional exception handlers at all
    // (include\helpers\except.h reacts to these defines).
    // I'm not sure if the handlers work well with EMX.

    #ifdef __EMX__00
        #define __NO_EXCEPTION_HANDLERS__
    #endif

    /*************************************************************
     *                                                           *
     *   Additional stuff for VAC++ 3.0                          *
     *                                                           *
     *************************************************************/

    // all this added V0.9.2 (2000-03-10) [umoeller]
    #if ( defined (  __IBMCPP__ ) && (  __IBMCPP__ < 400 ) )
        typedef int bool;
        #define true 1
        #define false 0
        #define _BooleanConst    // needed for some VAC headers, which define bool also
    #endif

    #ifndef __stdlib_h          // <stdlib.h>
        #include <stdlib.h>
    #endif
    #ifndef __string_h          // <string.h>
        #include <string.h>
    #endif

    /*************************************************************
     *                                                           *
     *   Debugging                                               *
     *                                                           *
     *************************************************************/

    // All the following redone (99-10-23) [umoeller]:
    // __DEBUG__ is defined as a macro on the compiler
    // command line by the makefiles if DEBUG was enabled
    // in \setup.in
    #ifdef __DEBUG__

        // with debug code, disable the exception handlers
        #define __NO_EXCEPTION_HANDLERS__

        // If the following is commented out, no PMPRINTF will be
        // used at all. WarpIN uses Dennis Bareis' PMPRINTF
        // package to do this.

        // NOTE: We cannot use PmPrintf with EMX,
        // because pmprintf.lib imports the VAC++ runtimes.
        // That's the strange errors I was reporting yesterday.
        #ifndef __EMX__
            #ifdef OS2_INCLUDED
                #define _PMPRINTF_
                #include "helpers/pmprintf.h"
            #endif
        #endif
    #endif

    #ifndef _PMPRINTF_
        // not defined: define empty macro so we don't
        // get compiler errors
        #define _Pmpf(x)
    #endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
