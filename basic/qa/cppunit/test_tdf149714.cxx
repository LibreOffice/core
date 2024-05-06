/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <basic/sbstar.hxx>
#include <basic/sbmeth.hxx>
#include <cppunit/extensions/HelperMacros.h>

#include <unotest/directories.hxx>

namespace
{
class TestTdf149714 : public CppUnit::TestFixture
{
    void testBitsPerPixel();

    CPPUNIT_TEST_SUITE(TestTdf149714);
    CPPUNIT_TEST(testBitsPerPixel);
    CPPUNIT_TEST_SUITE_END();

    StarBASICRef interpreter;

    SbModuleRef Module()
    {
        test::Directories aDirectories;
        OUString aDataFileName
            = aDirectories.getURLFromSrc(u"basic/qa/cppunit/data/") + u"tdf149714.png";
        OUString sBasic = uR"BAS(

Function GetBitsPerPixelAsString As String
    DIM oProps(0) As New "com.sun.star.beans.PropertyValue"
    DIM oProvider, oGraphic

    oProps(0).Name = "URL"
    oProps(0).Value = "$PNGFILENAME"

    oProvider = createUnoService("com.sun.star.graphic.GraphicProvider")
    oGraphic = oProvider.queryGraphic(oProps())

    GetBitsPerPixelAsString = oGraphic.BitsPerPixel

End Function

)BAS"_ustr;

        sBasic = sBasic.replaceFirst("$PNGFILENAME", aDataFileName);

        interpreter = new StarBASIC();
        auto mod = interpreter->MakeModule(u"BitsPerPixel"_ustr, sBasic);

        CPPUNIT_ASSERT(mod->Compile());
        CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, StarBASIC::GetErrBasic());
        CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, SbxBase::GetError());
        CPPUNIT_ASSERT(mod->IsCompiled());
        return mod;
    }
};

void TestTdf149714::testBitsPerPixel()
{
    auto m = Module();
    auto GetBitsPerPixelAsString
        = m->FindMethod(u"GetBitsPerPixelAsString"_ustr, SbxClassType::Method);
    CPPUNIT_ASSERT_MESSAGE("Could not Find GetBitsPerPixelAsString in module",
                           GetBitsPerPixelAsString != nullptr);

    SbxVariableRef returned = new SbxMethod{ *GetBitsPerPixelAsString };
    CPPUNIT_ASSERT(returned->IsString());

    // Without the fix in place this would fail with:
    // - Expected: 24
    // - Actual:   True
    CPPUNIT_ASSERT_EQUAL(u"24"_ustr, returned->GetOUString());
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(TestTdf149714);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
