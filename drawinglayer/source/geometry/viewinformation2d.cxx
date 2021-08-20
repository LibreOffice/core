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
#include <rtl/instance.hxx>
#include <com/sun/star/uno/Sequence.hxx>

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

    bool mbReducedDisplayQuality : 1;

public:
    ImpViewInformation2D(const basegfx::B2DHomMatrix& rObjectTransformation,
                         const basegfx::B2DHomMatrix& rViewTransformation,
                         const basegfx::B2DRange& rViewport,
                         const uno::Reference<drawing::XDrawPage>& rxDrawPage, double fViewTime,
                         bool bReducedDisplayQuality)
        : maObjectTransformation(rObjectTransformation)
        , maViewTransformation(rViewTransformation)
        , maViewport(rViewport)
        , mxVisualizedPage(rxDrawPage)
        , mfViewTime(fViewTime)
        , mbReducedDisplayQuality(bReducedDisplayQuality)
    {
    }

    ImpViewInformation2D()
        : mfViewTime(0.0)
        , mbReducedDisplayQuality(false)
    {
    }

    const basegfx::B2DHomMatrix& getObjectTransformation() const { return maObjectTransformation; }

    const basegfx::B2DHomMatrix& getViewTransformation() const { return maViewTransformation; }

    const basegfx::B2DRange& getViewport() const { return maViewport; }

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

    const uno::Reference<drawing::XDrawPage>& getVisualizedPage() const { return mxVisualizedPage; }

    bool getReducedDisplayQuality() const { return mbReducedDisplayQuality; }

    bool operator==(const ImpViewInformation2D& rCandidate) const
    {
        return (maObjectTransformation == rCandidate.maObjectTransformation
                && maViewTransformation == rCandidate.maViewTransformation
                && maViewport == rCandidate.maViewport
                && mxVisualizedPage == rCandidate.mxVisualizedPage
                && mfViewTime == rCandidate.mfViewTime);
    }
};

namespace
{
struct theGlobalDefault : public rtl::Static<ViewInformation2D::ImplType, theGlobalDefault>
{
};
}

ViewInformation2D::ViewInformation2D(const basegfx::B2DHomMatrix& rObjectTransformation,
                                     const basegfx::B2DHomMatrix& rViewTransformation,
                                     const basegfx::B2DRange& rViewport,
                                     const uno::Reference<drawing::XDrawPage>& rxDrawPage,
                                     double fViewTime, bool bReducedDisplayQuality)
    : mpViewInformation2D(ImpViewInformation2D(rObjectTransformation, rViewTransformation,
                                               rViewport, rxDrawPage, fViewTime,
                                               bReducedDisplayQuality))
{
}

