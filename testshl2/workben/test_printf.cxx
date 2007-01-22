/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test_printf.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-22 14:37:38 $
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

#include <string>
#include <stdarg.h>
#include <stdio.h>

void t_print(const char* _pFormatStr, va_list &args)
{
    vprintf(_pFormatStr, args);
}

void t_print(const char* _pFormatStr, ...)
{
    va_list  args;
    va_start( args, _pFormatStr );
    t_print(_pFormatStr, args);
    va_end( args );
}


// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int /* argc */ , char* /* argv */ [] )
#else
    int _cdecl main( int /* argc */ , char* /* argv */ [] )
#endif
{
    std::string sStr("TestString.");
    printf("Test %s\n", sStr.c_str() );
    t_print("Test %s\n", sStr.c_str() );
    return 0;
}
