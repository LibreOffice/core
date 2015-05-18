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
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

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


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(UnoViewContainerTest, "UnoViewContainerTest");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
