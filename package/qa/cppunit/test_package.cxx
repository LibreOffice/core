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

        virtual bool load(const OUString &,
            const OUString &rURL, const OUString &,
            SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

        void test();
        void testThreadedStreams();

        CPPUNIT_TEST_SUITE(PackageTest);
        CPPUNIT_TEST(test);
        CPPUNIT_TEST(testThreadedStreams);
        CPPUNIT_TEST_SUITE_END();
    };

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
            m_directories.getURLFromSrc("/package/qa/cppunit/data/"));
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

        OUString aURL = m_directories.getURLFromSrc("/package/qa/cppunit/data/a2z.zip");

        uno::Sequence<beans::NamedValue> aNVs(2);
        aNVs[0].Name = "URL";
        aNVs[0].Value <<= aURL;
        aNVs[1].Name = "UseBufferedStream";
        aNVs[1].Value <<= true;

        uno::Sequence<uno::Any> aArgs(1);
        aArgs[0] <<= aNVs;

        uno::Reference<uno::XComponentContext> xCxt = comphelper::getProcessComponentContext();
        uno::Reference<lang::XMultiComponentFactory> xSvcMgr = xCxt->getServiceManager();

        uno::Reference<packages::zip::XZipFileAccess2> xZip(
            xSvcMgr->createInstanceWithArgumentsAndContext(
                "com.sun.star.packages.zip.ZipFileAccess", aArgs, xCxt),
            uno::UNO_QUERY);

        CPPUNIT_ASSERT(xZip.is());

        uno::Reference<container::XNameAccess> xNA(xZip, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xNA.is());

        struct TestThreadPool
        {
            comphelper::ThreadPool aPool;
            TestThreadPool(sal_Int32 const i) : aPool(i) {}
            ~TestThreadPool() { aPool.shutdown(); }
        };

        {
            TestThreadPool aPool(4);
            comphelper::ThreadPool & rPool(aPool.aPool);
            std::shared_ptr<comphelper::ThreadTaskTag> pTag = comphelper::ThreadPool::createThreadTaskTag();

            std::vector<std::vector<char>> aTestBuffers(26);
            auto itBuf = aTestBuffers.begin();

            for (char c = 'a'; c <= 'z'; ++c, ++itBuf)
            {
                OUString aName(c);
                aName += ".txt";

                uno::Reference<io::XInputStream> xStrm;
                xNA->getByName(aName) >>= xStrm;

                CPPUNIT_ASSERT(xStrm.is());
                rPool.pushTask(new Worker(pTag, xStrm, *itBuf));
            }

            rPool.waitUntilDone(pTag);

            // Verify the streams.
            CPPUNIT_ASSERT_EQUAL(size_t(26), aTestBuffers.size());
            itBuf = aTestBuffers.begin();

            for (char c = 'a'; c <= 'z'; ++c, ++itBuf)
            {
                const std::vector<char>& rBuf = *itBuf;
                CPPUNIT_ASSERT_EQUAL(size_t(1048576), rBuf.size()); // 1 MB each.
                for (char check : rBuf)
                    CPPUNIT_ASSERT_EQUAL(c, check);
            }
        }
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(PackageTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
