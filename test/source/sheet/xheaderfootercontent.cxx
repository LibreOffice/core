/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xheaderfootercontent.hxx>

#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XHeaderFooterContent::testGetCenterText()
{
    uno::Reference<sheet::XHeaderFooterContent> xHFC(init(), UNO_QUERY_THROW);

    uno::Reference<text::XText> xText(xHFC->getCenterText(), UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get center text", u"CENTER"_ustr, xText->getString());
}

void XHeaderFooterContent::testGetLeftText()
{
    uno::Reference<sheet::XHeaderFooterContent> xHFC(init(), UNO_QUERY_THROW);

    uno::Reference<text::XText> xText(xHFC->getLeftText(), UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get left text", u"LEFT"_ustr, xText->getString());
}
void XHeaderFooterContent::testGetRightText()
{
    uno::Reference<sheet::XHeaderFooterContent> xHFC(init(), UNO_QUERY_THROW);

    uno::Reference<text::XText> xText(xHFC->getRightText(), UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get right text", u"RIGHT"_ustr, xText->getString());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
