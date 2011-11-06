/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef __FRAMEWORK_CLASSES_WILDCARD_HXX_
#define __FRAMEWORK_CLASSES_WILDCARD_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <macros/debug.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <rtl/ustring.hxx>

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

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

        static sal_Bool match(  const   ::rtl::OUString&    sText       ,
                                   const    ::rtl::OUString&    sPattern    );

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

        static sal_Bool impldbg_checkParameter_match(   const   ::rtl::OUString&    sText       ,
                                                           const    ::rtl::OUString&    sPattern    );

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
