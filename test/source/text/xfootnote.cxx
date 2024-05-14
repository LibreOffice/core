/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/text/xfootnote.hxx>

#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XFootnote::testGetSetLabel()
{
    Reference<text::XFootnote> xFootnote(init(), UNO_QUERY_THROW);

    rtl::OUString labelName = u"New XFootnote Label"_ustr;
    xFootnote->setLabel(labelName);
    rtl::OUString newLabel = xFootnote->getLabel();
    CPPUNIT_ASSERT(!newLabel.isEmpty());
    CPPUNIT_ASSERT_EQUAL(labelName, newLabel);
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
