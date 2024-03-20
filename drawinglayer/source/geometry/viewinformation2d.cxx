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

#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#include <o3tl/temporary.hxx>
#include <officecfg/Office/Common.hxx>
#include <unotools/configmgr.hxx>

#include <atomic>
#include <utility>

using namespace com::sun::star;

namespace drawinglayer::geometry
{
namespace
{
constexpr OUStringLiteral g_PropertyName_ObjectTransformation = u"ObjectTransformation";
constexpr OUStringLiteral g_PropertyName_ViewTransformation = u"ViewTransformation";
constexpr OUStringLiteral g_PropertyName_Viewport = u"Viewport";
constexpr OUStringLiteral g_PropertyName_Time = u"Time";
constexpr OUStringLiteral g_PropertyName_VisualizedPage = u"VisualizedPage";
constexpr OUStringLiteral g_PropertyName_ReducedDisplayQuality = u"ReducedDisplayQuality";
constexpr OUStringLiteral g_PropertyName_UseAntiAliasing = u"UseAntiAliasing";
constexpr OUStringLiteral g_PropertyName_PixelSnapHairline = u"PixelSnapHairline";
}

namespace
{
bool bForwardsAreInitialized(false);
bool bForwardPixelSnapHairline(true);
}

class ImpViewInformation2D
{
private:
    // ViewInformation2D implementation can change refcount, so we have only
    // two memory regions for pairs of ViewInformation2D/ImpViewInformation2D
    friend class ::drawinglayer::geometry::ViewInformation2D;

protected:
    // the object transformation
    basegfx::B2DHomMatrix maObjectTransformation;

    // the view transformation
    basegfx::B2DHomMatrix maViewTransformation;

    // the ObjectToView and it's inverse, both on demand from ObjectTransformation
    // and ViewTransformation
    basegfx::B2DHomMatrix maObjectToViewTransformation;
    basegfx::B2DHomMatrix maInverseObjectToViewTransformation;

    // the visible range and the on-demand one in ViewCoordinates
    basegfx::B2DRange maViewport;
    basegfx::B2DRange maDiscreteViewport;

    // the DrawPage which is target of visualisation. This is needed e.g. for
    // the view-dependent decomposition of PageNumber TextFields.
    // This parameter is buffered here, but mainly resides in mxExtendedInformation,
    // so it will be interpreted, but held there. It will also not be added
    // to mxExtendedInformation in impFillViewInformationFromContent (it's there already)
    uno::Reference<drawing::XDrawPage> mxVisualizedPage;

    // the point in time
    double mfViewTime;

    // color to use for automatic color
    Color maAutoColor;

    // a hint that the View that is being painted has an active TextEdit. This
    // is important for handling of TextHierarchyEditPrimitive2D to suppress
    // the text for objects in TextEdit - the text is visualized by the
    // active EditEngine/Outliner overlay, so it would be double visualized
    bool mbTextEditActive : 1;

    // processed view is an EditView
    bool mbEditViewActive : 1;

    // allow to reduce DisplayQuality (e.g. sw 3d fallback renderer for interactions)
    bool mbReducedDisplayQuality : 1;

    // determine if to use AntiAliasing on target pixel device
    bool mbUseAntiAliasing : 1;

    // determine if to use PixelSnapHairline on target pixel device
    bool mbPixelSnapHairline : 1;

public:
    ImpViewInformation2D()
        : maObjectTransformation()
        , maViewTransformation()
        , maObjectToViewTransformation()
        , maInverseObjectToViewTransformation()
        , maViewport()
        , maDiscreteViewport()
        , mxVisualizedPage()
        , mfViewTime(0.0)
        , maAutoColor(COL_AUTO)
        , mbTextEditActive(false)
        , mbEditViewActive(false)
        , mbReducedDisplayQuality(false)
        , mbUseAntiAliasing(ViewInformation2D::getGlobalAntiAliasing())
        , mbPixelSnapHairline(mbUseAntiAliasing && bForwardPixelSnapHairline)
    {
    }

    const basegfx::B2DHomMatrix& getObjectTransformation() const { return maObjectTransformation; }
    void setObjectTransformation(const basegfx::B2DHomMatrix& rNew)
    {
        maObjectTransformation = rNew;
        maObjectToViewTransformation.identity();
        maInverseObjectToViewTransformation.identity();
    }

    const basegfx::B2DHomMatrix& getViewTransformation() const { return maViewTransformation; }
    void setViewTransformation(const basegfx::B2DHomMatrix& rNew)
    {
        maViewTransformation = rNew;
        maDiscreteViewport.reset();
        maObjectToViewTransformation.identity();
        maInverseObjectToViewTransformation.identity();
    }

