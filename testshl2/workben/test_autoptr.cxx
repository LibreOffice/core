/*************************************************************************
 *
 *  $RCSfile: test_autoptr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: lla $ $Date: 2003-01-09 11:46:15 $
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
int main( int argc, char* argv[] )
#else
int _cdecl main( int argc, char* argv[] )
#endif
{
    test_autoptr();
    return 0;
}
