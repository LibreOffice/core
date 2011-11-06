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



#ifndef INCLUDED_SLIDESHOW_USERPAINTEVENTHANDLER_HXX
#define INCLUDED_SLIDESHOW_USERPAINTEVENTHANDLER_HXX

#include <boost/shared_ptr.hpp>

/* Definition of UserPaintEventHandler interface */

namespace slideshow
{
    namespace internal
    {
        class RGBColor;

        /** Interface for handling user paint state changes.

            Classes implementing this interface can be added to an
            EventMultiplexer object, and are called from there to
            handle user paint events.
         */
        class UserPaintEventHandler
        {
        public:
            virtual ~UserPaintEventHandler() {}
            virtual bool colorChanged( RGBColor const& rUserColor ) = 0;
        virtual bool widthChanged( double nUserStrokeWidth ) = 0;
        virtual bool eraseAllInkChanged(bool const& rEraseAllInk) =0;
            virtual bool eraseInkWidthChanged(sal_Int32 rEraseInkSize) =0;
            virtual bool switchEraserMode() = 0;
            virtual bool switchPenMode() = 0;
            virtual bool disable() = 0;
        };

        typedef ::boost::shared_ptr< UserPaintEventHandler > UserPaintEventHandlerSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_USERPAINTEVENTHANDLER_HXX */
