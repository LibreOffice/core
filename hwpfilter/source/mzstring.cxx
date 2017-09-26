/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

/* PURPOSE
 *   supposed to be used instead of std::string
 */

#include "mzstring.h"

#ifdef _WIN32
# if !defined WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
# define wsprintfA sprintf
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
    Data      = nullptr;
}


MzString::~MzString()
{
    if (Data)
        free(Data);
}


MzString &MzString::operator=(const MzString &s)
{
    int n = s.length();
    if (allocate(n))
    {
        if (n > 0) memcpy(Data, s.Data, n);
        Length = n;
    }
    return *this;
}


MzString &MzString::operator = (const char *s)
{
    if (s == nullptr)
        s = "";
    int n = strlen(s);
    if (allocate(n))
    {
        if (n > 0) memcpy(Data, s, n);
        Length = n;
    }
    return *this;
}


void MzString::append(const char *s, int slen)
{
    if(!s || slen <= 0)
        return;

    int new_len = Length + slen;
    if (allocate(new_len))
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
    if (s==nullptr) return 1;

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

    wsprintfA(str, "%d", i);
    append(str);
    return *this;
}


MzString &MzString::operator << (long l)
{
    char str[80];

    wsprintfA(str, "%ld", l);
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


// Private Methods.


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
            char *p = static_cast<char *>(realloc(Data, n));
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
        Data  = static_cast<char *>(malloc(n));
        if (Data)
        {
            Allocated = n;
            return true;
        }
    }

    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
