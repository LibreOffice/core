/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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



using namespace com::sun::star;



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
                    OUString aUnoControlTypeName;

                    if(aValue >>= aUnoControlTypeName)
                    {
                        if(!aUnoControlTypeName.isEmpty())
                        {
                            uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                            uno::Reference< awt::XControl > xXControl(
                                xContext->getServiceManager()->createInstanceWithContext(aUnoControlTypeName, xContext), uno::UNO_QUERY);

                            if(xXControl.is())
                            {
                                xXControl->setModel(getControlModel());

                                
                                mxXControl = xXControl;
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
                    
                    basegfx::B2DVector aScale, aTranslate;
                    double fRotate, fShearX;
                    getTransform().decompose(aScale, aTranslate, fRotate, fShearX);

                    
                    aScale = basegfx::absolute(aScale);
                    basegfx::B2DVector aDiscreteSize(rViewInformation.getObjectToViewTransformation() * aScale);

                    
                    const SvtOptionsDrawinglayer aDrawinglayerOpt;
                    const double fDiscreteMax(aDrawinglayerOpt.GetQuadraticFormControlRenderLimit());
                    const double fDiscreteQuadratic(aDiscreteSize.getX() * aDiscreteSize.getY());
                    const bool bScaleUsed(fDiscreteQuadratic > fDiscreteMax);
                    double fFactor(1.0);

                    if(bScaleUsed)
                    {
                        
                        fFactor = sqrt(fDiscreteMax / fDiscreteQuadratic);
                        aDiscreteSize *= fFactor;
                    }

                    
                    const sal_Int32 nSizeX(basegfx::fround(aDiscreteSize.getX()));
                    const sal_Int32 nSizeY(basegfx::fround(aDiscreteSize.getY()));

                    if(nSizeX > 0 && nSizeY > 0)
                    {
                        
                        VirtualDevice aVirtualDevice(*Application::GetDefaultDevice());
                        const Size aSizePixel(nSizeX, nSizeY);
                        aVirtualDevice.SetOutputSizePixel(aSizePixel);

                        
                        xControlWindow->setPosSize(0, 0, nSizeX, nSizeY, awt::PosSize::POSSIZE);

                        
                        uno::Reference< awt::XGraphics > xGraphics(aVirtualDevice.CreateUnoGraphics());
                        uno::Reference< awt::XView > xControlView(rXControl, uno::UNO_QUERY);

                        if(xGraphics.is() && xControlView.is())
                        {
                            
                            xControlView->setGraphics(xGraphics);

                            {   
                                
                                
                                
                                
                                
                                
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
                                    
                                    basegfx::B2DVector aScreenZoom(
                                        basegfx::fTools::equalZero(aScale.getX()) ? 1.0 : aDiscreteSize.getX() / aScale.getX(),
                                        basegfx::fTools::equalZero(aScale.getY()) ? 1.0 : aDiscreteSize.getY() / aScale.getY());
                                    static double fZoomScale(28.0); 
                                    aScreenZoom *= fZoomScale;

                                    
                                    xControlView->setZoom((float)aScreenZoom.getX(), (float)aScreenZoom.getY());
                                }
                            }

                            try
                            {
                                
                                xControlView->draw(0, 0);

                                
                                const Bitmap aContent(aVirtualDevice.GetBitmap(Point(), aSizePixel));

                                
                                const Size aBitmapSize(aContent.GetSizePixel());
                                basegfx::B2DVector aBitmapSizeLogic(
                                    rViewInformation.getInverseObjectToViewTransformation() *
                                    basegfx::B2DVector(aBitmapSize.getWidth() - 1, aBitmapSize.getHeight() - 1));

                                if(bScaleUsed)
                                {
                                    
                                    aBitmapSizeLogic /= fFactor;
                                }

                                
                                const basegfx::B2DHomMatrix aBitmapTransform(basegfx::tools::createScaleTranslateB2DHomMatrix(
                                    aBitmapSizeLogic.getX(), aBitmapSizeLogic.getY(), aTranslate.getX(), aTranslate.getY()));

                                
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
            
            basegfx::B2DRange aObjectRange(0.0, 0.0, 1.0, 1.0);
            aObjectRange.transform(getTransform());
            const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(aObjectRange));
            const basegfx::BColor aGrayTone(0xc0 / 255.0, 0xc0 / 255.0, 0xc0 / 255.0);

            
            Primitive2DReference xRetval(new PolygonHairlinePrimitive2D(aOutline, aGrayTone));

            return xRetval;
        }

        Primitive2DSequence ControlPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            
            
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
            
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const ControlPrimitive2D& rCompare = (ControlPrimitive2D&)rPrimitive;

                if(getTransform() == rCompare.getTransform())
                {
                    
                    bool bRetval(getControlModel().is() == rCompare.getControlModel().is());

                    if(bRetval && getControlModel().is())
                    {
                        
                        bRetval = (getControlModel() == rCompare.getControlModel());
                    }

                    if(bRetval)
                    {
                        
                        bRetval = (getXControl().is() == rCompare.getXControl().is());
                    }

                    if(bRetval && getXControl().is())
                    {
                        
                        bRetval = (getXControl() == rCompare.getXControl());
                    }

                    return bRetval;
                }
            }

            return false;
        }

        basegfx::B2DRange ControlPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
            aRetval.transform(getTransform());
            return aRetval;
        }

        Primitive2DSequence ControlPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            
            
            ::osl::MutexGuard aGuard( m_aMutex );
            const basegfx::B2DVector aNewScaling(rViewInformation.getObjectToViewTransformation() * basegfx::B2DVector(1.0, 1.0));

            if(getBuffered2DDecomposition().hasElements())
            {
                if(!maLastViewScaling.equal(aNewScaling))
                {
                    
                    const_cast< ControlPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
                }
            }

            if(!getBuffered2DDecomposition().hasElements())
            {
                
                const_cast< ControlPrimitive2D* >(this)->maLastViewScaling = aNewScaling;
            }

            
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }

        
        ImplPrimitive2DIDBlock(ControlPrimitive2D, PRIMITIVE2D_ID_CONTROLPRIMITIVE2D)

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
