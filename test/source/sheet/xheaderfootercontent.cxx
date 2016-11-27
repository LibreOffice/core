/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xheaderfootercontent.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/text/XText.hpp>

#include "cppunit/TestAssert.h"
#include <rtl/ustring.hxx>
#include <iostream>

using namespace css;
using namespace css::uno;

namespace apitest {

void XHeaderFooterContent::testLeftText()
{
    uno::Reference < sheet::XHeaderFooterContent > xHeaderFooterContent(init(), UNO_QUERY_THROW);
    uno::Reference < text::XText > xLeftText = xHeaderFooterContent->getLeftText();
    CPPUNIT_ASSERT(xLeftText->getString().equals("LEFT"));
}

void XHeaderFooterContent::testRightText()
{
    uno::Reference < sheet::XHeaderFooterContent > xHeaderFooterContent(init(), UNO_QUERY_THROW);
    uno::Reference < text::XText > xRightText = xHeaderFooterContent->getRightText();
    CPPUNIT_ASSERT(xRightText->getString().equals("RIGHT"));
}

void XHeaderFooterContent::testCenterText()
{
    uno::Reference < sheet::XHeaderFooterContent > xHeaderFooterContent(init(), UNO_QUERY_THROW);
    uno::Reference < text::XText > xCenterText = xHeaderFooterContent->getCenterText();
    CPPUNIT_ASSERT(xCenterText->getString().equals("CENTER"));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
