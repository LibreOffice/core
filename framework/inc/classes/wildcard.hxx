/*************************************************************************
 *
 *  $RCSfile: wildcard.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:22 $
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

#ifndef __FRAMEWORK_CLASSES_WILDCARD_HXX_
#define __FRAMEWORK_CLASSES_WILDCARD_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

#define OUSTRING            ::rtl::OUString

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          implement wildcard-mechanism for unicode
    @descr          This class can be used to get information about the matching of a pattern to a given text.
                    It's suitable for 8-Bit- AND 16-Bit-strings!

    @implements     -
    @base           -

    @ATTENTION      This class is'nt threadsafe!

    @devstatus      deprecated
*//*-*************************************************************************************************************/

class Wildcard
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      standard ctor
            @descr      We do nothing here.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        Wildcard();

        /*-****************************************************************************************************//**
            @short      standard dtor
            @descr      We do nothing here.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~Wildcard();

        //---------------------------------------------------------------------------------------------------------
        //  interface
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      try to find an agreement between given text and searchpattern
            @descr      You can use wildcards in pattern only!

            @seealso    -

            @param      "sText" is the text, in which we search given pattern.
            @param      "sPattern" is the searched pattern with includes wildcards.
            @return     sal_True , if pattern was found.
            @return     sal_False, if pattern don't match the text.

            @onerror    -
        *//*-*****************************************************************************************************/

        static sal_Bool match(  const   OUSTRING&   sText       ,
                                   const    OUSTRING&   sPattern    );

        //---------------------------------------------------------------------------------------------------------
        //  debug and test methods
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      debug-methods to check incoming parameter of some other mehods of this class
            @descr      The follow methods are used to check parameters for other methods
                        of this class. The return value is used directly for an ASSERT(...).
                        This mechanism is active in debug version only!

            @seealso    FRAMEWORK_ASSERT in implementation!

            @param      references to checking variables
            @return     sal_False on invalid parameter
            @return     sal_True otherwise

            @onerror    -
        *//*-*****************************************************************************************************/

        #ifdef ENABLE_ASSERTIONS

        static sal_Bool impldbg_checkParameter_match(   const   OUSTRING&   sText       ,
                                                           const    OUSTRING&   sPattern    );

        #endif  // #ifdef ENABLE_ASSERTIONS

        /*-****************************************************************************************************//**
            @short      test implementation of match() with different examples
            @descr      If TESTMODE activated, you cann call these method to start and log some special examples.
                        Do this if you have changed the implementation of method match() to test it.

            @seealso    -

            @param      -
            @return     -

            @onerror    Error-conditions are written to file or show in a messagebox.
                        Thhat depends from current setting of ASSERT_OUTPUTTYPE. (see debug.hxx for further informations.)
        *//*-*****************************************************************************************************/

        #ifdef ENABLE_CLASSDEBUG

        void impldbg_testWildcard();

        #endif // #ifdef ENABLE_CLASSDEBUG

};      //  class Wildcard

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_CLASSES_WILDCARD_HXX_
