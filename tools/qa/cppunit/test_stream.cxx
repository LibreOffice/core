/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
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
        void test_read_cstring();
        void test_read_pstring();
        void test_readline();

        CPPUNIT_TEST_SUITE(Test);
        CPPUNIT_TEST(test_stdstream);
        CPPUNIT_TEST(test_fastostring);
        CPPUNIT_TEST(test_read_cstring);
        CPPUNIT_TEST(test_read_pstring);
        CPPUNIT_TEST(test_readline);
        CPPUNIT_TEST_SUITE_END();
    };

    void Test::test_stdstream()
    {
        char foo[] = "foo";
        std::istringstream iss(foo, std::istringstream::in);
        SvMemoryStream aMemStream(foo, SAL_N_ELEMENTS(foo)-1, StreamMode::READ);

        char std_a(78);
        iss >> std_a;
        CPPUNIT_ASSERT_EQUAL('f', std_a);

        char tools_a(78);
        aMemStream.ReadChar( tools_a );
        CPPUNIT_ASSERT_EQUAL('f', tools_a);

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
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(78), std_a);
        //nothing wrong with the stream, so not bad
        CPPUNIT_ASSERT(!iss.bad());
        //yet, the read didn't succeed
        CPPUNIT_ASSERT(!iss.good());
        CPPUNIT_ASSERT_EQUAL((std::ios::failbit|std::ios::eofbit), iss.rdstate());

        tools_a = 78;
        aMemStream.ReadChar( tools_a );
        //so, now eof is set
        CPPUNIT_ASSERT(aMemStream.eof());
        //a failed read doesn't change the data, it remains unchanged
        CPPUNIT_ASSERT_EQUAL(static_cast<char>(78), tools_a);
        //nothing wrong with the stream, so not bad
        CPPUNIT_ASSERT(!aMemStream.bad());
        //yet, the read didn't succeed
        CPPUNIT_ASSERT(!aMemStream.good());

        //set things up so that there is only one byte available on an attempt
        //to read a two-byte sal_uInt16.  The byte should be consumed, but the
        //operation should fail, and tools_b should remain unchanged,
        sal_uInt16 tools_b = 0x1122;
        aMemStream.SeekRel(-1);
        CPPUNIT_ASSERT(!aMemStream.eof());
        CPPUNIT_ASSERT(aMemStream.good());
        aMemStream.ReadUInt16( tools_b );
        CPPUNIT_ASSERT(!aMemStream.good());
        CPPUNIT_ASSERT(aMemStream.eof());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0x1122), tools_b);

        iss.clear();
        iss.seekg(0);
        CPPUNIT_ASSERT(iss.good());
        iss >> std_a;
        CPPUNIT_ASSERT_EQUAL('f', std_a);

        aMemStream.Seek(0);
        CPPUNIT_ASSERT(aMemStream.good());
        aMemStream.ReadChar( tools_a );
        CPPUNIT_ASSERT_EQUAL('f', tools_a);

        //failbit is rather subtle wrt e.g seeks

        char buffer[1024];

        iss.clear();
        iss.seekg(0);
        CPPUNIT_ASSERT(iss.good());
        iss.read(buffer, sizeof(buffer));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::streamsize>(3), iss.gcount());
        CPPUNIT_ASSERT(!iss.good());
        CPPUNIT_ASSERT(!iss.bad());
        CPPUNIT_ASSERT(iss.eof());

        aMemStream.Seek(0);
        CPPUNIT_ASSERT(aMemStream.good());
        std::size_t nRet = aMemStream.ReadBytes(buffer, sizeof(buffer));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(3), nRet);
        CPPUNIT_ASSERT(!aMemStream.good());
        CPPUNIT_ASSERT(!aMemStream.bad());
        CPPUNIT_ASSERT(aMemStream.eof());
    }

    void Test::test_fastostring()
    {
        char foo[] = "foobar";
        SvMemoryStream aMemStream(foo, SAL_N_ELEMENTS(foo)-1, StreamMode::READ);

        OString aOne = read_uInt8s_ToOString(aMemStream, 3);
        CPPUNIT_ASSERT_EQUAL(OString("foo"), aOne);

        OString aTwo = read_uInt8s_ToOString(aMemStream, 3);
        CPPUNIT_ASSERT_EQUAL(OString("bar"), aTwo);

        OString aThree = read_uInt8s_ToOString(aMemStream, 3);
        CPPUNIT_ASSERT(aThree.isEmpty());

        aMemStream.Seek(0);

        OString aFour = read_uInt8s_ToOString(aMemStream, 100);
        CPPUNIT_ASSERT_EQUAL(OString(foo), aFour);
    }

    void Test::test_read_cstring()
    {
        char foo[] = "foobar";
        SvMemoryStream aMemStream(foo, SAL_N_ELEMENTS(foo)-1, StreamMode::READ);

        OString aOne = read_zeroTerminated_uInt8s_ToOString(aMemStream);
        CPPUNIT_ASSERT_EQUAL(OString("foobar"), aOne);
        CPPUNIT_ASSERT(!aMemStream.good());
        CPPUNIT_ASSERT(!aMemStream.bad());
        CPPUNIT_ASSERT(aMemStream.eof());

        aMemStream.Seek(0);
        foo[3] = 0;
        OString aTwo = read_zeroTerminated_uInt8s_ToOString(aMemStream);
        CPPUNIT_ASSERT_EQUAL(OString("foo"), aTwo);
        CPPUNIT_ASSERT(aMemStream.good());
    }

    void Test::test_read_pstring()
    {
        char foo[] = "\3foobar";
        SvMemoryStream aMemStream(foo, SAL_N_ELEMENTS(foo)-1, StreamMode::READ);

        OString aFoo = read_uInt8_lenPrefixed_uInt8s_ToOString(aMemStream);
        CPPUNIT_ASSERT_EQUAL(OString("foo"), aFoo);
        CPPUNIT_ASSERT(aMemStream.good());
        CPPUNIT_ASSERT(!aMemStream.bad());
        CPPUNIT_ASSERT(!aMemStream.eof());

        aMemStream.Seek(0);
        foo[0] = 10;
        aFoo = read_uInt8_lenPrefixed_uInt8s_ToOString(aMemStream);
        CPPUNIT_ASSERT_EQUAL(OString("foobar"), aFoo);
        CPPUNIT_ASSERT(!aMemStream.good());
        CPPUNIT_ASSERT(!aMemStream.bad());
        CPPUNIT_ASSERT(aMemStream.eof());

        aMemStream.SetEndian(SvStreamEndian::BIG);
        aMemStream.Seek(0);
        foo[0] = 0;
        foo[1] = 3;
        aFoo = read_uInt16_lenPrefixed_uInt8s_ToOString(aMemStream);
        CPPUNIT_ASSERT_EQUAL(OString("oob"), aFoo);
        CPPUNIT_ASSERT(aMemStream.good());
        CPPUNIT_ASSERT(!aMemStream.bad());
        CPPUNIT_ASSERT(!aMemStream.eof());
    }

    void Test::test_readline()
    {
        char foo[] = "foo\nbar\n\n";
        SvMemoryStream aMemStream(foo, SAL_N_ELEMENTS(foo)-1, StreamMode::READ);

        OString aFoo;
        bool bRet;

        bRet = aMemStream.ReadLine(aFoo);
        CPPUNIT_ASSERT(bRet);
        CPPUNIT_ASSERT_EQUAL(OString("foo"), aFoo);
        CPPUNIT_ASSERT(aMemStream.good());

        bRet = aMemStream.ReadLine(aFoo);
        CPPUNIT_ASSERT(bRet);
        CPPUNIT_ASSERT_EQUAL(OString("bar"), aFoo);
        CPPUNIT_ASSERT(aMemStream.good());

        bRet = aMemStream.ReadLine(aFoo);
        CPPUNIT_ASSERT(bRet);
        CPPUNIT_ASSERT(aFoo.isEmpty());
        CPPUNIT_ASSERT(aMemStream.good());

        bRet = aMemStream.ReadLine(aFoo);
        CPPUNIT_ASSERT(!bRet);
        CPPUNIT_ASSERT(aFoo.isEmpty());
        CPPUNIT_ASSERT(aMemStream.eof());

        foo[3] = 0; //test reading embedded nulls

        aMemStream.Seek(0);
        bRet = aMemStream.ReadLine(aFoo);
        CPPUNIT_ASSERT(bRet);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), aFoo.getLength());
        CPPUNIT_ASSERT_EQUAL('\0', aFoo[3]);
        CPPUNIT_ASSERT(aMemStream.good());

        std::string sStr(foo, RTL_CONSTASCII_LENGTH(foo));
        std::istringstream iss(sStr, std::istringstream::in);
        std::getline(iss, sStr, '\n');
        //embedded null read as expected
        CPPUNIT_ASSERT_EQUAL(std::string::size_type(7), sStr.size());
        CPPUNIT_ASSERT_EQUAL('\0', sStr[3]);
        CPPUNIT_ASSERT(iss.good());

        bRet = aMemStream.ReadLine(aFoo);
        CPPUNIT_ASSERT(bRet);
        CPPUNIT_ASSERT(aFoo.isEmpty());
        CPPUNIT_ASSERT(aMemStream.good());

        std::getline(iss, sStr, '\n');
        CPPUNIT_ASSERT(sStr.empty());
        CPPUNIT_ASSERT(iss.good());

        bRet = aMemStream.ReadLine(aFoo);
        CPPUNIT_ASSERT(!bRet);
        CPPUNIT_ASSERT(aFoo.isEmpty());
        CPPUNIT_ASSERT(aMemStream.eof());
        CPPUNIT_ASSERT(!aMemStream.bad());

        std::getline(iss, sStr, '\n');
        CPPUNIT_ASSERT(sStr.empty());
        CPPUNIT_ASSERT(iss.eof());
        CPPUNIT_ASSERT(!iss.bad());

        char bar[] = "foo";
        SvMemoryStream aMemStreamB(bar, SAL_N_ELEMENTS(bar)-1, StreamMode::READ);
        bRet = aMemStreamB.ReadLine(aFoo);
        CPPUNIT_ASSERT(bRet);
        CPPUNIT_ASSERT_EQUAL(OString("foo"), aFoo);
        CPPUNIT_ASSERT(!aMemStreamB.eof()); //<-- diff A

        std::istringstream issB(bar, std::istringstream::in);
        std::getline(issB, sStr, '\n');
        CPPUNIT_ASSERT_EQUAL(std::string("foo"), sStr);
        CPPUNIT_ASSERT(issB.eof());         //<-- diff A
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
