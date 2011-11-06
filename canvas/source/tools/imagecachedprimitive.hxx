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



#ifndef INCLUDED_CANVAS_IMAGECACHEDPRIMITIVE_HXX
#define INCLUDED_CANVAS_IMAGECACHEDPRIMITIVE_HXX

#include <canvas/rendering/icachedprimitive.hxx>

#include <boost/shared_ptr.hpp>


namespace canvas
{
    /** Objects with this interface are returned from every Image
        render operation.

        These objects can be used to implement the
        rendering::XCachedPrimitive interface, which in turn caches
        render state and objects to facilitate quick redraws.

        Derived from ICachedPrimitive, to add the setImage() method
        (which, strictly speaking, is a technicality, because Image
        cannot create objects with a shared_ptr to itself).
     */
    struct ImageCachedPrimitive : public ICachedPrimitive
    {
        virtual void setImage( const ::boost::shared_ptr< class Image >& rTargetImage ) = 0;
    };

    typedef ::boost::shared_ptr< ImageCachedPrimitive > ImageCachedPrimitiveSharedPtr;
}

#endif /* INCLUDED_CANVAS_IMAGECACHEDPRIMITIVE_HXX */
