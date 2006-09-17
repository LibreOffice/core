/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testHelperFunctions2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 09:04:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"
#include <cppunit/simpleheader.hxx>
#include "stringhelper.hxx"

namespace testOfHelperFunctions
{
    class test_valueequal : public CppUnit::TestFixture
    {
    public:
        void valueequal_001();

        CPPUNIT_TEST_SUITE( test_valueequal );
        CPPUNIT_TEST( valueequal_001 );
        CPPUNIT_TEST_SUITE_END( );
    };

    void test_valueequal::valueequal_001( )
    {
        rtl::OString sValue;
        rtl::OUString suValue(rtl::OUString::createFromAscii("This is only a test of some helper functions"));
        sValue <<= suValue;
        t_print("'%s'\n", sValue.getStr());
    }

} // namespace testOfHelperFunctions

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( testOfHelperFunctions::test_valueequal, "helperFunctions" );

// -----------------------------------------------------------------------------
// This is only allowed to be in one file!
// NOADDITIONAL;
