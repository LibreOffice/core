/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: std_outp.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 14:30:23 $
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

#include <precomp.h>
#include <cosv/std_outp.hxx>

// NOT FULLY DECLARED SERVICES
#include <stdlib.h>


namespace csv
{

#ifdef CSV_NO_IOSTREAMS
redirect_out *  redirect_out::pStdOut_ = 0;
redirect_out *  redirect_out::pStdErr_ = 0;
#endif // defined(CSV_NO_IOSTREAMS)


void
Endl( ostream & io_rStream, bool /*bUseRedirect*/, int* )
{
#ifndef CSV_NO_IOSTREAMS
//    if (NOT bUseRedirect)
        io_rStream << std::endl;
//    else
#endif
//        static_cast< redirect_out& >(io_rStream).re_endl();
}

void
Flush( ostream & io_rStream, bool /*bUseRedirect*/, int* )
{
#ifndef CSV_NO_IOSTREAMS
//    if (NOT bUseRedirect)
        io_rStream << std::flush;
//    else
#endif
//        static_cast< redirect_out& >(io_rStream).re_flush();
}

void
PerformAssertion(const char * condition, const char * file, unsigned line)
{
    Cout() << "assertion failed: "
         << condition
         << " in file: "
         << file
         << " at line: "
         << line
         << Endl;

    exit(3);
}

}   // namespace csv

