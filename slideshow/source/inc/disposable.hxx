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



#ifndef INCLUDED_SLIDESHOW_DISPOSABLE_HXX
#define INCLUDED_SLIDESHOW_DISPOSABLE_HXX

#include <sal/types.h>
#include <boost/shared_ptr.hpp>


/* Definition of Disposable interface */

namespace slideshow
{
    namespace internal
    {
        /** Disposable interface

            With ref-counted objects, deleting object networks
            containing cycles requires a dispose() call, to enforce
            every object to call dispose on and release local
            references.
         */
        class Disposable
        {
        public:
            virtual ~Disposable() {}

            /** Dispose all object references.

                An implementor of this method must first call
                dispose() on any of its external references, and
                release them after that.
             */
            virtual void dispose() = 0;
        };

    }
}

#endif /* INCLUDED_SLIDESHOW_DISPOSABLE_HXX */
