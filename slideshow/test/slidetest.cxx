/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <cppuhelper/compbase1.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <cppcanvas/spritecanvas.hxx>

#include "view.hxx"
#include "unoview.hxx"
#include "unoviewcontainer.hxx"
#include "shape.hxx"
#include "tests.hxx"
#include "../engine/slide/layermanager.hxx"
#include "../engine/slide/layer.hxx"
#include "gtest/gtest.h"
#include "com/sun/star/presentation/XSlideShowView.hpp"

namespace target = slideshow::internal;
using namespace ::com::sun::star;

// FIXME:
#define RUN_OLD_FAILING_TESTS 0

namespace
{

class LayerManagerTest : public ::testing::Test
{
protected:
    target::UnoViewContainer      maViews;
    target::LayerManagerSharedPtr mpLayerManager;
    TestViewSharedPtr             mpTestView;
    TestShapeSharedPtr            mpTestShape;

public:
    virtual void SetUp()
    {
        mpTestShape = createTestShape(
            basegfx::B2DRange(0.0,0.0,10.0,10.0),
            1.0);
        mpTestView = createTestView();
        maViews.addView( mpTestView );

        mpLayerManager.reset(
            new target::LayerManager(
                maViews,
                basegfx::B2DRange(0.0,0.0,100.0,100.0),
                false ));
    }

    virtual void TearDown()
    {
        mpLayerManager.reset();
        maViews.dispose();
    }
}; // class LayerManagerTest


TEST_F(LayerManagerTest, testLayer)
{
    target::LayerSharedPtr pBgLayer(
        target::Layer::createBackgroundLayer( basegfx::B2DRange(0,0,100,100) ) );
    pBgLayer->addView( mpTestView );

    target::LayerSharedPtr pFgLayer(
        target::Layer::createLayer( basegfx::B2DRange(0,0,100,100) ) );
    pFgLayer->addView( mpTestView );

    ASSERT_TRUE( pBgLayer->isBackgroundLayer() ) << "BG layer must confess that!";
    ASSERT_TRUE( !pFgLayer->isBackgroundLayer() ) << "FG layer lies!";

    ASSERT_TRUE( !pBgLayer->isUpdatePending() ) << "BG layer must not have pending updates!";
    pBgLayer->addUpdateRange( basegfx::B2DRange(0,0,10,10) );
    ASSERT_TRUE( pBgLayer->isUpdatePending() ) << "BG layer must have pending updates!";

    TestShapeSharedPtr pTestShape = createTestShape(
        basegfx::B2DRange(0.0,0.0,1000.0,1000.0),
        1.0);
    pBgLayer->updateBounds( pTestShape );
    ASSERT_TRUE( !pBgLayer->commitBounds() ) << "BG layer must not resize!";

    TestShapeSharedPtr pTestShape2 = createTestShape(
        basegfx::B2DRange(0.0,0.0,1.0,1.0),
        1.0);
    pFgLayer->updateBounds( pTestShape2 );
    ASSERT_TRUE( pFgLayer->commitBounds() ) << "FG layer must resize!";
}

TEST_F(LayerManagerTest, testBasics)
{
    mpLayerManager->activate( false );

    ASSERT_TRUE( mpTestShape->getViewLayers().empty() ) << "Un-added shape must have zero view layers";
    mpLayerManager->addShape(mpTestShape);
    ASSERT_TRUE( mpLayerManager->isUpdatePending() ) << "Adding a shape requires a LayerManager update";

    // update does the delayed viewAdded call to the shape
    ASSERT_TRUE( mpLayerManager->update() ) << "Update failed on LayerManager";
    ASSERT_TRUE( mpTestShape->getViewLayers().size() == 1 ) << "Added shape must have one view layer";
    ASSERT_TRUE( mpTestShape->getNumRenders() ) << "Shape must been rendered";
    ASSERT_TRUE( !mpTestShape->getNumUpdates() ) << "Shape must not been updated";

    // test second view, check whether shape gets additional view
    TestViewSharedPtr pTestView( createTestView() );
    ASSERT_TRUE( maViews.addView( pTestView ) ) << "Adding second View failed";
    ASSERT_TRUE( maViews.end() - maViews.begin() == 2 ) << "View container must have two views";
    mpLayerManager->viewAdded(pTestView);
    ASSERT_TRUE( mpTestShape->getViewLayers().size() == 2 ) << "Added shape must have two view layers";

    ASSERT_TRUE( maViews.removeView( pTestView ) ) << "Removing second View failed";
    mpLayerManager->viewRemoved(pTestView);
    ASSERT_TRUE( mpTestShape->getViewLayers().size() == 1 ) << "Added shape must have one view layer";

    mpLayerManager->deactivate();
}

TEST_F(LayerManagerTest, testShapeOrdering)
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

