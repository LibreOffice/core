/*************************************************************************
 *
 *  $RCSfile: rtl_old_testbyteseq.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-03 08:55:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
// LLA:
// this file is converted to use with testshl2
// original was placed in sal/test/textenc.cxx
// -----------------------------------------------------------------------------

#include <stdio.h>

// #include <osl/diagnose.h>
#include <rtl/byteseq.hxx>

using namespace ::rtl;

#include <cppunit/simpleheader.hxx>


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
