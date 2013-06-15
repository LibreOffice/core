/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>

#include <vcl/wrkwin.hxx>

#include <osl/file.hxx>
#include <osl/process.h>

class VclComplexTextTest : public test::BootstrapFixture
{
public:
    VclComplexTextTest() : BootstrapFixture(true, false) {}

    /// Play with font measuring etc.
    void testArabic();

    CPPUNIT_TEST_SUITE(VclComplexTextTest);
    CPPUNIT_TEST(testArabic);
    CPPUNIT_TEST_SUITE_END();
};

void VclComplexTextTest::testArabic()
{
#if !defined (LINUX)
    return;
#else // only tested on Linux so far
    OUString aOneTwoThree( "واحِدْ إثٍنين ثلاثةٌ" );
    Window* pWin = new WorkWindow( (Window *)NULL );
    CPPUNIT_ASSERT( pWin != NULL );

    OutputDevice *pOutDev = static_cast< OutputDevice * >( pWin );

    Font aFont = OutputDevice::GetDefaultFont(
                        DEFAULTFONT_CTL_SPREADSHEET,
                        LANGUAGE_ARABIC_SAUDI_ARABIA,
                        DEFAULTFONT_FLAGS_ONLYONE, 0 );
    pOutDev->SetFont( aFont );

    // normal orientation
    Rectangle aInput;
    Rectangle aRect = pOutDev->GetTextRect( aInput, aOneTwoThree );

    // now rotate 270 degress
    Font aRotated( aFont );
    aRotated.SetOrientation( 2700 );
    pOutDev->SetFont( aRotated );
    Rectangle aRectRot = pOutDev->GetTextRect( aInput, aOneTwoThree );

    // Check that we did do the rotation ...
#if 0
    fprintf( stderr, "%ld %ld %ld %ld\n",
             aRect.GetWidth(), aRect.GetHeight(),
             aRectRot.GetWidth(), aRectRot.GetHeight() );
    CPPUNIT_ASSERT( aRectRot.GetWidth() == aRect.GetHeight() );
    CPPUNIT_ASSERT( aRectRot.GetHeight() == aRect.GetWidth() );
#else
    (void)aRect; (void)aRectRot;
#endif
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclComplexTextTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
