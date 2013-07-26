/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
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
        SvMemoryStream aMemStream(foo, SAL_N_ELEMENTS(foo)-1, STREAM_READ);

        char std_a(78);
        iss >> std_a;
        CPPUNIT_ASSERT(std_a == 'f');

        char tools_a(78);
        aMemStream >> tools_a;
        CPPUNIT_ASSERT(tools_a == 'f');

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

        char buffer[1024];

        iss.clear();
        iss.seekg(0);
        CPPUNIT_ASSERT(iss.good());
        iss.read(buffer, sizeof(buffer));
        CPPUNIT_ASSERT(iss.gcount() == 3);
        CPPUNIT_ASSERT(!iss.good());
        CPPUNIT_ASSERT(!iss.bad());
        CPPUNIT_ASSERT(iss.eof());

        aMemStream.Seek(0);
        CPPUNIT_ASSERT(aMemStream.good());
        sal_Size nRet = aMemStream.Read(buffer, sizeof(buffer));
        CPPUNIT_ASSERT(nRet == 3);
        CPPUNIT_ASSERT(!aMemStream.good());
        CPPUNIT_ASSERT(!aMemStream.bad());
        CPPUNIT_ASSERT(aMemStream.eof());
    }

    void Test::test_fastostring()
    {
        char foo[] = "foobar";
        SvMemoryStream aMemStream(foo, SAL_N_ELEMENTS(foo)-1, STREAM_READ);

        OString aOne = read_uInt8s_ToOString(aMemStream, 3);
        CPPUNIT_ASSERT(aOne == "foo");

        OString aTwo = read_uInt8s_ToOString(aMemStream, 3);
        CPPUNIT_ASSERT(aTwo == "bar");

        OString aThree = read_uInt8s_ToOString(aMemStream, 3);
        CPPUNIT_ASSERT(aThree.isEmpty());

        aMemStream.Seek(0);

        OString aFour = read_uInt8s_ToOString(aMemStream, 100);
        CPPUNIT_ASSERT(aFour == foo);
    }

    void Test::test_read_cstring()
    {
        char foo[] = "foobar";
        SvMemoryStream aMemStream(foo, SAL_N_ELEMENTS(foo)-1, STREAM_READ);

        OString aOne = read_zeroTerminated_uInt8s_ToOString(aMemStream);
        CPPUNIT_ASSERT(aOne == "foobar");
        CPPUNIT_ASSERT(!aMemStream.good());
        CPPUNIT_ASSERT(!aMemStream.bad());
        CPPUNIT_ASSERT(aMemStream.eof());

        aMemStream.Seek(0);
        foo[3] = 0;
        OString aTwo = read_zeroTerminated_uInt8s_ToOString(aMemStream);
        CPPUNIT_ASSERT(aTwo == "foo");
        CPPUNIT_ASSERT(aMemStream.good());
    }

    void Test::test_read_pstring()
    {
        char foo[] = "\3foobar";
        SvMemoryStream aMemStream(foo, SAL_N_ELEMENTS(foo)-1, STREAM_READ);

        OString aFoo = read_lenPrefixed_uInt8s_ToOString<sal_uInt8>(aMemStream);
        CPPUNIT_ASSERT(aFoo == "foo");
        CPPUNIT_ASSERT(aMemStream.good());
        CPPUNIT_ASSERT(!aMemStream.bad());
        CPPUNIT_ASSERT(!aMemStream.eof());

        aMemStream.Seek(0);
        foo[0] = 10;
        aFoo = read_lenPrefixed_uInt8s_ToOString<sal_uInt8>(aMemStream);
        CPPUNIT_ASSERT(aFoo == "foobar");
        CPPUNIT_ASSERT(!aMemStream.good());
        CPPUNIT_ASSERT(!aMemStream.bad());
        CPPUNIT_ASSERT(aMemStream.eof());

        aMemStream.SetNumberFormatInt(NUMBERFORMAT_INT_BIGENDIAN);
        aMemStream.Seek(0);
        foo[0] = 0;
        foo[1] = 3;
        aFoo = read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(aMemStream);
        CPPUNIT_ASSERT(aFoo == "oob");
        CPPUNIT_ASSERT(aMemStream.good());
        CPPUNIT_ASSERT(!aMemStream.bad());
        CPPUNIT_ASSERT(!aMemStream.eof());

        aMemStream.SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
        aMemStream.Seek(0);
        foo[0] = 3;
        foo[1] = 0;
        foo[2] = 0;
        foo[3] = 0;
        aFoo = read_lenPrefixed_uInt8s_ToOString<sal_uInt32>(aMemStream);
        CPPUNIT_ASSERT(aFoo == "bar");
        CPPUNIT_ASSERT(aMemStream.good());
        CPPUNIT_ASSERT(!aMemStream.bad());
        CPPUNIT_ASSERT(!aMemStream.eof());
    }

    void Test::test_readline()
    {
        char foo[] = "foo\nbar\n\n";
        SvMemoryStream aMemStream(foo, SAL_N_ELEMENTS(foo)-1, STREAM_READ);

        OString aFoo;
        bool bRet;

        bRet = aMemStream.ReadLine(aFoo);
        CPPUNIT_ASSERT(bRet);
        CPPUNIT_ASSERT(aFoo == "foo");
        CPPUNIT_ASSERT(aMemStream.good());

        bRet = aMemStream.ReadLine(aFoo);
        CPPUNIT_ASSERT(bRet);
        CPPUNIT_ASSERT(aFoo == "bar");
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
        CPPUNIT_ASSERT(aFoo.getLength() == 7 && aFoo[3] == 0);
        CPPUNIT_ASSERT(aMemStream.good());

        std::string sStr(foo, RTL_CONSTASCII_LENGTH(foo));
        std::istringstream iss(sStr, std::istringstream::in);
        std::getline(iss, sStr, '\n');
        //embedded null read as expected
        CPPUNIT_ASSERT(sStr.size() == 7 && sStr[3] == 0);
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
        CPPUNIT_ASSERT(aMemStream.eof() && !aMemStream.bad());

        std::getline(iss, sStr, '\n');
        CPPUNIT_ASSERT(sStr.empty());
        CPPUNIT_ASSERT(iss.eof() && !iss.bad());

        char bar[] = "foo";
        SvMemoryStream aMemStreamB(bar, SAL_N_ELEMENTS(bar)-1, STREAM_READ);
        bRet = aMemStreamB.ReadLine(aFoo);
        CPPUNIT_ASSERT(bRet);
        CPPUNIT_ASSERT(aFoo == "foo");
        CPPUNIT_ASSERT(!aMemStreamB.eof()); //<-- diff A

        std::istringstream issB(bar, std::istringstream::in);
        std::getline(issB, sStr, '\n');
        CPPUNIT_ASSERT(sStr == "foo");
        CPPUNIT_ASSERT(issB.eof());         //<-- diff A
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
