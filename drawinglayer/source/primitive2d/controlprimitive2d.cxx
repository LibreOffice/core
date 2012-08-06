/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <drawinglayer/primitive2d/controlprimitive2d.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/XWindow2.hpp>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <vcl/bitmapex.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <tools/diagnose_ex.h>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <vcl/window.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

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
                    uno::Any aValue(xSet->getPropertyValue("DefaultControl"));
                    rtl::OUString aUnoControlTypeName;

                    if(aValue >>= aUnoControlTypeName)
                    {
                        if(!aUnoControlTypeName.isEmpty())
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
                    basegfx::B2DVector aDiscreteSize(rViewInformation.getObjectToViewTransformation() * aScale);

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

                            {   // #i93162# For painting the control setting a Zoom (using setZoom() at the xControlView)
                                // is needed to define the font size. Normally this is done in
                                // ViewObjectContactOfUnoControl::createPrimitive2DSequence by using positionControlForPaint().
                                // For some reason the difference between MAP_TWIPS and MAP_100TH_MM still plays
                                // a role there so that for Draw/Impress/Calc (the MAP_100TH_MM users) i need to set a zoom
                                // here, too. The factor includes the needed scale, but is calculated by pure comparisons. It
                                // is somehow related to the twips/100thmm relationship.
                                bool bUserIs100thmm(false);
                                const uno::Reference< awt::XControl > xControl(xControlView, uno::UNO_QUERY);

                                if(xControl.is())
                                {
                                    uno::Reference< awt::XWindowPeer > xWindowPeer(xControl->getPeer());

                                    if(xWindowPeer.is())
                                    {
                                        VCLXWindow* pVCLXWindow = VCLXWindow::GetImplementation(xWindowPeer);

                                        if(pVCLXWindow)
                                        {
                                            Window* pWindow = pVCLXWindow->GetWindow();

                                            if(pWindow)
                                            {
                                                pWindow = pWindow->GetParent();

                                                if(pWindow)
                                                {
                                                    if(MAP_100TH_MM == pWindow->GetMapMode().GetMapUnit())
                                                    {
                                                        bUserIs100thmm = true;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                if(bUserIs100thmm)
                                {
                                    // calc screen zoom for text display. fFactor is already added indirectly in aDiscreteSize
                                    basegfx::B2DVector aScreenZoom(
                                        basegfx::fTools::equalZero(aScale.getX()) ? 1.0 : aDiscreteSize.getX() / aScale.getX(),
                                        basegfx::fTools::equalZero(aScale.getY()) ? 1.0 : aDiscreteSize.getY() / aScale.getY());
                                    static double fZoomScale(28.0); // do not ask for this constant factor, but it gets the zoom right
                                    aScreenZoom *= fZoomScale;

                                    // set zoom at control view for text scaling
                                    xControlView->setZoom((float)aScreenZoom.getX(), (float)aScreenZoom.getY());
                                }
                            }

                            try
                            {
                                // try to paint it to VirtualDevice
                                xControlView->draw(0, 0);

                                // get bitmap
                                const Bitmap aContent(aVirtualDevice.GetBitmap(Point(), aSizePixel));

                                // to avoid scaling, use the Bitmap pixel size as primitive size
                                const Size aBitmapSize(aContent.GetSizePixel());
                                basegfx::B2DVector aBitmapSizeLogic(
                                    rViewInformation.getInverseObjectToViewTransformation() *
                                    basegfx::B2DVector(aBitmapSize.getWidth() - 1, aBitmapSize.getHeight() - 1));

                                if(bScaleUsed)
                                {
                                    // if scaled adapt to scaled size
                                    aBitmapSizeLogic /= fFactor;
                                }

                                // short form for scale and translate transformation
                                const basegfx::B2DHomMatrix aBitmapTransform(basegfx::tools::createScaleTranslateB2DHomMatrix(
                                    aBitmapSizeLogic.getX(), aBitmapSizeLogic.getY(), aTranslate.getX(), aTranslate.getY()));

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

        Primitive2DSequence ControlPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
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
        :   BufferedDecompositionPrimitive2D(),
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
        :   BufferedDecompositionPrimitive2D(),
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
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
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
            // destroy existing decomposition. To detect change, use size of unit size in view coordinates
            ::osl::MutexGuard aGuard( m_aMutex );
            const basegfx::B2DVector aNewScaling(rViewInformation.getObjectToViewTransformation() * basegfx::B2DVector(1.0, 1.0));

            if(getBuffered2DDecomposition().hasElements())
            {
                if(!maLastViewScaling.equal(aNewScaling))
                {
                    // conditions of last local decomposition have changed, delete
                    const_cast< ControlPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
                }
            }

            if(!getBuffered2DDecomposition().hasElements())
            {
                // remember ViewTransformation
                const_cast< ControlPrimitive2D* >(this)->maLastViewScaling = aNewScaling;
            }

            // use parent implementation
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(ControlPrimitive2D, PRIMITIVE2D_ID_CONTROLPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
