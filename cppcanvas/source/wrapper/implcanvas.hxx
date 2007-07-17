/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implcanvas.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:26:57 $
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

#ifndef _CPPCANVAS_IMPLCANVAS_HXX
#define _CPPCANVAS_IMPLCANVAS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/ViewState.hpp>
#include <cppcanvas/canvas.hxx>

#include <boost/optional.hpp>

namespace rtl
{
    class OUString;
}

namespace basegfx
{
    class B2DHomMatrix;
    class B2DPolyPolygon;
}

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XCanvas;
} } } }


/* Definition of ImplCanvas */

namespace cppcanvas
{

    namespace internal
    {

        class ImplCanvas : public virtual Canvas
        {
        public:
            ImplCanvas( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::rendering::XCanvas >& rCanvas );
            virtual ~ImplCanvas();

            virtual void                             setTransformation( const ::basegfx::B2DHomMatrix& rMatrix );
            virtual ::basegfx::B2DHomMatrix          getTransformation() const;

            virtual void                             setClip( const ::basegfx::B2DPolyPolygon& rClipPoly );
            virtual void                             setClip();
            virtual ::basegfx::B2DPolyPolygon const* getClip() const;

            virtual FontSharedPtr                    createFont( const ::rtl::OUString& rFontName, const double& rCellSize ) const;

            virtual ColorSharedPtr                   createColor() const;

            virtual CanvasSharedPtr                  clone() const;

            virtual void                             clear() const;

            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XCanvas >      getUNOCanvas() const;

            virtual ::com::sun::star::rendering::ViewState  getViewState() const;

            // take compiler-provided default copy constructor
            //ImplCanvas(const ImplCanvas&);

        private:
            // default: disabled assignment
            ImplCanvas& operator=( const ImplCanvas& );

            mutable ::com::sun::star::rendering::ViewState                                  maViewState;
            boost::optional<basegfx::B2DPolyPolygon>                                        maClipPolyPolygon;
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas >  mxCanvas;
        };

    }
}

#endif /* _CPPCANVAS_IMPLCANVAS_HXX */
