/*************************************************************************
 *
 *  $RCSfile: mzstring.cpp,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:41:56 $
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
 *  Copyright 1998 by Mizi Research Inc.
 *  Copyright 2003 by Sun Microsystems, Inc.
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
 *  The Initial Developer of the Original Code is: Mizi Research Inc.
 *
 *  Copyright: 1998 by Mizi Research Inc.
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* NAME $Id: mzstring.cpp,v 1.1 2003-10-15 14:41:56 dvo Exp $
 * PURPOSE
 *   supposed to be used instead of std::string
 * NOTES
 *
 * HISTORY
 *        frog - Oct 8, 1998: Created.
 */

#ifdef __GNUG__
#pragma implementation "mzstring.h"
#endif

//#include "stdafx.h"       //
#include "mzstring.h"

#ifndef WIN32
#else
# include <windows.h>
//# include <crtdbg.h>
#endif                                            /* WIN32 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
# define wsprintf sprintf
#endif

const int AllocSize = 8;

inline int get_alloc_size(int len)
{
    return (len + AllocSize - 1) / AllocSize * AllocSize;
}


MzString::MzString()
{
    Length    = 0;
    Allocated = 0;
    Data      = 0;
}


MzString::MzString(int init)
{
    Length    = 0;
    Allocated = 0;
    Data      = 0;
    allocate(init);
}


MzString::MzString(const char *s)
{
    Data      = 0;
    Allocated = 0;
    Length    = 0;
    if (s)
        copy(s, strlen(s));
}


MzString::MzString(const char *s, int len)
{
    Data      = 0;
    Allocated = 0;
    Length    = 0;
    copy(s, len);
}


MzString::MzString(MzString const *s)
{
    Data      = 0;
    Allocated = 0;
    Length    = 0;
    if (s)
        copy(s->Data, s->length());
}


//
// This can be used for performance reasons if it is known the
// MzString will need to grow.
//

/*?
MzString::MzString(MzString const &s, int len)
{
  Data    = 0;
  Allocated = 0;
  Length    = 0;
  if (len == 0) // default
    len = s.length();
  copy(s.Data, len);
}
*/

MzString::~MzString()
{
    if (Data)
        free(Data);
}


void MzString::operator = (MzString &s)
{
    int n = s.length();
    if (allocate(n))
    {
        if (n > 0) memcpy(Data, s.Data, n);
        Length = n;
    }
}


void MzString::operator = (const char *s)
{
    if (s == NULL)
        s = "";
    int n = strlen(s);
    if (allocate(n))
    {
        if (n > 0) memcpy(Data, s, n);
        Length = n;
    }
}


void MzString::append(const char *s, int slen)
{
    if(!s || slen <= 0)
        return;

    int new_len = Length + slen;
    if (resize(new_len))
    {
        memcpy(Data + Length, s, slen);
        Length = new_len;
    }
}


void MzString::append(MzString const &s)
{
    if (s.Data)
        append(s.Data, s.length());
}


void MzString::append(const char *s)
{
    if (!s) return;
    append(s, strlen(s));
}


void MzString::append(const char ch)
{
    append(&ch, 1);
}


int MzString::compare(const char *s)
{
    if (!Data)   return -1;
    if (s==NULL) return 1;

    Data[Length] = 0;
    return strcmp(Data, s);
}


#if 0
int MzString::find(char *str)
{
    if (Data==NULL) return -1;
    int   len = strlen(str);

    Data[Length] = '\0';
    for (int ii = 0; ii <= Length - len; ii++)
    {
        if (strncmp(Data + ii, str, len) == 0)
            return ii;
    }
    return -1;
}
#endif
int MzString::find(char ch)
{
    return find(ch,0);
}


int MzString::find(char ch, int pos)
{
    for (int i = pos; i < Length; i++)
    {
        if (Data[i] == ch)
            return i;
    }
    return -1;
}


int MzString::rfind(char ch)
{
    return rfind(ch, Length - 1);
}


int MzString::rfind(char ch, int pos)
{
    if (pos >= Length)
        return -1;

    while (pos >= 0)
    {
        if (Data[pos] == ch)
            return pos;
        pos--;
    }
    return -1;
}


// += operator

MzString &MzString::operator += (char ch)
{
    append(&ch, 1);
    return *this;
}


MzString &MzString::operator += (const char *str)
{
    append(str);
    return *this;
}


MzString &MzString::operator += (MzString const &s)
{
    append(s);
    return *this;
}


// << operator
MzString &MzString::operator << (const char *str)
{
    append(str);
    return *this;
}


MzString &MzString::operator << (char ch)
{
    append(&ch, 1);
    return *this;
}


MzString &MzString::operator << (int i)
{
    char str[80];

    wsprintf(str, "%d", i);
    append(str);
    return *this;
}


MzString &MzString::operator << (long l)
{
    char str[80];

    wsprintf(str, "%ld", l);
    append(str);
    return *this;
}


MzString &MzString::operator << (MzString const &s)
{
    append(s);
    return *this;
}


/*?
char MzString::operator >> (char &c)
{
  c = '\0';
  if( Data && *Data ) {
    c = Data[Length - 1];
    Data[Length - 1] = '\0';
    Length--;
  }
  return c;
}
*/

/*
char MzString::last()
{
  if (Length > 0)
    return Data[Length - 1];
  else
    return 0;
}
*/

/*?
char &MzString::operator [] (int n)
{
  static char   null = 0;
  if( n >= Length )
    return null;
  else if (n < 0)
    return (*this)[Length + n];
  else
    return Data[n];
}
*/

char MzString::operator [] (int n)
{
    if (Data && 0 <= n && n < Length)
        return Data[n];

    return 0;
}


void MzString::replace(int pos, char ch)
{
    if (Data && 0 <= pos && pos < Length)
        Data[pos] = ch;
}


MzString &MzString::chop(int n)
{
    Length -= n;
    if (Length < 0)
        Length = 0;
    return *this;
}


//------------------------------------------------------------------------
// Private Methods.
//

bool MzString::allocate(int len)
{
    len++;                                        // In case we want to add a null.

    if (len < 0)
        return false;

    if (Data)
    {
        if (len < Allocated)
            return true;
        else
        {
            int   n = get_alloc_size(len);
            char *p = (char *)realloc(Data, n);
            if (p)
            {
                Data      = p;
                Allocated = n;
                return true;
            }
        }
    }
    else
    {
// In case we want to add a null.
        int n = get_alloc_size(len);
        Data  = (char *)malloc(n);
        if (Data)
        {
            Allocated = n;
            return true;
        }
    }

    return false;
}


bool MzString::resize(int len)
{
    return allocate(len);
}


void MzString::copy(const char *s, int len)
{
    if (s==NULL) return;
    if (allocate(len))
    {
        if (len > 0) memcpy(Data, s, len);
        Length = len;
    }
}
