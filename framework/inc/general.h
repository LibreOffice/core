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



#ifndef __FRAMEWORK_GENERAL_H_
#define __FRAMEWORK_GENERAL_H_

//=============================================================================
// includes

#include <macros/generic.hxx>

/* Normaly XInterface isn't used here ...
   But we need it to be able to define namespace alias css to anything related to ::com::sun::star
   :-) */
#include <com/sun/star/uno/XInterface.hpp>

#include <rtl/ustring.hxx>

//=============================================================================
// namespace

//-----------------------------------------------------------------------------
/** will make our code more readable if we can use such short name css instead
    of typing ::com::sun::star everytimes.

    On the other side we had so many problems with "using namespace" so we dont use
    it here any longer.
 */
namespace css = ::com::sun::star;

namespace framework {

//-----------------------------------------------------------------------------
/** status event mapped from load event of frame loader

    @todo think about me
          should be moved to another more specific place.
 */
static const ::rtl::OUString FEATUREDESCRIPTOR_LOADSTATE = DECLARE_ASCII("loadFinishedOrCancelled");

//-----------------------------------------------------------------------------
/** Those macro is used to make it more clear where a synchronized block will start.
    Because normal documentation code wont be recognized by some developers to be real
    I need something where they are thinking about.

    At least this macro will do nothing ... it's empty.
    But it should make the code more clear .-))
*/
#define SYNCHRONIZED_START

//-----------------------------------------------------------------------------
/** Same then SYNCHRONIZED_START ... but instead it mark the end of such code block.
*/
#define SYNCHRONIZED_END

} // namespace framework

#endif // #ifndef __FRAMEWORK_GENERAL_H_