    mpLayerManager->activate( false );

    // update does the delayed viewAdded call to the shape
    ASSERT_TRUE( mpLayerManager->update() ) << "Update failed on LayerManager";
    ASSERT_TRUE( mpTestView->getViewLayers().empty() ) << "View must have background layer only";

    // LayerManager must now generate one extra view layer
    mpLayerManager->enterAnimationMode(pShape2);
    ASSERT_TRUE( mpLayerManager->isUpdatePending() ) << "No update pending on LayerManager";
    ASSERT_TRUE( mpLayerManager->update() ) << "Update failed on LayerManager";
    ASSERT_TRUE( mpTestView->getViewLayers().size() == 1 ) << "View must have one extra layer only";
    ASSERT_TRUE( mpTestView->getViewLayers().at(0)->getBounds() ==
                            basegfx::B2DRange(0.0,0.0,10.0,10.0) ) << "View layer must have 10x10 size";

    // LayerManager must now remove the extra view layer
    mpLayerManager->leaveAnimationMode(pShape2);
    ASSERT_TRUE( mpLayerManager->isUpdatePending() ) << "No update pending on LayerManager";
    ASSERT_TRUE( mpLayerManager->update() ) << "Update failed on LayerManager #2";
    ASSERT_TRUE( mpTestShape->getViewLayers().at(0).first == mpTestView ) << "Shape 1 must be on background layer";
    ASSERT_TRUE( pShape2->getViewLayers().at(0).first == mpTestView ) << "Shape 2 must be on background layer";
    ASSERT_TRUE( pShape3->getViewLayers().size() == 1 ) << "Shape 3 must have one layer";
    ASSERT_TRUE( pShape3->getViewLayers().at(0).first == mpTestView ) << "Shape 3 must be on background layer";
    ASSERT_TRUE( pShape4->getViewLayers().at(0).first == mpTestView ) << "Shape 4 must be on background layer";

    // checking deactivation (all layers except background layer
    // must vanish)
    mpLayerManager->enterAnimationMode(pShape3);
    ASSERT_TRUE( mpLayerManager->isUpdatePending() ) << "No update pending on LayerManager";
    ASSERT_TRUE( mpLayerManager->update() ) << "Update failed on LayerManager";
    ASSERT_TRUE( pShape4->getViewLayers().at(0).first != mpTestView ) << "Shape 4 must not be on background layer";
    mpLayerManager->leaveAnimationMode(pShape3);
    ASSERT_TRUE( mpLayerManager->update() ) << "Update failed on LayerManager";
    ASSERT_TRUE( pShape4->getViewLayers().at(0).first == mpTestView ) << "Shape 4 must be on background layer";

    mpLayerManager->deactivate();
    ASSERT_TRUE( !mpLayerManager->isUpdatePending() ) << "Update pending on deactivated LayerManager";
}

