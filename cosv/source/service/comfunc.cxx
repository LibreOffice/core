/*************************************************************************
 *
 *  $RCSfile: comfunc.cxx,v $
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
   buf[0] = char(day/10) + '0';
   buf[1] = char(day%10) + '0';
   buf[3] = char(month/10) + '0';
   buf[4] = char(month%10) + '0';

   if (year < 100)
   {
      buf[6] = char(year/10) + '0';
      buf[7] = char(year%10) + '0';
      buf[8] = 0;
   }
   else
   {
      buf[6] = char(year/1000) + '0';
      buf[7] = char(year%1000/100) + '0';
      buf[8] = char(year%100/10) + '0';
      buf[9] = char(year%10) + '0';
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
   buf[0] = char(hour/10) + '0';
   buf[1] = char(hour%10) + '0';
   buf[3] = char(min/10) + '0';
   buf[4] = char(min%10) + '0';
   buf[6] = char(sec/10) + '0';
   buf[7] = char(sec%10) + '0';
   out_Str = buf;
}



}   // namespace csv



