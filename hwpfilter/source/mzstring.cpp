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

/* NAME $Id: mzstring.cpp,v 1.5 2008-06-04 10:02:20 vg Exp $
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

#include "mzstring.h"

#ifndef WIN32
#else

    #if defined _MSC_VER
        #pragma warning(push, 1)
    #endif
# include <windows.h>
    #if defined _MSC_VER
        #pragma warning(pop)
    #endif
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


int MzString::compare(const char *s)
{
    if (!Data)   return -1;
    if (s==NULL) return 1;

    Data[Length] = 0;
    return strcmp(Data, s);
}


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
