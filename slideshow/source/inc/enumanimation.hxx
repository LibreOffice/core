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



#ifndef INCLUDED_SLIDESHOW_ENUMANIMATION_HXX
#define INCLUDED_SLIDESHOW_ENUMANIMATION_HXX

#include "animation.hxx"


/* Definition of EnumAnimation interface */

namespace slideshow
{
    namespace internal
    {
        /** Interface defining an enum animation.

            This interface is a specialization of the Animation
            interface, and is used to animate attributes representable
            by a set of constant values, such as UNO constantsm, or enums.
         */
        class EnumAnimation : public Animation
        {
        public:
            typedef sal_Int16 ValueType;

            /** Set the animation to value k

                @param k
                Current animation value (must be in an
                attribute-specific permissible range). Overflowing
                values will be clipped to the permissible range
                internally.
             */
            virtual bool operator()( ValueType k ) = 0;

            /** Request the underlying value for this animation.

                This is necessary for pure To or By animations, as the
                Activity cannot determine a sensible start value
                otherwise.

                @attention Note that you are only permitted to query
                for the underlying value, if the animation has actually
                been started (via start() call).
             */
            virtual ValueType getUnderlyingValue() const = 0;
        };

        typedef ::boost::shared_ptr< EnumAnimation > EnumAnimationSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_ENUMANIMATION_HXX */
