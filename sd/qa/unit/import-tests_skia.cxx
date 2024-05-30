/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "sdmodeltestbase.hxx"

#include <vcl/skia/SkiaHelper.hxx>

// Tests here are expected to run with Skia enabled, similar to CppunitTest_vcl_skia.
// At some point, this could be true for all tests?

class SdImportTestSkia : public SdModelTestBase
{
public:
    SdImportTestSkia()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SdImportTestSkia, testTdf156856)
{
    // Loading the document must not fail assertion
    createSdImpressDoc("pptx/tdf156856.pptx");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
