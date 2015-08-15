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

class TestVbaCompression : public test::BootstrapFixtureBase
{
public:

    // just a sequence of bytes that should not be compressed at all
    void testSimple1();

    // a sequence containing one subsequence that can be compressed
    void testSimple2();

    void testSimple3();

    // avoid the BootstrapFixtureBase::setUp and tearDown
    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    CPPUNIT_TEST_SUITE(TestVbaCompression);
    CPPUNIT_TEST(testSimple1);
    CPPUNIT_TEST(testSimple2);
    CPPUNIT_TEST(testSimple3);
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

    const sal_uInt8* pReferenceData = (const sal_uInt8*) aReferenceMemoryStream.GetData();
    const sal_uInt8* pData = (const sal_uInt8*)aOutputMemoryStream.GetData();

    size_t nSize = std::min(aReferenceMemoryStream.GetSize(),
            aOutputMemoryStream.GetSize());
    for (size_t i = 0; i < nSize; ++i)
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

    const sal_uInt8* pReferenceData = (const sal_uInt8*) aReferenceMemoryStream.GetData();
    const sal_uInt8* pData = (const sal_uInt8*)aOutputMemoryStream.GetData();

    size_t nSize = std::min(aReferenceMemoryStream.GetSize(),
            aOutputMemoryStream.GetSize());
    for (size_t i = 0; i < nSize; ++i)
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

    const sal_uInt8* pReferenceData = (const sal_uInt8*) aReferenceMemoryStream.GetData();
    const sal_uInt8* pData = (const sal_uInt8*)aOutputMemoryStream.GetData();

    size_t nSize = std::min(aReferenceMemoryStream.GetSize(),
            aOutputMemoryStream.GetSize());
    for (size_t i = 0; i < nSize; ++i)
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
