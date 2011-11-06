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



#ifndef _SDR_ANIMATION_OBJECTANIMATOR_HXX
#define _SDR_ANIMATION_OBJECTANIMATOR_HXX

#include <sal/types.h>
#include <vector>
#include <vcl/timer.hxx>
#include <svx/sdr/animation/scheduler.hxx>
#include <svx/sdr/animation/animationstate.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace animation
    {
        class primitiveAnimator : public Scheduler
        {
        public:
            // basic constructor and destructor
            primitiveAnimator();
            SVX_DLLPUBLIC virtual ~primitiveAnimator();
        };
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_ANIMATION_OBJECTANIMATOR_HXX

// eof
