/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/text/xtextrange.hxx>
#include <rtl/string.hxx>

#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;

namespace apitest
{
void XTextRange::testGetEnd()
{
    uno::Reference<text::XTextRange> xTextRange(init(), uno::UNO_QUERY_THROW);

    xTextRange->setString("UnitTest");
    uno::Reference<text::XTextRange> xTR_end(xTextRange->getEnd(), uno::UNO_QUERY_THROW);
    xTR_end->setString("End");

    CPPUNIT_ASSERT(xTextRange->getText()->getString().endsWith("End"));
}

void XTextRange::testGetSetString()
{
    uno::Reference<text::XTextRange> xTextRange(init(), uno::UNO_QUERY_THROW);

    xTextRange->setString("UnitTest");
    CPPUNIT_ASSERT_EQUAL(OUString("UnitTest"), xTextRange->getString());
}

void XTextRange::testGetStart()
{
    uno::Reference<text::XTextRange> xTextRange(init(), uno::UNO_QUERY_THROW);

    xTextRange->setString("UnitTest");
    uno::Reference<text::XTextRange> xTR_start(xTextRange->getStart(), uno::UNO_QUERY_THROW);
    xTR_start->setString("Start");

    CPPUNIT_ASSERT(xTextRange->getText()->getString().startsWith("Start"));
}

void XTextRange::testGetText()
{
    uno::Reference<text::XTextRange> xTextRange(init(), uno::UNO_QUERY_THROW);

    xTextRange->setString("UnitTest");
    uno::Reference<text::XText> xText(xTextRange->getText(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("UnitTest"), xTextRange->getString());
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
