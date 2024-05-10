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

#include <test/unoapixml_test.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/app.hxx>
#include <osl/file.hxx>


using namespace ::com::sun::star;


namespace {

class MiscTest
    : public UnoApiXmlTest
{
public:
    MiscTest()
        : UnoApiXmlTest(u"/sfx2/qa/cppunit/misc/"_ustr)
    {
    }

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

CPPUNIT_TEST_FIXTURE(MiscTest, testODFCustomMetadata)
{
    uno::Reference<document::XDocumentProperties> const xProps(
        ::com::sun::star::document::DocumentProperties::create(m_xContext));

    OUString const url(m_directories.getURLFromSrc(u"/sfx2/qa/complex/sfx2/testdocuments/CUSTOM.odt"));
    xProps->loadFromMedium(url, uno::Sequence<beans::PropertyValue>());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, xProps->getAuthor());
    uno::Sequence<beans::PropertyValue> mimeArgs({
        beans::PropertyValue(u"MediaType"_ustr, -1, uno::Any(u"application/vnd.oasis.opendocument.text"_ustr), beans::PropertyState_DIRECT_VALUE)
        });
    xProps->storeToMedium(maTempFile.GetURL(), mimeArgs);

    // check that custom metadata is preserved
    xmlDocUniquePtr pXmlDoc = parseExport(u"meta.xml"_ustr);
    assertXPathContent(pXmlDoc, "/office:document-meta/office:meta/bork"_ostr, u"bork"_ustr);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/foo:bar"_ostr, 1);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/foo:bar/baz:foo"_ostr, 1);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/foo:bar/baz:foo[@baz:bar='foo']"_ostr);
    assertXPathContent(pXmlDoc, "/office:document-meta/office:meta/foo:bar/foo:baz"_ostr, u"bar"_ustr);
}

CPPUNIT_TEST_FIXTURE(MiscTest, testNoThumbnail)
{
    // Load a document.
    loadFromFile(u"hello.odt");

    // Save it with the NoThumbnail option and assert that it has no thumbnail.
#ifndef _WIN32
    mode_t nMask = umask(022);
#endif
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xStorable.is());
    uno::Sequence<beans::PropertyValue> aProperties(
        comphelper::InitPropertySequence({ { "NoThumbnail", uno::Any(true) } }));
    osl::File::remove(maTempFile.GetURL());
    xStorable->storeToURL(maTempFile.GetURL(), aProperties);
    uno::Reference<packages::zip::XZipFileAccess2> xZipFile
        = packages::zip::ZipFileAccess::createWithURL(m_xContext, maTempFile.GetURL());
    CPPUNIT_ASSERT(!xZipFile->hasByName(u"Thumbnails/thumbnail.png"_ustr));

#ifndef _WIN32
    // Check permissions of the URL after store.
    osl::DirectoryItem aItem;
    CPPUNIT_ASSERT_EQUAL(osl::DirectoryItem::E_None,
                         osl::DirectoryItem::get(maTempFile.GetURL(), aItem));

    osl::FileStatus aStatus(osl_FileStatus_Mask_Attributes);
    CPPUNIT_ASSERT_EQUAL(osl::DirectoryItem::E_None, aItem.getFileStatus(aStatus));

    // The following checks used to fail in the past, osl_File_Attribute_GrpRead was not set even if
    // umask requested so:
    CPPUNIT_ASSERT(aStatus.getAttributes() & osl_File_Attribute_GrpRead);
    CPPUNIT_ASSERT(aStatus.getAttributes() & osl_File_Attribute_OthRead);

    // Now "save as" again to trigger the "overwrite" case.
    xStorable->storeToURL(maTempFile.GetURL(), {});
    CPPUNIT_ASSERT_EQUAL(osl::DirectoryItem::E_None, aItem.getFileStatus(aStatus));
    // The following check used to fail in the past, result had temp file
    // permissions.
    CPPUNIT_ASSERT(aStatus.getAttributes() & osl_File_Attribute_GrpRead);

    umask(nMask);
#endif
}

CPPUNIT_TEST_FIXTURE(MiscTest, testHardLinks)
{
#ifndef _WIN32
    OUString aTargetDir = m_directories.getURLFromWorkdir(u"/CppunitTest/sfx2_misc.test.user/");
    const OUString aURL(aTargetDir + "hello.odt");
    osl::File::copy(createFileURL(u"hello.odt"), aURL);
    OUString aTargetPath;
    osl::FileBase::getSystemPathFromFileURL(aURL, aTargetPath);
    OString aOld = aTargetPath.toUtf8();
    aTargetPath += ".2";
    OString aNew = aTargetPath.toUtf8();
    int nRet = link(aOld.getStr(), aNew.getStr());
    CPPUNIT_ASSERT_EQUAL(0, nRet);

    mxComponent = loadFromDesktop(aURL, u"com.sun.star.text.TextDocument"_ustr);

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
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
#endif
}

CPPUNIT_TEST_FIXTURE(MiscTest, testOverwrite)
{
    // tdf#60237 - try to overwrite an existing file using the different settings of the Overwrite option
    mxComponent
        = loadFromDesktop(maTempFile.GetURL(), u"com.sun.star.text.TextDocument"_ustr);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xStorable.is());

    // overwrite the file using the default case of the Overwrite option (true)
    CPPUNIT_ASSERT_NO_THROW(xStorable->storeToURL(maTempFile.GetURL(), {}));

    // explicitly overwrite the file using the Overwrite option
    CPPUNIT_ASSERT_NO_THROW(xStorable->storeToURL(
        maTempFile.GetURL(),
        comphelper::InitPropertySequence({ { "Overwrite", uno::Any(true) } })));

    try
    {
        // overwrite an existing file with the Overwrite flag set to false
        xStorable->storeToURL(maTempFile.GetURL(), comphelper::InitPropertySequence(
                                                      { { "Overwrite", uno::Any(false) } }));
        CPPUNIT_ASSERT_MESSAGE("We expect an exception on overwriting an existing file", false);
    }
    catch (const css::uno::Exception&)
    {
    }
}

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
