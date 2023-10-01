/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <docmodel/theme/Theme.hxx>

namespace
{
class ThemeTest : public CppUnit::TestFixture
{
};

CPPUNIT_TEST_FIXTURE(ThemeTest, testPitchFamilyConversion)
{
    model::ThemeFont aFont;
    aFont.maPitch = 2;
    aFont.maFamily = 1;

    CPPUNIT_ASSERT_EQUAL(sal_Int16(0x12), aFont.getPitchFamily());

    aFont.maPitch = sal_Int16(0x7FF2); // only lower 4-bit
    aFont.maFamily = sal_Int16(0x7FF3); // only lower 4-bit

    CPPUNIT_ASSERT_EQUAL(sal_Int16(0x32), aFont.getPitchFamily());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
