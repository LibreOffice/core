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
#include <editeng/Trie.hxx>

namespace {

class LookupTreeTest : public CppUnit::TestFixture
{
public:
    void testTrie();
    void testTrieGetAllEntries();

    CPPUNIT_TEST_SUITE(LookupTreeTest);
    CPPUNIT_TEST(testTrie);
    CPPUNIT_TEST(testTrieGetAllEntries);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LookupTreeTest);

void LookupTreeTest::testTrie()
{
    editeng::Trie trie;
    std::vector<OUString> suggestions;

    trie.findSuggestions( u"", suggestions);
    CPPUNIT_ASSERT_EQUAL( size_t(0), suggestions.size() );

    trie.insert( u"" );
    trie.findSuggestions( u"", suggestions);
    CPPUNIT_ASSERT_EQUAL( size_t(0), suggestions.size() );

    trie.findSuggestions( u"a", suggestions);
    CPPUNIT_ASSERT_EQUAL( size_t(0), suggestions.size() );

    trie.insert( u"abc" );
    trie.insert( u"abcdefghijklmnopqrstuvwxyz" );
    trie.findSuggestions( u"a", suggestions);
    CPPUNIT_ASSERT_EQUAL( size_t(2), suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( u"abc"_ustr, suggestions[0] );
    CPPUNIT_ASSERT_EQUAL( u"abcdefghijklmnopqrstuvwxyz"_ustr, suggestions[1] );
    suggestions.clear();

    trie.findSuggestions( u"abc", suggestions);
    CPPUNIT_ASSERT_EQUAL( size_t(1), suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( u"abcdefghijklmnopqrstuvwxyz"_ustr, suggestions[0] );
    suggestions.clear();

    trie.findSuggestions( u"abe", suggestions);
    CPPUNIT_ASSERT_EQUAL( size_t(0), suggestions.size() );
    suggestions.clear();

    trie.insert( u"abe" );
    trie.findSuggestions( u"", suggestions);
    CPPUNIT_ASSERT_EQUAL( size_t(3), suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( u"abc"_ustr, suggestions[0] );
    CPPUNIT_ASSERT_EQUAL( u"abcdefghijklmnopqrstuvwxyz"_ustr, suggestions[1] );
    CPPUNIT_ASSERT_EQUAL( u"abe"_ustr, suggestions[2] );
    suggestions.clear();

    trie.insert( u"H31l0" );
    trie.findSuggestions( u"H", suggestions);

    CPPUNIT_ASSERT_EQUAL( size_t(1), suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( u"H31l0"_ustr, suggestions[0] );
    suggestions.clear();

    trie.insert( u"H1" );
    trie.findSuggestions( u"H", suggestions);
    CPPUNIT_ASSERT_EQUAL( size_t(2), suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( u"H31l0"_ustr, suggestions[0] );
    CPPUNIT_ASSERT_EQUAL( u"H1"_ustr, suggestions[1] );
    suggestions.clear();

    trie.findSuggestions( u"H3", suggestions);
    CPPUNIT_ASSERT_EQUAL( size_t(1), suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( u"H31l0"_ustr, suggestions[0] );
    suggestions.clear();

    trie.insert( OStringToOUString( "H\xC3\xA4llo", RTL_TEXTENCODING_UTF8 ) );
    trie.findSuggestions( u"H", suggestions );
    CPPUNIT_ASSERT_EQUAL( size_t(3), suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( u"H31l0"_ustr, suggestions[0] );
    CPPUNIT_ASSERT_EQUAL( u"H1"_ustr, suggestions[1] );
    CPPUNIT_ASSERT_EQUAL( OStringToOUString( "H\xC3\xA4llo", RTL_TEXTENCODING_UTF8 ), suggestions[2] );
    suggestions.clear();

    trie.findSuggestions( u"H3", suggestions );
    CPPUNIT_ASSERT_EQUAL( size_t(1), suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( u"H31l0"_ustr, suggestions[0] );
    suggestions.clear();

    trie.findSuggestions( OStringToOUString("H\xC3\xA4", RTL_TEXTENCODING_UTF8), suggestions );
    CPPUNIT_ASSERT_EQUAL( size_t(1), suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( OStringToOUString("H\xC3\xA4llo", RTL_TEXTENCODING_UTF8), suggestions[0] );
    suggestions.clear();

    trie.findSuggestions( u"", suggestions);
    CPPUNIT_ASSERT_EQUAL( size_t(6), suggestions.size() );
    suggestions.clear();
}

void LookupTreeTest::testTrieGetAllEntries()
{
    editeng::Trie trie;

    CPPUNIT_ASSERT_EQUAL( size_t(0), trie.size() );

    trie.insert(u"A");
    CPPUNIT_ASSERT_EQUAL( size_t(1), trie.size() );

    trie.insert(u"B");
    trie.insert(u"C");
    CPPUNIT_ASSERT_EQUAL( size_t(3), trie.size() );

    trie.insert(u"AA");
    trie.insert(u"AAA");
    CPPUNIT_ASSERT_EQUAL( size_t(5), trie.size() );
}

} // namespace end

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
