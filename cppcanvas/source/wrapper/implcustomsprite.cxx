/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implcustomsprite.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:43:38 $
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

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#include <implcustomsprite.hxx>
#include <implcanvas.hxx>

using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        ImplCustomSprite::ImplCustomSprite( const uno::Reference< rendering::XSpriteCanvas >&       rParentCanvas,
                                            const uno::Reference< rendering::XCustomSprite >&       rSprite,
                                            const ImplSpriteCanvas::TransformationArbiterSharedPtr& rTransformArbiter ) :
            ImplSprite( rParentCanvas,
                        uno::Reference< rendering::XSprite >(rSprite,
                                                             uno::UNO_QUERY),
                        rTransformArbiter ),
            mpLastCanvas(),
            mxCustomSprite( rSprite )
        {
            OSL_ENSURE( rParentCanvas.is(), "ImplCustomSprite::ImplCustomSprite(): Invalid canvas" );
            OSL_ENSURE( mxCustomSprite.is(), "ImplCustomSprite::ImplCustomSprite(): Invalid sprite" );
        }

        ImplCustomSprite::~ImplCustomSprite()
        {
        }

        CanvasSharedPtr ImplCustomSprite::getContentCanvas() const
        {
            OSL_ENSURE( mxCustomSprite.is(), "ImplCustomSprite::getContentCanvas(): Invalid sprite" );

            if( !mxCustomSprite.is() )
                return CanvasSharedPtr();

            uno::Reference< rendering::XCanvas > xCanvas( mxCustomSprite->getContentCanvas() );

            if( !xCanvas.is() )
                return CanvasSharedPtr();

            // cache content canvas C++ wrapper
            if( mpLastCanvas.get() == NULL ||
                mpLastCanvas->getUNOCanvas() != xCanvas )
            {
                mpLastCanvas = CanvasSharedPtr( new ImplCanvas( xCanvas ) );
            }

            return mpLastCanvas;
        }
    }
}
