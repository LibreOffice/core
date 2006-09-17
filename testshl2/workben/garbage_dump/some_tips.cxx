/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: some_tips.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 03:50:35 $
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
#include <cppunit/simpleheader.hxx>

namespace rtl_ByteSequence
{
// -----------------------------------------------------------------------
// call this before any other classes
// -----------------------------------------------------------------------
class OSingleton
{
public:
    static OSingleton * GetSingleton();
    virtual ~OSingleton() {}
protected :
    OSingleton()
        {
            printf("Do something before.\n");
        }

    static OSingleton * m_pSingleton;
};

OSingleton * OSingleton::m_pSingleton = NULL;
OSingleton * OSingleton::GetSingleton()
{
    if(m_pSingleton  == NULL )
    {
        m_pSingleton = new OSingleton;
    }
    return m_pSingleton;
}

// -----------------------------------------------------------------------------

class before : public CppUnit::TestFixture
{
protected:
    before()
        {
            OSingleton::GetSingleton();
        }
};

// -----------------------------------------------------------------------------

class GlobalObject
{
public:
    GlobalObject()
        {
            // do nothing here
        }
    ~GlobalObject()
        {
            printf("shutdown");
        }
};

GlobalObject theGlobalObject;

// -----------------------------------------------------------------------------
class assign : public before
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    // insert your test code here.
    void assign_001()
    {
    }

    CPPUNIT_TEST_SUITE(assign);
    CPPUNIT_TEST(assign_001);
    CPPUNIT_TEST_SUITE_END();
}; // class operator=

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_ByteSequence::assign, "rtl_ByteSequence");
} // namespace ByteSequence


// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
void RegisterAdditionalFunctions(FktRegFuncPtr _pFunc)
{
    printf("init your work here\n");
}
// NOADDITIONAL;
