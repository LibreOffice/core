/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test_ostringstream.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-22 14:35:24 $
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
#include <sstream>
#include <iostream>
#include <bitset>

#include <sal/types.h>


class TestFactoryRegistry
{
public:
    void checkFunc(char* foo);
    void checkFunc2(char* foo);
    void checkFunc3(char* foo);
};


void TestFactoryRegistry::checkFunc(char* /* foo */ )
{
    static int serialNumber = 1;

    std::ostringstream ost;
//! could this be the problem?
    ost << "@Dummy@" << serialNumber++;
    std::cout << ost.str() << std::endl;
}

void TestFactoryRegistry::checkFunc2(char* /* foo */)
{
    static int serialNumber = 1;

    std::ostringstream ost;
//! could this be the problem?
    ost << "@Dummy@" << serialNumber;
    serialNumber++;
    std::cout << ost.str() << std::endl;
}

void TestFactoryRegistry::checkFunc3(char* /* foo */)
{
    static int serialNumber = 1;

    int nValue = serialNumber;
    std::ostringstream ost;
//! could this be the problem?
    ost << "@Dummy@" << nValue;
    serialNumber++;
    std::cout << ost.str() << std::endl;
}

static TestFactoryRegistry a;
static TestFactoryRegistry b;
static TestFactoryRegistry c;

// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int /* argc */ , char* /* argv */ [] )
#else
    int _cdecl main( int /* argc */, char* /* argv */ [] )
#endif
{
    std::ostringstream ost;
    // sal_Int32 nValue = 1;

    // ost << "@Dummy@" << nValue;
    // ost << "dec: " << 15 << std::hex << " hex: " << 15 << std::endl;

    // std::cout << ost.str() << std::endl;

    char* cFoo = NULL;

    a.checkFunc(cFoo);
    a.checkFunc2(cFoo);
    a.checkFunc3(cFoo);

    b.checkFunc(cFoo);
    b.checkFunc2(cFoo);
    b.checkFunc3(cFoo);

    c.checkFunc(cFoo);
    c.checkFunc2(cFoo);
    c.checkFunc3(cFoo);

    return 0;
}
