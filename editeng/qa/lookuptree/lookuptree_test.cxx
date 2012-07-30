/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <editeng/LookupTree.hxx>
#include <editeng/LatinLookupTree.hxx>

namespace {

class LookupTreeTest : public CppUnit::TestFixture
{
    public:
        void test();

    CPPUNIT_TEST_SUITE(LookupTreeTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LookupTreeTest);

void LookupTreeTest::test()
{
    LookupTree* a = new LatinLookupTree( "a" );

    a->insert( OUString("vorschlagnummer1"), 2 );
    a->insert( OUString("vorschlagnummer12") );
    a->insert( OUString("vorschlagnummer2") );

    CPPUNIT_ASSERT_EQUAL( OUString("vorschlagnummer1"), a->suggestAutoCompletion() );

    a->insert( OUString("vorschlagnummer12") );
    CPPUNIT_ASSERT_EQUAL( OUString("vorschlagnummer12"), a->suggestAutoCompletion() );

    a->insert( OUString("vorschlagnummer2") );
    a->insert( OUString("vorschlagnummer2") );
    CPPUNIT_ASSERT_EQUAL( OUString("vorschlagnummer2"), a->suggestAutoCompletion() );

    a->insert( OUString("vorschlag"), 15 );
    CPPUNIT_ASSERT_EQUAL( OUString("vorschlag"), a->suggestAutoCompletion() );

    a->insert( OUString("vorschlagnummer2"), 16 );
    CPPUNIT_ASSERT_EQUAL( OUString("vorschlagnummer2"), a->suggestAutoCompletion() );

    a->remove( OUString("vorschlagnummer2") );
    CPPUNIT_ASSERT_EQUAL( OUString("vorschlag"), a->suggestAutoCompletion() );

    a->insert( OUString("vorschlag20"), 20 );
    CPPUNIT_ASSERT_EQUAL( OUString("vorschlag20"), a->suggestAutoCompletion() );

    a->remove( OUString("vorschlag20") );
    CPPUNIT_ASSERT_EQUAL( OUString("vorschlag"), a->suggestAutoCompletion() );

    a->insert( OUString("vorschlagn"), 14 );
    CPPUNIT_ASSERT_EQUAL( OUString("vorschlag"), a->suggestAutoCompletion() );

    a->remove( OUString("vorschlag") );
    CPPUNIT_ASSERT_EQUAL( OUString("vorschlagn"), a->suggestAutoCompletion() );

    a->remove( OUString("vorschlagn") );
    CPPUNIT_ASSERT_EQUAL( OUString("vorschlagnummer12"), a->suggestAutoCompletion() );

    a->insert( OUString("aber"), 1 );
    CPPUNIT_ASSERT_EQUAL( OUString("vorschlagnummer12"), a->suggestAutoCompletion() );

    a->advance( 'a' );
    CPPUNIT_ASSERT_EQUAL( OUString("ber"), a->suggestAutoCompletion() );

    a->goBack();
    CPPUNIT_ASSERT_EQUAL( OUString("vorschlagnummer12"), a->suggestAutoCompletion() );

    a->insert( OUString("vorschlag"), 15 );
    CPPUNIT_ASSERT_EQUAL( OUString("vorschlag"), a->suggestAutoCompletion() );

    a->insert( OUString("vorschlag13"), 13 );
    CPPUNIT_ASSERT_EQUAL( OUString("vorschlag"), a->suggestAutoCompletion() );

    a->gotoNode( "vorsch" );
    CPPUNIT_ASSERT_EQUAL( OUString("lag"), a->suggestAutoCompletion() );

    a->advance( 'l' );
    CPPUNIT_ASSERT_EQUAL( OUString("ag"), a->suggestAutoCompletion() );

    a->advance( 'a' );
    CPPUNIT_ASSERT_EQUAL( OUString("g13"), a->suggestAutoCompletion() );

    a->advance( 'g' );
    CPPUNIT_ASSERT_EQUAL( OUString("13"), a->suggestAutoCompletion() );

    a->advance( '1' );
    CPPUNIT_ASSERT_EQUAL( OUString("3"), a->suggestAutoCompletion() );

    a->advance( '3' );
    CPPUNIT_ASSERT ( a->suggestAutoCompletion().isEmpty() );

    a->goBack();
    a->advance( 'z' );
    CPPUNIT_ASSERT ( a->suggestAutoCompletion().isEmpty() );

    a->gotoNode( "vorschlag13" );
    CPPUNIT_ASSERT ( a->suggestAutoCompletion().isEmpty() );

    a->advance( 'g' );
    a->advance( '1' );
    a->advance( '3' );
    a->remove( "vorschlag13" );
    CPPUNIT_ASSERT_EQUAL( OUString(""), a->suggestAutoCompletion() );

    a->insert( "VeraHatMichL1eb.", 1000000 );
    a->returnToRoot();
    CPPUNIT_ASSERT_EQUAL( OUString("VeraHatMichL1eb."), a->suggestAutoCompletion() );

    a->gotoNode( "VeraHatMich" );
    a->remove( "VeraHatMichL1eb." );
    CPPUNIT_ASSERT_EQUAL( OUString(""), a->suggestAutoCompletion() );

    a->returnToRoot();
    CPPUNIT_ASSERT_EQUAL( OUString("vorschlag"), a->suggestAutoCompletion() );

    a->gotoNode( "VeraLiebtMich" );
    a->insert( 600 );
    a->returnToRoot();
    CPPUNIT_ASSERT_EQUAL( OUString("VeraLiebtMich"), a->suggestAutoCompletion() );

    a->insert( "VeraHatMichL1eb.", 1000000 );
    a->returnToRoot();
    CPPUNIT_ASSERT_EQUAL( OUString("VeraHatMichL1eb."), a->suggestAutoCompletion() );

    a->gotoNode( "VeraHatMich" );
    a->remove( "VeraHatMichL1eb." );
    CPPUNIT_ASSERT ( a->suggestAutoCompletion().isEmpty() );

    a->advance( 'L' );
    CPPUNIT_ASSERT ( a->suggestAutoCompletion().isEmpty() );

    a->insert( "VeraHatMichL1eb.", 1000000 );
    a->returnToRoot();
    a->gotoNode( "VeraHatMich" );
    a->remove( "VeraHatMichL1eb." );
    CPPUNIT_ASSERT ( a->suggestAutoCompletion().isEmpty() );

    a->goBack();
    CPPUNIT_ASSERT ( a->suggestAutoCompletion().isEmpty() );

    a->insert( "VeraHatMichL1eb.", 1000000 );
    a->returnToRoot();
    a->gotoNode( "VeraHatMich" );
    a->remove( "VeraHatMichL1eb." );
    CPPUNIT_ASSERT ( a->suggestAutoCompletion().isEmpty() );

    a->goBack();
    CPPUNIT_ASSERT ( a->suggestAutoCompletion().isEmpty() );

    a->insert( "neu", 2000 );
    a->returnToRoot();
    CPPUNIT_ASSERT_EQUAL( OUString("neu"), a->suggestAutoCompletion() );

    a->gotoNode( "ne" );
    CPPUNIT_ASSERT_EQUAL( OUString("u"), a->suggestAutoCompletion() );

    a->advance( sal_Unicode('u') );
    a->advance( sal_Unicode('e') );
    a->advance( sal_Unicode('r') );
    a->insert();
    CPPUNIT_ASSERT ( a->suggestAutoCompletion().isEmpty() );

    a->returnToRoot();
    CPPUNIT_ASSERT_EQUAL( OUString("neu"), a->suggestAutoCompletion() );

    a->advance( 'n' );
    CPPUNIT_ASSERT_EQUAL( OUString("eu"), a->suggestAutoCompletion() );

    a->advance( 'e' );
    CPPUNIT_ASSERT_EQUAL( OUString("uer"), a->suggestAutoCompletion() );

    // Test unicode
    OUString aQueryString = rtl::OStringToOUString( "H\xC3\xA4llo", RTL_TEXTENCODING_UTF8 );
    a->insert( aQueryString );
    a->returnToRoot();
    a->advance( sal_Unicode('H') );

    OUString aAutocompletedString = a->suggestAutoCompletion();
    OUString aExpectedString = rtl::OStringToOUString( "\xC3\xA4llo", RTL_TEXTENCODING_UTF8 );

    CPPUNIT_ASSERT_EQUAL( aExpectedString, aAutocompletedString );

    OString aUtf8String( "\xe3\x81\x82\xe3\x81\x97\xe3\x81\x9f" );
    aQueryString = rtl::OStringToOUString( aUtf8String, RTL_TEXTENCODING_UTF8 );
    a->insert( aQueryString );

    OUString aGotoString = rtl::OStringToOUString( "\xe3\x81\x82", RTL_TEXTENCODING_UTF8 );
    a->gotoNode( aGotoString );

    aAutocompletedString = a->suggestAutoCompletion();
    aExpectedString      = rtl::OStringToOUString( "\xe3\x81\x97\xe3\x81\x9f", RTL_TEXTENCODING_UTF8 );
    CPPUNIT_ASSERT_EQUAL( aExpectedString, aAutocompletedString );

    delete a;
}

} // namespace end

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
