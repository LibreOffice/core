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
#include "gtest/gtest.h"
#include "com/sun/star/presentation/XSlideShowView.hpp"

namespace target = slideshow::internal;
using namespace ::com::sun::star;

namespace
{

class UnoViewContainerTest : public ::testing::Test
{
public:
}; // class UnoViewContainerTest

TEST_F(UnoViewContainerTest, testContainer)
{
    target::UnoViewContainer aContainer;

    TestViewSharedPtr pView = createTestView();
    aContainer.addView( pView );

    ASSERT_TRUE( 1 == std::distance( aContainer.begin(), aContainer.end() )) << "Testing container size";
    ASSERT_TRUE( pView->paintScreen() ) << "Testing disposedness";
    aContainer.dispose();
    ASSERT_TRUE( 0 == std::distance( aContainer.begin(), aContainer.end() )) << "Testing dispose: container must be empty";
    ASSERT_TRUE( !pView->paintScreen() ) << "Testing dispose: all elements must receive dispose";
}


} // namespace
