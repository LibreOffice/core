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



#ifndef INCLUDED_SLIDESHOW_GENERATEEVENT_HXX
#define INCLUDED_SLIDESHOW_GENERATEEVENT_HXX

#include "slideshowcontext.hxx"
#include "delayevent.hxx"
#include "com/sun/star/uno/Any.hxx"

namespace slideshow {
namespace internal {

/** Create an event for the given description, calling the given functor.

    @param rEventDescription
    Directly from API

    @param rFunctor
    Functor to call when event fires.

    @param rContext
    Context struct, to provide event queue

    @param nAdditionalDelay
    Additional delay, gets added on top of timeout.
*/
EventSharedPtr generateEvent(
    ::com::sun::star::uno::Any const& rEventDescription,
    Delay::FunctorT const& rFunctor,
    SlideShowContext const& rContext,
    double nAdditionalDelay );

} // namespace internal
} // namespace slideshow

#endif /* INCLUDED_SLIDESHOW_GENERATEEVENT_HXX */

