/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test_autoptr.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-22 14:34:34 $
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
