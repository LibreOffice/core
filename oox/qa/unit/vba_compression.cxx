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

    void testSimple1();

    // avoid the BootstrapFixtureBase::setUp and tearDown
    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    CPPUNIT_TEST_SUITE(TestVbaCompression);
    CPPUNIT_TEST(testSimple1);
    CPPUNIT_TEST_SUITE_END();

private:
};

void TestVbaCompression::testSimple1()
{
    OUString aTestFile = getPathFromSrc("/oox/qa/unit/data/vba/simple1.bin");
    OUString aReference = getPathFromSrc("/oox/qa/unit/data/vba/reference/simple1.bin");

    SvFileStream aInputStream(aTestFile, StreamMode::READ);
    SvMemoryStream aInputMemoryStream(4096, 4096);
    aInputMemoryStream.WriteStream(aInputStream);

    SvMemoryStream aOutputMemoryStream(4096, 4096);
    VBACompression aCompression(aOutputMemoryStream, aInputMemoryStream);
    aCompression.write();

    SvFileStream aReferenceStream(aReference, StreamMode::READ);
    SvMemoryStream aReferenceMemoryStream(4096, 4096);
    aReferenceMemoryStream.WriteStream(aReferenceStream);

    aOutputMemoryStream.Seek(0);
    SvFileStream aDebugStream("/tmp/vba_debug.bin", StreamMode::WRITE);
    aDebugStream.WriteStream(aOutputMemoryStream);

    // CPPUNIT_ASSERT_EQUAL(aReferenceMemoryStream.GetSize(), aOutputMemoryStream.GetSize());

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
