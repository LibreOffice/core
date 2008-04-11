/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: implcustomsprite.hxx,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
