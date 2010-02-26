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
