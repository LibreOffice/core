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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"


#ifdef WNT
# if defined _MSC_VER
# pragma warning(push,1)
# endif

# include <windows.h>

# if defined _MSC_VER
# pragma warning(pop)
# endif
#endif

#include "osl/diagnose.h"
#include "canvas/prioritybooster.hxx"


namespace canvas
{
    namespace tools
    {
        struct PriorityBooster_Impl
        {
            int mnOldPriority;
        };

        PriorityBooster::PriorityBooster( sal_Int32 nDelta ) :
            mpImpl( new PriorityBooster_Impl )
        {
#ifdef WNT
            HANDLE aCurrThread = GetCurrentThread();
            mpImpl->mnOldPriority = GetThreadPriority( aCurrThread );

            if ( 0 == SetThreadPriority( aCurrThread, mpImpl->mnOldPriority + nDelta ) )
            {
                OSL_ENSURE( false,
                            "PriorityBooster::PriorityBooster(): Was not able to modify thread priority" );
            }
#else
            nDelta = 0; // #i55991# placate gcc warning
#endif
        }

        PriorityBooster::~PriorityBooster()
        {
#ifdef WNT
            SetThreadPriority( GetCurrentThread(),
                               mpImpl->mnOldPriority );
#endif
        }
    } // namespace tools

} // namespace canvas
