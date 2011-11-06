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



#ifndef INCLUDED_SLIDESHOW_SHAPEMAPS_HXX
#define INCLUDED_SLIDESHOW_SHAPEMAPS_HXX

#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/uno/Reference.hxx>

#include <boost/shared_ptr.hpp>
#include <map>

namespace com { namespace sun { namespace star { namespace drawing
{
    class XShape;
} } } }

/* Definition of two shape maps */

namespace slideshow
{
    namespace internal
    {
        /// Maps XShape to shape listener
        typedef ::std::map< ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShape>,
                            boost::shared_ptr< ::cppu::OInterfaceContainerHelper >
                            >                  ShapeEventListenerMap;

        /// Maps XShape to mouse cursor
        typedef ::std::map< ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShape>,
                            sal_Int16>         ShapeCursorMap;
    }
}

#endif /* INCLUDED_SLIDESHOW_SHAPEMAPS_HXX */
