/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: skeleton.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 03:51:04 $
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
#include "precompiled_testshl2.hxx"
/*
 * LLA: 20040527
 */

#ifndef CPPUNIT_SIMPLEHEADER_HXX
#include <cppunit/simpleheader.hxx>
#endif

namespace skeleton
{
//------------------------------------------------------------------------
// testing constructors
//------------------------------------------------------------------------

    class  ctor : public CppUnit::TestFixture
    {
    public:
        ctor()
            {
                printf("ctor: called\n");
            }

        void ctor_test()
            {
                printf("ctor test: called\n");
            }
        void ctor_test2()
            {
                printf("ctor test2: called\n");
            }
        void ctor_test3()
            {
                printf("ctor test3: called\n");
            }

        CPPUNIT_TEST_SUITE(ctor);
        CPPUNIT_TEST(ctor_test);
        CPPUNIT_TEST(ctor_test2);
        CPPUNIT_TEST(ctor_test3);
        CPPUNIT_TEST_SUITE_END();
    };

    // -----------------------------------------------------------------------------
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(skeleton::ctor, "skeleton");
} // unotest

// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.

NOADDITIONAL;
