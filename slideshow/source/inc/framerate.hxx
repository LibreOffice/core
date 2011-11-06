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



#ifndef INCLUDED_SLIDESHOW_FRAMERATE_HXX
#define INCLUDED_SLIDESHOW_FRAMERATE_HXX

#include <sal/types.h>

namespace slideshow { namespace internal {

/** Some frame rate related data.
*/
class FrameRate
{
public:
    /** The minimum number of frames per second is used to calculate the
        minimum number of frames that is to be shown for active activities.
    */
    static const sal_Int32 MinimumFramesPerSecond = 10;

    /** Aim high with the number of preferred number of frames per second.
        This number is the maximum as well and the true number will be lower.
    */
    static const sal_Int32 PreferredFramesPerSecond = 50;
};

} } // end of namespace slideshow::internal

#endif
