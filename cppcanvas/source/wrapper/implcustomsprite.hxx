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



#ifndef _CPPCANVAS_IMPLCUSTOMSPRITE_HXX
#define _CPPCANVAS_IMPLCUSTOMSPRITE_HXX

#include <sal/types.h>

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif
#include <cppcanvas/canvas.hxx>
#include <cppcanvas/customsprite.hxx>

#include <implsprite.hxx>
#include <implspritecanvas.hxx>


namespace cppcanvas
{
    namespace internal
    {
        // share Sprite implementation of ImplSprite
        class ImplCustomSprite : public virtual CustomSprite, protected virtual ImplSprite
        {
        public:
            ImplCustomSprite( const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::rendering::XSpriteCanvas >&     rParentCanvas,
                              const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::rendering::XCustomSprite >&     rSprite,
                              const ImplSpriteCanvas::TransformationArbiterSharedPtr&       rTransformArbiter );
            virtual ~ImplCustomSprite();

            virtual CanvasSharedPtr getContentCanvas() const;

        private:
            // default: disabled copy/assignment
            ImplCustomSprite(const ImplCustomSprite&);
            ImplCustomSprite& operator=( const ImplCustomSprite& );

            mutable CanvasSharedPtr                                                                         mpLastCanvas;
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCustomSprite >    mxCustomSprite;
        };
    }
}

#endif /* _CPPCANVAS_IMPLCUSTOMSPRITE_HXX */
