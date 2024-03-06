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

#include <sot/storage.hxx>
#include <sot/storinfo.hxx>
#include <sysformats.hxx>

using namespace ::com::sun::star;

namespace
{
    auto FindFormatIndex(std::span<const SotAction_Impl> pFormats, SotClipboardFormatId eFormat)
    {
        auto it = std::find_if(pFormats.begin(), pFormats.end(),
                               [eFormat](const auto& item) { return item.nFormatId == eFormat; });
        return std::distance(pFormats.begin(), it);
    }

    class SotTest
        : public test::FiltersTest
        , public test::BootstrapFixtureBase
    {
    public:
        SotTest() {}

        bool checkStream( const rtl::Reference<SotStorage> &xObjStor,
                          const OUString &rStreamName,
                          sal_uInt64 nSize );
        bool checkStorage(const rtl::Reference<SotStorage>& xObjStor);

        virtual bool load(const OUString &,
            const OUString &rURL, const OUString &,
            SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

        void test();
        void testSize();
        void testClipboard();

        CPPUNIT_TEST_SUITE(SotTest);
        CPPUNIT_TEST(test);
        CPPUNIT_TEST(testSize);
        CPPUNIT_TEST(testClipboard);
        CPPUNIT_TEST_SUITE_END();
    };

    bool SotTest::checkStream( const rtl::Reference<SotStorage> &xObjStor,
                               const OUString &rStreamName,
                               sal_uInt64 nSize )
    {
        unsigned char *pData = static_cast<unsigned char*>(malloc( nSize ));
        sal_uInt64 nReadableSize = 0;
        if( !pData )
            return true;

        {   // Read the data in one block
            rtl::Reference<SotStorageStream> xStream(xObjStor->OpenSotStream(rStreamName));
            xStream->Seek(0);
            sal_uInt64 nRemaining = xStream->GetSize() - xStream->Tell();

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "check size", nSize, nRemaining );
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "check size #2", nSize, xStream->remainingSize());

            // Read as much as we can, a corrupted FAT chain can cause real grief here
            nReadableSize = xStream->ReadBytes(static_cast<void *>(pData), nSize);
        }
        {   // Read the data backwards as well
            rtl::Reference<SotStorageStream> xStream(xObjStor->OpenSotStream(rStreamName));
            for( sal_uInt64 i = nReadableSize; i > 0; i-- )
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

    bool SotTest::checkStorage(const rtl::Reference<SotStorage>& xObjStor)
    {
        SvStorageInfoList aInfoList;
        xObjStor->FillInfoList( &aInfoList );

        for (auto& rInfo : aInfoList)
        {
            if( rInfo.IsStorage() )
            {
                rtl::Reference<SotStorage> xChild(xObjStor->OpenSotStorage(rInfo.GetName()));
                checkStorage( xChild );
            }
            else if( rInfo.IsStream() )
                checkStream( xObjStor, rInfo.GetName(), rInfo.GetSize() );
        }

        return true;
    }

    bool SotTest::load(const OUString &,
        const OUString &rURL, const OUString &,
        SfxFilterFlags, SotClipboardFormatId, unsigned int)
    {
        SvFileStream aStream(rURL, StreamMode::READ);
        rtl::Reference<SotStorage> xObjStor = new SotStorage(aStream);
        if (!xObjStor.is() || xObjStor->GetError())
            return false;

        CPPUNIT_ASSERT_MESSAGE("sot storage is not valid", xObjStor->Validate());
        return checkStorage (xObjStor);
    }

    void SotTest::test()
    {
        testDir(OUString(),
            m_directories.getURLFromSrc(u"/sot/qa/cppunit/data/"));
    }

    void SotTest::testSize()
    {
        OUString aURL(
            m_directories.getURLFromSrc(u"/sot/qa/cppunit/data/pass/fdo84229-1.compound"));
        SvFileStream aStream(aURL, StreamMode::READ);
        rtl::Reference<SotStorage> xObjStor = new SotStorage(aStream);
        CPPUNIT_ASSERT_MESSAGE("sot storage failed to open",
                               xObjStor.is());
        CPPUNIT_ASSERT_MESSAGE("sot storage failed to open",
                               !xObjStor->GetError());
        rtl::Reference<SotStorageStream> xStream = xObjStor->OpenSotStream("Book");
        CPPUNIT_ASSERT_MESSAGE("stream failed to open",
                               xStream.is());
        CPPUNIT_ASSERT_MESSAGE("stream failed to open",
                               !xObjStor->GetError());
        CPPUNIT_ASSERT_MESSAGE("error in opened stream", !xStream->GetError());
        sal_uInt64 nPos = xStream->GetSize();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("odd stream length", static_cast<sal_uInt64>(13312), nPos);

        xStream->Seek(STREAM_SEEK_TO_END);
        CPPUNIT_ASSERT_MESSAGE("error seeking to end", !xStream->GetError());
        // cf. comment in Pos2Page, not extremely intuitive ...
        CPPUNIT_ASSERT_EQUAL_MESSAGE("stream not at beginning", static_cast<sal_uInt64>(xStream->GetSize()), xStream->Tell());
        xStream->Seek(STREAM_SEEK_TO_BEGIN);

        CPPUNIT_ASSERT_MESSAGE("error seeking to beginning", !xStream->GetError());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("stream not at beginning", static_cast<sal_uInt64>(0), xStream->Tell());
    }

    void SotTest::testClipboard()
    {
        auto pFormats = sot::GetExchangeDestinationWriterFreeAreaCopy();
        // tdf#52547 prefer BITMAP over HTML
        // tdf#78801 prefer image over html over text
        CPPUNIT_ASSERT(FindFormatIndex(pFormats, SotClipboardFormatId::BITMAP) < FindFormatIndex(pFormats, SotClipboardFormatId::HTML));
        CPPUNIT_ASSERT(FindFormatIndex(pFormats, SotClipboardFormatId::HTML) < FindFormatIndex(pFormats, SotClipboardFormatId::STRING));
        // tdf#81835 prefer RTF/HTML over GDI Metafile
        CPPUNIT_ASSERT(FindFormatIndex(pFormats, SotClipboardFormatId::RTF) < FindFormatIndex(pFormats, SotClipboardFormatId::GDIMETAFILE));
        CPPUNIT_ASSERT(FindFormatIndex(pFormats, SotClipboardFormatId::HTML) < FindFormatIndex(pFormats, SotClipboardFormatId::GDIMETAFILE));
#ifndef MACOSX
        // tdf#115574 prefer RTF over BITMAP (Excel provides a BITMAP we can't
        // read, also Excel paste result used to be an editable table)
        CPPUNIT_ASSERT(FindFormatIndex(pFormats, SotClipboardFormatId::RTF) < FindFormatIndex(pFormats, SotClipboardFormatId::BITMAP));
#else
        CPPUNIT_ASSERT(FindFormatIndex(pFormats, SotClipboardFormatId::BITMAP) < FindFormatIndex(pFormats, SotClipboardFormatId::RTF));
#endif
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(SotTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
