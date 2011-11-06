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



#ifndef INCLUDED_SLIDESHOW_STRINGANIMATION_HXX
#define INCLUDED_SLIDESHOW_STRINGANIMATION_HXX

#include "animation.hxx"
#include <rtl/ustring.hxx>


/* Definition of StringAnimation interface */

namespace slideshow
{
    namespace internal
    {
        /** Interface defining a string animation.

            This interface is a specialization of the Animation
            interface, and is used to animate attributes representable
            by a discrete character string (e.g. font names)
         */
        class StringAnimation : public Animation
        {
        public:
            typedef ::rtl::OUString ValueType;

            /** Set the animation to value rStr

                @param rStr
                Current animation value.
             */
            virtual bool operator()( const ValueType& rStr ) = 0;

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

        typedef ::boost::shared_ptr< StringAnimation > StringAnimationSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_STRINGANIMATION_HXX */
