/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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



