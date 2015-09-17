/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <boost/property_tree/json_parser.hpp>
#include <cppunit/TestFixture.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cstdlib>
#include <string>
#include <stdio.h>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>

#include <config_options.h>
    // see use of ENABLE_RUNTIME_OPTIMIZATIONS in LibreOfficeKintInit.h
#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKitInit.h>
#include <LibreOfficeKit/LibreOfficeKit.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

using namespace ::boost;
using namespace ::lok;
using namespace ::std;

OUString getFileURLFromSystemPath(OUString const & path)
{
    OUString url;
    osl::FileBase::RC e = osl::FileBase::getFileURLFromSystemPath(path, url);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, e);
    if (!url.endsWith("/"))
        url += "/";
    return url;
}

// We specifically don't use the usual BootStrapFixture, as LOK does
// all it's own setup and bootstrapping, and should be useable in a
// raw C++ program.
class TiledRenderingTest : public ::CppUnit::TestFixture
{
public:
    const string m_sSrcRoot;
    const string m_sInstDir;
    const string m_sLOPath;

    TiledRenderingTest()
        : m_sSrcRoot( getenv( "SRC_ROOT" ) )
        , m_sInstDir( getenv( "INSTDIR" ) )
        , m_sLOPath( m_sInstDir + "/program" )
    {
    }

    // Currently it isn't possible to do multiple startup/shutdown
    // cycle of LOK in a single process -- hence we run all our tests
    // as one test, which simply carries out the individual test
    // components on the one Office instance that we retrieve.
    void runAllTests();

    void testDocumentLoadFail( Office* pOffice );
    void testDocumentTypes( Office* pOffice );
    void testImpressSlideNames( Office* pOffice );
    void testCalcSheetNames( Office* pOffice );
#if 0
    void testOverlay( Office* pOffice );
#endif

    CPPUNIT_TEST_SUITE(TiledRenderingTest);
    CPPUNIT_TEST(runAllTests);
    CPPUNIT_TEST_SUITE_END();
};

void TiledRenderingTest::runAllTests()
{
    // set UserInstallation to user profile dir in test/user-template
    const char* pWorkdirRoot = getenv("WORKDIR_FOR_BUILD");
    OUString aWorkdirRootPath = OUString::createFromAscii(pWorkdirRoot);
    OUString aWorkdirRootURL = getFileURLFromSystemPath(aWorkdirRootPath);
    OUString sUserInstallURL = aWorkdirRootURL + "/unittest";
    rtl::Bootstrap::set(OUString("UserInstallation"), sUserInstallURL);

    std::unique_ptr< Office > pOffice( lok_cpp_init(
                                      m_sLOPath.c_str() ) );
    CPPUNIT_ASSERT( pOffice.get() );

    testDocumentLoadFail( pOffice.get() );
    testDocumentTypes( pOffice.get() );
    testImpressSlideNames( pOffice.get() );
    testCalcSheetNames( pOffice.get() );
#if 0
    testOverlay( pOffice.get() );
#endif
}

void TiledRenderingTest::testDocumentLoadFail( Office* pOffice )
{
    const string sDocPath = m_sSrcRoot + "/libreofficekit/qa/data/IDONOTEXIST.odt";
    std::unique_ptr< Document> pDocument( pOffice->documentLoad( sDocPath.c_str() ) );
    CPPUNIT_ASSERT( !pDocument.get() );
    // TODO: we probably want to have some way of returning what
    // the cause of failure was. getError() will return
    // something along the lines of:
    // "Unsupported URL <file:///SRC_ROOT/libreofficekit/qa/data/IDONOTEXIST.odt>: "type detection failed""
}

// Our dumped .png files end up in
// workdir/CppunitTest/libreofficekit_tiledrendering.test.core

int getDocumentType( Office* pOffice, const string& rPath )
{
    std::unique_ptr< Document> pDocument( pOffice->documentLoad( rPath.c_str() ) );
    CPPUNIT_ASSERT( pDocument.get() );
    return pDocument->getDocumentType();
}

