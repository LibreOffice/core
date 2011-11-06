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



#ifndef INCLUDED_SLIDESHOW_EVENTHANDLER_HXX
#define INCLUDED_SLIDESHOW_EVENTHANDLER_HXX

#include <boost/shared_ptr.hpp>


/* Definition of EventHandler interface */

namespace slideshow
{
    namespace internal
    {

        /** Interface for event handling objects.

            Classes implementing this interface can be added to an
            EventMultiplexer object, and are called from there to
            handle events.
         */
        class EventHandler
        {
        public:
            virtual ~EventHandler() {}

            /** Handle the event.

                @return true, if this handler has successfully
                processed the event. When this method returns false,
                possibly other, less prioritized handlers are called,
                too.
             */
            virtual bool handleEvent() = 0;
        };

        typedef ::boost::shared_ptr< EventHandler > EventHandlerSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_EVENTHANDLER_HXX */
