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

#include <sal/types.h>
#include <tools/long.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <unoview.hxx>
#include <unoviewcontainer.hxx>
#include "tests.hxx"
#include <../engine/slide/layermanager.hxx>
#include <../engine/slide/layer.hxx>

namespace target = slideshow::internal;
using namespace ::com::sun::star;

namespace
{

class LayerManagerTest : public CppUnit::TestFixture
{
    target::UnoViewContainer      maViews;
    target::LayerManagerSharedPtr mpLayerManager;
    TestViewSharedPtr             mpTestView;
    TestShapeSharedPtr            mpTestShape;

public:
    void setUp() override
    {
        mpTestShape = createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            1.0);
        mpTestView = createTestView();
        maViews.addView( mpTestView );

        mpLayerManager =
            std::make_shared<target::LayerManager>(
                maViews,
                false );
    }

    void tearDown() override
    {
        mpLayerManager.reset();
        maViews.dispose();
    }

    void testLayer()
    {
        target::LayerSharedPtr pBgLayer(
            target::Layer::createBackgroundLayer() );
        pBgLayer->addView( mpTestView );

        target::LayerSharedPtr pFgLayer(
            target::Layer::createLayer() );
        pFgLayer->addView( mpTestView );

        CPPUNIT_ASSERT_MESSAGE( "BG layer must confess that!",
                                pBgLayer->isBackgroundLayer() );
        CPPUNIT_ASSERT_MESSAGE( "FG layer lies!",
                                !pFgLayer->isBackgroundLayer() );

        CPPUNIT_ASSERT_MESSAGE( "BG layer must not have pending updates!",
                                !pBgLayer->isUpdatePending() );
        pBgLayer->addUpdateRange( basegfx::B2DRange(0,0,10,10) );
        CPPUNIT_ASSERT_MESSAGE( "BG layer must have pending updates!",
                                pBgLayer->isUpdatePending() );

        TestShapeSharedPtr pTestShape = createTestShape(
            basegfx::B2DRange(0.0,0.0,1000.0,1000.0),
            1.0);
        pBgLayer->updateBounds( pTestShape );
        CPPUNIT_ASSERT_MESSAGE( "BG layer must not resize!",
                                !pBgLayer->commitBounds() );

        TestShapeSharedPtr pTestShape2 = createTestShape(
            basegfx::B2DRange(0.0,0.0,1.0,1.0),
            1.0);
        pFgLayer->updateBounds( pTestShape2 );
        CPPUNIT_ASSERT_MESSAGE( "FG layer must resize!",
                                pFgLayer->commitBounds() );
    }

    void testBasics()
    {
        mpLayerManager->activate();

        CPPUNIT_ASSERT_MESSAGE( "Un-added shape must have zero view layers",
                                mpTestShape->getViewLayers().empty() );
        mpLayerManager->addShape(mpTestShape);
        CPPUNIT_ASSERT_MESSAGE( "Adding a shape requires a LayerManager update",
                                mpLayerManager->isUpdatePending() );

        // update does the delayed viewAdded call to the shape
        CPPUNIT_ASSERT_MESSAGE( "Update failed on LayerManager",
                                mpLayerManager->update() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Added shape must have one view layer",
                                      size_t(1), mpTestShape->getViewLayers().size() );
        CPPUNIT_ASSERT_MESSAGE( "Shape must been rendered",
                                mpTestShape->getNumRenders() );
        CPPUNIT_ASSERT_MESSAGE( "Shape must not been updated",
                                !mpTestShape->getNumUpdates() );

        // test second view, check whether shape gets additional view
        TestViewSharedPtr pTestView( createTestView() );
        CPPUNIT_ASSERT_MESSAGE( "Adding second View failed",
                                maViews.addView( pTestView ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "View container must have two views",
                                      std::ptrdiff_t(2),
                                      maViews.end() - maViews.begin() );
        mpLayerManager->viewAdded(pTestView);
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Added shape must have two view layers",
                                      size_t(2),
                                      mpTestShape->getViewLayers().size() );

        mpLayerManager->deactivate();
    }

