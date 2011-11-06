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

class noncopyable
{
  protected:
                        noncopyable() {}
                        ~noncopyable() {}
  private:
    // Private to make copying impossible:
                        noncopyable(const noncopyable&);
    noncopyable &       operator=(const noncopyable&);
};




// IMPLEMENTATION
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




#define NON_COPYABLE(xy) \
    private: xy(const xy &); xy & operator=(const xy &)




#endif
