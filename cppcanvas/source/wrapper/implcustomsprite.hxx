/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implcustomsprite.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:27:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CPPCANVAS_IMPLCUSTOMSPRITE_HXX
#define _CPPCANVAS_IMPLCUSTOMSPRITE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _CPPCANVAS_CANVAS_HXX
#include <cppcanvas/canvas.hxx>
#endif
#ifndef _CPPCANVAS_CUSTOMSPRITE_HXX
#include <cppcanvas/customsprite.hxx>
#endif

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
