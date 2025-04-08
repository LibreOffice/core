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

#ifndef INCLUDED_SLIDESHOW_TEST_TESTS_HXX
#define INCLUDED_SLIDESHOW_TEST_TESTS_HXX

#include <animatableshape.hxx>
#include <unoview.hxx>
#include <memory>

namespace basegfx{ class B2DRange; }

class TestView : public slideshow::internal::UnoView
{
public:
    virtual basegfx::B2DRange getBounds() const = 0;

    virtual std::vector<std::shared_ptr<TestView> > getViewLayers() const = 0;
};

typedef std::shared_ptr<TestView> TestViewSharedPtr;
TestViewSharedPtr createTestView();


class TestShape : public slideshow::internal::AnimatableShape
{
public:
    virtual std::vector<
    std::pair<slideshow::internal::ViewLayerSharedPtr,bool> > getViewLayers() const = 0;
    virtual sal_Int32 getNumUpdates() const = 0;
    virtual sal_Int32 getNumRenders() const = 0;
};

typedef std::shared_ptr<TestShape> TestShapeSharedPtr;
TestShapeSharedPtr createTestShape(const basegfx::B2DRange& rRect,
                                   double                   nPrio);

#endif // INCLUDED_SLIDESHOW_TEST_TESTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