ViewInformation2D::ViewInformation2D()
    : mpViewInformation2D(theGlobalDefault::get())
{
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

const basegfx::B2DHomMatrix& ViewInformation2D::getViewTransformation() const
{
    return mpViewInformation2D->getViewTransformation();
}

const basegfx::B2DRange& ViewInformation2D::getViewport() const
{
    return mpViewInformation2D->getViewport();
}

double ViewInformation2D::getViewTime() const { return mpViewInformation2D->getViewTime(); }

const uno::Reference<drawing::XDrawPage>& ViewInformation2D::getVisualizedPage() const
{
    return mpViewInformation2D->getVisualizedPage();
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

ViewInformation2D
createViewInformation2D(const css::uno::Sequence<css::beans::PropertyValue>& rViewParameters)
{
    if (!rViewParameters.hasElements())
        return ViewInformation2D();

    bool bReducedDisplayQuality = false;
    basegfx::B2DHomMatrix aObjectTransformation;
    basegfx::B2DHomMatrix aViewTransformation;
    basegfx::B2DRange aViewport;
    double fViewTime = 0.0;
    uno::Reference<drawing::XDrawPage> xVisualizedPage;

    for (auto const& rPropertyValue : rViewParameters)
    {
        if (rPropertyValue.Name == g_PropertyName_ReducedDisplayQuality)
        {
            rPropertyValue.Value >>= bReducedDisplayQuality;
        }
        else if (rPropertyValue.Name == g_PropertyName_ObjectTransformation)
        {
            css::geometry::AffineMatrix2D aAffineMatrix2D;
            rPropertyValue.Value >>= aAffineMatrix2D;
            basegfx::unotools::homMatrixFromAffineMatrix(aObjectTransformation, aAffineMatrix2D);
        }
        else if (rPropertyValue.Name == g_PropertyName_ViewTransformation)
        {
            css::geometry::AffineMatrix2D aAffineMatrix2D;
            rPropertyValue.Value >>= aAffineMatrix2D;
            basegfx::unotools::homMatrixFromAffineMatrix(aViewTransformation, aAffineMatrix2D);
        }
        else if (rPropertyValue.Name == g_PropertyName_Viewport)
        {
            css::geometry::RealRectangle2D aUnoViewport;
            rPropertyValue.Value >>= aUnoViewport;
            aViewport = basegfx::unotools::b2DRectangleFromRealRectangle2D(aUnoViewport);
        }
        else if (rPropertyValue.Name == g_PropertyName_Time)
        {
            rPropertyValue.Value >>= fViewTime;
        }
        else if (rPropertyValue.Name == g_PropertyName_VisualizedPage)
        {
            rPropertyValue.Value >>= xVisualizedPage;
        }
    }

    return ViewInformation2D(aObjectTransformation, aViewTransformation, aViewport, xVisualizedPage,
                             fViewTime, bReducedDisplayQuality);
}

uno::Sequence<beans::PropertyValue>
createPropertyValues(const ViewInformation2D& rViewInformation2D)
{
    const bool bObjectTransformationUsed(
        !rViewInformation2D.getObjectTransformation().isIdentity());
    const bool bViewTransformationUsed(!rViewInformation2D.getViewTransformation().isIdentity());
    const bool bViewportUsed(!rViewInformation2D.getViewport().isEmpty());
    const bool bTimeUsed(0.0 < rViewInformation2D.getViewTime());
    const bool bVisualizedPageUsed(rViewInformation2D.getVisualizedPage().is());
    const bool bReducedDisplayQualityUsed(rViewInformation2D.getReducedDisplayQuality());
    uno::Sequence<beans::PropertyValue> aPropertyValues;

    sal_uInt32 nIndex = 0;

    const sal_uInt32 nCount((bObjectTransformationUsed ? 1 : 0) + (bViewTransformationUsed ? 1 : 0)
                            + (bViewportUsed ? 1 : 0) + (bTimeUsed ? 1 : 0)
                            + (bVisualizedPageUsed ? 1 : 0) + (bReducedDisplayQualityUsed ? 1 : 0));

    aPropertyValues.realloc(nCount);

    if (bObjectTransformationUsed)
    {
        css::geometry::AffineMatrix2D aAffineMatrix2D;
        basegfx::unotools::affineMatrixFromHomMatrix(aAffineMatrix2D,
                                                     rViewInformation2D.getObjectTransformation());
        aPropertyValues[nIndex].Name = g_PropertyName_ObjectTransformation;
        aPropertyValues[nIndex].Value <<= aAffineMatrix2D;
        nIndex++;
    }

    if (bViewTransformationUsed)
    {
        css::geometry::AffineMatrix2D aAffineMatrix2D;
        basegfx::unotools::affineMatrixFromHomMatrix(aAffineMatrix2D,
                                                     rViewInformation2D.getViewTransformation());
        aPropertyValues[nIndex].Name = g_PropertyName_ViewTransformation;
        aPropertyValues[nIndex].Value <<= aAffineMatrix2D;
        nIndex++;
    }

    if (bViewportUsed)
    {
        const css::geometry::RealRectangle2D aViewport(
            basegfx::unotools::rectangle2DFromB2DRectangle(rViewInformation2D.getViewport()));
        aPropertyValues[nIndex].Name = g_PropertyName_Viewport;
        aPropertyValues[nIndex].Value <<= aViewport;
        nIndex++;
    }

    if (bTimeUsed)
    {
        aPropertyValues[nIndex].Name = g_PropertyName_Time;
        aPropertyValues[nIndex].Value <<= rViewInformation2D.getViewTime();
        nIndex++;
    }

    if (bVisualizedPageUsed)
    {
        aPropertyValues[nIndex].Name = g_PropertyName_VisualizedPage;
        aPropertyValues[nIndex].Value <<= rViewInformation2D.getVisualizedPage();
        nIndex++;
    }

    return aPropertyValues;
}

} // end of namespace drawinglayer::geometry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
