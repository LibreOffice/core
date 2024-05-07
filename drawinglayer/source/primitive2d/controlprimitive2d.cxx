/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <drawinglayer/primitive2d/controlprimitive2d.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <utility>
#include <rtl/ustrbuf.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <vcl/bitmapex.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <vcl/window.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <officecfg/Office/Common.hxx>

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
        void ControlPrimitive2D::createXControl()
        {
            if(mxXControl.is() || !getControlModel().is())
                return;

            uno::Reference< beans::XPropertySet > xSet(getControlModel(), uno::UNO_QUERY);

            if(!xSet.is())
                return;

            uno::Any aValue(xSet->getPropertyValue(u"DefaultControl"_ustr));
            OUString aUnoControlTypeName;

            if(!(aValue >>= aUnoControlTypeName))
                return;

            if(aUnoControlTypeName.isEmpty())
                return;

            uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
            uno::Reference< awt::XControl > xXControl(
                xContext->getServiceManager()->createInstanceWithContext(aUnoControlTypeName, xContext), uno::UNO_QUERY);

            if(xXControl.is())
            {
                xXControl->setModel(getControlModel());

                // remember XControl
                mxXControl = xXControl;
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
                    const double fDiscreteMax(officecfg::Office::Common::Drawinglayer::QuadraticFormControlRenderLimit::get());
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
                        ScopedVclPtrInstance< VirtualDevice > aVirtualDevice(*Application::GetDefaultDevice());
                        const Size aSizePixel(nSizeX, nSizeY);
                        aVirtualDevice->SetOutputSizePixel(aSizePixel);

                        // set size at control
                        xControlWindow->setPosSize(0, 0, nSizeX, nSizeY, awt::PosSize::POSSIZE);

                        // get graphics and view
                        uno::Reference< awt::XGraphics > xGraphics(aVirtualDevice->CreateUnoGraphics());
                        uno::Reference< awt::XView > xControlView(rXControl, uno::UNO_QUERY);

                        if(xGraphics.is() && xControlView.is())
                        {
                            // link graphics and view
                            xControlView->setGraphics(xGraphics);

                            {   // #i93162# For painting the control setting a Zoom (using setZoom() at the xControlView)
                                // is needed to define the font size. Normally this is done in
                                // ViewObjectContactOfUnoControl::createPrimitive2DSequence by using positionControlForPaint().
                                // For some reason the difference between MapUnit::MapTwipS and MapUnit::Map100thMM still plays
                                // a role there so that for Draw/Impress/Calc (the MapUnit::Map100thMM users) i need to set a zoom
                                // here, too. The factor includes the needed scale, but is calculated by pure comparisons. It
                                // is somehow related to the twips/100thmm relationship.
                                bool bUserIs100thmm(false);
                                const uno::Reference< awt::XControl > xControl(xControlView, uno::UNO_QUERY);

                                if(xControl.is())
                                {
                                    uno::Reference<awt::XWindowPeer> xWindowPeer(xControl->getPeer());
                                    if (xWindowPeer)
                                    {
                                        uno::Reference<awt::XVclWindowPeer> xPeerProps(xWindowPeer, uno::UNO_QUERY_THROW);
                                        uno::Any aAny = xPeerProps->getProperty(u"ParentIs100thmm"_ustr); // see VCLXWindow::getProperty
                                        aAny >>= bUserIs100thmm;
                                    }
                                }

                                if(bUserIs100thmm)
                                {
                                    // calc screen zoom for text display. fFactor is already added indirectly in aDiscreteSize
                                    basegfx::B2DVector aScreenZoom(
                                        basegfx::fTools::equalZero(aScale.getX()) ? 1.0 : aDiscreteSize.getX() / aScale.getX(),
                                        basegfx::fTools::equalZero(aScale.getY()) ? 1.0 : aDiscreteSize.getY() / aScale.getY());
                                    static const double fZoomScale(28.0); // do not ask for this constant factor, but it gets the zoom right
                                    aScreenZoom *= fZoomScale;

                                    // set zoom at control view for text scaling
                                    xControlView->setZoom(static_cast<float>(aScreenZoom.getX()), static_cast<float>(aScreenZoom.getY()));
                                }
                            }

                            try
                            {
                                // try to paint it to VirtualDevice
                                xControlView->draw(0, 0);

                                // get bitmap
                                const BitmapEx aContent(aVirtualDevice->GetBitmapEx(Point(), aSizePixel));

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
                                const basegfx::B2DHomMatrix aBitmapTransform(basegfx::utils::createScaleTranslateB2DHomMatrix(
                                    aBitmapSizeLogic.getX(), aBitmapSizeLogic.getY(), aTranslate.getX(), aTranslate.getY()));

                                // create primitive
                                xRetval = new BitmapPrimitive2D(
                                    aContent,
                                    aBitmapTransform);
                            }
                            catch( const uno::Exception& )
                            {
                                DBG_UNHANDLED_EXCEPTION("drawinglayer");
                            }
                        }
                    }
                }
            }

            return xRetval;
        }

        Primitive2DReference ControlPrimitive2D::createPlaceholderDecomposition() const
        {
            // create a gray placeholder hairline polygon in object size
            basegfx::B2DRange aObjectRange(0.0, 0.0, 1.0, 1.0);
            aObjectRange.transform(getTransform());
            basegfx::B2DPolygon aOutline(basegfx::utils::createPolygonFromRect(aObjectRange));
            const basegfx::BColor aGrayTone(0xc0 / 255.0, 0xc0 / 255.0, 0xc0 / 255.0);

            // The replacement object may also get a text like 'empty group' here later
            Primitive2DReference xRetval(new PolygonHairlinePrimitive2D(std::move(aOutline), aGrayTone));

            return xRetval;
        }

        Primitive2DReference ControlPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            // try to create a bitmap decomposition. If that fails for some reason,
            // at least create a replacement decomposition.
            Primitive2DReference xReference(createBitmapDecomposition(rViewInformation));

            if(!xReference.is())
            {
                xReference = createPlaceholderDecomposition();
            }

            return xReference;
        }

        ControlPrimitive2D::ControlPrimitive2D(
            basegfx::B2DHomMatrix aTransform,
            uno::Reference< awt::XControlModel > xControlModel,
            uno::Reference<awt::XControl> xXControl,
            ::std::u16string_view const rTitle,
            ::std::u16string_view const rDescription,
            void const*const pAnchorKey)
        :   maTransform(std::move(aTransform)),
            mxControlModel(std::move(xControlModel)),
            mxXControl(std::move(xXControl))
        , m_pAnchorStructureElementKey(pAnchorKey)
        {
            ::rtl::OUStringBuffer buf(rTitle);
            if (!rTitle.empty() && !rDescription.empty())
            {
                buf.append(" - ");
            }
            buf.append(rDescription);
            m_AltText = buf.makeStringAndClear();
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
            if(!BufferedDecompositionPrimitive2D::operator==(rPrimitive))
                return false;

            const ControlPrimitive2D& rCompare = static_cast<const ControlPrimitive2D&>(rPrimitive);

            if(getTransform() != rCompare.getTransform())
                return false;

            // check if ControlModel references both are/are not
            if (getControlModel().is() != rCompare.getControlModel().is())
                return false;

            if(getControlModel().is())
            {
                // both exist, check for equality
                if (getControlModel() != rCompare.getControlModel())
                    return false;
            }

                // check if XControl references both are/are not
            if (getXControl().is() != rCompare.getXControl().is())
                return false;

            if(getXControl().is())
            {
                // both exist, check for equality
                if (getXControl() != rCompare.getXControl())
                    return false;
            }

            return true;
        }

        basegfx::B2DRange ControlPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // simply derivate from unit range
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
            aRetval.transform(getTransform());
            return aRetval;
        }

        void ControlPrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const
        {
            // this primitive is view-dependent related to the scaling. If scaling has changed,
            // destroy existing decomposition. To detect change, use size of unit size in view coordinates
            const basegfx::B2DVector aNewScaling(rViewInformation.getObjectToViewTransformation() * basegfx::B2DVector(1.0, 1.0));

            if(getBuffered2DDecomposition())
            {
                if(!maLastViewScaling.equal(aNewScaling))
                {
                    // conditions of last local decomposition have changed, delete
                    const_cast< ControlPrimitive2D* >(this)->setBuffered2DDecomposition(nullptr);
                }
            }

            if(!getBuffered2DDecomposition())
            {
                // remember ViewTransformation
                const_cast< ControlPrimitive2D* >(this)->maLastViewScaling = aNewScaling;
            }

            // use parent implementation
            BufferedDecompositionPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
        }

        // provide unique ID
        sal_uInt32 ControlPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_CONTROLPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
