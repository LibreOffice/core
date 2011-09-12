/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *        Caolán McNamara <caolanm@redhat.com> (Red Hat, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "precompiled_tools.hxx"
#include "sal/config.h"
#include "sal/precppunit.hxx"

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include <tools/stream.hxx>
#include <sstream>

//Tests for eofbit/badbit/goodbit/failbit

namespace
{

    class Test: public CppUnit::TestFixture
    {
    public:
        void test_stdstream();
        void test_fastostring();

        CPPUNIT_TEST_SUITE(Test);
        CPPUNIT_TEST(test_stdstream);
        CPPUNIT_TEST(test_fastostring);
        CPPUNIT_TEST_SUITE_END();
    };

    void Test::test_stdstream()
    {
        char foo[] = "foo";
        std::istringstream iss(foo, std::istringstream::in);
        SvMemoryStream aMemStream(RTL_CONSTASCII_STRINGPARAM(foo), STREAM_READ);

        char std_a(78);
        iss >> std_a;
        CPPUNIT_ASSERT(std_a == 'f');

        char tools_a(78);
        aMemStream >> tools_a;
        CPPUNIT_ASSERT(std_a == 'f');

        iss.seekg(0, std::ios_base::end);
        //seeking to end doesn't set eof, reading past eof does
        CPPUNIT_ASSERT(!iss.eof());
        CPPUNIT_ASSERT(iss.good());

        aMemStream.Seek(STREAM_SEEK_TO_END);
        //seeking to end doesn't set eof, reading past eof does
        CPPUNIT_ASSERT(!aMemStream.eof());
        CPPUNIT_ASSERT(aMemStream.good());

        std_a = 78;
        iss >> std_a;
        //so, now eof is set
        CPPUNIT_ASSERT(iss.eof());
        //a failed read doesn't change the data, it remains unchanged
        CPPUNIT_ASSERT(std_a == 78);
        //nothing wrong with the stream, so not bad
        CPPUNIT_ASSERT(!iss.bad());
        //yet, the read didn't succeed
        CPPUNIT_ASSERT(!iss.good());
        CPPUNIT_ASSERT(iss.rdstate() == (std::ios::failbit|std::ios::eofbit));

        tools_a = 78;
        aMemStream >> tools_a;
        //so, now eof is set
        CPPUNIT_ASSERT(aMemStream.eof());
        //a failed read doesn't change the data, it remains unchanged
        CPPUNIT_ASSERT(tools_a == 78);
        //nothing wrong with the stream, so not bad
        CPPUNIT_ASSERT(!aMemStream.GetError());
        //yet, the read didn't succeed
        CPPUNIT_ASSERT(!aMemStream.good());

        //set things up so that there is only one byte available on an attempt
        //to read a two-byte sal_uInt16.  The byte should be consumed, but the
        //operation should fail, and tools_b should remain unchanged,
        sal_uInt16 tools_b = 0x1122;
        aMemStream.SeekRel(-1);
        CPPUNIT_ASSERT(!aMemStream.eof());
        CPPUNIT_ASSERT(aMemStream.good());
        aMemStream >> tools_b;
        CPPUNIT_ASSERT(!aMemStream.good());
        CPPUNIT_ASSERT(aMemStream.eof());
        CPPUNIT_ASSERT(tools_b == 0x1122);

        iss.clear();
        iss.seekg(0);
        CPPUNIT_ASSERT(iss.good());
        iss >> std_a;
        CPPUNIT_ASSERT(std_a == 'f');

        aMemStream.Seek(0);
        CPPUNIT_ASSERT(aMemStream.good());
        aMemStream >> tools_a;
        CPPUNIT_ASSERT(tools_a == 'f');

        //failbit is rather subtle wrt e.g seeks
    }

    void Test::test_fastostring()
    {
        char foo[] = "foobar";
        SvMemoryStream aMemStream(RTL_CONSTASCII_STRINGPARAM(foo), STREAM_READ);

        rtl::OString aOne = read_uInt8s_AsOString(aMemStream, 3);
        CPPUNIT_ASSERT(aOne.equalsL(RTL_CONSTASCII_STRINGPARAM("foo")));

        rtl::OString aTwo = read_uInt8s_AsOString(aMemStream, 3);
        CPPUNIT_ASSERT(aTwo.equalsL(RTL_CONSTASCII_STRINGPARAM("bar")));

        rtl::OString aThree = read_uInt8s_AsOString(aMemStream, 3);
        CPPUNIT_ASSERT(!aThree.getLength());

        aMemStream.Seek(0);

        rtl::OString aFour = read_uInt8s_AsOString(aMemStream, 100);
        CPPUNIT_ASSERT(aFour.equalsL(RTL_CONSTASCII_STRINGPARAM(foo)));
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
