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

#include <test/bootstrapfixture.hxx>
#include <vcl/canvastools.hxx>

using namespace ::com::sun::star;
using namespace vcl::unotools;

namespace
{
class CanvasToolsTest : public CppUnit::TestFixture
{
public:
    void testB2IRectangleFromClosedRectangle();

    CPPUNIT_TEST_SUITE(CanvasToolsTest);
    CPPUNIT_TEST(testB2IRectangleFromClosedRectangle);
    CPPUNIT_TEST_SUITE_END();
};

void CanvasToolsTest::testB2IRectangleFromClosedRectangle()
{
    // tdf#124987: Characterization test for a use that was causing an off-by-one
    // error when clipping draw objects to the page containing the anchor

    tools::Rectangle aRect{ /*nLeft*/ 100, /*nTop*/ 100, /*nRight*/ 200, /*nBottom*/ 300 };

    // With the original code, the simulated RTL anchor would be considered off-screen
    auto aBadPageFrame = b2IRectangleFromRectangle(aRect);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(200), aBadPageFrame.getMaxX());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(300), aBadPageFrame.getMaxY());

    // With the new code, the simulated RTL anchor would be considered on-screen
    auto aPageFrame = b2IRectangleFromClosedRectangle(aRect);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(201), aPageFrame.getMaxX());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(301), aPageFrame.getMaxY());
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(CanvasToolsTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