void TiledRenderingTest::testDocumentTypes( Office* pOffice )
{
    const string sTextDocPath = m_sSrcRoot + "/libreofficekit/qa/data/blank_text.odt";
    const string sTextLockFile = m_sSrcRoot +"/libreofficekit/qa/data/.~lock.blank_text.odt#";

    // FIXME: same comment as below wrt lockfile removal.
    remove( sTextLockFile.c_str() );

    std::unique_ptr<Document> pDocument(pOffice->documentLoad( sTextDocPath.c_str()));
    CPPUNIT_ASSERT(pDocument.get());
    CPPUNIT_ASSERT_EQUAL(LOK_DOCTYPE_TEXT, static_cast<LibreOfficeKitDocumentType>(pDocument->getDocumentType()));
    // This crashed.
    pDocument->postUnoCommand(".uno:Bold");

    const string sPresentationDocPath = m_sSrcRoot + "/libreofficekit/qa/data/blank_presentation.odp";
    const string sPresentationLockFile = m_sSrcRoot +"/libreofficekit/qa/data/.~lock.blank_presentation.odp#";

    // FIXME: same comment as below wrt lockfile removal.
    remove( sPresentationLockFile.c_str() );

    CPPUNIT_ASSERT( getDocumentType( pOffice, sPresentationDocPath ) == LOK_DOCTYPE_PRESENTATION );

    // TODO: do this for all supported document types
}

void TiledRenderingTest::testImpressSlideNames( Office* pOffice )
{
    const string sDocPath = m_sSrcRoot + "/libreofficekit/qa/data/impress_slidenames.odp";
    const string sLockFile = m_sSrcRoot +"/libreofficekit/qa/data/.~lock.impress_slidenames.odp#";

    // FIXME: this is a temporary hack: LOK will fail when trying to open a
    // locked file, and since we're reusing the file for a different unit
    // test it's entirely possible that an unwanted lock file will remain.
    // Hence forcefully remove it here.
    remove( sLockFile.c_str() );

    std::unique_ptr< Document> pDocument( pOffice->documentLoad( sDocPath.c_str() ) );

    CPPUNIT_ASSERT( pDocument->getParts() == 3 );
    CPPUNIT_ASSERT( strcmp( pDocument->getPartName( 0 ), "TestText1" ) == 0 );
    CPPUNIT_ASSERT( strcmp( pDocument->getPartName( 1 ), "TestText2" ) == 0 );
    // The third slide hasn't had a name given to it (i.e. using the rename
    // context menu in Impress), thus it should (as far as I can determine)
    // have a localised version of "Slide 3".
}

void TiledRenderingTest::testCalcSheetNames( Office* pOffice )
{
    const string sDocPath = m_sSrcRoot + "/libreofficekit/qa/data/calc_sheetnames.ods";
    const string sLockFile = m_sSrcRoot +"/libreofficekit/qa/data/.~lock.calc_sheetnames.ods#";

    // FIXME: LOK will fail when trying to open a locked file
    remove( sLockFile.c_str() );

    std::unique_ptr< Document> pDocument( pOffice->documentLoad( sDocPath.c_str() ) );

    CPPUNIT_ASSERT( pDocument->getParts() == 3 );
    CPPUNIT_ASSERT( strcmp( pDocument->getPartName( 0 ), "TestText1" ) == 0 );
    CPPUNIT_ASSERT( strcmp( pDocument->getPartName( 1 ), "TestText2" ) == 0 );
    CPPUNIT_ASSERT( strcmp( pDocument->getPartName( 2 ), "Sheet3" ) == 0 );
}

#if 0
static void dumpRGBABitmap( const OUString& rPath, const unsigned char* pBuffer,
                            const int nWidth, const int nHeight )
{
    Bitmap aBitmap( Size( nWidth, nHeight ), 32 );
    Bitmap::ScopedWriteAccess pWriteAccess( aBitmap );
    memcpy( pWriteAccess->GetBuffer(), pBuffer, 4*nWidth*nHeight );

    BitmapEx aBitmapEx( aBitmap );
    vcl::PNGWriter aWriter( aBitmapEx );
    SvFileStream sOutput( rPath, StreamMode::WRITE );
    aWriter.Write( sOutput );
    sOutput.Close();
}

