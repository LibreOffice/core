/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <atomic>
#include <memory>
#include <thread>
#include <boost/property_tree/json_parser.hpp>
#include <cppunit/TestFixture.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cstdlib>
#include <string>
#include <stdio.h>
#include <unistd.h>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>

#include <com/sun/star/awt/Key.hpp>

#if defined __clang__ && defined __linux__
#include <cxxabi.h>
#include <config_options.h>
#if defined _LIBCPPABI_VERSION || !ENABLE_RUNTIME_OPTIMIZATIONS
#define KIT_LOADLIB_GLOBAL
#endif
#endif

#include <COKit/COKitInit.h>
#include <COKit/COKit.hxx>

using namespace ::boost;
using namespace ::kit;
using namespace ::std;

namespace {

void processEventsToIdle()
{
    typedef void (ProcessEventsToIdleFn)(void);
    static ProcessEventsToIdleFn *processFn = nullptr;
    if (!processFn)
    {
        void *me = dlopen(nullptr, RTLD_NOW);
        processFn = reinterpret_cast<ProcessEventsToIdleFn *>(dlsym(me, "unit_kit_process_events_to_idle"));
    }

    CPPUNIT_ASSERT(processFn);

    (*processFn)();
}

void insertString(COKitDocument& rDocument, const std::string& s)
{
    for (const char c : s)
    {
        rDocument.postKeyEvent(COKitKeyEventType::DOWN, c, 0);
        rDocument.postKeyEvent(COKitKeyEventType::UP, c, 0);
        processEventsToIdle();
    }
}

}

static OUString getFileURLFromSystemPath(OUString const & path)
{
    OUString url;
    osl::FileBase::RC e = osl::FileBase::getFileURLFromSystemPath(path, url);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, e);
    if (!url.endsWith("/"))
        url += "/";
    return url;
}

// We specifically don't use the usual BootStrapFixture, as COKit does
// all its own setup and bootstrapping, and should be usable in a
// raw C++ program.
class TiledRenderingTest : public ::CppUnit::TestFixture
{
public:
    const string m_sSrcRoot;
    const string m_sInstDir;
    const string m_sLOPath;

    std::unique_ptr<COKitDocument> loadDocument( COKit *pOffice, const string &pName,
                                            const char *pFilterOptions = nullptr );

    TiledRenderingTest()
        : m_sSrcRoot( getenv( "SRC_ROOT" ) )
        , m_sInstDir( getenv( "INSTDIR" ) )
        , m_sLOPath( m_sInstDir + "/program" )
    {
    }

    // Currently it isn't possible to do multiple startup/shutdown
    // cycle of COKit in a single process -- hence we run all our tests
    // as one test, which simply carries out the individual test
    // components on the one Office instance that we retrieve.
    void runAllTests();

    void testDocumentLoadFail( COKit* pOffice );
    void testDocumentTypes( COKit* pOffice );
    void testImpressSlideNames( COKit* pOffice );
    void testCalcSheetNames( COKit* pOffice );
    void testPaintPartTile( COKit* pOffice );
    void testDocumentLoadLanguage(COKit* pOffice);
    void testMultiKeyInput(COKit *pOffice);
    void testSetDocumentPasswordAfterLoad(COKit* pOffice);
    void testSaveTemplateContentUnderMismatchedExtension(COKit* pOffice);
#if 0
    void testOverlay( COKit* pOffice );
#endif

    CPPUNIT_TEST_SUITE(TiledRenderingTest);
    CPPUNIT_TEST(runAllTests);
    CPPUNIT_TEST_SUITE_END();
};

