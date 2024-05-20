/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <store/store.hxx>

#include <memory>

namespace
{
class StoreTest : public CppUnit::TestFixture
{
public:
    void createMemoryStream();
    void createStream();
    void writeAndReadByte();

    CPPUNIT_TEST_SUITE(StoreTest);
    CPPUNIT_TEST(createMemoryStream);
    CPPUNIT_TEST(createStream);
    CPPUNIT_TEST(writeAndReadByte);
    CPPUNIT_TEST_SUITE_END();
};

void StoreTest::createMemoryStream()
{
    store::OStoreFile aFile;
    CPPUNIT_ASSERT_EQUAL(store_E_None, aFile.createInMemory());
}

void StoreTest::createStream()
{
    store::OStoreFile aFile;
    CPPUNIT_ASSERT_EQUAL(store_E_None, aFile.createInMemory());

    store::OStoreStream aStream;
    CPPUNIT_ASSERT_EQUAL(store_E_None, aStream.create(aFile, u"testnode"_ustr, u"testname"_ustr,
                                                      storeAccessMode::Create));

    aFile.close();
}

void StoreTest::writeAndReadByte()
{
    store::OStoreFile aFile;
    CPPUNIT_ASSERT_EQUAL(store_E_None, aFile.createInMemory());

    store::OStoreStream aStream;
    CPPUNIT_ASSERT_EQUAL(store_E_None, aStream.create(aFile, u"testnode"_ustr, u"testname"_ustr,
                                                      storeAccessMode::Create));

    {
        std::unique_ptr<sal_uInt8[]> pWriteBuffer(new sal_uInt8[1]);
        pWriteBuffer[0] = 'a';

        sal_uInt32 writtenBytes;

        CPPUNIT_ASSERT_EQUAL(store_E_None, aStream.writeAt(0, pWriteBuffer.get(), 1, writtenBytes));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(1), writtenBytes);
    }

    {
        std::unique_ptr<sal_uInt8[]> pReadBuffer(new sal_uInt8[1]);
        sal_uInt32 readBytes;

        CPPUNIT_ASSERT_EQUAL(store_E_None, aStream.readAt(0, pReadBuffer.get(), 1, readBytes));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(1), readBytes);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('a'), pReadBuffer[0]);
    }

    aFile.close();
}

CPPUNIT_TEST_SUITE_REGISTRATION(StoreTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
