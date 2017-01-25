/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/filters-test.hxx>
#include <unotest/bootstrapfixturebase.hxx>

#include <osl/file.hxx>
#include <osl/process.h>
#include <sot/storage.hxx>
#include <sot/storinfo.hxx>

using namespace ::com::sun::star;

namespace
{
    class SotTest
        : public test::FiltersTest
        , public test::BootstrapFixtureBase
    {
    public:
        SotTest() {}

        bool checkStream( const tools::SvRef<SotStorage> &xObjStor,
                          const OUString &rStreamName,
                          sal_uLong nSize );
        bool checkStorage( const tools::SvRef<SotStorage> &xObjStor );

        virtual bool load(const OUString &,
            const OUString &rURL, const OUString &,
            SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

        void test();
        void testSize();

        CPPUNIT_TEST_SUITE(SotTest);
        CPPUNIT_TEST(test);
        CPPUNIT_TEST(testSize);
        CPPUNIT_TEST_SUITE_END();
    };

    bool SotTest::checkStream( const tools::SvRef<SotStorage> &xObjStor,
                               const OUString &rStreamName,
                               sal_uLong nSize )
    {
        unsigned char *pData = static_cast<unsigned char*>(malloc( nSize ));
        sal_uLong nReadableSize = 0;
        if( !pData )
            return true;

        {   // Read the data in one block
            tools::SvRef<SotStorageStream> xStream( xObjStor->OpenSotStream( rStreamName ) );
            xStream->Seek(0);
            sal_uLong nRemaining = xStream->GetSize() - xStream->Tell();

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "check size", nSize, nRemaining );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "check size #2", static_cast<sal_uInt64>(nSize), xStream->remainingSize());

            // Read as much as we can, a corrupted FAT chain can cause real grief here
            nReadableSize = xStream->ReadBytes(static_cast<void *>(pData), nSize);
//            fprintf(stderr, "readable size %d vs size %d remaining %d\n", nReadableSize, nSize, nReadableSize);
        }
        {   // Read the data backwards as well
            tools::SvRef<SotStorageStream> xStream( xObjStor->OpenSotStream( rStreamName ) );
            for( sal_uLong i = nReadableSize; i > 0; i-- )
            {
                CPPUNIT_ASSERT_MESSAGE( "sot reading error", !xStream->GetError() );
                unsigned char c;
                xStream->Seek( i - 1 );
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "sot storage reading byte",
                                              static_cast<size_t>(1), xStream->ReadBytes(&c, 1));
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "mismatching data storage reading byte",
                                              c, pData[i - 1] );
            }
        }
        free(pData);
        return true;
    }

    bool SotTest::checkStorage( const tools::SvRef<SotStorage> &xObjStor )
    {
        SvStorageInfoList aInfoList;
        xObjStor->FillInfoList( &aInfoList );

        for( SvStorageInfoList::iterator aIt = aInfoList.begin();
             aIt != aInfoList.end(); ++aIt )
        {
            if( aIt->IsStorage() )
            {
                tools::SvRef<SotStorage> xChild( xObjStor->OpenSotStorage( aIt->GetName() ) );
                checkStorage( xChild );
            }
            else if( aIt->IsStream() )
                checkStream( xObjStor, aIt->GetName(), aIt->GetSize() );
        }

        return true;
    }

    bool SotTest::load(const OUString &,
        const OUString &rURL, const OUString &,
        SfxFilterFlags, SotClipboardFormatId, unsigned int)
    {
        SvFileStream aStream(rURL, StreamMode::READ);
        tools::SvRef<SotStorage> xObjStor = new SotStorage(aStream);
        if (!xObjStor.is() || xObjStor->GetError())
            return false;

        CPPUNIT_ASSERT_MESSAGE("sot storage is not valid", xObjStor->Validate());
        return checkStorage (xObjStor);
    }

    void SotTest::test()
    {
        testDir(OUString(),
            m_directories.getURLFromSrc("/sot/qa/cppunit/data/"));
    }

    void SotTest::testSize()
    {
        OUString aURL(m_directories.getURLFromSrc("/sot/qa/cppunit/data/pass/fdo84229-1.compound"));
        SvFileStream aStream(aURL, StreamMode::READ);
        tools::SvRef<SotStorage> xObjStor = new SotStorage(aStream);
        CPPUNIT_ASSERT_MESSAGE("sot storage failed to open",
                               xObjStor.is() && !xObjStor->GetError());
        tools::SvRef<SotStorageStream> xStream = xObjStor->OpenSotStream("Book");
        CPPUNIT_ASSERT_MESSAGE("stream failed to open",
                               xStream.is() && !xObjStor->GetError());
        CPPUNIT_ASSERT_MESSAGE("error in opened stream", !xStream->GetError());
        sal_uLong nPos = xStream->GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("odd stream length", static_cast<sal_uLong>(13312), nPos);

        xStream->Seek(STREAM_SEEK_TO_END);
        CPPUNIT_ASSERT_MESSAGE("error seeking to end", !xStream->GetError());
        // cf. comment in Pos2Page, not extremely intuitive ...
        CPPUNIT_ASSERT_EQUAL_MESSAGE("stream not at beginning", static_cast<sal_uInt64>(xStream->GetSize()), xStream->Tell());
        xStream->Seek(STREAM_SEEK_TO_BEGIN);

        CPPUNIT_ASSERT_MESSAGE("error seeking to beginning", !xStream->GetError());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("stream not at beginning", static_cast<sal_uInt64>(0), xStream->Tell());
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(SotTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
