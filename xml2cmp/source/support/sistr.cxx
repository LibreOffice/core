/*************************************************************************
 *
 *  $RCSfile: sistr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: np $ $Date: 2001-03-23 13:23:17 $
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


#include <sistr.hxx>

// The following two header-files declare
//   standard ANSI-C++ functions. They may be replaced
//   by the corresponding header-file-names of the
//   actually used runtime library.
#include <string.h>  // strlen(), memcpy(), memset()
#include <ctype.h>   // tolower()
#include <limits.h>  // INT_MAX

const char NULCH = '\0';
const int  NO_POS = -1;


Simstr::Simstr(const char * str)
{
   if (str == 0)
      {
         len = 0;
         sz = new char[1];
         *sz = 0;
      }
   else
      {
         len = strlen(str);
         sz = new char[len+1];
         memcpy(sz,str,len+1);
      }
}

Simstr::Simstr(const char * txt, int anzahl)
{
   int txtlen = txt != 0 ? strlen(txt) : 0;
   if (txt == 0 || anzahl < 1 || long(anzahl) * txtlen > INT_MAX)
   {
      len = 0;
      sz = new char[1];
      *sz = 0;
   }
   else
   {
      len = anzahl * txtlen;
      sz = new char[len+1];
      for (int i = 0; i < anzahl; i++)
         memcpy(sz + (i*txtlen), txt, txtlen);
      sz[len] = 0;
   }
}

Simstr::Simstr(char c, int anzahl)
{
   if (anzahl < 1)
      {
         len = 0;
         sz = new char[1];
         *sz = 0;
      }
   else
      {
         len = anzahl;
         sz = new char[len+1];
         memset(sz,c,anzahl);
         sz[len] = 0;
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

char &
Simstr::ch(int  n)
{
   static char nullCh = NULCH;
   nullCh = NULCH;
   if (n >= long(len) || n < 0)
      return nullCh;
   else
      return sz[unsigned(n)];
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


// Einzelzugriff

char
Simstr::get(int  n) const     { return (n >= len || n < 0) ? 0 : sz[n]; }

char
Simstr::get_front() const        { return sz[0]; }

char
Simstr::get_back() const        { return  len ? sz[len-1] : 0; }

Simstr
Simstr::get(int  startPos, int  anzahl) const
{
   if (startPos >= len || startPos < 0 || anzahl < 1)
      return "";

   int anz = len - startPos < anzahl ? len - startPos : anzahl;

   Simstr ret(' ',anz);
   memcpy(ret.sz, sz+startPos, anz);
   return ret;
}

Simstr
Simstr::get_front(int  anzahl) const
{
   int anz = len < anzahl ? len : anzahl;
   if (anz < 1)
      return "";

   Simstr ret(' ',anz);
   memcpy(ret.sz, sz, anz);
   return ret;
}

Simstr
Simstr::get_back(int  anzahl) const
{
   int anz = len < anzahl ? len : anzahl;
   if (anz < 1)
      return "";
   int start = len-anz;

   Simstr ret(' ',anz);
   memcpy(ret.sz, sz+start, anz);
   return ret;
}

Simstr
Simstr::get_first_token(char c) const
{
   int posc = pos_first(c);
   if (posc != NO_POS)
      return get_front(posc);
   else
      return sz;
}

Simstr
Simstr::get_last_token(char c) const
{
   int posc = pos_last(c);
   if (posc != NO_POS)
      return get_back(len-posc-1);
   else
      return sz;
}



// Insert

void
Simstr::insert(int  pos, char c)
{
   if (pos < 0 || pos > len)
      return;

   char * result = new char[len+2];

   memcpy(result,sz,pos);
   result[pos] = c;
   memcpy(result+pos+1,sz+pos,len-pos+1);

   delete [] sz;
   sz = result;
   len++;
}

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
Simstr::insert(int  pos, const Simstr & S)
{
   if (pos < 0 || pos > len)
      return;

   char * result = new char[len+1+S.len];

   memcpy(result,sz,pos);
   memcpy(result+pos,S.sz,S.len);
   memcpy(result+pos+S.len,sz+pos,len-pos+1);

   delete [] sz;
   sz = result;
   len += S.len;
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

void
Simstr::pop_front(int  anzahl)
{
   if (anzahl < 1)
      return;
   int anz = len < anzahl ? len : anzahl;

   char * result = new char[len-anz+1];

   memcpy(result,sz+anz,len-anz+1);

   delete [] sz;
   sz = result;
   len -= anz;
}

void
Simstr::pop_back(int  anzahl)
{
   if (anzahl < 1)
      return;

   int anz = len < anzahl ? len : anzahl;

   char * result = new char[len-anz+1];

   memcpy(result,sz,len-anz);
   result[len-anz] = 0;

   delete [] sz;
   sz = result;
   len -= anz;
}

void
Simstr::rem_back_from(int  removeStartPos)
{
   if (removeStartPos != NO_POS)
      pop_back(len-removeStartPos);
}

void
Simstr::remove_all(char c)
{
   if (!len)
      return;
   char * result = new char[len];
   int i,j=0;
   for (i = 0; i < len; i++)
       if (sz[i] != c)
          result[j++] = sz[i];

   delete [] sz;
   sz = new char[j+1];
   memcpy(sz,result,j);
   sz[j] = 0;
   len = j;
   delete [] result;
}

void
Simstr::remove_all(const Simstr & S)
{
   int  pos;
   while ( (pos=pos_first(S)) != NO_POS )
      remove(pos,S.len);
}

void
Simstr::strip(char c)
{
    int start = 0;
   if (c == ' ')
   {  // Sonderbehandlung: SPC entfernt auch TABs:
       while ( start < len
                 ?  sz[start] == ' '
                    || sz[start] == '\t'
                 :  false )
           start++;
   }
   else
   {
       while (start < len && sz[start] == c)
           start++;
   }

    int ende = len-1;
   if (c == ' ')
   {  // Sonderbehandlung: SPC entfernt auch TABs:
       while ( ende >= start
                 ?  sz[ende] == ' '
                    || sz[ende] == '\t'
                 :  false  )
           ende--;
   }
   else
   {
       while (ende >= start && sz[ende] == c)
           ende--;
   }
    *this = get(start,ende-start+1);
}

void
Simstr::empty()
{
   if (len > 0)
   {
      delete [] sz;
      sz = new char[1];
      *sz = 0;
      len = 0;
   }
}

Simstr
Simstr::take_first_token(char c)
{
   Simstr ret;
   int pos = pos_first(c);
   if (pos != NO_POS)
      {
         ret = get_front(pos);
         pop_front(pos+1);
      }
   else
      {
         ret = sz;
         delete [] sz;
         sz = new char[1];
         *sz = NULCH;
         len = 0;
      }

   return ret;
}

Simstr
Simstr::take_last_token(char c)
{
   Simstr ret;
   int pos = pos_last(c);
   if (pos != NO_POS)
      {
         ret = get_back(len-pos-1);
         pop_back(len-pos);
      }
   else
      {
         ret = sz;
         delete [] sz;
         sz = new char[1];
         *sz = NULCH;
         len = 0;
      }

   return ret;
}



// Find

int
Simstr::pos_first(char c) const
{
   int i = 0;
   for (i = 0; i < len ? sz[i] != c : false; i++);
   if (i >= len)
      return NO_POS;
   else
      return i;
}

int
Simstr::pos_first_after( char           c,
                         int            startSearchPos) const
{
   int i = 0;
   if (startSearchPos >= i)
      i = startSearchPos+1;
   for (; i < len ? sz[i] != c : false; i++);
   if (i >= len)
      return NO_POS;
   else
      return i;
}


int
Simstr::pos_last(char c) const
{
   int i = 0;
   for (i = len-1; i >= 0 ? sz[i] != c : false; i--);
   if (i < 0)
      return NO_POS;
   else
      return i;
}

int
Simstr::pos_first(const Simstr & S) const
{
   char * ptr = strstr(sz,S.sz);
   if (ptr)
      return int(ptr-sz);
   else
      return NO_POS;
}

int
Simstr::pos_last(const Simstr & S) const
{
   Simstr vgl;
   int i;
   for (i = len-S.len; i >= 0 ; i--)
      {
         vgl = get(i,S.len);
         if (vgl == S)
            break;
      }
   if (i >= 0)
      return i;
   else
      return NO_POS;
}

int
Simstr::count(char c) const
{
   int ret = 0;
   for (int i =0; i < len; i++)
      if (sz[i] == c)
         ret++;
   return ret;
}

bool
Simstr::is_no_text() const
{
   if (!len)
      return true;

   int i;
   for (i = 0; sz[i] <= 32 && i < len; i++);
   if (i < len)
        return false;
    return true;
}

// Change

void
Simstr::replace(int  pos, char c)
{
    if (pos < 0 || pos >= len)
      return;
   else
      sz[unsigned(pos)] = c;
}

void
Simstr::replace(int  startPos, int  anzahl, const Simstr & S)
{
   if (startPos >= len || startPos < 0 || anzahl < 1)
      return;

   int anz = len - startPos < anzahl ? len - startPos : anzahl;

   char * result = new char[len-anz+S.len+1];

   memcpy(result,sz,startPos);
   memcpy(result+startPos, S.sz, S.len);
   memcpy(result+startPos+S.len, sz+startPos+anz, len-startPos-anz+1);

   delete [] sz;
   sz = result;
   len = len-anz+S.len;
}

void
Simstr::replace_all(char oldCh, char newCh)
{
   for (int i=0; i < len; i++)
      if (sz[i] == oldCh)
         sz[i] = newCh;
}

void
Simstr::replace_all(const Simstr & oldS, const Simstr & newS)
{
   Simstr vgl;
   int i = 0;
    while (i <= len-oldS.len)
        {
         vgl = get(i,oldS.len);
         if (strcmp(vgl.sz,oldS.sz) == 0)
            {
               replace(i,oldS.len,newS);
               i += newS.len;
            }
         else
            i++;
      }
}

void
Simstr::to_lower()
{
    for (int i = 0; i < len; i++)
       sz[i] = (char) tolower(sz[i]);
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


