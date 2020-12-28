/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>

#ifndef _WIN32
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <memory>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/DocumentProperties.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <test/bootstrapfixture.hxx>
#include <test/xmltesttools.hxx>
#include <unotest/macros_test.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/app.hxx>
#include <osl/file.hxx>


using namespace ::com::sun::star;


namespace {

class MiscTest
    : public test::BootstrapFixture
    , public unotest::MacrosTest
    , public XmlTestTools
{
public:
    virtual void setUp() override;

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override
    {
        // ODF
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("office"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:office:1.0"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("meta"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:meta:1.0"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dc"), BAD_CAST("http://purl.org/dc/elements/1.1/"));
        // used in testCustomMetadata
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("foo"), BAD_CAST("http://foo.net"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("baz"), BAD_CAST("http://baz.net"));
    }
};

void MiscTest::setUp()
{
    m_xContext = comphelper::getProcessComponentContext();
    mxDesktop.set(frame::Desktop::create(m_xContext));
    SfxApplication::GetOrCreate();
}

CPPUNIT_TEST_FIXTURE(MiscTest, testODFCustomMetadata)
{
    uno::Reference<document::XDocumentProperties> const xProps(
        ::com::sun::star::document::DocumentProperties::create(m_xContext));

    OUString const url(m_directories.getURLFromSrc(u"/sfx2/qa/complex/sfx2/testdocuments/CUSTOM.odt"));
    xProps->loadFromMedium(url, uno::Sequence<beans::PropertyValue>());
    CPPUNIT_ASSERT_EQUAL(OUString(""), xProps->getAuthor());
    uno::Sequence<beans::PropertyValue> mimeArgs({
        beans::PropertyValue("MediaType", -1, uno::Any(OUString("application/vnd.oasis.opendocument.text")), beans::PropertyState_DIRECT_VALUE)
        });
    utl::TempFile aTempFile;
    xProps->storeToMedium(aTempFile.GetURL(), mimeArgs);

    // check that custom metadata is preserved
    uno::Reference<packages::zip::XZipFileAccess2> const xZip(
        packages::zip::ZipFileAccess::createWithURL(m_xContext, aTempFile.GetURL()));
    uno::Reference<io::XInputStream> const xInputStream(xZip->getByName("meta.xml"), uno::UNO_QUERY);
    std::unique_ptr<SvStream> const pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    assertXPathContent(pXmlDoc, "/office:document-meta/office:meta/bork", "bork");
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/foo:bar", 1);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/foo:bar/baz:foo", 1);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/foo:bar/baz:foo[@baz:bar='foo']");
    assertXPathContent(pXmlDoc, "/office:document-meta/office:meta/foo:bar/foo:baz", "bar");

    aTempFile.EnableKillingFile();
}

CPPUNIT_TEST_FIXTURE(MiscTest, testNoThumbnail)
{
    // Load a document.
    const OUString aURL(m_directories.getURLFromSrc(u"/sfx2/qa/cppunit/misc/hello.odt"));
    uno::Reference<lang::XComponent> xComponent
        = loadFromDesktop(aURL, "com.sun.star.text.TextDocument");
    CPPUNIT_ASSERT(xComponent.is());

    // Save it with the NoThumbnail option and assert that it has no thumbnail.
#ifndef _WIN32
    mode_t nMask = umask(022);
#endif
    uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xStorable.is());
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Sequence<beans::PropertyValue> aProperties(
        comphelper::InitPropertySequence({ { "NoThumbnail", uno::makeAny(true) } }));
    osl::File::remove(aTempFile.GetURL());
    xStorable->storeToURL(aTempFile.GetURL(), aProperties);
    uno::Reference<packages::zip::XZipFileAccess2> xZipFile
        = packages::zip::ZipFileAccess::createWithURL(m_xContext, aTempFile.GetURL());
    CPPUNIT_ASSERT(!xZipFile->hasByName("Thumbnails/thumbnail.png"));

#ifndef _WIN32
    // Check permissions of the URL after store.
    osl::DirectoryItem aItem;
    CPPUNIT_ASSERT_EQUAL(osl::DirectoryItem::E_None,
                         osl::DirectoryItem::get(aTempFile.GetURL(), aItem));

    osl::FileStatus aStatus(osl_FileStatus_Mask_Attributes);
    CPPUNIT_ASSERT_EQUAL(osl::DirectoryItem::E_None, aItem.getFileStatus(aStatus));

    // The following checks used to fail in the past, osl_File_Attribute_GrpRead was not set even if
    // umask requested so:
    CPPUNIT_ASSERT(aStatus.getAttributes() & osl_File_Attribute_GrpRead);
    CPPUNIT_ASSERT(aStatus.getAttributes() & osl_File_Attribute_OthRead);

    // Now "save as" again to trigger the "overwrite" case.
    xStorable->storeToURL(aTempFile.GetURL(), {});
    CPPUNIT_ASSERT_EQUAL(osl::DirectoryItem::E_None, aItem.getFileStatus(aStatus));
    // The following check used to fail in the past, result had temp file
    // permissions.
    CPPUNIT_ASSERT(aStatus.getAttributes() & osl_File_Attribute_GrpRead);

    umask(nMask);
#endif

    xComponent->dispose();
}

CPPUNIT_TEST_FIXTURE(MiscTest, testHardLinks)
{
#ifndef _WIN32
    OUString aSourceDir = m_directories.getURLFromSrc(u"/sfx2/qa/cppunit/misc/");
    OUString aTargetDir = m_directories.getURLFromWorkdir(u"/CppunitTest/sfx2_misc.test.user/");
    const OUString aURL(aTargetDir + "hello.odt");
    osl::File::copy(aSourceDir + "hello.odt", aURL);
    OUString aTargetPath;
    osl::FileBase::getSystemPathFromFileURL(aURL, aTargetPath);
    OString aOld = aTargetPath.toUtf8();
    aTargetPath += ".2";
    OString aNew = aTargetPath.toUtf8();
    int nRet = link(aOld.getStr(), aNew.getStr());
    CPPUNIT_ASSERT_EQUAL(0, nRet);

    uno::Reference<lang::XComponent> xComponent = loadFromDesktop(aURL, "com.sun.star.text.TextDocument");
    CPPUNIT_ASSERT(xComponent.is());

    uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
    xStorable->store();

    struct stat buf;
    // coverity[fs_check_call] - this is legitimate in the context of this test
    nRet = stat(aOld.getStr(), &buf);
    CPPUNIT_ASSERT_EQUAL(0, nRet);
    // This failed: hard link count was 1, the hard link broke on store.
    CPPUNIT_ASSERT(buf.st_nlink > 1);

    // Test that symlinks are preserved as well.
    nRet = remove(aNew.getStr());
    CPPUNIT_ASSERT_EQUAL(0, nRet);
    nRet = symlink(aOld.getStr(), aNew.getStr());
    CPPUNIT_ASSERT_EQUAL(0, nRet);
    xStorable->storeToURL(aURL + ".2", {});
    nRet = lstat(aNew.getStr(), &buf);
    CPPUNIT_ASSERT_EQUAL(0, nRet);
    // This failed, the hello.odt.2 symlink was replaced with a real file.
    CPPUNIT_ASSERT(bool(S_ISLNK(buf.st_mode)));

    xComponent->dispose();
#endif
}

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
