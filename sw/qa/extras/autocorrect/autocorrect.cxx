/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <docsh.hxx>
#include <unotxdoc.hxx>

// If you want to add a test for a language that doesn't exists yet
// copy an existing document and adapt "fo:language" and "fo:country"

class SwAutoCorrect : public SwModelTestBase
{
public:
    SwAutoCorrect()
        : SwModelTestBase(u"/sw/qa/extras/autocorrect/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwAutoCorrect, nl_BE)
{
    createSwDoc("nl-BE.fodt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    // tdf#92029: Without the fix in place, this test would have failed with
    // - Expected: Ik ben ’s morgens opgestaan
    // - Actual  : Ik ben ‘s morgens opgestaan
    emulateTyping(*pTextDoc, u"Ik ben 's morgens opgestaan");
    OUString sReplaced(u"Ik ben ’s morgens opgestaan"_ustr);
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
