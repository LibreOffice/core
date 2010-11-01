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

#ifndef SOLTOOLS_SIMSTR_HXX
#define SOLTOOLS_SIMSTR_HXX


class  Simstr   /// Simple string class.
{
// INTERFACE
  public:
    // LIFECYCLE
                        Simstr(
                           const char *         str = 0);
                        Simstr(                 /** Creates Simstr out of a copy of the first
                                                    'nrOfBytes' bytes of 'anyBytes'.
                                                    Adds a '\0' at the end.  */
                           const char *         anybytes,
                           int                  nrOfBytes);
                        Simstr(                 /** Creates Simstr out of a copy of the described bytes within 'anyBytes'.
                                                    Adds a '\0' at the end.  */
                           const char *         anybytes,
                           int                  firstBytesPos,
                           int                  nrOfBytes );
                        Simstr(                 /// Creates Simstr of 'anzahl' times 'c'.
                           char                 c,
                           int                  anzahl);
                        Simstr(
                           const Simstr &       S);
                        ~Simstr();


    // OPERATORS
                        operator const char*() const;

    Simstr &            operator=(
                           const Simstr &       S );

    Simstr              operator+(
                           const Simstr &       S ) const;
    Simstr &            operator+=(
                           const Simstr &       S );
    Simstr &            operator+=(
                           const char *         s );

    bool                operator==(
                           const Simstr &       S ) const;
    bool                operator!=(
                           const Simstr &       S ) const;
    bool                operator<(
                           const Simstr &       S ) const;
    bool                operator>(
                           const Simstr &       S ) const;
    bool                operator<=(
                           const Simstr &       S ) const;
    bool                operator>=(
                           const Simstr &       S ) const;
    // INFO
    static const Simstr &
                        null_();

    const char *        str() const;
    int                 l() const;                    // Length of string without '\0' at end.
    char *              s();   // ATTENTION !!!       // Only to be used, when a function needs a 'char*' but
                                                      //   nevertheless THAT WILL BE NOT CHANGED!
                                                      //   Typecasts to 'const char*' are performed automatically.
    char                get(
                            int                 n) const;
    char                get_front() const;
    char                get_back() const;
    Simstr              get(
                            int                 startPos,
                            int                 anzahl ) const;
    Simstr              get_front(
                            int                 anzahl ) const;
    Simstr              get_back(
                            int                 anzahl ) const;

    int                 pos_first(
                            char                c ) const;
    int                 pos_first_after(
                            char                c,
                            int                 startSearchPos ) const;
    int                 pos_last(
                            char                c ) const;
    int                 pos_first(
                            const Simstr &      S ) const;
    int                 pos_last(
                            const Simstr &      S ) const;
    int                 count(
                             char                 c ) const;
    bool                is_empty() const;                         // Only true if object == "".
    bool                is_no_text() const;                       // String may contain spaces or tabs.

    Simstr              get_first_token(
                            char                c ) const;
    Simstr              get_last_token(
                           char                 c ) const;

    // ACCESS
    char &              ch(                     /** Reference to sz[n]. Allows change of this char.
                                                    !!! No safety, if n is out of the allowed range! */
                            int                 n );

    // OPERATIONS
    void                insert(
                            int                 pos,
                            char                c );
    void                push_front(
                            char                c );
    void                push_back(
                            char                c );
    void                insert(
                            int                 pos,
                            const Simstr &      S );
    void                push_front(
                            const Simstr &      S );
    void                push_back(
                            const Simstr &      S );

    void                remove(
                              int                 pos,
                            int                 anzahl = 1 );
    void                remove_trailing_blanks();
    void                pop_front(
                            int                 anzahl = 1 );
    void                pop_back(
                            int                 anzahl = 1 );
    void                rem_back_from(
                            int                 removeStartPos );
    void                remove_all(
                            char                c );
    void                remove_all(                             // Starts search left.
                            const Simstr &      S );
    void                strip(
                            char                c );                // Removes all characters == c from front and back.
                                                             //   c == ' ' removes also TABs !!!
    void                empty();                                // Changes object to the value "".

    void                replace(
                            int                 pos,
                            char                c );
    void                replace(
                            int                 startPos,
                            int                 anzahl,
                            const Simstr &      S );
    void                replace_all(
                            char                oldCh,
                            char                newCh );
    void                replace_all(
                            const Simstr &      oldS,
                            const Simstr &      newS );
    void                to_lower();

    Simstr              take_first_token(       /// Token is removed from the Simstr.
                           char                 c );
    Simstr              take_last_token(        /// Token is removed from the Simstr.
                           char                 c );
  private:
    // DATA
    char *              sz;
    int                 len;
};

// Simstr - char* / char - concatenations
Simstr operator+(const char * str, const Simstr & S);
Simstr operator+(const Simstr & S, const char * str);
Simstr operator+(char c, const Simstr & S);
Simstr operator+(const Simstr & S, char c);

// Simstr - char* - comparison operators
bool  operator==(const Simstr & S, const char * str);
bool  operator!=(const Simstr & S, const char * str);
bool  operator<(const Simstr & S, const char * str);
bool  operator>(const Simstr & S, const char * str);
bool  operator<=(const Simstr & S, const char * str);
bool  operator>=(const Simstr & S, const char * str);
bool  operator==(const char * str, const Simstr & S);
bool  operator!=(const char * str, const Simstr & S);
bool  operator<(const char * str, const Simstr & S);
bool  operator>(const char * str, const Simstr & S);
bool  operator<=(const char * str, const Simstr & S);
bool  operator>=(const char * str, const Simstr & S);


inline const char *
Simstr::str() const                    { return sz; }
inline char *
Simstr::s()                            { return sz; }
inline int
Simstr::l() const                      { return len; }
inline
Simstr::operator const char*() const   { return sz; }
inline bool
Simstr::is_empty() const               { return len == 0; }


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
