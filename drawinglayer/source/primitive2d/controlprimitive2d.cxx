/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: controlprimitive2d.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:42 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CONTROLPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/controlprimitive2d.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW2_HPP_
#include <com/sun/star/awt/XWindow2.hpp>
#endif

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX
#include <drawinglayer/geometry/viewinformation2d.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BITMAPPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#endif

#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_OPTIONSDRAWINGLAYER_HXX
#include <svtools/optionsdrawinglayer.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        void ControlPrimitive2D::createXControl()
        {
            if(!mxXControl.is() && getControlModel().is())
            {
                uno::Reference< beans::XPropertySet > xSet(getControlModel(), uno::UNO_QUERY);

                if(xSet.is())
                {
                    uno::Any aValue(xSet->getPropertyValue(rtl::OUString::createFromAscii("DefaultControl")));
                    rtl::OUString aUnoControlTypeName;

                    if(aValue >>= aUnoControlTypeName)
                    {
                        if(aUnoControlTypeName.getLength())
                        {
                            uno::Reference< lang::XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );

                            if(xFactory.is())
                            {
                                uno::Reference< awt::XControl > xXControl(xFactory->createInstance(aUnoControlTypeName), uno::UNO_QUERY);

                                if(xXControl.is())
                                {
                                    xXControl->setModel(getControlModel());

                                    // remember XControl
                                    mxXControl = xXControl;
                                }
                            }
                        }
                    }
                }
            }
        }

        Primitive2DReference ControlPrimitive2D::createBitmapDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DReference xRetval;
            const uno::Reference< awt::XControl >& rXControl(getXControl());

            if(rXControl.is())
            {
                uno::Reference< awt::XWindow > xControlWindow(rXControl, uno::UNO_QUERY);

                if(xControlWindow.is())
                {
                    // get decomposition to get size
                    basegfx::B2DVector aScale, aTranslate;
                    double fRotate, fShearX;
                    getTransform().decompose(aScale, aTranslate, fRotate, fShearX);

                    // get absolute discrete size (no mirror or rotate here)
                    aScale = basegfx::absolute(aScale);
                    basegfx::B2DVector aDiscreteSize(rViewInformation.getViewTransformation() * aScale);

                    // calc screen zoom for text display
                    basegfx::B2DVector aScreenZoom(
                        basegfx::fTools::equalZero(aScale.getX()) ? 1.0 : aDiscreteSize.getX() / aScale.getX(),
                        basegfx::fTools::equalZero(aScale.getY()) ? 1.0 : aDiscreteSize.getY() / aScale.getY());
                    static double fZoomScale(26.0); // do not ask for this constant factor, but it gets the zoom right
                    aScreenZoom *= fZoomScale;

                    // limit to a maximum square size, e.g. 300x150 pixels (45000)
                    const SvtOptionsDrawinglayer aDrawinglayerOpt;
                    const double fDiscreteMax(aDrawinglayerOpt.GetQuadraticFormControlRenderLimit());
                    const double fDiscreteQuadratic(aDiscreteSize.getX() * aDiscreteSize.getY());
                    const bool bScaleUsed(fDiscreteQuadratic > fDiscreteMax);
                    double fFactor(1.0);

                    if(bScaleUsed)
                    {
                        // get factor and adapt to scaled size
                        fFactor = sqrt(fDiscreteMax / fDiscreteQuadratic);
                        aDiscreteSize *= fFactor;
                        aScreenZoom *= fFactor;
                    }

                    // go to integer
                    const sal_Int32 nSizeX(basegfx::fround(aDiscreteSize.getX()));
                    const sal_Int32 nSizeY(basegfx::fround(aDiscreteSize.getY()));

                    if(nSizeX > 0 && nSizeY > 0)
                    {
                        // prepare VirtualDevice
                        VirtualDevice aVirtualDevice(*Application::GetDefaultDevice());
                        const Size aSizePixel(nSizeX, nSizeY);
                        aVirtualDevice.SetOutputSizePixel(aSizePixel);

                        // set size at control
                        xControlWindow->setPosSize(0, 0, nSizeX, nSizeY, awt::PosSize::POSSIZE);

                        // get graphics and view
                        uno::Reference< awt::XGraphics > xGraphics(aVirtualDevice.CreateUnoGraphics());
                        uno::Reference< awt::XView > xControlView(rXControl, uno::UNO_QUERY);

                        if(xGraphics.is() && xControlView.is())
                        {
                            // link graphics and view
                            xControlView->setGraphics(xGraphics);

                            // set zoom at control view for text scaling
                            xControlView->setZoom((float)aScreenZoom.getX(), (float)aScreenZoom.getY());

                            try
                            {
                                // try to paint it to VirtualDevice
                                xControlView->draw(0, 0);

                                // get bitmap
                                const Bitmap aContent(aVirtualDevice.GetBitmap(Point(), aSizePixel));

                                // to avoid scaling, use the Bitmap pixel size as primitive size
                                const Size aBitmapSize(aContent.GetSizePixel());
                                basegfx::B2DVector aBitmapSizeLogic(
                                    rViewInformation.getInverseViewTransformation() *
                                    basegfx::B2DVector(aBitmapSize.getWidth() - 1, aBitmapSize.getHeight() - 1));

                                if(bScaleUsed)
                                {
                                    // if scaled adapt to scaled size
                                    aBitmapSizeLogic /= fFactor;
                                }

                                // short form for scale and translate transformation
                                basegfx::B2DHomMatrix aBitmapTransform;

                                aBitmapTransform.set(0L, 0L, aBitmapSizeLogic.getX());
                                aBitmapTransform.set(1L, 1L, aBitmapSizeLogic.getY());
                                aBitmapTransform.set(0L, 2L, aTranslate.getX());
                                aBitmapTransform.set(1L, 2L, aTranslate.getY());

                                // create primitive
                                xRetval = new BitmapPrimitive2D(BitmapEx(aContent), aBitmapTransform);
                            }
                            catch( const uno::Exception& )
                            {
                                DBG_UNHANDLED_EXCEPTION();
                            }
                        }
                    }
                }
            }

            return xRetval;
        }

        Primitive2DReference ControlPrimitive2D::createPlaceholderDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // create a gray placeholder hairline polygon in object size
            basegfx::B2DRange aObjectRange(0.0, 0.0, 1.0, 1.0);
            aObjectRange.transform(getTransform());
            const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(aObjectRange));
            const basegfx::BColor aGrayTone(0xc0 / 255.0, 0xc0 / 255.0, 0xc0 / 255.0);

            // The replacement object may also get a text like 'empty group' here later
            Primitive2DReference xRetval(new PolygonHairlinePrimitive2D(aOutline, aGrayTone));

            return xRetval;
        }

        Primitive2DSequence ControlPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            // try to create a bitmap decomposition. If that fails for some reason,
            // at least create a replacement decomposition.
            Primitive2DReference xReference(createBitmapDecomposition(rViewInformation));

            if(!xReference.is())
            {
                xReference = createPlaceholderDecomposition(rViewInformation);
            }

            return Primitive2DSequence(&xReference, 1L);
        }

        ControlPrimitive2D::ControlPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const uno::Reference< awt::XControlModel >& rxControlModel)
        :   BasePrimitive2D(),
            maTransform(rTransform),
            mxControlModel(rxControlModel),
            mxXControl(),
            maLastViewScaling()
        {
        }

        ControlPrimitive2D::ControlPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const uno::Reference< awt::XControlModel >& rxControlModel,
            const uno::Reference< awt::XControl >& rxXControl)
        :   BasePrimitive2D(),
            maTransform(rTransform),
            mxControlModel(rxControlModel),
            mxXControl(rxXControl),
            maLastViewScaling()
        {
        }

        const uno::Reference< awt::XControl >& ControlPrimitive2D::getXControl() const
        {
            if(!mxXControl.is())
            {
                const_cast< ControlPrimitive2D* >(this)->createXControl();
            }

            return mxXControl;
        }

        bool ControlPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            // use base class compare operator
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const ControlPrimitive2D& rCompare = (ControlPrimitive2D&)rPrimitive;

                if(getTransform() == rCompare.getTransform())
                {
                    // check if ControlModel references both are/are not
                    bool bRetval(getControlModel().is() == rCompare.getControlModel().is());

                    if(bRetval && getControlModel().is())
                    {
                        // both exist, check for equality
                        bRetval = (getControlModel() == rCompare.getControlModel());
                    }

                    if(bRetval)
                    {
                        // check if XControl references both are/are not
                        bRetval = (getXControl().is() == rCompare.getXControl().is());
                    }

                    if(bRetval && getXControl().is())
                    {
                        // both exist, check for equality
                        bRetval = (getXControl() == rCompare.getXControl());
                    }

                    return bRetval;
                }
            }

            return false;
        }

        basegfx::B2DRange ControlPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // simply derivate from unit range
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
            aRetval.transform(getTransform());
            return aRetval;
        }

        Primitive2DSequence ControlPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            // this primitive is view-dependent related to the scaling. If scaling has changed,
            // destroy existing decomposition
            ::osl::MutexGuard aGuard( m_aMutex );
            const basegfx::B2DVector aNewScaling(rViewInformation.getViewTransformation() * basegfx::B2DVector(1.0, 1.0));

            if(getLocalDecomposition().hasElements())
            {
                if(!maLastViewScaling.equal(aNewScaling))
                {
                    // conditions of last local decomposition have changed, delete
                    const_cast< ControlPrimitive2D* >(this)->setLocalDecomposition(Primitive2DSequence());
                }
            }

            if(!getLocalDecomposition().hasElements())
            {
                // remember ViewTransformation
                const_cast< ControlPrimitive2D* >(this)->maLastViewScaling = aNewScaling;
            }

            // use parent implementation
            return BasePrimitive2D::get2DDecomposition(rViewInformation);
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(ControlPrimitive2D, PRIMITIVE2D_ID_CONTROLPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
