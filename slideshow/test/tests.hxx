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

#ifndef INCLUDED_TESTS_HXX
#define INCLUDED_TESTS_HXX

#include "animatableshape.hxx"
#include "unoview.hxx"
#include <boost/shared_ptr.hpp>

namespace basegfx{ class B1DRange; class B2DRange; class B2DVector; }

class TestView : public slideshow::internal::UnoView
{
public:
    /// true iff clear() has been called
    virtual bool isClearCalled() const = 0;
    virtual std::vector<std::pair<basegfx::B2DVector,double> > getCreatedSprites() const = 0;
    virtual basegfx::B1DRange getPriority() const = 0;
    /// true iff setClip was called (on and off)
    virtual bool wasClipSet() const = 0;
    virtual basegfx::B2DRange getBounds() const = 0;

    virtual std::vector<boost::shared_ptr<TestView> > getViewLayers() const = 0;
};

typedef boost::shared_ptr<TestView> TestViewSharedPtr;
TestViewSharedPtr createTestView();


///////////////////////////////////////////////////////////////////////////////////////


class TestShape : public slideshow::internal::AnimatableShape
{
public:
    virtual std::vector<
    std::pair<slideshow::internal::ViewLayerSharedPtr,bool> > getViewLayers() const = 0;
    virtual sal_Int32 getNumUpdates() const = 0;
    virtual sal_Int32 getNumRenders() const = 0;
    virtual sal_Int32 getAnimationCount() const = 0;
};

typedef boost::shared_ptr<TestShape> TestShapeSharedPtr;
TestShapeSharedPtr createTestShape(const basegfx::B2DRange& rRect,
                                   double                   nPrio);

#endif /* INCLUDED_TESTS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
