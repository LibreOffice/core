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




