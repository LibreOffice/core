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



#ifndef _CPPCANVAS_BITMAPCANVAS_HXX
#define _CPPCANVAS_BITMAPCANVAS_HXX

#include <sal/types.h>
#include <osl/diagnose.h>

#include <boost/shared_ptr.hpp>
#include <basegfx/vector/b2isize.hxx>
#include <cppcanvas/canvas.hxx>


/* Definition of BitmapCanvas */

namespace cppcanvas
{
    class BitmapCanvas;

    // forward declaration, since cloneBitmapCanvas() also references BitmapCanvas
    typedef ::boost::shared_ptr< BitmapCanvas > BitmapCanvasSharedPtr;

    /** BitmapCanvas interface
     */
    class BitmapCanvas : public virtual Canvas
    {
    public:
        virtual ::basegfx::B2ISize      getSize() const = 0;

        // shared_ptr does not allow for covariant return types
        BitmapCanvasSharedPtr           cloneBitmapCanvas() const
        {
            BitmapCanvasSharedPtr p( ::boost::dynamic_pointer_cast< BitmapCanvas >(this->clone()) );
            OSL_ENSURE(p.get(), "BitmapCanvas::cloneBitmapCanvas(): dynamic cast failed");
            return p;
        }
    };

}

#endif /* _CPPCANVAS_BITMAPCANVAS_HXX */
