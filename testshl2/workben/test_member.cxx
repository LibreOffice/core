/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test_member.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 03:47:48 $
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
