/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: comfunc.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:53:19 $
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

#ifndef CSV_COMFUNC_HXX
#define CSV_COMFUNC_HXX

#include <stdlib.h>

namespace csv
{

class String;

// min, max and range functions
template <class E>
inline E            max(E in1, E in2);
template <class E>
inline E            min(E in1, E in2);
template <class E>
inline bool         in_range(E low, E val, E high);    // return low <= val < high;


// string functions
inline const char * valid_str(const char * str);
inline bool         no_str(const char * str);       // return !str || !strlen(str)
intt                count_chars(const char * str, char c);


// endian functions
template <class NUMTYPE>
void                switch_endian(
                        NUMTYPE &       o_rNumber,
                        const NUMTYPE & i_rNumber );

// Zeit-Typecasts
bool  str2date(const char * str, int & out_day, int & out_month, int & out_year);
void  date2str(String       & out_Str, int day, int month, int year);
bool  str2time(const char * str, int & out_hour, int & out_min, int & out_sec);
void  time2str(String       & out_Str, int hour, int min, int sec);


template <class E>
inline E
max(E in1, E in2)   { return in1 < in2 ? in2 : in1; }
template <class E>
inline E
min(E in1, E in2)   { return in1 < in2 ? in1 : in2; }
template <class E>
inline bool
in_range(E low, E val, E high) { return low <= val AND val < high; }

inline const char *
valid_str(const char * str) { return str != 0 ? str : ""; }
inline bool
no_str(const char * str) { return str != 0 ? *str == '\0' : true; }


template <class NUMTYPE>
void
switch_endian( NUMTYPE &       o_rNumber,
               const NUMTYPE & i_rNumber )
{
    char *          pFront = reinterpret_cast< char* >(&o_rNumber);
    const char *    pBack  = reinterpret_cast< const char* >(&i_rNumber) + sizeof(NUMTYPE);

    for ( size_t p = 0; p < sizeof(NUMTYPE); --p )
    {
        *pFront++ = *(--pBack);
    }
}



}   // namespace csv




#endif


