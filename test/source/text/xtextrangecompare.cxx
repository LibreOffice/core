/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/text/xtextrangecompare.hxx>

#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XTextRangeCompare::testCompareRegionStarts()
{
    Reference<text::XTextRangeCompare> xCompare(init(), UNO_QUERY_THROW);
    Reference<text::XText> xText(xCompare, UNO_QUERY_THROW);

    try
    {
        Reference<text::XTextCursor> xCursor1 = xText->createTextCursor();
        xText->insertString(xCursor1, u"XTextRangeCompare"_ustr, false);

        xCursor1->gotoStart(false);
        xCursor1->goRight(5, true);
        Reference<text::XTextCursor> xCursor2 = xText->createTextCursor();
        xCursor2->gotoEnd(false);
        xCursor2->goLeft(7, true);

        CPPUNIT_ASSERT(xCompare->compareRegionStarts(xCursor1, xCursor2));
    }
    catch (lang::IllegalArgumentException&)
    {
    }
}

void XTextRangeCompare::testCompareRegionEnds()
{
    Reference<text::XTextRangeCompare> xCompare(init(), UNO_QUERY_THROW);
    Reference<text::XText> xText(xCompare, UNO_QUERY_THROW);

    try
    {
        Reference<text::XTextCursor> xCursor1 = xText->createTextCursor();
        xText->insertString(xCursor1, u"XTextRangeCompare"_ustr, false);

        xCursor1->gotoStart(false);
        xCursor1->goRight(5, true);
        Reference<text::XTextCursor> xCursor2 = xText->createTextCursor();
        xCursor2->gotoEnd(false);
        xCursor2->goLeft(7, true);

        CPPUNIT_ASSERT(xCompare->compareRegionEnds(xCursor1, xCursor2));
    }
    catch (lang::IllegalArgumentException&)
    {
    }
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