    void testShapeOrdering()
    {
        TestShapeSharedPtr pShape2( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            2.0));
        TestShapeSharedPtr pShape3( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            3.0));
        TestShapeSharedPtr pShape4( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            4.0));

        mpLayerManager->addShape(mpTestShape);
        mpLayerManager->addShape(pShape2);
        mpLayerManager->addShape(pShape3);
        mpLayerManager->addShape(pShape4);

        mpLayerManager->activate();

        // update does the delayed viewAdded call to the shape
        CPPUNIT_ASSERT_MESSAGE( "Update failed on LayerManager",
                                mpLayerManager->update() );
        CPPUNIT_ASSERT_MESSAGE( "View must have background layer only",
                                mpTestView->getViewLayers().empty() );

        // LayerManager must now generate one extra view layer
        mpLayerManager->enterAnimationMode(pShape2);
        CPPUNIT_ASSERT_MESSAGE( "No update pending on LayerManager",
                                mpLayerManager->isUpdatePending() );
        CPPUNIT_ASSERT_MESSAGE( "Update failed on LayerManager",
                                mpLayerManager->update() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "View must have one extra layer only",
                                      size_t(1), mpTestView->getViewLayers().size() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "View layer must have 10x10 size",
                                      basegfx::B2DRange(0.0,0.0,10.0,10.0),
                                      mpTestView->getViewLayers().at(0)->getBounds() );

        // LayerManager must now remove the extra view layer
        mpLayerManager->leaveAnimationMode(pShape2);
        CPPUNIT_ASSERT_MESSAGE( "No update pending on LayerManager",
                                mpLayerManager->isUpdatePending() );
        CPPUNIT_ASSERT_MESSAGE( "Update failed on LayerManager #2",
                                mpLayerManager->update() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Shape 1 must be on background layer",
                                      static_cast<slideshow::internal::ViewLayer*>(mpTestView.get()),
                                      mpTestShape->getViewLayers().at(0).first.get() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Shape 2 must be on background layer",
                                      static_cast<slideshow::internal::ViewLayer*>(mpTestView.get()),
                                      pShape2->getViewLayers().at(0).first.get() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Shape 3 must have one layer",
                                      size_t(1), pShape3->getViewLayers().size() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Shape 3 must be on background layer",
                                      static_cast<slideshow::internal::ViewLayer*>(mpTestView.get()),
                                      pShape3->getViewLayers().at(0).first.get() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Shape 4 must be on background layer",
                                      static_cast<slideshow::internal::ViewLayer*>(mpTestView.get()),
                                      pShape4->getViewLayers().at(0).first.get() );

        // checking deactivation (all layers except background layer
        // must vanish)
        mpLayerManager->enterAnimationMode(pShape3);
        CPPUNIT_ASSERT_MESSAGE( "No update pending on LayerManager",
                                mpLayerManager->isUpdatePending() );
        CPPUNIT_ASSERT_MESSAGE( "Update failed on LayerManager",
                                mpLayerManager->update() );
        CPPUNIT_ASSERT_MESSAGE( "Shape 4 must not be on background layer",
                                pShape4->getViewLayers().at(0).first != mpTestView );
        mpLayerManager->leaveAnimationMode(pShape3);
        CPPUNIT_ASSERT_MESSAGE( "Update failed on LayerManager",
                                mpLayerManager->update() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Shape 4 must be on background layer",
                                      static_cast<slideshow::internal::ViewLayer*>(mpTestView.get()),
                                      pShape4->getViewLayers().at(0).first.get() );

        mpLayerManager->deactivate();
        CPPUNIT_ASSERT_MESSAGE( "Update pending on deactivated LayerManager",
                                !mpLayerManager->isUpdatePending() );
    }

    void testShapeRepaint()
    {
        TestShapeSharedPtr pShape2( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            2.0));
        TestShapeSharedPtr pShape3( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            3.0));
        TestShapeSharedPtr pShape4( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            4.0));
        TestShapeSharedPtr pShape5( createTestShape(
            basegfx::B2DRange(20.0,20.0,30.0,30.0),
            4.0));

        mpLayerManager->addShape(mpTestShape);
        mpLayerManager->addShape(pShape2);
        mpLayerManager->addShape(pShape3);
        mpLayerManager->addShape(pShape4);
        mpLayerManager->addShape(pShape5);

        mpLayerManager->activate();

        mpLayerManager->enterAnimationMode(pShape2);
        mpLayerManager->update();

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "First shape not rendered",
                                      sal_Int32(1), mpTestShape->getNumRenders() );
        // CPPUNIT_ASSERT_MESSAGE( "Second shape not rendered",
        //                         pShape2->getNumRenders() == 1 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Second shape not rendered",
                                      sal_Int32(0), pShape2->getNumRenders() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Third shape not rendered",
                                      sal_Int32(1), pShape3->getNumRenders() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Fourth shape not rendered",
                                      sal_Int32(1), pShape4->getNumRenders() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Fifth shape not rendered",
                                      sal_Int32(1), pShape5->getNumRenders() );

        mpLayerManager->enterAnimationMode(pShape4);
        mpLayerManager->update();

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "First shape not rendered",
                                      sal_Int32(1), mpTestShape->getNumRenders() );
        // CPPUNIT_ASSERT_MESSAGE( "Second shape not rendered",
        //                         pShape2->getNumRenders() == 1 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Second shape not rendered",
                                      sal_Int32(0), pShape2->getNumRenders() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Third shape not rendered",
                                      sal_Int32(2), pShape3->getNumRenders() );
        // interesting - windows does not render this? # == 1...
        // CPPUNIT_ASSERT_EQUAL_MESSAGE( "Fourth shape not rendered",
        //                               sal_Int32(2), pShape4->getNumRenders() );
        // interesting - windows does not render this? # == 1...
        // CPPUNIT_ASSERT_EQUAL_MESSAGE( "Fifth shape not rendered",
        //                              sal_Int32(2), pShape5->getNumRenders() );

        mpLayerManager->leaveAnimationMode(pShape2);
        mpLayerManager->leaveAnimationMode(pShape4);
        mpLayerManager->update();

        // first shape is on slide background, *now* gets rendered
        // CPPUNIT_ASSERT_MESSAGE( "First shape not rendered #2",
        //                         mpTestShape->getNumRenders() == 1 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "First shape not rendered #2",
                                      sal_Int32(2), mpTestShape->getNumRenders() );
        // CPPUNIT_ASSERT_MESSAGE( "Second shape not rendered #2",
        //                         pShape2->getNumRenders() == 2 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Second shape not rendered #2",
                                      sal_Int32(1), pShape2->getNumRenders() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Third shape not rendered #2",
                                      sal_Int32(3), pShape3->getNumRenders() );
        // interesting - windows does not render this? # == 2...
        // CPPUNIT_ASSERT_EQUAL_MESSAGE( "Fourth shape not rendered #2",
        //                               sal_Int32(3), pShape4->getNumRenders() );
        // interesting - windows does not render this? # == 2...
        // CPPUNIT_ASSERT_EQUAL_MESSAGE( "Fifth shape not rendered #2",
        //                               sal_Int32(3), pShape5->getNumRenders() );
    }

    void testRefCounting()
    {
        TestShapeSharedPtr pShape2( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            2.0));
        TestShapeSharedPtr pShape3( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            3.0));
        TestShapeSharedPtr pShape4( createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            4.0));

        mpLayerManager->addShape(mpTestShape);
        mpLayerManager->addShape(pShape2);
        mpLayerManager->addShape(pShape3);
        mpLayerManager->addShape(pShape4);

        mpLayerManager->removeShape(mpTestShape);
        mpLayerManager->removeShape(pShape2);
        mpLayerManager->removeShape(pShape3);
        mpLayerManager->removeShape(pShape4);

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Shape 1 must have refcount of 1",
                                      tools::Long(1), mpTestShape.use_count() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Shape 2 must have refcount of ",
                                      tools::Long(1), pShape2.use_count() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Shape 3 must have refcount of 1",
                                      tools::Long(1), pShape3.use_count() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Shape 4 must have refcount of",
                                      tools::Long(1), pShape4.use_count() );


        mpLayerManager->addShape(mpTestShape);
        mpLayerManager->addShape(pShape2);
        mpLayerManager->addShape(pShape3);
        mpLayerManager->addShape(pShape4);

        mpLayerManager->activate();
        mpLayerManager->update();

        mpLayerManager->removeShape(mpTestShape);
        mpLayerManager->removeShape(pShape2);
        mpLayerManager->removeShape(pShape3);
        mpLayerManager->removeShape(pShape4);

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Shape 1 must have refcount of 1",
                                      tools::Long(1), mpTestShape.use_count() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Shape 2 must have refcount of ",
                                      tools::Long(1), pShape2.use_count() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Shape 3 must have refcount of 1",
                                      tools::Long(1), pShape3.use_count() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Shape 4 must have refcount of 1",
                                      tools::Long(1), pShape4.use_count() );
    }

    // hook up the test
    CPPUNIT_TEST_SUITE(LayerManagerTest);
    CPPUNIT_TEST(testBasics);
    CPPUNIT_TEST(testLayer);
    CPPUNIT_TEST(testShapeOrdering);
    CPPUNIT_TEST(testShapeRepaint);
    CPPUNIT_TEST(testRefCounting);
    CPPUNIT_TEST_SUITE_END();

}; // class LayerManagerTest


CPPUNIT_TEST_SUITE_REGISTRATION(LayerManagerTest);
} // namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
