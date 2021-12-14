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

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <drawinglayer/primitive2d/Primitive2DVisitor.hxx>

#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/util/XAccounting.hpp>
#include <basegfx/range/b2drange.hxx>
#include <com/sun/star/graphic/XPrimitive2D.hpp>
#include <mutex>

namespace drawinglayer::geometry
{
class ViewInformation2D;
}

/** This is a custom re-implementation of cppu::WeakComponentImplHelper which uses
   std::mutex and skips parts of the XComponent stuff.
*/
class DRAWINGLAYER_DLLPUBLIC BasePrimitive2DImplBase : public cppu::OWeakObject,
                                                       public css::lang::XComponent,
                                                       public css::lang::XTypeProvider,
                                                       public css::graphic::XPrimitive2D,
                                                       public css::util::XAccounting
{
public:
    virtual ~BasePrimitive2DImplBase() override;

    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;
    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const& aType) override;

    // css::lang::XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL
    addEventListener(css::uno::Reference<css::lang::XEventListener> const& xListener) override;
    virtual void SAL_CALL
    removeEventListener(css::uno::Reference<css::lang::XEventListener> const& xListener) override;

    // css::lang::XTypeProvider
    virtual css::uno::Sequence<css::uno::Type> SAL_CALL getTypes() override;
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId() override
    {
        return css::uno::Sequence<sal_Int8>();
    }

protected:
    mutable std::mutex m_aMutex;
};

namespace drawinglayer::primitive2d
{
/** BasePrimitive2D class

    Baseclass for all C++ implementations of css::graphic::XPrimitive2D

    This class is strongly virtual due to the lack of getPrimitiveID() implementation.
    This is by purpose, this base class shall not be incarnated and be used directly as
    a XPrimitive2D.

    It is noncopyable to make clear that a primitive is a read-only
    instance and copying or changing values is not intended. The idea is to hold all data
    needed for visualisation of this primitive in unchangeable form.

    It is derived from cppu::BaseMutex to have a Mutex at hand; in a base
    implementation this may not be needed, but e.g. when buffering at last decomposition
    in a local member, multiple threads may try to decompose at the same time, so locking
    is needed to avoid race conditions seen from the UNO object implementation.

    A method to get a simplified representation is provided by get2DDecomposition. The
    default implementation returns an empty sequence. The idea is that processors
    using this primitive and do not know it, may get the decomposition and process
    these instead. An example is e.g. a fat line, who's decomposition may contain
    the geometric representation of that line using filled polygon primitives. When
    the renderer knows how to handle fat lines, he may process this primitive directly;
    if not he can use the decomposition. With this functionality, renderers may operate by
    knowing only a small set of primitives.

    When a primitive does not implement get2DDecomposition, it is called a 'Basic Primitive' and
    belongs to the set of primitives which a processor should be able to handle. Practice
    will define this minimal sets of primitives. When defined and the concept is proved,
    unique UNO APIs may be defined/implemented for these set to allow more intense work
    with primitives using UNO.

    Current Basic 2D Primitives are:

    - BitmapPrimitive2D (bitmap data, evtl. with transparence)
    - PointArrayPrimitive2D (single points)
    - PolygonHairlinePrimitive2D (hairline curves/polygons)
    - PolyPolygonColorPrimitive2D (colored polygons)

    UPDATE: MetafilePrimitive2D (VCL Metafile) is taken off this list since
    it is implemented with the integration of CWS aw078 into DV300m69.

    All other implemented primitives have a defined decomposition and can thus be
    decomposed down to this small set.

    A renderer implementing support for this minimal set of primitives can completely
    render primitive-based visualisations. Of course, he also has to take states into account
    which are represented by GroupPrimitive2D derivations, see groupprimitive2d.hxx

    To support getting the geometric BoundRect, getB2DRange is used. The default
    implementation will use the get2DDecomposition result and merge a range from the
    entries. Thus, an implementation is only necessary for the Basic Primitives, but
    of course speedups are possible (and are used) by implementing the method at higher-level
    primitives.

    For primitive identification, getPrimitiveID is used currently in this implementations
    to allow a fast switch/case processing. This needs a unique identifier mechanism which
    currently uses defines (see drawinglayer_primitivetypes2d.hxx). For UNO primitive API
    it will be needed to add a unique descriptor (Name?) later to the API.

    This base implementation provides mappings from the methods from XPrimitive2D
    (getDecomposition/getRange) to the appropriate methods in the C++ implementations
    (get2DDecomposition/getB2DRange). The PropertyValue ViewParameters is converted to
    the appropriate C++ implementation class ViewInformation2D.

    This base class does not implement any buffering; e.g. buffering the decomposition
    and/or the range. These may be buffered anytime since the definition is that the primitive
    is read-only and thus unchangeable. This implies that the decomposition and/or getting
    the range will lead to the same result as last time, under the precondition that
    the parameter ViewInformation2D is the same as the last one. This is usually the case
    for view-independent primitives which are defined by not using ViewInformation2D
    in their get2DDecomposition/getB2DRange implementations.
*/
class DRAWINGLAYER_DLLPUBLIC BasePrimitive2D : public BasePrimitive2DImplBase
{
    BasePrimitive2D(const BasePrimitive2D&) = delete;
    BasePrimitive2D& operator=(const BasePrimitive2D&) = delete;

public:
    // constructor/destructor
    BasePrimitive2D();
    virtual ~BasePrimitive2D() override;

    /** the ==operator is mainly needed to allow testing newly-created primitives against their last
        incarnation which buffers/holds the made decompositions. The default implementation
        uses getPrimitive2DID()-calls to test if it's the same ID at last.
        Overridden implementations are then based on this implementation
     */
    virtual bool operator==(const BasePrimitive2D& rPrimitive) const;
    bool operator!=(const BasePrimitive2D& rPrimitive) const { return !operator==(rPrimitive); }

    /// The default implementation will use getDecomposition results to create the range
    virtual basegfx::B2DRange
    getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

    /** provide unique ID for fast identifying of known primitive implementations in renderers. These use
        the defines from drawinglayer_primitivetypes2d.hxx to define unique IDs.
     */
    virtual sal_uInt32 getPrimitive2DID() const = 0;

    /// The default implementation will return an empty sequence
    virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor,
                                    const geometry::ViewInformation2D& rViewInformation) const;

    // Methods from XPrimitive2D

    /** The getDecomposition implementation for UNO API will use getDecomposition from this implementation. It
        will construct a ViewInformation2D from the ViewParameters for that purpose
     */
    virtual css::uno::Sequence<::css::uno::Reference<::css::graphic::XPrimitive2D>> SAL_CALL
    getDecomposition(const css::uno::Sequence<css::beans::PropertyValue>& rViewParameters) override;

    /** The getRange implementation for UNO API will use getRange from this implementation. It
        will construct a ViewInformation2D from the ViewParameters for that purpose
     */
    virtual css::geometry::RealRectangle2D SAL_CALL
    getRange(const css::uno::Sequence<css::beans::PropertyValue>& rViewParameters) override;

    // XAccounting
    virtual sal_Int64 SAL_CALL estimateUsage() override;
};

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
