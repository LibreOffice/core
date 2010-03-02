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
#include <iostream>
#include <string>

using namespace std;

class Aint
{
    int n;

public:

    Aint():n(0)
        {
            cout << "ctor Aint()" << endl;
        }
    Aint(int _n):n(_n)
        {
            cout << "ctor Aint(int(" << _n << "))" << endl;
        }
    int getA() {return n;}
    ~Aint()
        {
            cout << "dtor Aint()" << endl;
        }

};

// -----------------------------------------------------------------------------

class B
{
    Aint a;
    Aint b;
    std::string aStr;

public:

    B()
            :a(2)
        {
            cout << "ctor B()" << endl;
        }
    B(int _a)
            :a(_a)
        {
            cout << "ctor B(int)" << endl;
        }

    void foo()
        {
            cout << "a.getA() := " << a.getA() << endl;
            cout << "b.getA() := " << b.getA() << endl;
        }

};


// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int argc, char* argv[] )
#else
int _cdecl main( int argc, char* argv[] )
#endif
{
    B b(4);
    b.foo();
}
