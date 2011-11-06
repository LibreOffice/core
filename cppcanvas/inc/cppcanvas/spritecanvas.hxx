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



#ifndef _CPPCANVAS_SPRITECANVAS_HXX
#define _CPPCANVAS_SPRITECANVAS_HXX

#include <sal/types.h>
#include <osl/diagnose.h>
#include <basegfx/vector/b2dsize.hxx>

#include <boost/shared_ptr.hpp>


#include <cppcanvas/bitmapcanvas.hxx>
#include <cppcanvas/sprite.hxx>
#include <cppcanvas/customsprite.hxx>

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XSpriteCanvas;
} } } }


/* Definition of SpriteCanvas */

namespace cppcanvas
{
    class SpriteCanvas;

    // forward declaration, since cloneSpriteCanvas() also references SpriteCanvas
    typedef ::boost::shared_ptr< ::cppcanvas::SpriteCanvas > SpriteCanvasSharedPtr;

    /** SpriteCanvas interface
     */
    class SpriteCanvas : public virtual BitmapCanvas
    {
    public:
        virtual bool                    updateScreen( bool bUpdateAll ) const = 0;

        virtual CustomSpriteSharedPtr   createCustomSprite( const ::basegfx::B2DSize& ) const = 0;
        virtual SpriteSharedPtr         createClonedSprite( const SpriteSharedPtr& ) const = 0;

        // shared_ptr does not allow for covariant return types
        SpriteCanvasSharedPtr           cloneSpriteCanvas() const
        {
            SpriteCanvasSharedPtr p( ::boost::dynamic_pointer_cast< SpriteCanvas >(this->clone()) );
            OSL_ENSURE(p.get(), "SpriteCanvas::cloneSpriteCanvas(): dynamic cast failed");
            return p;
        }

        virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::rendering::XSpriteCanvas >    getUNOSpriteCanvas() const = 0;
    };

}

#endif /* _CPPCANVAS_SPRITECANVAS_HXX */
