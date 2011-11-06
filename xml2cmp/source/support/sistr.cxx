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




#include <sistr.hxx>

// The following two header-files declare
//   standard ANSI-C++ functions. They may be replaced
//   by the corresponding header-file-names of the
//   actually used runtime library.
#include <string.h>  // strlen(), memcpy(), memset()
#include <ctype.h>   // tolower()
#include <limits.h>  // INT_MAX

#if (_MSC_VER >=1400)
#pragma warning(disable:4365)
#endif

const char NULCH = '\0';
const int  NO_POS = -1;


Simstr::Simstr(const char * str_)
{
   if (str_ == 0)
      {
         len = 0;
         sz = new char[1];
         *sz = 0;
      }
   else
      {
         len = strlen(str_);
         sz = new char[len+1];
         memcpy(sz,str_,len+1);
      }
}

Simstr::Simstr( const char *   anybytes,
                int            firstBytesPos,
                int            nrOfBytes)
{
   unsigned slen = strlen(anybytes);
   if (anybytes == 0 || slen <= unsigned(firstBytesPos))
      {
         len = 0;
         sz = new char[1];
         *sz = 0;
      }
   else
      {
         int maxLen = slen - unsigned(firstBytesPos);
         len =  maxLen < nrOfBytes
                  ? maxLen
                  : nrOfBytes;
         sz = new char[len+1];
         memcpy(sz,anybytes+firstBytesPos,len);
         *(sz+len) = 0;
      }
}


Simstr::Simstr(const Simstr & S)
{
   len = S.len;
   sz = new char[len+1];
   memcpy(sz,S.sz,len+1);
}

Simstr & Simstr::operator=(const Simstr & S)
{
   if (sz == S.sz)
      return *this;

   delete [] sz;

   len = S.len;
   sz = new char[len+1];
   memcpy(sz,S.sz,len+1);

   return *this;
}

Simstr::~Simstr()
{
   delete [] sz;
}

Simstr
Simstr::operator+(const Simstr & S) const
{
   Simstr ret = sz;
   ret.push_back(S);
   return ret;
}

Simstr &
Simstr::operator+=(const Simstr & S)
{
   push_back(S);
   return *this;
}


// REL

bool
Simstr::operator==(const Simstr & S) const
{ return !strcmp(sz,S.sz) ? true : false; }

bool
Simstr::operator!=(const Simstr & S) const
{ return strcmp(sz,S.sz) ? true : false; }

bool
Simstr::operator<(const Simstr & S) const
{ return (strcmp(sz,S.sz) < 0) ? true : false; }

bool
Simstr::operator>(const Simstr & S) const
{ return (strcmp(sz,S.sz) > 0) ? true : false; }

bool
Simstr::operator<=(const Simstr & S) const
{ return (strcmp(sz,S.sz) <= 0) ? true : false; }

bool
Simstr::operator>=(const Simstr & S) const
{ return (strcmp(sz,S.sz) >= 0) ? true : false; }




// **************          LIST - Funktionen        *****************

// Insert

void
Simstr::push_front(char c)
{
   char * result = new char[len+2];

   result[0] = c;
   memcpy(result+1,sz,len+1);

   delete [] sz;
   sz = result;
   len++;
}

void
Simstr::push_back(char c)
{
   char * result = new char[len+2];

   memcpy(result,sz,len);
   result[len] = c;
   result[len+1] = 0;

   delete [] sz;
   sz = result;
   len++;
}

void
Simstr::push_front(const Simstr & S)
{
   char * result = new char[len+1+S.len];

   memcpy(result,S.sz,S.len);
   memcpy(result+S.len,sz,len+1);

   delete [] sz;
   sz = result;
   len += S.len;
}

void
Simstr::push_back(const Simstr & S)
{
   char * result = new char[len+1+S.len];

   memcpy(result,sz,len);
   memcpy(result+len,S.sz,S.len+1);

   delete [] sz;
   sz = result;
   len += S.len;
}


// Remove

void
Simstr::remove(int  pos, int  anzahl)
{
   if (pos >= len || pos < 0 || anzahl < 1)
      return;

    int anz = len - pos < anzahl ? len - pos : anzahl;

    char * result = new char[len-anz+1];

    memcpy(result,sz,pos);
   memcpy(result+pos,sz+pos+anz,len-pos-anz+1);

   delete [] sz;
   sz = result;
   len -= anz;
}

void
Simstr::remove_trailing_blanks()
{
    int newlen = len-1;
    for ( ; newlen > 1 && sz[newlen] <= 32; --newlen ) {}

    if (newlen < len-1)
        remove ( newlen+1, len-newlen);
}

// Find

int
Simstr::pos_first(char c) const
{
   int i = 0;
   for (i = 0; i < len ? sz[i] != c : false; i++) ;
   if (i >= len)
      return NO_POS;
   else
      return i;
}

int
Simstr::pos_last(char c) const
{
   int i = 0;
   for (i = len-1; i >= 0 ? sz[i] != c : false; i--) ;
   if (i < 0)
      return NO_POS;
   else
      return i;
}

bool
Simstr::is_no_text() const
{
   if (!len)
      return true;

   int i;
   for (i = 0; sz[i] <= 32 && i < len; i++) ;
   if (i < len)
        return false;
    return true;
}

// Change

void
Simstr::replace_all(char oldCh, char newCh)
{
   for (int i=0; i < len; i++)
      if (sz[i] == oldCh)
         sz[i] = newCh;
}

//   Simstr addition
Simstr
operator+(const char * str, const Simstr & S)
{
   Simstr ret = S;
   ret.push_front(str);
   return ret;
}

Simstr
operator+(const Simstr & S, const char * str)
{
   Simstr ret = S;
   ret.push_back(str);
   return ret;
}

Simstr
operator+(char c, const Simstr & S)
{
   Simstr ret = S;
   ret.push_front(c);
   return ret;
}

Simstr
operator+(const Simstr & S, char c)
{
   Simstr ret = S;
   ret.push_back(c);
   return ret;
}


// Simstr-Vergleiche mit char *
bool
operator==(const Simstr & S, const char * str)
{
   return strcmp(S,str) == 0;
}

bool
operator!=(const Simstr & S, const char * str)
{
   return strcmp(S,str) != 0;
}

bool
operator<(const Simstr & S, const char * str)
{
   return strcmp(S,str) < 0;
}

bool
operator>(const Simstr & S, const char * str)
{
   return strcmp(S,str) > 0;
}

bool
operator<=(const Simstr & S, const char * str)
{
   return strcmp(S,str) <= 0;
}

bool
operator>=(const Simstr & S, const char * str)
{
   return strcmp(S,str) >= 0;
}

bool
operator==(const char * str, const Simstr & S)
{
   return strcmp(str,S) == 0;
}

bool
operator!=(const char * str, const Simstr & S)
{
   return strcmp(str,S) != 0;
}

bool
operator<(const char * str, const Simstr & S)
{
   return strcmp(str,S) < 0;
}

bool
operator>(const char * str, const Simstr & S)
{
   return strcmp(str,S) > 0;
}

bool
operator<=(const char * str, const Simstr & S)
{
   return strcmp(str,S) <= 0;
}

bool
operator>=(const char * str, const Simstr & S)
{
   return strcmp(str,S) >= 0;
}


