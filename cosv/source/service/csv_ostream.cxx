/*************************************************************************
 *
 *  $RCSfile: csv_ostream.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:25:39 $
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




