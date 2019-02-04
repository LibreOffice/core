/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>

#include <cppunit/extensions/HelperMacros.h>

#include <comphelper/processfactory.hxx>

#include <ucbhelper/content.hxx>

#include <test/bootstrapfixture.hxx>

#include <DirectoryStream.hxx>
#include <com/sun/star/ucb/XContent.hpp>

namespace ucb = com::sun::star::ucb;
namespace uno = com::sun::star::uno;

using std::unique_ptr;

using librevenge::RVNGInputStream;

using writerperfect::DirectoryStream;

namespace
{
class DirectoryStreamTest : public test::BootstrapFixture
{
public:
    DirectoryStreamTest();

public:
    CPPUNIT_TEST_SUITE(DirectoryStreamTest);
    CPPUNIT_TEST(testConstruction);
    CPPUNIT_TEST(testDetection);
    CPPUNIT_TEST(testDataOperations);
    CPPUNIT_TEST(testStructuredOperations);
    CPPUNIT_TEST_SUITE_END();

private:
    void testConstruction();
    void testDetection();
    void testDataOperations();
    void testStructuredOperations();

private:
    uno::Reference<ucb::XContent> m_xDir;
    uno::Reference<ucb::XContent> m_xFile;
    uno::Reference<ucb::XContent> m_xNonexistent;
};

static const char g_aDirPath[] = "/writerperfect/qa/unit/data/stream/test.dir";
static const char g_aNondirPath[] = "/writerperfect/qa/unit/data/stream/test.dir/mimetype";
static const char g_aNonexistentPath[] = "/writerperfect/qa/unit/data/stream/foo/bar";

DirectoryStreamTest::DirectoryStreamTest()
{
    const uno::Reference<ucb::XCommandEnvironment> xCmdEnv;
    const uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());

    using ucbhelper::Content;

    m_xDir = Content(m_directories.getURLFromSrc(g_aDirPath), xCmdEnv, xContext).get();
    m_xFile = Content(m_directories.getURLFromSrc(g_aNondirPath), xCmdEnv, xContext).get();
    m_xNonexistent
        = Content(m_directories.getURLFromSrc(g_aNonexistentPath), xCmdEnv, xContext).get();
}

void DirectoryStreamTest::testConstruction()
{
    const unique_ptr<DirectoryStream> pDir(DirectoryStream::createForParent(m_xFile));
    CPPUNIT_ASSERT(bool(pDir));
    CPPUNIT_ASSERT(pDir->isStructured());

    // this should work for dirs too
    const unique_ptr<DirectoryStream> pDir2(DirectoryStream::createForParent(m_xDir));
    CPPUNIT_ASSERT(bool(pDir2));
    CPPUNIT_ASSERT(pDir2->isStructured());

    // for nonexistent dirs nothing is created
    const unique_ptr<DirectoryStream> pNondir(DirectoryStream::createForParent(m_xNonexistent));
    CPPUNIT_ASSERT(!pNondir);

    // even if we try harder, just an empty shell is created
    DirectoryStream aNondir2(m_xNonexistent);
    CPPUNIT_ASSERT(!aNondir2.isStructured());
}

void DirectoryStreamTest::testDetection()
{
    CPPUNIT_ASSERT(DirectoryStream::isDirectory(m_xDir));
    CPPUNIT_ASSERT(!DirectoryStream::isDirectory(m_xFile));
    CPPUNIT_ASSERT(!DirectoryStream::isDirectory(m_xNonexistent));
}

void lcl_testDataOperations(RVNGInputStream& rStream)
{
    CPPUNIT_ASSERT(rStream.isEnd());
    CPPUNIT_ASSERT_EQUAL(0L, rStream.tell());
    CPPUNIT_ASSERT_EQUAL(-1, rStream.seek(0, librevenge::RVNG_SEEK_CUR));

    unsigned long numBytesRead = 0;
    CPPUNIT_ASSERT(!rStream.read(1, numBytesRead));
    CPPUNIT_ASSERT_EQUAL(0UL, numBytesRead);
}

void DirectoryStreamTest::testDataOperations()
{
    // data operations do not make sense on a directory -> just dummy
    // impls.
    DirectoryStream aDir(m_xDir);
    lcl_testDataOperations(aDir);

    // ... and they are equally empty if we try to pass a file
    DirectoryStream aFile(m_xFile);
    lcl_testDataOperations(aFile);
}

void lcl_testStructuredOperations(RVNGInputStream& rStream)
{
    CPPUNIT_ASSERT(rStream.isStructured());
    unique_ptr<RVNGInputStream> pSubstream(rStream.getSubStreamByName("mimetype"));
    CPPUNIT_ASSERT(bool(pSubstream));

    // TODO: test for other operations when they are implemented =)
}

void DirectoryStreamTest::testStructuredOperations()
{
    DirectoryStream aDir(m_xDir);
    lcl_testStructuredOperations(aDir);

    unique_ptr<DirectoryStream> pDir(DirectoryStream::createForParent(m_xFile));
    CPPUNIT_ASSERT(bool(pDir));
    lcl_testStructuredOperations(*pDir);
}

CPPUNIT_TEST_SUITE_REGISTRATION(DirectoryStreamTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
