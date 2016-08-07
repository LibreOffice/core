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



#ifndef INCLUDED_CANVAS_PRIORITYBOOSTER_HXX
#define INCLUDED_CANVAS_PRIORITYBOOSTER_HXX

#include <sal/types.h>

#include <memory>
#include <canvas/canvastoolsdllapi.h>


namespace canvas
{
    namespace tools
    {
        struct PriorityBooster_Impl;

        /** Simplistic RAII object, to temporarily boost execution
            priority for the current scope.
         */
        class PriorityBooster
        {
        public:
            /** Create booster, with given priority delta

                @param nDelta
                Difference in execution priority. Positive values
                increase prio, negative values decrease prio.
             */
             CANVASTOOLS_DLLPUBLIC explicit PriorityBooster( sal_Int32 nDelta );
             CANVASTOOLS_DLLPUBLIC ~PriorityBooster();

        private:
            // also disables copy constructor and assignment operator
            const ::std::auto_ptr< PriorityBooster_Impl > mpImpl;
        };
    }
}

#endif /* INCLUDED_CANVAS_PRIORITYBOOSTER_HXX */
// eof
