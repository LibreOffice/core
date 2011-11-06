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



#ifndef _NULLCANVAS_USAGECOUNTER_HXX
#define _NULLCANVAS_USAGECOUNTER_HXX

#include <osl/interlck.h>
#include <boost/current_function.hpp>

namespace nullcanvas
{
    /** Little resource tracking counter.

        When using this object, a global use counter, specific to the
        given type is incremented on object construction, and
        decremented on object destruction.
     */
    template< class Type > class UsageCounter
    {
    public:
        UsageCounter()
        {
            OSL_TRACE( "%s, %d objects currently in use.\n",
                       BOOST_CURRENT_FUNCTION,
                       osl_incrementInterlockedCount( &s_nCount ) );
        }

        ~UsageCounter()
        {
            const sal_Int32 nCount( osl_decrementInterlockedCount( &s_nCount ) );

            if( !nCount )
            {
                OSL_TRACE( "%s, last instance deleted.\n",
                           BOOST_CURRENT_FUNCTION );
            }
            else
            {
                OSL_TRACE( "%s, %d instances left.\n",
                           BOOST_CURRENT_FUNCTION,
                           nCount );
            }
        }

    private:
        static oslInterlockedCount s_nCount;
    };

    template< class Type > oslInterlockedCount UsageCounter<Type>::s_nCount = 0;
}

#endif /* _NULLCANVAS_USAGECOUNTER_HXX */
