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

    trie.findSuggestions( OUString(), suggestions);
    CPPUNIT_ASSERT_EQUAL( (size_t) 0, suggestions.size() );

    trie.insert( OUString() );
    trie.findSuggestions( OUString(), suggestions);
    CPPUNIT_ASSERT_EQUAL( (size_t) 0, suggestions.size() );

    trie.findSuggestions( "a", suggestions);
    CPPUNIT_ASSERT_EQUAL( (size_t) 0, suggestions.size() );

    trie.insert( "abc" );
    trie.insert( "abcdefghijklmnopqrstuvwxyz" );
    trie.findSuggestions( "a", suggestions);
    CPPUNIT_ASSERT_EQUAL( (size_t) 2, suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( OUString("abc"), suggestions[0] );
    CPPUNIT_ASSERT_EQUAL( OUString("abcdefghijklmnopqrstuvwxyz"), suggestions[1] );
    suggestions.clear();

    trie.findSuggestions( "abc", suggestions);
    CPPUNIT_ASSERT_EQUAL( (size_t) 1, suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( OUString("abcdefghijklmnopqrstuvwxyz"), suggestions[0] );
    suggestions.clear();

    trie.findSuggestions( "abe", suggestions);
    CPPUNIT_ASSERT_EQUAL( (size_t) 0, suggestions.size() );
    suggestions.clear();

    trie.insert( "abe" );
    trie.findSuggestions( "", suggestions);
    CPPUNIT_ASSERT_EQUAL( (size_t) 3, suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( OUString("abc"), suggestions[0] );
    CPPUNIT_ASSERT_EQUAL( OUString("abcdefghijklmnopqrstuvwxyz"), suggestions[1] );
    CPPUNIT_ASSERT_EQUAL( OUString("abe"), suggestions[2] );
    suggestions.clear();

    trie.insert( "H31l0" );
    trie.findSuggestions( "H", suggestions);

    CPPUNIT_ASSERT_EQUAL( (size_t) 1, suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( OUString("H31l0"), suggestions[0] );
    suggestions.clear();

    trie.insert( "H1" );
    trie.findSuggestions( "H", suggestions);
    CPPUNIT_ASSERT_EQUAL( (size_t) 2, suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( OUString("H31l0"), suggestions[0] );
    CPPUNIT_ASSERT_EQUAL( OUString("H1"), suggestions[1] );
    suggestions.clear();

    trie.findSuggestions( "H3", suggestions);
    CPPUNIT_ASSERT_EQUAL( (size_t) 1, suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( OUString("H31l0"), suggestions[0] );
    suggestions.clear();

    trie.insert( OStringToOUString( "H\xC3\xA4llo", RTL_TEXTENCODING_UTF8 ) );
    trie.findSuggestions( "H", suggestions );
    CPPUNIT_ASSERT_EQUAL( (size_t) 3, suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( OUString("H31l0"), suggestions[0] );
    CPPUNIT_ASSERT_EQUAL( OUString("H1"), suggestions[1] );
    CPPUNIT_ASSERT_EQUAL( OStringToOUString( "H\xC3\xA4llo", RTL_TEXTENCODING_UTF8 ), suggestions[2] );
    suggestions.clear();

    trie.findSuggestions( "H3", suggestions );
    CPPUNIT_ASSERT_EQUAL( (size_t) 1, suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( OUString("H31l0"), suggestions[0] );
    suggestions.clear();

    trie.findSuggestions( OStringToOUString("H\xC3\xA4", RTL_TEXTENCODING_UTF8), suggestions );
    CPPUNIT_ASSERT_EQUAL( (size_t) 1, suggestions.size() );
    CPPUNIT_ASSERT_EQUAL( OStringToOUString("H\xC3\xA4llo", RTL_TEXTENCODING_UTF8), suggestions[0] );
    suggestions.clear();

    trie.findSuggestions( "", suggestions);
    CPPUNIT_ASSERT_EQUAL( (size_t) 6, suggestions.size() );
    suggestions.clear();
}

void LookupTreeTest::testTrieGetAllEntries()
{
    editeng::Trie trie;

    std::vector<OUString> entries;

    trie.getAllEntries(entries);
    CPPUNIT_ASSERT_EQUAL( (size_t) 0, entries.size() );

    trie.insert("A");
    trie.getAllEntries(entries);
    CPPUNIT_ASSERT_EQUAL( (size_t) 1, entries.size() );
    entries.clear();

    trie.insert("B");
    trie.insert("C");
    trie.getAllEntries(entries);
    CPPUNIT_ASSERT_EQUAL( (size_t) 3, entries.size() );
    entries.clear();

    trie.insert("AA");
    trie.insert("AAA");
    trie.getAllEntries(entries);
    CPPUNIT_ASSERT_EQUAL( (size_t) 5, entries.size() );
    entries.clear();
}

} // namespace end

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
