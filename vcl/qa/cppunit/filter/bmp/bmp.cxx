/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <test/bootstrapfixture.hxx>

#include <tools/stream.hxx>
#include <unotest/directories.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>

using namespace com::sun::star;

namespace
{
char const DATA_DIRECTORY[] = "/vcl/qa/cppunit/filter/bmp/data/";

/// Covers vcl/source/gdi/dibtools.cxx fixes.
class Test : public test::BootstrapFixture
{
};

CPPUNIT_TEST_FIXTURE(Test, testTdf73523)
{
    GraphicFilter aGraphicFilter;
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "tdf73523.bmp";
    SvFileStream aStream(aURL, StreamMode::READ);
    Graphic aGraphic = aGraphicFilter.ImportUnloadedGraphic(aStream);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: c[80000000]
    // - Actual  : c[00000000]
    // i.e. the pixel is red not black
    CPPUNIT_ASSERT_EQUAL(COL_RED, aGraphic.GetBitmapEx().GetPixelColor(0, 0));
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
