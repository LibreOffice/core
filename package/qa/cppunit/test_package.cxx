/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/processfactory.hxx>
#include <unotest/filters-test.hxx>
#include <unotest/bootstrapfixturebase.hxx>
#include <comphelper/threadpool.hxx>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <iterator>

using namespace ::com::sun::star;

namespace
{
    class PackageTest
        : public test::FiltersTest
        , public test::BootstrapFixtureBase
    {
    public:
        PackageTest() {}

        virtual void setUp() override;

        virtual bool load(const OUString &,
            const OUString &rURL, const OUString &,
            SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

        void test();
        void testThreadedStreams();
        void testBufferedThreadedStreams();
        void testZip64();

        CPPUNIT_TEST_SUITE(PackageTest);
        CPPUNIT_TEST(test);
        CPPUNIT_TEST(testThreadedStreams);
        CPPUNIT_TEST(testBufferedThreadedStreams);
        CPPUNIT_TEST(testZip64);
        CPPUNIT_TEST_SUITE_END();

    private:
        uno::Reference<container::XNameAccess> mxNA;
        void verifyStreams( std::vector<std::vector<char>> &aBuffers );
    };

    void PackageTest::setUp()
    {
        BootstrapFixtureBase::setUp();
        OUString aURL = m_directories.getURLFromSrc(u"/package/qa/cppunit/data/a2z.zip");

        uno::Sequence<beans::NamedValue> aNVs{ { u"URL"_ustr, uno::Any(aURL) } };
        uno::Sequence<uno::Any> aArgs{ uno::Any(aNVs) };

        uno::Reference<uno::XComponentContext> xCxt = comphelper::getProcessComponentContext();
        uno::Reference<lang::XMultiComponentFactory> xSvcMgr = xCxt->getServiceManager();

        uno::Reference<packages::zip::XZipFileAccess2> xZip(
            xSvcMgr->createInstanceWithArgumentsAndContext(
                u"com.sun.star.packages.zip.ZipFileAccess"_ustr, aArgs, xCxt),
            uno::UNO_QUERY);

        CPPUNIT_ASSERT(xZip.is());

        mxNA = xZip;
        CPPUNIT_ASSERT(mxNA.is());
    }

    bool PackageTest::load(const OUString &,
        const OUString &rURL, const OUString &,
        SfxFilterFlags, SotClipboardFormatId, unsigned int)
    {
        try
        {
            uno::Reference<css::packages::zip::XZipFileAccess2> xZip(
                css::packages::zip::ZipFileAccess::createWithURL(comphelper::getProcessComponentContext(), rURL));
            return xZip.is();
        }
        catch(...)
        {
            return false;
        }
    }

    void PackageTest::test()
    {
        testDir(OUString(),
            m_directories.getURLFromSrc(u"/package/qa/cppunit/data/"));
    }

    void PackageTest::verifyStreams( std::vector<std::vector<char>> &aBuffers )
    {
            CPPUNIT_ASSERT_EQUAL(size_t(26), aBuffers.size());
            auto itBuf = aBuffers.begin();

            for (char c = 'a'; c <= 'z'; ++c, ++itBuf)
            {
                const std::vector<char>& rBuf = *itBuf;
                CPPUNIT_ASSERT_EQUAL(size_t(1048576), rBuf.size()); // 1 MB each.
                for (char check : rBuf)
                    if (c != check)
                        CPPUNIT_ASSERT_MESSAGE("stream does not contain expected data", false);
            }
    }

    // TODO : This test currently doesn't fail even when you set
    // UseBufferedStream to false. Look into this and replace it with a better
    // test that actually fails when the aforementioned flag is set to false.
    void PackageTest::testThreadedStreams()
    {
        class Worker : public comphelper::ThreadTask
        {
            uno::Reference<io::XInputStream> mxStrm;
            std::vector<char>& mrBuf;

        public:
            Worker(
                const std::shared_ptr<comphelper::ThreadTaskTag>& pTag,
                const uno::Reference<io::XInputStream>& xStrm,
                std::vector<char>& rBuf ) :
                comphelper::ThreadTask(pTag), mxStrm(xStrm), mrBuf(rBuf) {}

            virtual void doWork() override
            {
                sal_Int32 nSize = mxStrm->available();

                uno::Sequence<sal_Int8> aBytes;
                while (nSize > 0)
                {
                    sal_Int32 nBytesRead = mxStrm->readBytes(aBytes, 4096);
                    const sal_Int8* p = aBytes.getArray();
                    const sal_Int8* pEnd = p + nBytesRead;
                    std::copy(p, pEnd, std::back_inserter(mrBuf));
                    nSize -= nBytesRead;
                }
            }
        };

        {
            comphelper::ThreadPool aPool(4);
            std::shared_ptr<comphelper::ThreadTaskTag> pTag = comphelper::ThreadPool::createThreadTaskTag();

            std::vector<std::vector<char>> aTestBuffers(26);
            auto itBuf = aTestBuffers.begin();

            for (char c = 'a'; c <= 'z'; ++c, ++itBuf)
            {
                OUString aName = OUStringChar(c) + ".txt";

                uno::Reference<io::XInputStream> xStrm;
                mxNA->getByName(aName) >>= xStrm;

                CPPUNIT_ASSERT(xStrm.is());
                aPool.pushTask(std::make_unique<Worker>(pTag, xStrm, *itBuf));
            }

            aPool.waitUntilDone(pTag);
            verifyStreams( aTestBuffers );
        }
    }

    void PackageTest::testBufferedThreadedStreams()
    {
        std::vector<std::vector<char>> aTestBuffers(26);
        auto itBuf = aTestBuffers.begin();
        sal_Int32 nReadSize = 0;

        for (char c = 'a'; c <= 'z'; ++c, ++itBuf)
        {
            itBuf->reserve(1024*1024);
            OUString aName = OUStringChar(c) + ".txt";

            uno::Reference<io::XInputStream> xStrm;
            //Size of each stream is 1mb (>10000) => XBufferedThreadedStream
            mxNA->getByName(aName) >>= xStrm;

            CPPUNIT_ASSERT(xStrm.is());
            sal_Int32 nSize = xStrm->available();

            uno::Sequence<sal_Int8> aBytes;
            //Read chunks of increasing size
            nReadSize += 1024;

            while (nSize > 0)
            {
                sal_Int32 nBytesRead = xStrm->readBytes(aBytes, nReadSize);
                const sal_Int8* p = aBytes.getArray();
                const sal_Int8* pEnd = p + nBytesRead;
                std::copy(p, pEnd, std::back_inserter(*itBuf));
                nSize -= nBytesRead;
            }
        }

        verifyStreams( aTestBuffers );
    }

    void PackageTest::testZip64()
    {
        // This small zip file have 2 files (content.xml, styles.xml) that have
        // Zip64 Extended Information Extra Field in both
        // "Local file header" and "Central directory file header",
        // and have ZIP64 format "Data descriptor".
        OUString aURL2 = m_directories.getURLFromSrc(u"/package/qa/cppunit/data/export64.zip");

        uno::Sequence<beans::NamedValue> aNVs2{ { u"URL"_ustr, uno::Any(aURL2) } };
        uno::Sequence<uno::Any> aArgs2{ uno::Any(aNVs2) };

        uno::Reference<uno::XComponentContext> xCxt = comphelper::getProcessComponentContext();
        uno::Reference<lang::XMultiComponentFactory> xSvcMgr = xCxt->getServiceManager();

        // Without Zip64 support, it would crash here
        uno::Reference<packages::zip::XZipFileAccess2> xZip2(
            xSvcMgr->createInstanceWithArgumentsAndContext(
                u"com.sun.star.packages.zip.ZipFileAccess"_ustr, aArgs2, xCxt),
            uno::UNO_QUERY);

        CPPUNIT_ASSERT(xZip2.is());

        uno::Reference<container::XNameAccess> xNA;
        xNA = xZip2;
        CPPUNIT_ASSERT(xNA.is());

        // Check if the styles.xml seems to be right
        uno::Reference<io::XInputStream> xStrm;
        xNA->getByName(u"styles.xml"_ustr) >>= xStrm;
        CPPUNIT_ASSERT(xStrm.is());
        // Filesize check
        sal_Int32 nSize = xStrm->available();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1112), nSize);

        uno::Sequence<sal_Int8> aBytes;
        sal_Int32 nBytesRead = xStrm->readBytes(aBytes, nSize);
        const sal_Int8* p = aBytes.getArray();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1112), nBytesRead);

        // Check the uncompressed styles.xml file content.
        OString aFile(static_cast<const char*>(static_cast<const void*>(p)), nSize);
        CPPUNIT_ASSERT(aFile.startsWith(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<office:document-styles"));
        CPPUNIT_ASSERT(aFile.endsWith(
            "</number:time-style>\r\n </office:styles>\r\n</office:document-styles>\r\n"));
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(PackageTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
