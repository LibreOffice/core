/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <test/bootstrapfixture.hxx>

#include <osl/file.hxx>
#include <osl/process.h>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/canvastools.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

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
    Window* pWin = new WorkWindow( (Window *)NULL );
    CPPUNIT_ASSERT( pWin != NULL );

    uno::Reference<rendering::XCanvas> xCanvas = pWin->GetCanvas ();
    if( !xCanvas.is() )
        return; 

    
    Size aSize (1, 1);
    uno::Reference<rendering::XBitmap> xBitmap;
    xBitmap = xCanvas->getDevice ()->createCompatibleAlphaBitmap(
                        vcl::unotools::integerSize2DFromSize( aSize ) );
    CPPUNIT_ASSERT( xBitmap.is() );

    uno::Reference< rendering::XBitmapCanvas > xBitmapCanvas( xBitmap, uno::UNO_QUERY );
    CPPUNIT_ASSERT( xBitmapCanvas.is() );

    BitmapEx aBitmapEx;
    {
        
        xBitmapCanvas->clear();
        CPPUNIT_ASSERT( aBitmapEx.Create( xBitmapCanvas, aSize ) );
        CPPUNIT_ASSERT( aBitmapEx.IsAlpha() );
        CPPUNIT_ASSERT( aBitmapEx.GetAlpha() );
    }

    {
        
        rendering::RenderState aDefaultState;
        uno::Sequence< double > aRedTransparent( 4 );
        aRedTransparent[0] = 1.0; 
        aRedTransparent[1] = 0.0; 
        aRedTransparent[2] = 0.0; 
        aRedTransparent[3] = 0.5; 
        aDefaultState.DeviceColor = aRedTransparent;
#if 0
        
        
        
        XCachedPrimitive    fillPolyPolygon( [in] XPolyPolygon2D xPolyPolygon, [in] ViewState aViewState, [in] RenderState aRenderState )
#endif
    }

#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(CanvasTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
