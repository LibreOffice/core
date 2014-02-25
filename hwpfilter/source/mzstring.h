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

#ifndef _MZSTRING_H_
#define _MZSTRING_H_

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

/** @name MzString class

  It was supposed to be used instead of std::string.

  Notes for usage:

  When you declare an MzString, it is initially empty. There is no need to
  do things like #MzString a = "";#, especially not in constructors.

  If you want to use a default empty MzString as a parameter, use

#void foo(MzString par = MzString());   // Correct#

rather than

#void foo(MzString par = "");   // WRONG!#
#void foo(MzString par = 0);    // WRONG!#

(The last one is only wrong because some compilers can't handle it.)

Methods that take an index as parameter all follow this rule: Valid indexes
go from 0 to length()-1.
\begin{tabular}{rl}
Correct: & #foo.substr(0, length()-1);# \\
Wrong:   & #bar.substr(0, length());#
\end{tabular}

It is important that you declare MzStrings as const if possible, because
some methods are much more efficient in const versions.

If you want to check whether a string is empty, do

#if (foo.empty()) something right#

rather than something along the lines of

#if (!foo) completely wrong#

When you use the #.copy()# method, MzString calls "#new []#", so you have to
release the memory with #delete[]#. Don't preallocate memory.

When you want to copy an MzString, just do

#MzString a, b = "String";#
#a = b; // That's it!#

not something like

#MzString a, b = "String";#
#a = b.copy();#

The class automatically handles deep copying when required.
*/

class MzString
{
    public:
        MzString();                               // Create an empty string
// if len = 0, len becomes s.length)
        MzString(MzString const &s, int len = 0);
        ~MzString();

        int       length() const;
        const char*   c_str() const;
        operator  char*()         { return (char *)c_str(); }

// If it is not posible to use the constructor with an initial
// allocation size, use the following member to set the size.
        bool      resize(int len);

// Assignment
        MzString  &operator = (MzString &s);
        MzString  &operator = (const char *s);

// Appending
        MzString  &operator += (char);
        MzString  &operator += (const char *);
        MzString  &operator += (MzString const &);

        MzString  &operator << (const char *);
        MzString  &operator << (char);
        MzString  &operator << (unsigned char c)  { return *this<<(char)c; }
        MzString  &operator << (int);
        MzString  &operator << (long);
        MzString  &operator << (short i)      { return *this<<(int)i; }
        MzString  &operator << (MzString const &);
/* MzString &operator << (MzString *s)  { return *this<<*s; }

  // Removing
  char      operator >> (char &c);
*/
// Access to specific characters
//char      &operator [] (int n);
        char      operator [] (int n);
        char      last();

// Comparison
//  Return:
//   0 : 'this' is equal to 's'.
//  -1 : 'this' is less than 's'.
//   1 : 'this' is greater than 's'.
        int       compare(const char *s);

// Searching for parts
        int       find    (char c);
        int       find    (char c, int pos);
        int       find    (char *);
        int       find    (char *, int pos);
        int       rfind   (char c);
        int       rfind   (char c, int pos);

// Manipulation
        void      replace(int, char c);

        void      append  (MzString const &s);
        void      append  (const char *s);
        void      append  (const char *s, int n);

    private:
        int       Length;                         // Current Length
        int       Allocated;                      // Total space allocated
        char      *Data;                          // The actual contents

// Allocate some space for the data.
// Delete Data if it has been allocated.
        bool      allocate(int len);
};

inline int MzString::length() const
{
    return Length;
}


inline const char* MzString::c_str() const
{
    if (Data)
    {
        Data[Length] = '\0';                      // We always leave room for this.
        return (const char *)Data;
    } else
    return "";
}



// Non friend, non member operators

#endif                                            /* _MZSTRING_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
