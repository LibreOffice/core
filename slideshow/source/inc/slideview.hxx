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



#ifndef INCLUDED_SLIDESHOW_SLIDEVIEW_HXX
#define INCLUDED_SLIDESHOW_SLIDEVIEW_HXX

#include "unoview.hxx"

/* Definition of SlideView factory method */
namespace slideshow
{
    namespace internal
    {
        class EventQueue;
        class EventMultiplexer;

        /** Factory for SlideView

            @param xView
            UNO slide view this object should encapsulate

            @param rEventQueue
            Global event queue, to be used for notification
            messages.

            @param rViewChangeFunc
            Functor to call, when the UNO view signals a repaint.
        */
        UnoViewSharedPtr createSlideView(
            ::com::sun::star::uno::Reference<
                ::com::sun::star::presentation::XSlideShowView> const& xView,
            EventQueue&                                                rEventQueue,
            EventMultiplexer&                                          rEventMultiplexer );
    }
}

#endif /* INCLUDED_SLIDESHOW_SLIDEVIEW_HXX */

