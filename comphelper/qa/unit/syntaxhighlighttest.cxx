/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/syntaxhighlight.hxx>
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include "rtl/ustring.hxx"

#include <vector>

class SyntaxHighlightTest : public CppUnit::TestFixture
{
public:
    void testBasicString();

    CPPUNIT_TEST_SUITE(SyntaxHighlightTest);
    CPPUNIT_TEST(testBasicString);
    CPPUNIT_TEST_SUITE_END();
};

void SyntaxHighlightTest::testBasicString()
{
    OUString aBasicString("        if Mid(sText,iRun,1 )<> \" \" then Mid( sText ,iRun, 1, Chr( 1 + Asc( Mid(sText,iRun,1 )) ) '");

    SyntaxHighlighter aHighlighter;
    aHighlighter.initialize( HIGHLIGHT_BASIC );

    std::vector<HighlightPortion> aPortions;
    aHighlighter.getHighlightPortions( 0, aBasicString, aPortions );

    sal_Int32 prevEnd = 0;
    for(std::vector<HighlightPortion>::const_iterator itr =
            aPortions.begin(), itrEnd = aPortions.end(); itr != itrEnd; ++itr)
    {
        CPPUNIT_ASSERT_EQUAL(prevEnd, itr->nBegin);
        CPPUNIT_ASSERT(itr->nBegin < itr->nEnd);
        prevEnd = itr->nEnd;
    }
    CPPUNIT_ASSERT_EQUAL(aBasicString.getLength(), prevEnd);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SyntaxHighlightTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
