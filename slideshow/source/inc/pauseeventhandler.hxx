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



#ifndef INCLUDED_SLIDESHOW_PAUSEEVENTHANDLER_HXX
#define INCLUDED_SLIDESHOW_PAUSEEVENTHANDLER_HXX

#include <boost/shared_ptr.hpp>


/* Definition of PauseHandler interface */

namespace slideshow
{
    namespace internal
    {

        /** Interface for handling pause events.

            Classes implementing this interface can be added to an
            EventMultiplexer object, and are called from there to
            handle pause events.
         */
        class PauseEventHandler
        {
        public:
            /** Handle the event.

                @param bPauseShow
                When true, the show is paused. When false, the show is
                started again

                @return true, if this handler has successfully
                processed the pause event. When this method returns
                false, possibly other, less prioritized handlers are
                called, too.
             */
            virtual bool handlePause( bool bPauseShow ) = 0;
        };

        typedef ::boost::shared_ptr< PauseEventHandler > PauseEventHandlerSharedPtr;
    }
}

#endif /* INCLUDED_SLIDESHOW_PAUSEEVENTHANDLER_HXX */
