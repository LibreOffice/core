/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: comfunc.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:03:49 $
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

#include <ctype.h>
#include <cosv/comfunc.hxx>
#include <cosv/string.hxx>
#include <cosv/x.hxx>
#include <cosv/std_outp.hxx>



namespace csv
{


void
X_Default::GetInfo( ostream & o_rOutputMedium ) const
{
    o_rOutputMedium << "Error (general exception): ";
    o_rOutputMedium << sMessage
                    << Endl;
}

intt
count_chars(const char * str, char c)
{
    intt nCount = 0;
    for ( const char * pSpc = strchr(str, c);
          pSpc != 0;
          pSpc = strchr(pSpc+1, c) )
    {
        nCount++;
    }
    return nCount;
}



// Zeit-Typecasts
bool
str2date(const char * str, int & out_day, int & out_month, int & out_year)
{
   const char * z = str;
   out_day = 0;
   out_month = 0;
   out_year = 0;

   while (isdigit(*z))
      out_day = 10*out_day + *(z++) - '0';
   if (*z == 0)
      return false;
   z++;
   while (isdigit(*z))
      out_month = 10*out_month + *(z++) - '0';
   if (*z == 0)
      return false;
   z++;
   while (isdigit(*z))
      out_year = 10*out_year + *(z++) - '0';
   return true;
}

void
date2str(String & out_Str, int day, int month, int year)
{
   char buf[11] = "00.00.0000";
   buf[0] = static_cast<char>(day/10 + '0');
   buf[1] = static_cast<char>(day%10 + '0');
   buf[3] = static_cast<char>(month/10 + '0');
   buf[4] = static_cast<char>(month%10 + '0');

   if (year < 100)
   {
      buf[6] = static_cast<char>(year/10 + '0');
      buf[7] = static_cast<char>(year%10 + '0');
      buf[8] = 0;
   }
   else
   {
      buf[6] = static_cast<char>(year/1000 + '0');
      buf[7] = static_cast<char>(year%1000/100 + '0');
      buf[8] = static_cast<char>(year%100/10 + '0');
      buf[9] = static_cast<char>(year%10 + '0');
   }
   out_Str = buf;
}

bool
str2time(const char * str, int & out_hour, int & out_min, int & out_sec)
{
   const char * z = str;
   out_hour = 0;
   out_min = 0;
   out_sec = 0;

   while (isdigit(*z))
      out_hour = 10*out_hour + *(z++) - '0';
   if (*z == 0)
      return false;
   z++;
   while (isdigit(*z))
      out_min = 10*out_min + *(z++) - '0';
   if (*z == 0)
      return false;
   z++;
   while (isdigit(*z))
      out_sec = 10*out_sec + *(z++) - '0';
   return true;
}

void
time2str(String & out_Str, int hour, int min, int sec)
{
   char buf[9] = "00:00:00";
   buf[0] = static_cast<char>(hour/10 + '0');
   buf[1] = static_cast<char>(hour%10 + '0');
   buf[3] = static_cast<char>(min/10 + '0');
   buf[4] = static_cast<char>(min%10 + '0');
   buf[6] = static_cast<char>(sec/10 + '0');
   buf[7] = static_cast<char>(sec%10 + '0');
   out_Str = buf;
}



}   // namespace csv



