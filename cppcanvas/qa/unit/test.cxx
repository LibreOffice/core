/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <test/bootstrapfixture.hxx>

#include <vcl/wrkwin.hxx>
#include <vcl/canvastools.hxx>

#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>

using namespace ::com::sun::star;

class CanvasTest : public test::BootstrapFixture
{
public:
    CanvasTest() : BootstrapFixture(true, false) {}

    void testComposite();

    CPPUNIT_TEST_SUITE(CanvasTest);
    CPPUNIT_TEST(testComposite);
    CPPUNIT_TEST_SUITE_END();
};

void CanvasTest::testComposite()
{
#ifdef LINUX
    ScopedVclPtrInstance<WorkWindow> pWin( nullptr, WB_STDWORK );

    uno::Reference<rendering::XCanvas> xCanvas = pWin->GetCanvas ();
    if( !xCanvas.is() )
        return; // can't get a canvas working at all - truly headless ?

    // a huge canvas ...
    Size aSize (1, 1);
    uno::Reference<rendering::XBitmap> xBitmap = xCanvas->getDevice ()->createCompatibleAlphaBitmap(
                        vcl::unotools::integerSize2DFromSize( aSize ) );
    CPPUNIT_ASSERT( xBitmap.is() );

    uno::Reference< rendering::XBitmapCanvas > xBitmapCanvas( xBitmap, uno::UNO_QUERY );
    CPPUNIT_ASSERT( xBitmapCanvas.is() );

    BitmapEx aBitmapEx;
    {
        // clear the canvas and basic sanity check ...
        xBitmapCanvas->clear();
        CPPUNIT_ASSERT( aBitmapEx.Create( xBitmapCanvas, aSize ) );
        CPPUNIT_ASSERT( aBitmapEx.IsAlpha() );
        CPPUNIT_ASSERT( !aBitmapEx.GetAlpha().IsEmpty() );
    }

    {
        // render something
        rendering::RenderState aDefaultState;
        uno::Sequence< double > aRedTransparent( 4 );
        aRedTransparent[0] = 1.0; // R
        aRedTransparent[1] = 0.0; // G
        aRedTransparent[2] = 0.0; // B
        aRedTransparent[3] = 0.5; // A
        aDefaultState.DeviceColor = aRedTransparent;
#if 0
        // words fail me to describe the sheer beauty of allocating a UNO
        // object to represent a polygon, and manually handling the ViewState
        // and there being no public helper for this - to render ... a rectangle.
        XCachedPrimitive    fillPolyPolygon( [in] XPolyPolygon2D xPolyPolygon, [in] ViewState aViewState, [in] RenderState aRenderState )
#endif
    }

#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(CanvasTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