    const basegfx::B2DRange& getViewport() const { return maViewport; }
    void setViewport(const basegfx::B2DRange& rNew)
    {
        maViewport = rNew;
        maDiscreteViewport.reset();
    }

    const basegfx::B2DRange& getDiscreteViewport() const
    {
        if (maDiscreteViewport.isEmpty() && !maViewport.isEmpty())
        {
            basegfx::B2DRange aDiscreteViewport(maViewport);
            aDiscreteViewport.transform(getViewTransformation());
            const_cast<ImpViewInformation2D*>(this)->maDiscreteViewport = aDiscreteViewport;
        }

        return maDiscreteViewport;
    }

    const basegfx::B2DHomMatrix& getObjectToViewTransformation() const
    {
        if (maObjectToViewTransformation.isIdentity()
            && (!maObjectTransformation.isIdentity() || !maViewTransformation.isIdentity()))
        {
            basegfx::B2DHomMatrix aObjectToView(maViewTransformation * maObjectTransformation);
            const_cast<ImpViewInformation2D*>(this)->maObjectToViewTransformation = aObjectToView;
        }

        return maObjectToViewTransformation;
    }

    const basegfx::B2DHomMatrix& getInverseObjectToViewTransformation() const
    {
        if (maInverseObjectToViewTransformation.isIdentity()
            && (!maObjectTransformation.isIdentity() || !maViewTransformation.isIdentity()))
        {
            basegfx::B2DHomMatrix aInverseObjectToView(maViewTransformation
                                                       * maObjectTransformation);
            aInverseObjectToView.invert();
            const_cast<ImpViewInformation2D*>(this)->maInverseObjectToViewTransformation
                = aInverseObjectToView;
        }

        return maInverseObjectToViewTransformation;
    }

    double getViewTime() const { return mfViewTime; }
    void setViewTime(double fNew)
    {
        if (fNew >= 0.0)
        {
            mfViewTime = fNew;
        }
    }

    const uno::Reference<drawing::XDrawPage>& getVisualizedPage() const { return mxVisualizedPage; }
    void setVisualizedPage(const uno::Reference<drawing::XDrawPage>& rNew)
    {
        mxVisualizedPage = rNew;
    }

    Color getAutoColor() const { return maAutoColor; }
    void setAutoColor(Color aNew) { maAutoColor = aNew; }

    bool getTextEditActive() const { return mbTextEditActive; }
    void setTextEditActive(bool bNew) { mbTextEditActive = bNew; }

    bool getEditViewActive() const { return mbEditViewActive; }
    void setEditViewActive(bool bNew) { mbEditViewActive = bNew; }

    bool getReducedDisplayQuality() const { return mbReducedDisplayQuality; }
    void setReducedDisplayQuality(bool bNew) { mbReducedDisplayQuality = bNew; }

    bool getUseAntiAliasing() const { return mbUseAntiAliasing; }
    void setUseAntiAliasing(bool bNew) { mbUseAntiAliasing = bNew; }

    bool getPixelSnapHairline() const { return mbPixelSnapHairline; }
    void setPixelSnapHairline(bool bNew) { mbPixelSnapHairline = bNew; }

