/*************************************************************************
 *
 *  $RCSfile: mzstring.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:38:07 $
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

#ifndef _MZSTRING_H_
#define _MZSTRING_H_

#ifdef __GNUG__
#  pragma interface
#endif

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
        MzString(int init);                       // initial allocated length
        MzString(const char *s);                  // from null terminated s
        MzString(const char *s, int len);         // from s with length len
        MzString(MzString const *s);              // Copy constructor
// if len = 0, len becomes s.length)
        MzString(MzString const &s, int len = 0);
        ~MzString();

        bool      empty(void) const;
        int       length() const;
        const char*   c_str() const;
        operator  char*()         { return (char *)c_str(); }

// If it is not posible to use the constructor with an initial
// allocation size, use the following member to set the size.
        bool      resize(int len);

// Assignment
        void      operator = (MzString &s);
        void      operator = (const char *s);

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
        void      append  (const char ch);

/// 스트링의 끝에서 글자를 지운다.
        MzString  &chop(int n = 1);

    private:
        int       Length;                         // Current Length
        int       Allocated;                      // Total space allocated
        char      *Data;                          // The actual contents

// Allocate some space for the data.
// Delete Data if it has been allocated.
        bool      allocate(int len);
        void      copy(const char *s, int len);
};

//
// Inline methods.
//
inline bool MzString::empty(void) const
{
    return Data == 0L || Data[0] == '\0';
}


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


//
// Non friend, non member operators
//
#endif                                            /* _MZSTRING_H_ */
