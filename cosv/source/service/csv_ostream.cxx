/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
