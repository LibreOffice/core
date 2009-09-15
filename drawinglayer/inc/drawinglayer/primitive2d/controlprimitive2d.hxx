/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: controlprimitive2d.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:17 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CONTROLPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CONTROLPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class ControlPrimitive2D : public BufDecPrimitive2D
        {
        private:
            // object's base data
            basegfx::B2DHomMatrix                                                   maTransform;
            com::sun::star::uno::Reference< com::sun::star::awt::XControlModel >    mxControlModel;

            // the created an cached awt::XControl
            com::sun::star::uno::Reference< com::sun::star::awt::XControl >         mxXControl;

            // the last used scaling, used from getDecomposition for buffering
            basegfx::B2DVector                                                      maLastViewScaling;

            // used from getXControl() to create a local awt::XControl which is remembered in mxXControl
            // and from thereon always used and returned by getXControl()
            void createXControl();

            // single local decompositions, used from createLocal2DDecomposition()
            Primitive2DReference createBitmapDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
            Primitive2DReference createPlaceholderDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        protected:
            // local decomposition
            virtual Primitive2DSequence createLocal2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            ControlPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const com::sun::star::uno::Reference< com::sun::star::awt::XControlModel >& rxControlModel);

            // constructor with an additional XControl as parameter to allow to hand it over at incarnation time
            // if it exists. This will avoid to create a 2nd one on demand in createXControl()
            // and thus double the XControls.
            ControlPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const com::sun::star::uno::Reference< com::sun::star::awt::XControlModel >& rxControlModel,
                const com::sun::star::uno::Reference< com::sun::star::awt::XControl >& rxXControl);

            // get data
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const com::sun::star::uno::Reference< com::sun::star::awt::XControlModel >& getControlModel() const { return mxControlModel; }

            // mxControl access. This will on demand create the awt::XControl using createXControl()
            // if it does not exist. It may already have been created or even handed over at
            // incarnation
            const com::sun::star::uno::Reference< com::sun::star::awt::XControl >& getXControl() const;

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()

            // Overload standard getDecomposition call to be view-dependent here
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CONTROLPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
