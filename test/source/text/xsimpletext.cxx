/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/text/xsimpletext.hxx>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XSimpleText::testCreateTextCursor()
{
    uno::Reference<text::XSimpleText> xSimpleText(init(), UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xSimpleText->createTextCursor());
}

void XSimpleText::testCreateTextCursorByRange()
{
    uno::Reference<text::XSimpleText> xSimpleText(init(), UNO_QUERY_THROW);
    uno::Reference<text::XTextCursor> xCursor(xSimpleText->createTextCursor(), UNO_QUERY_THROW);

    xCursor->gotoStart(false);

    CPPUNIT_ASSERT(xSimpleText->createTextCursorByRange(xCursor));
}

void XSimpleText::testInsertString()
{
    uno::Reference<text::XSimpleText> xSimpleText(init(), UNO_QUERY_THROW);
    uno::Reference<text::XTextRange> xCursor(xSimpleText->createTextCursor(), UNO_QUERY_THROW);
    ::rtl::OUString sString = "TestString";

    xSimpleText->insertString(xCursor, sString, false);
    ::rtl::OUString gString = xSimpleText->getText()->getString();

    CPPUNIT_ASSERT(!gString.isEmpty());
    CPPUNIT_ASSERT(gString.indexOf(sString) >= 0);
}

void XSimpleText::testInsertControlCharacter()
{
    bool bOK = true;

    uno::Reference<text::XSimpleText> xSimpleText(init(), UNO_QUERY_THROW);
    uno::Reference<text::XTextRange> xCursor(xSimpleText->createTextCursor(), UNO_QUERY_THROW);

    try
    {
        xSimpleText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK,
                                            false);
        xSimpleText->insertControlCharacter(xCursor, text::ControlCharacter::LINE_BREAK, false);
        xSimpleText->insertString(xSimpleText->createTextCursor(), "newLine", false);
    }
    catch (const lang::IllegalArgumentException&)
    {
        bOK = false;
    }

    OUString gString = xSimpleText->getString();
    CPPUNIT_ASSERT(bOK);
    CPPUNIT_ASSERT(gString.indexOf("\n") > -1);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
