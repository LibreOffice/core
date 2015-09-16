/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>

#include <comphelper/makesequence.hxx>
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"

class MakeSequenceTest : public CppUnit::TestFixture
{
public:
    void testMakeSequence();
    CPPUNIT_TEST_SUITE(MakeSequenceTest);
    CPPUNIT_TEST(testMakeSequence);
    CPPUNIT_TEST_SUITE_END();
};

namespace {
  template< int... Ns > void _makeSequenceTestFunction() {
    sal_uInt32 packSize = sizeof...( Ns );
    ::com::sun::star::uno::Sequence< int > seq1( packSize );
    int arr[ sizeof...( Ns ) ] = { Ns... };
    int* p = arr;
    ::std::for_each( seq1.begin(), seq1.end(),
                     [&p]( int& val ) { return val = *p++; } );

    ::com::sun::star::uno::Sequence< int > seq2 = ::comphelper::makeSequence( Ns... );

    sal_uInt32 seq1Size = seq1.end() - seq1.begin();
    sal_uInt32 seq2Size = seq2.end() - seq2.begin();
    CPPUNIT_ASSERT_EQUAL( seq1Size, seq2Size );

    for( sal_uInt32 ix = 0; ix < seq1Size; ++ix ) {
      CPPUNIT_ASSERT_EQUAL( seq1[ix], seq2[ix] );
    }
  }
}

void MakeSequenceTest::testMakeSequence() {
  _makeSequenceTestFunction< 451, 336, 130, 255, 350, 907, 518, 161 >();
  _makeSequenceTestFunction< 773, 220, 635,867, 919 >();
  _makeSequenceTestFunction< 49, 688, 801, 457 >();
  _makeSequenceTestFunction< 746, 65, 908 >();
  _makeSequenceTestFunction< 588, 172 >();
  _makeSequenceTestFunction< 277 >();
}

CPPUNIT_TEST_SUITE_REGISTRATION(MakeSequenceTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
