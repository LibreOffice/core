/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <vcl/svapp.hxx>

namespace
{
CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, LocalDigits)
{
    // Test the case where there are no digits to convert
    OUString sNoDigits = "hello";
    sal_Int32 nLen = sNoDigits.getLength();
    // It should return the same string
    CPPUNIT_ASSERT_EQUAL(reinterpret_cast<sal_uIntPtr>(sNoDigits.getStr()),
                         reinterpret_cast<sal_uIntPtr>(
                             LocalizeDigitsInString(sNoDigits, LANGUAGE_FARSI, 0, nLen).getStr()));
    // The length should not change
    CPPUNIT_ASSERT_EQUAL(sNoDigits.getLength(), nLen);

    // Test the case where there are digits but they are already correct for the locale.
    OUString sDozen = "There are 12 eggs in a dozen.";
    nLen = sDozen.getLength();
    // It should return the same string
    CPPUNIT_ASSERT_EQUAL(reinterpret_cast<sal_uIntPtr>(sDozen.getStr()),
                         reinterpret_cast<sal_uIntPtr>(
                             LocalizeDigitsInString(sDozen, LANGUAGE_SYSTEM, 0, nLen).getStr()));
    // The length should not change
    CPPUNIT_ASSERT_EQUAL(sDozen.getLength(), nLen);

    // Test an actual conversion
    CPPUNIT_ASSERT_EQUAL(u"There are ۱۲ eggs in a dozen."_ustr,
                         LocalizeDigitsInString(sDozen, LANGUAGE_FARSI, 0, nLen));
    CPPUNIT_ASSERT_EQUAL(sDozen.getLength(), nLen);

    // Test converting a subrange
    nLen = 12;
    CPPUNIT_ASSERT_EQUAL(
        u"There are ۱۲ eggs in a dozen but 13 in a baker's dozen."_ustr,
        LocalizeDigitsInString("There are 12 eggs in a dozen but 13 in a baker's dozen.",
                               LANGUAGE_FARSI, 2, nLen));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12), nLen);

    // Test with characters outside of the bmp
    CPPUNIT_ASSERT_EQUAL(
        u"𐑞𐑺 𐑸 ۱۲ 𐑧𐑜𐑟 𐑦𐑯 𐑩 𐑚𐑱𐑒𐑼𐑟 𐑛𐑳𐑟𐑩𐑯."_ustr,
        LocalizeDigitsInString(u"𐑞𐑺 𐑸 12 𐑧𐑜𐑟 𐑦𐑯 𐑩 𐑚𐑱𐑒𐑼𐑟 𐑛𐑳𐑟𐑩𐑯."_ustr, LANGUAGE_FARSI));
}
} // namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
