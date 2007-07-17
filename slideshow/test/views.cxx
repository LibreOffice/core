/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: views.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:22:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <cppunit/simpleheader.hxx>
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

