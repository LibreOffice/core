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



#ifndef SDROBJECTFILTER_HXX
#define SDROBJECTFILTER_HXX

#include "svx/svxdllapi.h"

/** === begin UNO includes === **/
/** === end UNO includes === **/

class SdrObject;

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= SdrObjectFilter
    //====================================================================
    /** specifies a boolean predicate on the set of all SdrObjects - vulgo a filter.
    */
    class SVX_DLLPUBLIC SAL_NO_VTABLE ISdrObjectFilter
    {
    public:
        virtual bool    includeObject( const SdrObject& i_rObject ) const = 0;

        virtual         ~ISdrObjectFilter() = 0;
    };

//........................................................................
} // namespace svx
//........................................................................

#endif // SDROBJECTFILTER_HXX
