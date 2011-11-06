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



#ifndef TOOLKIT_INC_TOOLKIT_HELPER_SOLARRELEASE_HXX
#define TOOLKIT_INC_TOOLKIT_HELPER_SOLARRELEASE_HXX

#include <vcl/svapp.hxx>

//........................................................................
namespace toolkit
{
//........................................................................

    //====================================================================
    //= ReleaseSolarMutex
    //====================================================================
    class ReleaseSolarMutex
    {
        sal_uInt32  mnLockCount;
        const bool  mbRescheduleDuringAcquire;

    public:
        enum
        {
            RescheduleDuringAcquire = true
        };

    public:
        ReleaseSolarMutex( const bool i_rescheduleDuringAcquire = false )
            :mnLockCount( Application::ReleaseSolarMutex() )
            ,mbRescheduleDuringAcquire( i_rescheduleDuringAcquire )
        {

        }

        ~ReleaseSolarMutex()
        {
            if ( mnLockCount > 0 )
            {
                if ( mbRescheduleDuringAcquire )
                {
                    while ( !Application::GetSolarMutex().tryToAcquire() )
                    {
                        Application::Reschedule();
                    }
                    --mnLockCount;
                }
                Application::AcquireSolarMutex( mnLockCount );
            }
        }
    };

//........................................................................
} // namespace toolkit
//........................................................................

#endif // TOOLKIT_INC_TOOLKIT_HELPER_SOLARRELEASE_HXX