void TiledRenderingTest::runAllTests()
{
    // Use a private user-installation directory for this test. It must NOT be
    // the shared workdir/unittest template that every CppunitTest copies its
    // profile from: kit_cpp_init() initializes the profile from the shipped
    // presets (including autotbl.fmt with predefined table styles), so pointing
    // UserInstallation at the shared template pollutes it and breaks unrelated
    // tests that assume a pristine profile (e.g. the sw/sc autoformat tests).
    const char* pWorkdirRoot = getenv("WORKDIR_FOR_BUILD");
    OUString aWorkdirRootPath = OUString::createFromAscii(pWorkdirRoot);
    OUString aWorkdirRootURL = getFileURLFromSystemPath(aWorkdirRootPath);
    OUString sUserInstallURL = aWorkdirRootURL + "/unittest_kit";
    rtl::Bootstrap::set(u"UserInstallation"_ustr, sUserInstallURL);

    std::unique_ptr<COKit> pOffice( kit_cpp_init(
                                      m_sLOPath.c_str() ) );
    CPPUNIT_ASSERT( pOffice );

    // A failure in any one of these must still reach the cleanup below: skip
    // it and the process crashes on shutdown instead of reporting a normal
    // test failure (see the comment on that cleanup for why). So run them
    // under a catch-all and defer acting on a failure until after cleanup.
    bool bAllPassed = true;
    string sFailureWhat;
    try
    {
        testDocumentLoadFail( pOffice.get() );
        testSaveTemplateContentUnderMismatchedExtension(pOffice.get());
        testDocumentTypes( pOffice.get() );
        testMultiKeyInput(pOffice.get());
        testImpressSlideNames( pOffice.get() );
        testCalcSheetNames( pOffice.get() );
        testPaintPartTile( pOffice.get() );
        testDocumentLoadLanguage(pOffice.get());
        testSetDocumentPasswordAfterLoad(pOffice.get());
#if 0
        testOverlay( pOffice.get() );
#endif
    }
    catch (const std::exception& e)
    {
        bAllPassed = false;
        sFailureWhat = e.what();
    }

    // tdf#113311: all tests have passed at this point. Destroy the Office and
    // then leave the process via _Exit() to skip C++ global destructors and
    // atexit handlers. The COKit lifecycle does not support an in-process
    // teardown: static singletons such as sw's SwDLLInstance are destroyed at
    // exit and tear down a document whose item-pool vtable has already gone,
    // causing a pure-virtual call / use-after-free that used to abort this
    // otherwise-green test on shutdown. Exiting here is the workaround
    // suggested in the bug for running this test reliably.
    pOffice.reset();

    if (!bAllPassed)
    {
        fprintf(stderr, "TiledRenderingTest::runAllTests failed: %s\n", sFailureWhat.c_str());
        std::_Exit(EXIT_FAILURE);
    }
    std::_Exit(EXIT_SUCCESS);
}

void TiledRenderingTest::testDocumentLoadFail( COKit* pOffice )
{
    const string sDocPath = m_sSrcRoot + "/kit/qa/data/IDONOTEXIST.odt";
    std::unique_ptr<COKitDocument> pDocument( pOffice->documentLoad( sDocPath.c_str() ) );
    CPPUNIT_ASSERT( !pDocument );
    // TODO: we probably want to have some way of returning what
    // the cause of failure was. getError() will return
    // something along the lines of:
    // "Unsupported URL <file:///SRC_ROOT/kit/qa/data/IDONOTEXIST.odt>: "type detection failed""
}

// Our dumped .png files end up in
// workdir/CppunitTest/kit_tiledrendering.test.core

static COKitDocumentType getDocumentType( COKit* pOffice, const string& rPath )
{
    std::unique_ptr<COKitDocument> pDocument( pOffice->documentLoad( rPath.c_str() ) );
    CPPUNIT_ASSERT( pDocument );
    return pDocument->getDocumentType();
}

std::unique_ptr<COKitDocument> TiledRenderingTest::loadDocument( COKit *pOffice, const string &pName,
                                                            const char *pFilterOptions )
{
    const string sDocPath = m_sSrcRoot + "/kit/qa/data/" + pName;
    const string sLockFile = m_sSrcRoot +"/kit/qa/data/.~lock." + pName + "#";

    remove( sLockFile.c_str() );

    return std::unique_ptr<COKitDocument>(
        pOffice->documentLoadWithOptions(sDocPath.c_str(), pFilterOptions));
}

void TiledRenderingTest::testDocumentTypes( COKit* pOffice )
{
    std::unique_ptr<COKitDocument> pDocument(loadDocument(pOffice, "blank_text.odt"));

    CPPUNIT_ASSERT(pDocument);
    CPPUNIT_ASSERT_EQUAL(COKitDocumentType::TEXT, pDocument->getDocumentType());
    // This crashed.
    pDocument->postUnoCommand(".uno:Bold", nullptr, false);
    processEventsToIdle();

    const string sPresentationDocPath = m_sSrcRoot + "/kit/qa/data/blank_presentation.odp";
    const string sPresentationLockFile = m_sSrcRoot +"/kit/qa/data/.~lock.blank_presentation.odp#";

    // FIXME: same comment as below wrt lockfile removal.
    remove( sPresentationLockFile.c_str() );

    CPPUNIT_ASSERT_EQUAL(COKitDocumentType::PRESENTATION, getDocumentType(pOffice, sPresentationDocPath));

    // TODO: do this for all supported document types
}