    bool operator==(const ImpViewInformation2D& rCandidate) const
    {
        return (maObjectTransformation == rCandidate.maObjectTransformation
                && maViewTransformation == rCandidate.maViewTransformation
                && maViewport == rCandidate.maViewport
                && mxVisualizedPage == rCandidate.mxVisualizedPage
                && mfViewTime == rCandidate.mfViewTime && maAutoColor == rCandidate.maAutoColor
                && mbTextEditActive == rCandidate.mbTextEditActive
                && mbEditViewActive == rCandidate.mbEditViewActive
                && mbReducedDisplayQuality == rCandidate.mbReducedDisplayQuality
                && mbUseAntiAliasing == rCandidate.mbUseAntiAliasing
                && mbPixelSnapHairline == rCandidate.mbPixelSnapHairline);
    }
};

namespace
{
ViewInformation2D::ImplType& theGlobalDefault()
{
    static ViewInformation2D::ImplType SINGLETON;
    return SINGLETON;
}
}

ViewInformation2D::ViewInformation2D()
    : mpViewInformation2D(theGlobalDefault())
{
    if (!bForwardsAreInitialized)
    {
        bForwardsAreInitialized = true;
        if (!comphelper::IsFuzzing())
        {
            bForwardPixelSnapHairline
                = officecfg::Office::Common::Drawinglayer::SnapHorVerLinesToDiscrete::get();
        }
    }

    setUseAntiAliasing(ViewInformation2D::getGlobalAntiAliasing());
    setPixelSnapHairline(bForwardPixelSnapHairline);
}

ViewInformation2D::ViewInformation2D(const ViewInformation2D&) = default;

ViewInformation2D::ViewInformation2D(ViewInformation2D&&) = default;

ViewInformation2D::~ViewInformation2D() = default;

ViewInformation2D& ViewInformation2D::operator=(const ViewInformation2D&) = default;

ViewInformation2D& ViewInformation2D::operator=(ViewInformation2D&&) = default;

bool ViewInformation2D::operator==(const ViewInformation2D& rCandidate) const
{
    return rCandidate.mpViewInformation2D == mpViewInformation2D;
}

const basegfx::B2DHomMatrix& ViewInformation2D::getObjectTransformation() const
{
    return mpViewInformation2D->getObjectTransformation();
}

void ViewInformation2D::setObjectTransformation(const basegfx::B2DHomMatrix& rNew)
{
    if (std::as_const(mpViewInformation2D)->getObjectTransformation() != rNew)
        mpViewInformation2D->setObjectTransformation(rNew);
}

const basegfx::B2DHomMatrix& ViewInformation2D::getViewTransformation() const
{
    return mpViewInformation2D->getViewTransformation();
}

void ViewInformation2D::setViewTransformation(const basegfx::B2DHomMatrix& rNew)
{
    if (std::as_const(mpViewInformation2D)->getViewTransformation() != rNew)
        mpViewInformation2D->setViewTransformation(rNew);
}

const basegfx::B2DRange& ViewInformation2D::getViewport() const
{
    return mpViewInformation2D->getViewport();
}

void ViewInformation2D::setViewport(const basegfx::B2DRange& rNew)
{
    if (rNew != std::as_const(mpViewInformation2D)->getViewport())
        mpViewInformation2D->setViewport(rNew);
}

double ViewInformation2D::getViewTime() const { return mpViewInformation2D->getViewTime(); }

void ViewInformation2D::setViewTime(double fNew)
{
    if (fNew != std::as_const(mpViewInformation2D)->getViewTime())
        mpViewInformation2D->setViewTime(fNew);
}

const uno::Reference<drawing::XDrawPage>& ViewInformation2D::getVisualizedPage() const
{
    return mpViewInformation2D->getVisualizedPage();
}

void ViewInformation2D::setVisualizedPage(const uno::Reference<drawing::XDrawPage>& rNew)
{
    if (rNew != std::as_const(mpViewInformation2D)->getVisualizedPage())
        mpViewInformation2D->setVisualizedPage(rNew);
}

const basegfx::B2DHomMatrix& ViewInformation2D::getObjectToViewTransformation() const
{
    return mpViewInformation2D->getObjectToViewTransformation();
}

const basegfx::B2DHomMatrix& ViewInformation2D::getInverseObjectToViewTransformation() const
{
    return mpViewInformation2D->getInverseObjectToViewTransformation();
}

const basegfx::B2DRange& ViewInformation2D::getDiscreteViewport() const
{
    return mpViewInformation2D->getDiscreteViewport();
}

bool ViewInformation2D::getReducedDisplayQuality() const
{
    return mpViewInformation2D->getReducedDisplayQuality();
}

void ViewInformation2D::setReducedDisplayQuality(bool bNew)
{
    if (bNew != std::as_const(mpViewInformation2D)->getReducedDisplayQuality())
        mpViewInformation2D->setReducedDisplayQuality(bNew);
}

bool ViewInformation2D::getUseAntiAliasing() const
{
    return mpViewInformation2D->getUseAntiAliasing();
}

void ViewInformation2D::setUseAntiAliasing(bool bNew)
{
    if (bNew != std::as_const(mpViewInformation2D)->getUseAntiAliasing())
        mpViewInformation2D->setUseAntiAliasing(bNew);
}

Color ViewInformation2D::getAutoColor() const { return mpViewInformation2D->getAutoColor(); }

void ViewInformation2D::setAutoColor(Color aNew) { mpViewInformation2D->setAutoColor(aNew); }

bool ViewInformation2D::getTextEditActive() const
{
    return mpViewInformation2D->getTextEditActive();
}

void ViewInformation2D::setTextEditActive(bool bNew)
{
    mpViewInformation2D->setTextEditActive(bNew);
}

bool ViewInformation2D::getEditViewActive() const
{
    return mpViewInformation2D->getEditViewActive();
}

void ViewInformation2D::setEditViewActive(bool bNew)
{
    mpViewInformation2D->setEditViewActive(bNew);
}

bool ViewInformation2D::getPixelSnapHairline() const
{
    return mpViewInformation2D->getPixelSnapHairline();
}

void ViewInformation2D::setPixelSnapHairline(bool bNew)
{
    if (bNew != std::as_const(mpViewInformation2D)->getPixelSnapHairline())
        mpViewInformation2D->setPixelSnapHairline(bNew);
}

static std::atomic<bool>& globalAntiAliasing()
{
    static std::atomic<bool> g_GlobalAntiAliasing
        = comphelper::IsFuzzing() || officecfg::Office::Common::Drawinglayer::AntiAliasing::get();
    return g_GlobalAntiAliasing;
}

/**
  * Some code like to turn this stuff on and off during a drawing operation
  * so it can "tunnel" information down through several layers,
  * so we don't want to actually do a config write all the time.
  */
void ViewInformation2D::setGlobalAntiAliasing(bool bAntiAliasing, bool bTemporary)
{
    if (globalAntiAliasing().compare_exchange_strong(o3tl::temporary(!bAntiAliasing), bAntiAliasing)
        && !bTemporary)
    {
        auto batch = comphelper::ConfigurationChanges::create();
        officecfg::Office::Common::Drawinglayer::AntiAliasing::set(bAntiAliasing, batch);
        batch->commit();
    }
}
bool ViewInformation2D::getGlobalAntiAliasing() { return globalAntiAliasing(); }

void ViewInformation2D::forwardPixelSnapHairline(bool bPixelSnapHairline)
{
    bForwardPixelSnapHairline = bPixelSnapHairline;
}

ViewInformation2D
createViewInformation2D(const css::uno::Sequence<css::beans::PropertyValue>& rViewParameters)
{
    if (!rViewParameters.hasElements())
        return ViewInformation2D();

    ViewInformation2D aRetval;

    for (auto const& rPropertyValue : rViewParameters)
    {
        if (rPropertyValue.Name == g_PropertyName_ReducedDisplayQuality)
        {
            bool bNew(false);
            rPropertyValue.Value >>= bNew;
            aRetval.setReducedDisplayQuality(bNew);
        }
        else if (rPropertyValue.Name == g_PropertyName_PixelSnapHairline)
        {
            bool bNew(
                true); //SvtOptionsDrawinglayer::IsAntiAliasing() && SvtOptionsDrawinglayer::IsSnapHorVerLinesToDiscrete());
            rPropertyValue.Value >>= bNew;
            aRetval.setPixelSnapHairline(bNew);
        }
        else if (rPropertyValue.Name == g_PropertyName_UseAntiAliasing)
        {
            bool bNew(true); //SvtOptionsDrawinglayer::IsAntiAliasing());
            rPropertyValue.Value >>= bNew;
            aRetval.setUseAntiAliasing(bNew);
        }
        else if (rPropertyValue.Name == g_PropertyName_ObjectTransformation)
        {
            css::geometry::AffineMatrix2D aAffineMatrix2D;
            rPropertyValue.Value >>= aAffineMatrix2D;
            basegfx::B2DHomMatrix aTransformation;
            basegfx::unotools::homMatrixFromAffineMatrix(aTransformation, aAffineMatrix2D);
            aRetval.setObjectTransformation(aTransformation);
        }
        else if (rPropertyValue.Name == g_PropertyName_ViewTransformation)
        {
            css::geometry::AffineMatrix2D aAffineMatrix2D;
            rPropertyValue.Value >>= aAffineMatrix2D;
            basegfx::B2DHomMatrix aTransformation;
            basegfx::unotools::homMatrixFromAffineMatrix(aTransformation, aAffineMatrix2D);
            aRetval.setViewTransformation(aTransformation);
        }
        else if (rPropertyValue.Name == g_PropertyName_Viewport)
        {
            css::geometry::RealRectangle2D aUnoViewport;
            rPropertyValue.Value >>= aUnoViewport;
            const basegfx::B2DRange aViewport(
                basegfx::unotools::b2DRectangleFromRealRectangle2D(aUnoViewport));
            aRetval.setViewport(aViewport);
        }
        else if (rPropertyValue.Name == g_PropertyName_Time)
        {
            double fViewTime(0.0);
            rPropertyValue.Value >>= fViewTime;
            aRetval.setViewTime(fViewTime);
        }
        else if (rPropertyValue.Name == g_PropertyName_VisualizedPage)
        {
            css::uno::Reference<css::drawing::XDrawPage> xVisualizedPage;
            rPropertyValue.Value >>= xVisualizedPage;
            aRetval.setVisualizedPage(xVisualizedPage);
        }
    }

    return aRetval;
}

} // end of namespace drawinglayer::geometry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
