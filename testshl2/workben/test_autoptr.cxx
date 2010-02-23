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

#include <stdlib.h>
#include <memory>
#include <iostream>

#include <stdio.h>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

class AutoPtrTest
{
    int m_nValue;

    // NOCOPY
    AutoPtrTest(AutoPtrTest const& );
    AutoPtrTest& operator =(AutoPtrTest const& );

public:
    AutoPtrTest(int _nValue = 1)
            :m_nValue(_nValue)
        {
            std::cout << "build Value" << std::endl;
        }
    ~AutoPtrTest()
        {
            std::cout << "release Value" << std::endl;
            m_nValue = 0;
        }
//    int operator int() const {return m_nValue;}
    int getValue() const {return m_nValue;}
};

std::auto_ptr<AutoPtrTest> getPtr()
{
    return std::auto_ptr<AutoPtrTest>(new AutoPtrTest(2));
}

// auto_ptr get() gibt den Pointer ohne das ownership
// auto_ptr release()gibt den Pointer und (!) das ownership

void test_autoptr()
{
    AutoPtrTest *pValuePtr;
    std::cout << "start test" << std::endl;
    {
        std::auto_ptr<AutoPtrTest> pValue = getPtr();
        std::cout << "Value with getValue() " << pValue->getValue() << std::endl;
        // std::cout << "Value operator int()  " << *pValue << std::endl;
        // ownership weiterreichen
        pValuePtr = pValue.release();
    }
    std::cout << "ValuePtr with getValue() " << pValuePtr->getValue() << std::endl;

    // ownership an neuen Pointer uebergeben
    std::auto_ptr<AutoPtrTest> pValue2(pValuePtr);

    // AutoPtrTest *pStr = getPtr();
    std::cout << "end test" << std::endl;
}

// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int /* argc */ , char* /* argv */ [] )
#else
    int _cdecl main( int /* argc*/ , char* /*argv*/ [] )
#endif
{
    test_autoptr();
    return 0;
}
