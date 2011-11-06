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


#ifndef _SWTHREADJOINER_HXX
#define _SWTHREADJOINER_HXX

#include "com/sun/star/util/XJobManager.hpp"
#include <osl/mutex.hxx>

/** Testing

    @author OD
*/
class SwThreadJoiner
{
    public:

        static ::com::sun::star::uno::Reference< ::com::sun::star::util::XJobManager >& GetThreadJoiner();

        static void ReleaseThreadJoiner();

    private:
        static ::com::sun::star::uno::Reference< ::com::sun::star::util::XJobManager > mpThreadJoiner;
        static osl::Mutex* mpGetJoinerMutex;

        SwThreadJoiner();
};
#endif