void TiledRenderingTest::testImpressSlideNames( COKit* pOffice )
{
    std::unique_ptr<COKitDocument> pDocument(loadDocument(pOffice, "impress_slidenames.odp"));

    CPPUNIT_ASSERT_EQUAL(3, pDocument->getParts());
    CPPUNIT_ASSERT_EQUAL(std::string("TestText1"), std::string(pDocument->getPartName(0)));
    CPPUNIT_ASSERT_EQUAL(std::string("TestText2"), std::string(pDocument->getPartName(1)));
    // The third slide hasn't had a name given to it (i.e. using the rename
    // context menu in Impress), thus it should (as far as I can determine)
    // have a localised version of "Slide 3".
}

void TiledRenderingTest::testCalcSheetNames( COKit* pOffice )
{
    std::unique_ptr<COKitDocument> pDocument(loadDocument(pOffice, "calc_sheetnames.ods"));

    CPPUNIT_ASSERT_EQUAL(3, pDocument->getParts());
    CPPUNIT_ASSERT_EQUAL(std::string("TestText1"), std::string(pDocument->getPartName(0)));
    CPPUNIT_ASSERT_EQUAL(std::string("TestText2"), std::string(pDocument->getPartName(1)));
    CPPUNIT_ASSERT_EQUAL(std::string("Sheet3"), std::string(pDocument->getPartName(2)));
}

void TiledRenderingTest::testPaintPartTile(COKit* pOffice)
{
    std::unique_ptr<COKitDocument> pDocument(loadDocument(pOffice, "blank_text.odt"));

    CPPUNIT_ASSERT(pDocument);
    CPPUNIT_ASSERT_EQUAL(COKitDocumentType::TEXT, pDocument->getDocumentType());

    // Create two views.
    pDocument->getView();
    pDocument->createViewWithOptions(nullptr);

    int nView2 = pDocument->getView();

    // Destroy the current view
    pDocument->destroyView(nView2);

    int nCanvasWidth = 256;
    int nCanvasHeight = 256;
    std::vector<unsigned char> aBuffer(nCanvasWidth * nCanvasHeight * 4);

    // And try to paintPartTile() - this used to crash when the current viewId
    // was destroyed
    pDocument->paintPartTile(aBuffer.data(), /*nPart=*/0, /*nMode=*/0, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0, /*nTilePosY=*/0, /*nTileWidth=*/3840, /*nTileHeight=*/3840);
}

void TiledRenderingTest::testDocumentLoadLanguage(COKit* pOffice)
{
    std::unique_ptr<COKitDocument> pDocument(loadDocument(pOffice, "blank_text.odt", "Language=en-US"));

    // assert that '.' is the decimal separator
    insertString(*pDocument, "1.5");

    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, css::awt::Key::RIGHT);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 0, css::awt::Key::RIGHT);
    processEventsToIdle();

    insertString(*pDocument, "=2*A1");

    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, css::awt::Key::RETURN);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 0, css::awt::Key::RETURN);
    processEventsToIdle();
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, css::awt::Key::UP);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 0, css::awt::Key::UP);
    processEventsToIdle();

#if 0
    // FIXME disabled, as occasionally fails
    // we've got a meaningful result
    OString aResult = pDocument->getTextSelection("text/plain;charset=utf-8", nullptr);
    CPPUNIT_ASSERT_EQUAL(OString("3\n"), aResult);

    pDocument.reset();

    // FIXME: COKit will fail when trying to open a locked file
    remove(sLockFile.c_str());

    // load the file again, now in another language
    pDocument.reset(pOffice->documentLoadWithOptions(sDocPath.c_str(), "Language=cs-CZ"));

    // with cs-CZ, the decimal separator is ',' instead, assert that
    insertString(*pDocument, "1,5");

    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, css::awt::Key::RIGHT);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 0, css::awt::Key::RIGHT);
    processEventsToIdle();

    insertString(*pDocument, "=2*A1");

    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, css::awt::Key::RETURN);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 0, css::awt::Key::RETURN);
    processEventsToIdle();
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, css::awt::Key::UP);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 0, css::awt::Key::UP);
    processEventsToIdle();

    // we've got a meaningful result
    aResult = pDocument->getTextSelection("text/plain;charset=utf-8", nullptr);
    CPPUNIT_ASSERT_EQUAL(OString("3\n"), aResult);
#endif
}

