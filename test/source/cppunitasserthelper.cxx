/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/cppunitasserthelper.hxx>

CPPUNIT_NS_BEGIN

void AssertRectEqualWithTolerance(std::string_view sInfo, const tools::Rectangle& rExpected,
                                  const tools::Rectangle& rActual, const sal_Int32 nTolerance)
{
    // Left
    OString sMsg = OString::Concat(sInfo) + " Left expected " + OString::number(rExpected.Left())
                   + " actual " + OString::number(rActual.Left()) + " Tolerance "
                   + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(),
                           std::abs(rExpected.Left() - rActual.Left()) <= nTolerance);

    // Top
    sMsg = OString::Concat(sInfo) + " Top expected " + OString::number(rExpected.Top()) + " actual "
           + OString::number(rActual.Top()) + " Tolerance " + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(), std::abs(rExpected.Top() - rActual.Top()) <= nTolerance);

    // Width
    sMsg = OString::Concat(sInfo) + " Width expected " + OString::number(rExpected.GetWidth())
           + " actual " + OString::number(rActual.GetWidth()) + " Tolerance "
           + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(),
                           std::abs(rExpected.GetWidth() - rActual.GetWidth()) <= nTolerance);

    // Height
    sMsg = OString::Concat(sInfo) + " Height expected " + OString::number(rExpected.GetHeight())
           + " actual " + OString::number(rActual.GetHeight()) + " Tolerance "
           + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(),
                           std::abs(rExpected.GetHeight() - rActual.GetHeight()) <= nTolerance);
}

void AssertPointEqualWithTolerance(std::string_view sInfo, const Point rExpected,
                                   const Point rActual, const sal_Int32 nTolerance)
{
    // X
    OString sMsg = OString::Concat(sInfo) + " X expected " + OString::number(rExpected.X())
                   + " actual " + OString::number(rActual.X()) + " Tolerance "
                   + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(), std::abs(rExpected.X() - rActual.X()) <= nTolerance);
    // Y
    sMsg = OString::Concat(sInfo) + " Y expected " + OString::number(rExpected.Y()) + " actual "
           + OString::number(rActual.Y()) + " Tolerance " + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(), std::abs(rExpected.Y() - rActual.Y()) <= nTolerance);
}

CPPUNIT_NS_END

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
