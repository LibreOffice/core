/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <config_oox.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <vcl/bitmapaccess.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <tools/stream.hxx>
#include <unotest/directories.hxx>
#include <comphelper/hash.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>

using namespace css;

namespace
{
class GraphicTest : public CppUnit::TestFixture
{
public:
    ~GraphicTest();

private:
    void testWMFRoundtrip();

    CPPUNIT_TEST_SUITE(GraphicTest);
    CPPUNIT_TEST(testWMFRoundtrip);
    CPPUNIT_TEST_SUITE_END();
};

GraphicTest::~GraphicTest()
{
}

void GraphicTest::testWMFRoundtrip()
{
    // Load a WMF file.
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc("vcl/qa/cppunit/data/roundtrip.wmf");
    SvFileStream aStream(aURL, StreamMode::READ);
    sal_uInt64 nPos = aStream.Tell();
    aStream.Seek(STREAM_SEEK_TO_END);
    sal_uInt64 nExpectedSize = aStream.Tell();
    aStream.Seek(nPos);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    rGraphicFilter.ImportGraphic(aGraphic, OUString(), aStream);

    // Save as WMF.
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    sal_uInt16 nFormat = rGraphicFilter.GetExportFormatNumberForShortName(u"WMF");
    SvStream& rOutStream = *aTempFile.GetStream(StreamMode::READWRITE);
    rGraphicFilter.ExportGraphic(aGraphic, OUString(), rOutStream, nFormat);

    // Check if we preserved the WMF data perfectly.
    nPos = rOutStream.Tell();
    rOutStream.Seek(STREAM_SEEK_TO_END);
    sal_uInt64 nActualSize = rOutStream.Tell();
    rOutStream.Seek(nPos);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less or equal than: 10
    // - Actual  : 3637
    // i.e. we lost most of the WMF data on roundtrip. Still allow loosing some padding bytes at the
    // very end, that's harmless.
    CPPUNIT_ASSERT_LESSEQUAL(static_cast<sal_uInt64>(10), nExpectedSize - nActualSize);
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(GraphicTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