#if 0
static void dumpRGBABitmap( const OUString& rPath, const unsigned char* pBuffer,
                            const int nWidth, const int nHeight )
{
    Bitmap aBitmap( Size( nWidth, nHeight ), 32 );
    BitmapScopedWriteAccess pWriteAccess( aBitmap );
    memcpy( pWriteAccess->GetBuffer(), pBuffer, 4*nWidth*nHeight );

    Bitmap aBitmap( aBitmap );
    vcl::PNGWriter aWriter( aBitmap );
    SvFileStream sOutput( rPath, StreamMode::WRITE );
    aWriter.Write( sOutput );
    sOutput.Close();
}

void TiledRenderingTest::testOverlay( COKit* /*pOffice*/ )
{
    const string sDocPath = m_sSrcRoot + "/odk/examples/java/DocumentHandling/test/test1.odt";
    const string sLockFile = m_sSrcRoot + "/odk/examples/java/DocumentHandling/test/.~lock.test1.odt#";

    // FIXME: this is a temporary hack: COKit will fail when trying to open a
    // locked file, and since we're reusing the file for a different unit
    // test it's entirely possible that an unwanted lock file will remain.
    // Hence forcefully remove it here.
    remove( sLockFile.c_str() );
    std::unique_ptr<COKit> pOffice( kit_cpp_init(
                                      m_sLOPath.c_str() ) );
    assert( pOffice.get() );

    std::unique_ptr<COKitDocument> pDocument( pOffice->documentLoad(
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

void TiledRenderingTest::testMultiKeyInput(COKit *pOffice)
{
    std::unique_ptr<COKitDocument> pDocument(loadDocument(pOffice, "blank_text.odt"));

    CPPUNIT_ASSERT(pDocument);
    CPPUNIT_ASSERT_EQUAL(COKitDocumentType::TEXT, pDocument->getDocumentType());

    // Create two views.
    int nViewA = pDocument->getView();
    pDocument->initializeForRendering("{\".uno:Author\":{\"type\":\"string\",\"value\":\"jill\"}}");

    pDocument->createViewWithOptions(nullptr);
    int nViewB = pDocument->getView();
    pDocument->initializeForRendering("{\".uno:Author\":{\"type\":\"string\",\"value\":\"jack\"}}");

    // Enable change tracking
    pDocument->postUnoCommand(".uno:TrackChanges", nullptr, false);

    // First a key-stroke from a
    pDocument->setView(nViewA);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 97, 0); // a
    pDocument->postKeyEvent(COKitKeyEventType::UP, 0, 512);   // 'a

    // A space on 'a' - force commit
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 32, 0); // ' '
    pDocument->postKeyEvent(COKitKeyEventType::UP, 0, 1284);   // '' '

    // Another 'a'
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 97, 0); // a
    pDocument->postKeyEvent(COKitKeyEventType::UP, 0, 512);   // 'a

    // FIXME: Wait for writer input handler to commit that.
    // without this we fall foul of edtwin's KeyInputFlushTimer
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Quickly a new key-stroke from b
    pDocument->setView(nViewB);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 98, 0); // b
    pDocument->postKeyEvent(COKitKeyEventType::UP, 0, 514);   // 'b

    // A space on 'b' - force commit
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 32, 0); // ' '
    pDocument->postKeyEvent(COKitKeyEventType::UP, 0, 1284);   // '' '

    // Wait for writer input handler to commit that.
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // get track changes ?
    std::string values = pDocument->getCommandValues(".uno:AcceptTrackedChanges");
    std::cerr << "Values: '" << values << "'\n";
}

// Regression test: a password request can be raised after the initial load has
// returned - e.g. when a reload switches the document to editable. In that case
// the kit answers it by calling setDocumentPassword() for the document's URL.
// The load interaction handler used to be removed from the interaction map as
// soon as documentLoad() returned, so this lookup found nothing and dereferenced
// an end() iterator, crashing in KitInteractionHandler::SetPassword(). The
// handler must stay registered for the whole document lifetime.
void TiledRenderingTest::testSetDocumentPasswordAfterLoad(COKit* pOffice)
{
    const string sDocPath = m_sSrcRoot + "/kit/qa/data/blank_text.odt";
    const string sLockFile = m_sSrcRoot + "/kit/qa/data/.~lock.blank_text.odt#";
    remove(sLockFile.c_str());

    // documentLoad() turns its argument into an absolute URL and uses that as
    // the interaction-map key, so pass a file URL and reuse the exact same
    // string for setDocumentPassword() below.
    OUString aDocURL;
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None,
        osl::FileBase::getFileURLFromSystemPath(
            OUString::createFromAscii(sDocPath), aDocURL));
    const OString aDocURLUtf8 = aDocURL.toUtf8();

    std::unique_ptr<COKitDocument> pDocument(pOffice->documentLoad(aDocURLUtf8.getStr()));
    CPPUNIT_ASSERT(pDocument);

    // Supplying a password now (as the kit does in response to a post-load
    // password request) must not crash. nullptr means "no password given".
    pOffice->setDocumentPassword(aDocURLUtf8.getStr(), nullptr);
}

