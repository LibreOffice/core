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



#ifndef INCLUDED_SLIDESHOW_ANIMATIONFUNCTION_HXX
#define INCLUDED_SLIDESHOW_ANIMATIONFUNCTION_HXX

#include <sal/config.h>
#include <boost/shared_ptr.hpp>


/* Definition of AnimationFunction interface */

namespace slideshow
{
    namespace internal
    {
        /** Interface describing an abstract animation function.

            Use this interface to model time-dependent animation
            functions of one variable.
         */
        class AnimationFunction
        {
        public:
            virtual ~AnimationFunction() {}

            /** Operator to calculate function value.

                This method calculates the function value for the
                given time instant t.

                @param t
                Current time instant, must be in the range [0,1]

                @return the function value, typically in relative
                user coordinate space ([0,1] range).
             */
            virtual double operator()( double t ) const = 0;

        };

        typedef ::boost::shared_ptr< AnimationFunction > AnimationFunctionSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_ANIMATIONFUNCTION_HXX */
