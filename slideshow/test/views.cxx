/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: views.cxx,v $
 * $Revision: 1.4 $
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

#include <testshl/simpleheader.hxx>
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
#include "com/sun/star/presentation/XSlideShowView.hpp"

namespace target = slideshow::internal;
using namespace ::com::sun::star;

namespace
{

class UnoViewContainerTest : public CppUnit::TestFixture
{
public:
    void testContainer()
    {
        target::UnoViewContainer aContainer;

        TestViewSharedPtr pView = createTestView();
        aContainer.addView( pView );

        CPPUNIT_ASSERT_MESSAGE( "Testing container size",
                                1 == std::distance( aContainer.begin(),
                                                    aContainer.end() ));
        CPPUNIT_ASSERT_MESSAGE( "Testing disposedness",
                                pView->paintScreen() );
        aContainer.dispose();
        CPPUNIT_ASSERT_MESSAGE( "Testing dispose: container must be empty",
                                0 == std::distance( aContainer.begin(),
                                                    aContainer.end() ));
        CPPUNIT_ASSERT_MESSAGE( "Testing dispose: all elements must receive dispose",
                                !pView->paintScreen() );
    }

    // hook up the test
    CPPUNIT_TEST_SUITE(UnoViewContainerTest);
    CPPUNIT_TEST(testContainer);
    //CPPUNIT_TEST(testLayerManager);
    CPPUNIT_TEST_SUITE_END();

}; // class UnoViewContainerTest

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(UnoViewContainerTest, "UnoViewContainerTest");
} // namespace


// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
NOADDITIONAL;