TEST_F(LayerManagerTest, testShapeRepaint)
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
    mpLayerManager->enterAnimationMode(pShape2);
    mpLayerManager->addShape(pShape3);
    mpLayerManager->addShape(pShape4);
    mpLayerManager->addShape(pShape5);

    mpLayerManager->activate( false );
    mpLayerManager->update();

    ASSERT_TRUE( mpTestShape->getNumRenders() == 1 ) << "First shape not rendered";
#if RUN_OLD_FAILING_TESTS
    ASSERT_TRUE( pShape2->getNumRenders() == 1 ) << "Second shape not rendered";
#endif
    ASSERT_TRUE( pShape3->getNumRenders() == 1 ) << "Third shape not rendered";
    ASSERT_TRUE( pShape4->getNumRenders() == 1 ) << "Fourth shape not rendered";
    ASSERT_TRUE( pShape5->getNumRenders() == 1 ) << "Fifth shape not rendered";

    mpLayerManager->enterAnimationMode(pShape4);
    mpLayerManager->update();

    ASSERT_TRUE( mpTestShape->getNumRenders() == 1 ) << "First shape not rendered";
#if RUN_OLD_FAILING_TESTS
    ASSERT_TRUE( pShape2->getNumRenders() == 1 ) << "Second shape not rendered";
#endif
    ASSERT_TRUE( pShape3->getNumRenders() == 2 ) << "Third shape not rendered";
    ASSERT_TRUE( pShape4->getNumRenders() == 2 ) << "Fourth shape not rendered";
    ASSERT_TRUE( pShape5->getNumRenders() == 2 ) << "Fifth shape not rendered";

    mpLayerManager->leaveAnimationMode(pShape2);
    mpLayerManager->leaveAnimationMode(pShape4);
    mpLayerManager->update();

    ASSERT_TRUE( mpTestShape->getNumRenders() == 2 ) << "First shape not rendered #2";
#if RUN_OLD_FAILING_TESTS
    ASSERT_TRUE( pShape2->getNumRenders() == 2 ) << "Second shape not rendered #2"
#endif
    ASSERT_TRUE( pShape3->getNumRenders() == 3 ) << "Third shape not rendered #2";
    ASSERT_TRUE( pShape4->getNumRenders() == 3 ) << "Fourth shape not rendered #2";
    ASSERT_TRUE( pShape5->getNumRenders() == 3 ) << "Fifth shape not rendered #2";
}

TEST_F(LayerManagerTest, testRefCounting)
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

    ASSERT_TRUE( mpTestShape.use_count() == 1 ) << "Shape 1 must have refcount of 1";
    ASSERT_TRUE( pShape2.use_count() == 1 ) << "Shape 2 must have refcount of 1";
    ASSERT_TRUE( pShape3.use_count() == 1 ) << "Shape 3 must have refcount of 1";
    ASSERT_TRUE( pShape4.use_count() == 1 ) << "Shape 4 must have refcount of 1";


    mpLayerManager->addShape(mpTestShape);
    mpLayerManager->addShape(pShape2);
    mpLayerManager->addShape(pShape3);
    mpLayerManager->addShape(pShape4);

    mpLayerManager->activate( false );
    mpLayerManager->update();

    mpLayerManager->removeShape(mpTestShape);
    mpLayerManager->removeShape(pShape2);
    mpLayerManager->removeShape(pShape3);
    mpLayerManager->removeShape(pShape4);

    ASSERT_TRUE( mpTestShape.use_count() == 1 ) << "Shape 1 must have refcount of 1";
    ASSERT_TRUE( pShape2.use_count() == 1 ) << "Shape 2 must have refcount of 1";
    ASSERT_TRUE( pShape3.use_count() == 1 ) << "Shape 3 must have refcount of 1";
    ASSERT_TRUE( pShape4.use_count() == 1 ) << "Shape 4 must have refcount of 1";

    maViews.removeView(mpTestView);
    mpLayerManager->viewRemoved(mpTestView);
    ASSERT_TRUE( mpTestView.use_count() == 1 ) << "View must have refcount of 1";
}


} // namespace


