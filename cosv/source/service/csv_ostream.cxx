/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: csv_ostream.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:04:10 $
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
#include <cosv/csv_ostream.hxx>

// NOT FULLY DECLARED SERVICES


#ifndef CSV_NO_IOSTREAMS
#else

#include <cosv/streamstr.hxx>

namespace csv
{

ostream::~ostream() { }

ostream & ostream::operator<<(
                            const char *        i_s )   { *pData << i_s; return *this; }
ostream & ostream::operator<<(
                            char                i_c )   { *pData << i_c; return *this; }
ostream & ostream::operator<<(
                            unsigned char       i_c )   { *pData << i_c; return *this; }
ostream & ostream::operator<<(
                            signed char         i_c )   { *pData << i_c; return *this; }

ostream & ostream::operator<<(
                            short               i_n )   { *pData << i_n; return *this; }
ostream & ostream::operator<<(
                            unsigned short      i_n )   { *pData << i_n; return *this; }
ostream & ostream::operator<<(
                            int                 i_n )   { *pData << i_n; return *this; }
ostream & ostream::operator<<(
                            unsigned int        i_n )   { *pData << i_n; return *this; }
ostream & ostream::operator<<(
                            long                i_n )   { *pData << i_n; return *this; }
ostream & ostream::operator<<(
                            unsigned long       i_n )   { *pData << i_n; return *this; }

ostream & ostream::operator<<(
                            float               i_n )   { *pData << i_n; return *this; }
ostream & ostream::operator<<(
                            double              i_n )   { *pData << i_n; return *this; }

ostream &
ostream::seekp( intt                i_nOffset,
                seek_dir            i_eStart )
{
    pData->seekp(i_nOffset, csv::seek_dir(int(i_eStart)) );
    return *this;
}

ostream::ostream( uintt i_nStartSize )
    :   pData( new StreamStr(i_nStartSize) )
{
}



}   // namespace csv

#endif




