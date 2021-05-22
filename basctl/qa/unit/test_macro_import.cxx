/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>

namespace
{
class Macro_Import : public CppUnit::TestFixture
{
public:

    void testTdf139196();

    CPPUNIT_TEST_SUITE(Macro_Import);

    CPPUNIT_TEST(testTdf139196);

    CPPUNIT_TEST_SUITE_END();
};

void Macro_Import::testTdf139196()
{
    // How I import such a file using this specific method, since the edit engine already uses utf8
}

CPPUNIT_TEST_SUITE_REGISTRATION(Macro_Import);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
