/*************************************************************************
 *
 *  $RCSfile: csv_env.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 16:00:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CSV_CSV_ENV_HXX
#define CSV_CSV_ENV_HXX



// BEGIN Compiler dependent defines regarding standard compliance,
//   subject to changes:
// #define CSV_NO_BOOL_TYPE       // States that no system type 'bool' exists
// #define CSV_NO_MUTABLE         // No keyword mutable
// #define CSV_NO_EXPLICIT        // No keyword explicit
// #define CSV_NO_IOSTREAMS       // No iostreams
// END Compiler dependent defines, subject to changes


// BEGIN Compiler dependant defines, controlled by above defines
#ifdef CSV_NO_BOOL_TYPE
typedef int     bool_int;
#define bool    bool_int
#define false   0
#define true    1
#endif // CSV_NO_BOOL_TYPE

#ifdef CSV_NO_MUTABLE
#define mutable
#endif // CSV_NO_MUTABLE

#ifdef CSV_NO_EXPLICIT
#define explicit
#endif  // CSV_NO_EXPLICIT
// END Compiler dependent defines, controlled by above defines


//*******       Include c-language-types        ************//
// size_t, wchar_t
#include <stdlib.h>



//*******       Builtin types of exact length        ************//

// Exact length builtin types
typedef signed char     INT8;
typedef unsigned char   UINT8;
typedef short           INT16;
typedef unsigned short  UINT16;
typedef long            INT32;
typedef unsigned long   UINT32;
typedef float           REAL32;
typedef double          REAL64;


// Additional builtin types
typedef INT32        intt;      // Um ein exakt definiertes Standard-int zu haben.
typedef UINT32       uintt;     // Das dazu passende Standard-unsigned-int.
typedef REAL64       real;

//  Constants
//  ---------
// Zero-pointer for use in ellipsed (...) parameter lists which expect a
//   pointer which may have another size than an int.
//   Must be a define to be used in precompiled headers:
#define NIL   ((void*)0)
// char '\0'
#define NULCH '\0'



// Boolesche Operatoren
#define AND &&
#define OR  ||
#define NOT !

// Macro for distinguishing dynamic allocated pointers from
//   referencing pointers
#define DYN     // Exact specification: DYN has to be used if and only if:
                //  1. DYN specifies a class member pointer or reference variable and
                //     the class must free the referenced memory.
                //  2. DYN specifies a pointer or reference (return-) parameter of a function
                //     and for in-parameters the function or its class
                //     must free the referenced memory, the parameter is then called
                //     a let-parameter.
                //     For out- and inout-parameters
                //     or return values the caller of the function hast to
                //     free the referenced memory.
                //
                //     It is irrelevant who allocated the memory!
                //
                //     DYN - variables use the prefixes "dp" or "dr" instead of "p" or "r".


//******        Assertions          ******//

namespace csv
{
void                PerformAssertion(
                        const char *        condition,
                        const char *        file,
                        unsigned            line );
}

// Programming by contract
#ifndef CSV_NO_ASSERTIONS
//   Subject to change to more sophisticated handling
#define precond(x)      csv_assert(x)
#define postcond(x)     csv_assert(x)

#ifdef CSV_USE_CSV_ASSERTIONS
#define csv_assert(x)       ( (x) ? (void)(0) : ::csv::PerformAssertion( #x, __FILE__, __LINE__) )
#define csv_noimpl(x)       ::csv::PerformAssertion( "Functon " #x " is not yet implemented.", __FILE__, __LINE__)
#define csv_exception       ::csv::PerformAssertion( "Exception to be raised.", __FILE__, __LINE__)
#else

// Save NDEBUG state
#ifdef NDEBUG
#define CSV_CSV_ENV_HXX_HAD_NDEBUG
#undef NDEBUG
#endif

#if OSL_DEBUG_LEVEL == 0
#define NDEBUG
#endif
#include <assert.h>

#define csv_assert(x)       assert(x);
#define csv_noimpl(x)       assert(x);
#define csv_exception       assert(x);

// Restore NDEBUG state
#ifdef CSV_CSV_ENV_HXX_HAD_NDEBUG
#define NDEBUG
#else
#undef NDEBUG
#endif

#endif

#else // #ifndef CSV_NO_ASSERTIONS else

#define precond(x)
#define postcond(x)
#define csv_assert(x)
#define csv_noimpl(x)
#define csv_exception

#endif  // end ifndef CSV_NO_ASSERTIONS else



/* Additional Programming Conventions
1. see above at "#define DYN"
2. function parameters get one of these prefixes:
    - i_     := Function uses only the value, but must not change a referenced variable.
    - o_     := Parameter is undefined until function has set it.
                Parametere must be set by the function.
    - io_    := Function may use and change the referenced variable.
    - let_   := Funktion may use and change the referenced variable and HAS TO free the
                associated memory.
3. Global constants get the prefix 'C_', global variables the prefix
   'G_', local constants the prefix 'c_' .
4. Static members end with an underscore '_'.

*/


#endif

