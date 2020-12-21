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
char const DATA_DIRECTORY[] = "/vcl/qa/cppunit/filter/igif/data/";

/// Covers vcl/source/filter/igif/ fixes.
class Test : public test::BootstrapFixture
{
};

CPPUNIT_TEST_FIXTURE(Test, testLogicLazyRead)
{
    GraphicFilter aGraphicFilter;
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(DATA_DIRECTORY) + "logic-lazy-read.gif";
    SvFileStream aStream(aURL, StreamMode::READ);
    Graphic aGraphic = aGraphicFilter.ImportUnloadedGraphic(aStream);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 10
    // i.e. the preferred unit was pixels, not mm100.
    CPPUNIT_ASSERT_EQUAL(MapUnit::Map100thMM, aGraphic.GetPrefMapMode().GetMapUnit());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
