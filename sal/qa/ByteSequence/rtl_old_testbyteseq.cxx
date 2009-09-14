/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rtl_old_testbyteseq.cxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"
// LLA:
// this file is converted to use with testshl2
// original was placed in sal/test/textenc.cxx
// -----------------------------------------------------------------------------

#include <stdio.h>

// #include <osl/diagnose.h>
#include <rtl/byteseq.hxx>

using namespace ::rtl;

#include <testshl/simpleheader.hxx>


namespace rtl_testbyteseq
{

// -----------------------------------------------------------------------------

class oldbyteseq : public CppUnit::TestFixture
{
public:
    void test_bytesequence_001();

    CPPUNIT_TEST_SUITE( oldbyteseq );
    CPPUNIT_TEST( test_bytesequence_001 );
    CPPUNIT_TEST_SUITE_END( );
};

// -----------------------------------------------------------------------------

void oldbyteseq::test_bytesequence_001()
{
    signed char a[5] = { 1 , 2 , 3 , 4 , 5 };

    // test the c++ wrapper
    {
        ByteSequence seq;
        OSL_ENSURE( ! seq.getLength() , "" );

        ByteSequence seq2( a , 5 );

        OSL_ENSURE( !( seq == seq2) , "" );

        seq = seq2;
        OSL_ENSURE( seq == seq2 , "" );

        seq[0] = 2;
        OSL_ENSURE( !(seq == seq2) , "" );

        seq = ByteSequence( a , 5 );
        OSL_ENSURE( seq == seq2 , "" );

        seq = ByteSequence( 5 ); // default value is 0 for each byte
        OSL_ENSURE( !( seq == seq2 ) , "" );
    }

    {
        sal_Sequence *pSeq = 0;
        rtl_byte_sequence_construct( &pSeq , 0 );

        // implementation dependent test.
        OSL_ENSURE( pSeq->nRefCount == 2 , "invalid refcount for empty sequence" );

        sal_Sequence *pSeq2 = 0;
        rtl_byte_sequence_constructFromArray( &pSeq2 , a , 5 );

        OSL_ENSURE( ! rtl_byte_sequence_equals( pSeq , pSeq2 ) , "" );

        rtl_byte_sequence_assign( &pSeq , pSeq2 );
        OSL_ENSURE( pSeq == pSeq2 , "" );
        OSL_ENSURE( rtl_byte_sequence_equals( pSeq , pSeq2 ) , "" );

        rtl_byte_sequence_reference2One( &pSeq );
        (( sal_Int8*) rtl_byte_sequence_getConstArray( pSeq ) )[0] = 2;

        OSL_ENSURE( ! rtl_byte_sequence_equals( pSeq , pSeq2 ) , "" );

        rtl_byte_sequence_constructFromArray( &pSeq , a , 5 );
        OSL_ENSURE( rtl_byte_sequence_equals( pSeq , pSeq2 ) , "" );

        rtl_byte_sequence_construct( &pSeq , 5 );
        OSL_ENSURE( ! rtl_byte_sequence_equals( pSeq , pSeq2 ) , "" );



        rtl_byte_sequence_release( pSeq2 );
        rtl_byte_sequence_release( pSeq );
    }


    printf( "test bytesequence OK\n" );

}

} // namespace osl_test_file

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( rtl_testbyteseq::oldbyteseq, "rtl_ByteSequence" );

// -----------------------------------------------------------------------------
NOADDITIONAL;
