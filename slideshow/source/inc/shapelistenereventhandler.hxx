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



#ifndef INCLUDED_SLIDESHOW_SHAPELISTENEREVENTHANDLER_HXX
#define INCLUDED_SLIDESHOW_SHAPELISTENEREVENTHANDLER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <boost/shared_ptr.hpp>

namespace com { namespace sun { namespace star { namespace drawing
{
    class XShape;
} namespace presentation {
    class XShapeEventListener;
} } } }

/* Definition of ShapeListenerEventHandler interface */

namespace slideshow
{
    namespace internal
    {

        /** Interface for handling view events.

            Classes implementing this interface can be added to an
            EventMultiplexer object, and are called from there to
            handle view events.
         */
        class ShapeListenerEventHandler
        {
        public:
            virtual ~ShapeListenerEventHandler() {}

            virtual bool listenerAdded( const ::com::sun::star::uno::Reference<
                                           ::com::sun::star::presentation::XShapeEventListener>& xListener,
                                        const ::com::sun::star::uno::Reference<
                                           ::com::sun::star::drawing::XShape>&                   xShape ) = 0;

            virtual bool listenerRemoved( const ::com::sun::star::uno::Reference<
                                             ::com::sun::star::presentation::XShapeEventListener>& xListener,
                                          const ::com::sun::star::uno::Reference<
                                             ::com::sun::star::drawing::XShape>&                   xShape ) = 0;
        };

        typedef ::boost::shared_ptr< ShapeListenerEventHandler > ShapeListenerEventHandlerSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_SHAPELISTENEREVENTHANDLER_HXX */
