/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/bootstrapfixturebase.hxx>

#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <oox/ole/vbaexport.hxx>
#include <tools/stream.hxx>
#include <algorithm>

class TestVbaCompression : public test::BootstrapFixtureBase
{
public:

    // just a sequence of bytes that should not be compressed at all
    void testSimple1();

    // a sequence containing one subsequence that can be compressed
    void testSimple2();

    void testSimple3();

    // real stream from a document
    void testComplex1();

    // tests taken from the VBA specification
    // section 3.2

    // section 3.2.1
    void testSpec321();
    // section 3.2.2
    void testSpec322();
    // section 3.2.3
    void testSpec323();

    // avoid the BootstrapFixtureBase::setUp and tearDown
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(TestVbaCompression);
    CPPUNIT_TEST(testSimple1);
    CPPUNIT_TEST(testSimple2);
    CPPUNIT_TEST(testSimple3);
    CPPUNIT_TEST(testComplex1);
    CPPUNIT_TEST(testSpec321);
    CPPUNIT_TEST(testSpec322);
    CPPUNIT_TEST(testSpec323);
    CPPUNIT_TEST_SUITE_END();

private:
};

namespace {

void ReadFiles(const OUString& rTestFile, const OUString& rReference,
        SvMemoryStream& rOutputMemoryStream, SvMemoryStream& rReferenceMemoryStream,
        const OUString& rDebugPath)
{
    SvFileStream aInputStream(rTestFile, StreamMode::READ);
    SvMemoryStream aInputMemoryStream(4096, 4096);
    aInputMemoryStream.WriteStream(aInputStream);

    VBACompression aCompression(rOutputMemoryStream, aInputMemoryStream);
    aCompression.write();

    SvFileStream aReferenceStream(rReference, StreamMode::READ);
    rReferenceMemoryStream.WriteStream(aReferenceStream);

    rOutputMemoryStream.Seek(0);
    SvFileStream aDebugStream(rDebugPath, StreamMode::WRITE);
    aDebugStream.WriteStream(rOutputMemoryStream);
}

}

void TestVbaCompression::testSimple1()
{
    OUString aTestFile = getPathFromSrc("/oox/qa/unit/data/vba/simple1.bin");
    OUString aReference = getPathFromSrc("/oox/qa/unit/data/vba/reference/simple1.bin");

    SvMemoryStream aOutputMemoryStream(4096, 4096);
    SvMemoryStream aReferenceMemoryStream(4096, 4096);
    ReadFiles(aTestFile, aReference, aOutputMemoryStream,
            aReferenceMemoryStream, "/tmp/vba_debug.bin");

    CPPUNIT_ASSERT_EQUAL(aReferenceMemoryStream.GetSize(), aOutputMemoryStream.GetSize());

    const sal_uInt8* pReferenceData = static_cast<const sal_uInt8*>( aReferenceMemoryStream.GetData() );
    const sal_uInt8* pData = static_cast<const sal_uInt8*>( aOutputMemoryStream.GetData() );

    const sal_uInt64 nSize = std::min(aReferenceMemoryStream.GetSize(),
            aOutputMemoryStream.GetSize());
    for (sal_uInt64 i = 0; i < nSize; ++i)
    {
        CPPUNIT_ASSERT_EQUAL((int)pReferenceData[i], (int)pData[i]);
    }
}

void TestVbaCompression::testSimple2()
{
    OUString aTestFile = getPathFromSrc("/oox/qa/unit/data/vba/simple2.bin");
    OUString aReference = getPathFromSrc("/oox/qa/unit/data/vba/reference/simple2.bin");

    SvMemoryStream aOutputMemoryStream(4096, 4096);
    SvMemoryStream aReferenceMemoryStream(4096, 4096);
    ReadFiles(aTestFile, aReference, aOutputMemoryStream, aReferenceMemoryStream, "/tmp/vba_debug2.bin");

    CPPUNIT_ASSERT_EQUAL(aReferenceMemoryStream.GetSize(), aOutputMemoryStream.GetSize());

    const sal_uInt8* pReferenceData = static_cast<const sal_uInt8*>( aReferenceMemoryStream.GetData() );
    const sal_uInt8* pData = static_cast<const sal_uInt8*>( aOutputMemoryStream.GetData() );

    const sal_uInt64 nSize = std::min(aReferenceMemoryStream.GetSize(),
            aOutputMemoryStream.GetSize());
    for (sal_uInt64 i = 0; i < nSize; ++i)
    {
        CPPUNIT_ASSERT_EQUAL((int)pReferenceData[i], (int)pData[i]);
    }
}

void TestVbaCompression::testSimple3()
{
    OUString aTestFile = getPathFromSrc("/oox/qa/unit/data/vba/simple3.bin");
    OUString aReference = getPathFromSrc("/oox/qa/unit/data/vba/reference/simple3.bin");

    SvMemoryStream aOutputMemoryStream(4096, 4096);
    SvMemoryStream aReferenceMemoryStream(4096, 4096);
    ReadFiles(aTestFile, aReference, aOutputMemoryStream, aReferenceMemoryStream, "/tmp/vba_debug3.bin");

    CPPUNIT_ASSERT_EQUAL(aReferenceMemoryStream.GetSize(), aOutputMemoryStream.GetSize());

    const sal_uInt8* pReferenceData = static_cast<const sal_uInt8*>( aReferenceMemoryStream.GetData()  );
    const sal_uInt8* pData = static_cast<const sal_uInt8*>( aOutputMemoryStream.GetData() );

    const sal_uInt64 nSize = std::min(aReferenceMemoryStream.GetSize(),
            aOutputMemoryStream.GetSize());
    for (sal_uInt64 i = 0; i < nSize; ++i)
    {
        CPPUNIT_ASSERT_EQUAL((int)pReferenceData[i], (int)pData[i]);
    }
}