namespace {

// Reads one entry out of a zip-format file into an OString, for checking the
// raw XML of an OOXML part after it has been saved to disk. Shells out to
// unzip rather than pulling in the UNO zip package service, so this stays as
// light on dependencies as the rest of this COKit-only test binary.
OString readZipEntry(const string& rZipPath, const string& rEntryName)
{
    const string sCommand
        = "/usr/bin/unzip -p '" + rZipPath + "' '" + rEntryName + "' 2>&1";
    FILE* pPipe = popen(sCommand.c_str(), "r");
    CPPUNIT_ASSERT(pPipe);

    OStringBuffer aBuffer;
    char aChunk[4096];
    size_t nRead;
    while ((nRead = fread(aChunk, 1, sizeof(aChunk), pPipe)) > 0)
        aBuffer.append(aChunk, nRead);
    const int nExit = pclose(pPipe);

    const OString aResult = aBuffer.makeStringAndClear();
    const string sDiag = sCommand + " -> exit " + std::to_string(nExit) + ", output: "
        + aResult.getStr();
    CPPUNIT_ASSERT_MESSAGE(sDiag, !aResult.isEmpty());
    return aResult;
}

std::atomic<bool> gSaveResultSeen(false);

void saveResultCallback(COKitCallbackType eType, const char* /*pPayload*/, void* /*pData*/)
{
    if (eType == COKitCallbackType::UNO_COMMAND_RESULT)
        gSaveResultSeen = true;
}

}

void TiledRenderingTest::testSaveTemplateContentUnderMismatchedExtension(COKit* pOffice)
{
    // Regression test for cool#12091: online creates a presentation from a
    // .potx template by downloading the template's own bytes to a path
    // named after the target document, so the file that gets loaded has a
    // .pptx name but .potx content. Load such a file the same way and check
    // that a plain save (no special filter arguments, as an ordinary
    // autosave would issue) writes out the non-template presentation
    // content type, not the template one the file's content was originally
    // detected as.
    const string sTemplatePath = m_sSrcRoot + "/kit/qa/data/blank_presentation_template.potx";
    const string sWorkPath = string(getenv("WORKDIR_FOR_BUILD")) + "/cool12091.pptx";
    const string sLockFile = string(getenv("WORKDIR_FOR_BUILD")) + "/.~lock.cool12091.pptx#";
    remove(sLockFile.c_str());

    {
        FILE* pIn = fopen(sTemplatePath.c_str(), "rb");
        CPPUNIT_ASSERT(pIn);
        FILE* pOut = fopen(sWorkPath.c_str(), "wb");
        CPPUNIT_ASSERT(pOut);
        char aBuf[4096];
        size_t nRead;
        while ((nRead = fread(aBuf, 1, sizeof(aBuf), pIn)) > 0)
            CPPUNIT_ASSERT_EQUAL(nRead, fwrite(aBuf, 1, nRead, pOut));
        fclose(pIn);
        fclose(pOut);
    }

    std::unique_ptr<COKitDocument> pDocument(
        pOffice->documentLoadWithOptions(sWorkPath.c_str(), nullptr));
    CPPUNIT_ASSERT(pDocument);

    pDocument->registerCallback(saveResultCallback, nullptr);
    gSaveResultSeen = false;
    pDocument->postUnoCommand(".uno:Save", nullptr, true);
    for (int i = 0; i < 1000 && !gSaveResultSeen; ++i)
    {
        processEventsToIdle();
        if (!gSaveResultSeen)
            usleep(1000);
    }
    CPPUNIT_ASSERT_MESSAGE("timed out waiting for .uno:Save to complete",
                          gSaveResultSeen.load());
    pDocument.reset();

    // unzip's own pattern matching treats [ and ] as a character class, so
    // the literal brackets in this entry's name need escaping.
    const OString aContentTypes = readZipEntry(sWorkPath, "\\[Content_Types\\].xml");
    CPPUNIT_ASSERT_MESSAGE(
        string("expected the non-template presentation content type; got: ")
            + aContentTypes.getStr(),
        aContentTypes.indexOf("presentationml.presentation.main+xml"_ostr) != -1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1),
        aContentTypes.indexOf("presentationml.template.main+xml"_ostr));
}

CPPUNIT_TEST_SUITE_REGISTRATION(TiledRenderingTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
