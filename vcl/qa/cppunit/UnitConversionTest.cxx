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

#include <tools/fldunit.hxx>
#include <tools/mapunit.hxx>
#include <vcl/fieldvalues.hxx>

namespace
{
CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, UnitConversionTest)
{
    CPPUNIT_ASSERT_EQUAL(sal_Int64(100),
                         vcl::ConvertAndScaleValue(10000, 0, MapUnit::Map100thMM, FieldUnit::MM));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(1000),
                         vcl::ConvertAndScaleValue(10000, 1, MapUnit::Map100thMM, FieldUnit::MM));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(10000),
                         vcl::ConvertAndScaleValue(10000, 2, MapUnit::Map100thMM, FieldUnit::MM));
}
} // namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