void TestVbaCompression::testComplex1()
{
    OUString aTestFile = getPathFromSrc("/oox/qa/unit/data/vba/complex1.bin");
    OUString aReference = getPathFromSrc("/oox/qa/unit/data/vba/reference/complex1.bin");

    SvMemoryStream aOutputMemoryStream(4096, 4096);
    SvMemoryStream aReferenceMemoryStream(4096, 4096);
    ReadFiles(aTestFile, aReference, aOutputMemoryStream, aReferenceMemoryStream, "/tmp/vba_debug_complex1.bin");

    CPPUNIT_ASSERT_EQUAL(aReferenceMemoryStream.GetSize(), aOutputMemoryStream.GetSize());

    const sal_uInt8* pReferenceData = static_cast<const sal_uInt8*>( aReferenceMemoryStream.GetData() );
    const sal_uInt8* pData = static_cast<const sal_uInt8*>( aOutputMemoryStream.GetData() );

    const sal_uInt64 nSize = std::min(aReferenceMemoryStream.GetSize(),
            aOutputMemoryStream.GetSize());
    for (sal_uInt64 i = 0; i < nSize; ++i)
    {
        CPPUNIT_ASSERT_EQUAL((int)pReferenceData[i], (int)pData[i]);
    }
}

void TestVbaCompression::testSpec321()
{
    OUString aTestFile = getPathFromSrc("/oox/qa/unit/data/vba/spec321.bin");
    OUString aReference = getPathFromSrc("/oox/qa/unit/data/vba/reference/spec321.bin");

    SvMemoryStream aOutputMemoryStream(4096, 4096);
    SvMemoryStream aReferenceMemoryStream(4096, 4096);
    ReadFiles(aTestFile, aReference, aOutputMemoryStream, aReferenceMemoryStream, "/tmp/vba_debug_spec321.bin");

    CPPUNIT_ASSERT_EQUAL(aReferenceMemoryStream.GetSize(), aOutputMemoryStream.GetSize());

    const sal_uInt8* pReferenceData = static_cast<const sal_uInt8*>( aReferenceMemoryStream.GetData() );
    const sal_uInt8* pData = static_cast<const sal_uInt8*>( aOutputMemoryStream.GetData() );

    const sal_uInt64 nSize = std::min(aReferenceMemoryStream.GetSize(),
            aOutputMemoryStream.GetSize());
    for (sal_uInt64 i = 0; i < nSize; ++i)
    {
        CPPUNIT_ASSERT_EQUAL((int)pReferenceData[i], (int)pData[i]);
    }
}

void TestVbaCompression::testSpec322()
{
    OUString aTestFile = getPathFromSrc("/oox/qa/unit/data/vba/spec322.bin");
    OUString aReference = getPathFromSrc("/oox/qa/unit/data/vba/reference/spec322.bin");

    SvMemoryStream aOutputMemoryStream(4096, 4096);
    SvMemoryStream aReferenceMemoryStream(4096, 4096);
    ReadFiles(aTestFile, aReference, aOutputMemoryStream, aReferenceMemoryStream, "/tmp/vba_debug_spec322.bin");

    CPPUNIT_ASSERT_EQUAL(aReferenceMemoryStream.GetSize(), aOutputMemoryStream.GetSize());

    const sal_uInt8* pReferenceData = static_cast<const sal_uInt8*>( aReferenceMemoryStream.GetData() );
    const sal_uInt8* pData = static_cast<const sal_uInt8*>( aOutputMemoryStream.GetData() );

    const sal_uInt64 nSize = std::min(aReferenceMemoryStream.GetSize(),
            aOutputMemoryStream.GetSize());
    for (sal_uInt64 i = 0; i < nSize; ++i)
    {
        CPPUNIT_ASSERT_EQUAL((int)pReferenceData[i], (int)pData[i]);
    }
}

void TestVbaCompression::testSpec323()
{
    OUString aTestFile = getPathFromSrc("/oox/qa/unit/data/vba/spec323.bin");
    OUString aReference = getPathFromSrc("/oox/qa/unit/data/vba/reference/spec323.bin");

    SvMemoryStream aOutputMemoryStream(4096, 4096);
    SvMemoryStream aReferenceMemoryStream(4096, 4096);
    ReadFiles(aTestFile, aReference, aOutputMemoryStream, aReferenceMemoryStream, "/tmp/vba_debug_spec323.bin");

    CPPUNIT_ASSERT_EQUAL(aReferenceMemoryStream.GetSize(), aOutputMemoryStream.GetSize());

    const sal_uInt8* pReferenceData = static_cast<const sal_uInt8*>( aReferenceMemoryStream.GetData() );
    const sal_uInt8* pData = static_cast<const sal_uInt8*>( aOutputMemoryStream.GetData() );

    const sal_uInt64 nSize = std::min(aReferenceMemoryStream.GetSize(),
            aOutputMemoryStream.GetSize());
    for (sal_uInt64 i = 0; i < nSize; ++i)
    {
        CPPUNIT_ASSERT_EQUAL((int)pReferenceData[i], (int)pData[i]);
    }
}

void TestVbaCompression::setUp()
{
}

void TestVbaCompression::tearDown()
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestVbaCompression);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
