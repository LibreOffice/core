/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