void TiledRenderingTest::testOverlay( Office* /*pOffice*/ )
{
    const string sDocPath = m_sSrcRoot + "/odk/examples/java/DocumentHandling/test/test1.odt";
    const string sLockFile = m_sSrcRoot + "/odk/examples/java/DocumentHandling/test/.~lock.test1.odt#";

    // FIXME: this is a temporary hack: LOK will fail when trying to open a
    // locked file, and since we're reusing the file for a different unit
    // test it's entirely possible that an unwanted lock file will remain.
    // Hence forcefully remove it here.
    remove( sLockFile.c_str() );
    std::unique_ptr< Office > pOffice( lok_cpp_init(
                                      m_sLOPath.c_str() ) );
    assert( pOffice.get() );

    std::unique_ptr< Document> pDocument( pOffice->documentLoad(
                                         sDocPath.c_str() ) );

    if ( !pDocument.get() )
    {
        fprintf( stderr, "documentLoad failed: %s\n", pOffice->getError() );
        CPPUNIT_FAIL( "Document could not be loaded -- tiled rendering not possible." );
    }

    // We render one large tile, then subdivide it into 4 and render those parts, and finally
    // iterate over each smaller tile and check whether their contents match the large
    // tile.
    const int nTotalWidthPix = 512;
    const int nTotalHeightPix = 512;
    int nRowStride;

    long nTotalWidthDoc;
    long nTotalHeightDoc;
    // pDocument->getDocumentSize( &nTotalWidthDoc, &nTotalHeightDoc );
    // TODO: make sure we select an actually interesting part of the document
    // for this comparison, i.e. ideally an image and lots of text, in order
    // to test as many edge cases as possible.
    // Alternatively we could rewrite this to actually grab the document size
    // and iterate over it (subdividing into an arbitrary number of tiles rather
    // than our less sophisticated test of just 4 sub-tiles).
    nTotalWidthDoc = 8000;
    nTotalHeightDoc = 9000;

    std::unique_ptr< unsigned char []> pLarge( new unsigned char[ 4*nTotalWidthPix*nTotalHeightPix ] );
    pDocument->paintTile( pLarge.get(),  nTotalWidthPix, nTotalHeightPix, &nRowStride,
                          0, 0,
                          nTotalWidthDoc, nTotalHeightDoc );
    dumpRGBABitmap( "large.png", pLarge.get(), nTotalWidthPix, nTotalHeightPix );

    std::unique_ptr< unsigned char []> pSmall[4];
    for ( int i = 0; i < 4; i++ )
    {
        pSmall[i].reset( new unsigned char[ 4*(nTotalWidthPix/2)*(nTotalHeightPix/2) ] );
        pDocument->paintTile( pSmall[i].get(),  nTotalWidthPix / 2, nTotalHeightPix / 2, &nRowStride,
                              // Tile 0/2: left. Tile 1/3: right. Tile 0/1: top. Tile 2/3: bottom
                              ((i%2 == 0) ?  0 : nTotalWidthDoc / 2), ((i < 2 ) ? 0 : nTotalHeightDoc / 2),
                              nTotalWidthDoc / 2, nTotalHeightDoc / 2);
        dumpRGBABitmap( "small_" + OUString::number(i) + ".png",
                        pSmall[i].get(), nTotalWidthPix/2, nTotalHeightPix/2 );
    }

    // Iterate over each pixel of the sub-tile, and compare that pixel for every
    // tile with the equivalent super-tile pixel.
    for ( int i = 0; i < 4*nTotalWidthPix / 2 * nTotalHeightPix / 2; i++ )
    {
        int xSmall = i % (4*nTotalWidthPix/2);
        int ySmall = i / (4*nTotalWidthPix/2);
        // Iterate over our array of tiles
        // However for now we only bother with the top-left
        // tile as the other ones don't match yet...
        for ( int x = 0; x < 2; x++ )
        {
            for ( int y = 0; y < 2; y++ )
            {
                int xLarge = (x * (4 * nTotalWidthPix / 2)) + xSmall;
                int yLarge = (y * (nTotalHeightPix / 2)) + ySmall;
                CPPUNIT_ASSERT( pSmall[2*y+x][i] == pLarge[yLarge*4*nTotalWidthPix + xLarge] );
            }
        }
    }
}
#endif

CPPUNIT_TEST_SUITE_REGISTRATION(TiledRenderingTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
