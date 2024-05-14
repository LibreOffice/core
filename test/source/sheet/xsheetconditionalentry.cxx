/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetconditionalentry.hxx>

#include <com/sun/star/sheet/XSheetConditionalEntry.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XSheetConditionalEntry::testGetSetStyleName()
{
    uno::Reference<sheet::XSheetConditionalEntry> xSheetConditionalEntry(init(), UNO_QUERY_THROW);
    OUString aStyleName = xSheetConditionalEntry->getStyleName();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get StyleName", u"Result2"_ustr, aStyleName);

    xSheetConditionalEntry->setStyleName(u"Heading"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Unable to set StyleName",
                           aStyleName != xSheetConditionalEntry->getStyleName());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
