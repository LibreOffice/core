/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 09:09:51 $
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
#include "precompiled_sal.hxx"

#include <stdio.h>

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    void test_int64();
    test_int64();

    void test_profile(void);
    test_profile();

    void test_OString();
    test_OString();

/*      void test_OWString(); */
/*      test_OWString(); */

/*      void test_OStringBuffer(); */
/*      test_OStringBuffer(); */

/*      void test_OWStringBuffer(); */
/*      test_OWStringBuffer(); */

/*      void test_OString2OWStringAndViceVersa(); */
/*      test_OString2OWStringAndViceVersa(); */

    void test_uuid();
    test_uuid();

    return(0);
}


