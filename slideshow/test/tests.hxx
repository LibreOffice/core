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
