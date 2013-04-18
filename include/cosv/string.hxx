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

#ifndef COSV_STRING_HXX
#define COSV_STRING_HXX

// USED SERVICES
#include <string.h>
#include <cosv/stringdata.hxx>
#include <cosv/str_types.hxx>
#include <cosv/csv_ostream.hxx>
#include <vector>




namespace csv
{


/** The Simple String:
        It is used to just hold short to middle long texts as
        data, which are constant at most times. They are reference
        counted, so they are space efficient and have constant time
        copy semantics.

    For all compare() functions the return value is like in strcmp().

    @attention
    The present version of this class is NOT thread safe.
*/


class String
{
  public:
    typedef String              self;

    typedef str::size           size_type;
    typedef str::position       position_type;

    typedef const char *        const_iterator;

    // LIFECYCLE
                        String();

    /// Intentionally not explicit, smooth casting is intended.
                        String(
                            const char *        i_str );
    /// @precond i_nLength <= strlen(i_str) or i_nLength == str::maxsize.
                        String(
                            const char *        i_str,
                            size_type           i_nLength );

    /** @precond i_itBegin and i_itEnd are in the same valid
        memory-area, such that zero to finite times repetition of
        ++i_itBegin leads to i_itBegin == i_itEnd.
    */
                        String(
                            const_iterator      i_itBegin,
                            const_iterator      i_itEnd );

                        String(
                            const self &        i_rStr );

                        ~String();
    // OPERATORS
    self &              operator=(
                            const self &        i_rStr );
    self &              operator=(
                            const char *        i_str );

                        operator const char * () const;

    bool                operator==(
                            const self &        i_rStr ) const;
    bool                operator!=(
                            const self &        i_rStr ) const;
    bool                operator<(
                            const self &        i_rStr ) const;
    bool                operator>(
                            const self &        i_rStr ) const;
    bool                operator<=(
                            const self &        i_rStr ) const;
    bool                operator>=(
                            const self &        i_rStr ) const;

    // OPERATIONS
    void                clear();

    /// @precond i_nLength == str::maxsize OR i_nLength < strlen(i_str) .
    void                assign(
                            const char *        i_str,
                            size_type           i_nLength );
    /// Create a string consisting of a sequence of i_nCount times the same char.
    void                assign(
                            size_type           i_nCount,
                            char                i_c );

    // INQUIRY
    const char *        c_str() const;
    const char *        data() const;

    bool                empty() const;
    size_type           size() const;
    size_type           length() const;

    const char &        char_at(
                            position_type       i_nPosition ) const;

    const_iterator      begin() const;

    /// This is inefficient, so shouldn't be used within loops.
    const_iterator      end() const;

    int                 compare(
                            const self &        i_rStr ) const;
    int                 compare(
                            const CharOrder_Table &
                                                i_rOrder,
                            const self &        i_rStr ) const;

//***********   Not yet implemented    *********************//
    position_type       rfind(
                            const char *        i_strToSearch,
                            position_type       i_nSearchStartPosition = str::npos ) const;
    position_type       rfind(
                            char                i_charToSearch,
                            position_type       i_nSearchStartPosition = str::npos ) const;

    position_type       find_first_not_of(
                            const char *        i_strToSearch,
                            position_type       i_nSearchStartPosition = 0 ) const;
    position_type       find_first_not_of(
                            char                i_charToSearch,
                            position_type       i_nSearchStartPosition = 0 ) const;

    position_type       find_last_not_of(
                            const char *        i_strToSearch,
                            position_type       i_nSearchStartPosition = str::npos ) const;
    position_type       find_last_not_of(
                            char                i_charToSearch,
                            position_type       i_nSearchStartPosition = str::npos ) const;
//***********   end - not yet implemented    *****************//

    static const self & Null_();
    static const char & Nulch_();

  private:
    struct S_Data
    {
                            S_Data();
        /// @precond i_nValidLength <= strlen(i_sData) or i_nValidLength == str::maxsize.
        explicit            S_Data(
                                const char *        i_sData,
                                size_type           i_nValidLength = str::maxsize );
                            ~S_Data();

        const S_Data *      Acquire() const;

        /// Deletes this, if nCount becomes 0.
        void                Release() const;

        StringData<char>    aStr;
        mutable UINT32      nCount;

      private:
        // Forbidden functions, because this is a refcounted structure.
                            S_Data(const S_Data&);
        S_Data &            operator=(const S_Data&);
    };

    // Locals
    const StringData<char> &
                        Str() const;

    // DATA
    const S_Data *      pd;
};


//**********            Global compare functions            ***************//

    //*** Natural order, no substrings

inline int          compare(
                        const String &          i_s1,
                        const String &          i_s2 );
inline int          compare(
                        const String &          i_s1,
                        const char *            i_s2 );
inline int          compare(
                        const char *            i_s1,
                        const String &          i_s2 );
inline int          compare(
                        const char *            i_s1,
                        const char *            i_s2 );

    //*** Natural order, substrings

int                 compare(
                        const String &          i_s1,
                        csv::str::position      i_nStartPosition1,
                        const char *            i_s2,
                        csv::str::size          i_nLength );
inline int          compare(
                        const char *            i_s1,
                        const char *            i_s2,
                        csv::str::size          i_nLength );

    //*** Defined order, no substrings

inline int          compare(
                        const CharOrder_Table & i_rOrder,
                        const String &          i_s1,
                        const char *            i_s2 );
inline int          compare(
                        const CharOrder_Table & i_rOrder,
                        const char *            i_s1,
                        const String &          i_s2 );
int                 compare(
                        const CharOrder_Table & i_rOrder,
                        const char *            i_s1,
                        const char *            i_s2 );
}   // namespace csv




//******************    global comparation operators   *********************//

inline bool         operator==(
                        const csv::String &         i_s1,
                        const char *                i_s2 );
inline bool         operator!=(
                        const csv::String &         i_s1,
                        const char *                i_s2 );
inline bool         operator<(
                        const csv::String &         i_s1,
                        const char *                i_s2 );
inline bool         operator>(
                        const csv::String &         i_s1,
                        const char *                i_s2 );
inline bool         operator<=(
                        const csv::String &         i_s1,
                        const char *                i_s2 );
inline bool         operator>=(
                        const csv::String &         i_s1,
                        const char *                i_s2 );

inline bool         operator==(
                        const char *                i_s1,
                        const csv::String &         i_s2 );
inline bool         operator!=(
                        const char *                i_s1,
                        const csv::String &         i_s2 );
inline bool         operator<(
                        const char *                i_s1,
                        const csv::String &         i_s2 );
inline bool         operator>(
                        const char *                i_s1,
                        const csv::String &         i_s2 );
inline bool         operator<=(
                        const char *                i_s1,
                        const csv::String &         i_s2 );
inline bool         operator>=(
                        const char *                i_s1,
                        const csv::String &         i_s2 );


//******************    global stream operators   *********************//


inline csv::ostream &
operator<<( csv::ostream &        o_rOut,
            const csv::String &   i_rSrc );




// IMPLEMENTATION
namespace csv
{


inline const StringData<char> &
String::Str() const
{ return pd->aStr; }


inline const char &
String::char_at( position_type i_nPosition ) const
{ if ( i_nPosition < Str().Size() )
      return Str().Data()[i_nPosition];
  return Nulch_();
}

inline bool
String::operator==( const self &        i_rStr ) const
{ return compare(i_rStr) == 0; }

inline bool
String::operator!=( const self &        i_rStr ) const
{ return compare(i_rStr) != 0; }

inline bool
String::operator<( const self &        i_rStr ) const
{ return compare(i_rStr) < 0; }

inline bool
String::operator>( const self &        i_rStr ) const
{ return compare(i_rStr) > 0; }

inline bool
String::operator<=( const self &        i_rStr ) const
{ return compare(i_rStr) <= 0; }

inline bool
String::operator>=( const self &        i_rStr ) const
{ return compare(i_rStr) >= 0; }

inline void
String::clear()
{ operator=( String::Null_() ); }

inline const char *
String::c_str() const
{ return Str().Data(); }

inline
String::operator const char * () const
{ return c_str(); }

inline const char *
String::data() const
{ return c_str(); }

inline String::size_type
String::size() const
{ return Str().Size(); }

inline bool
String::empty() const
{ return size() == 0; }

inline String::size_type
String::length() const
{ return size(); }

inline String::const_iterator
String::begin() const
{ return data(); }

inline String::const_iterator
String::end() const
{ return data() + size(); }



//******************     global compare-functions      ********************//
inline int
compare( const String &          i_s1,
         const String &          i_s2 )
{ return i_s1.compare(i_s2); }

inline int
compare( const String &                     i_s1,
         const char *                       i_s2 )
{ return strcmp(i_s1.c_str(), i_s2); }

inline int
compare( const char *                       i_s1,
         const String &                     i_s2 )
{ return strcmp(i_s1, i_s2.c_str()); }

inline int
compare( const char *            i_s1,
         const char *            i_s2 )
{ return strcmp(i_s1, i_s2); }

inline int
compare( const char *                    i_s1,
         const char *                    i_s2,
         str::size                       i_nLength )
{ return strncmp( i_s1, i_s2, i_nLength ); }

inline int
compare( const CharOrder_Table &            i_rOrder,
         const String &                     i_s1,
         const char *                       i_s2 )
{ return compare( i_rOrder, i_s1.c_str(), i_s2 ); }

inline int
compare( const CharOrder_Table &            i_rOrder,
         const char *                       i_s1,
         const String &                     i_s2 )
{ return compare( i_rOrder, i_s1, i_s2.c_str() ); }


}   // namespace csv


inline bool
operator==( const csv::String &         i_s1,
            const char *                i_s2 )
{ return csv::compare( i_s1, i_s2 ) == 0; }

inline bool
operator!=( const csv::String &         i_s1,
            const char *                i_s2 )
{ return csv::compare( i_s1, i_s2 ) != 0; }

inline bool
operator<( const csv::String &         i_s1,
           const char *                i_s2 )
{ return csv::compare( i_s1, i_s2 ) < 0; }

inline bool
operator>( const csv::String &         i_s1,
           const char *                i_s2 )
{ return csv::compare( i_s1, i_s2 ) > 0; }

inline bool
operator<=( const csv::String &         i_s1,
            const char *                i_s2 )
{ return csv::compare( i_s1, i_s2 ) <= 0; }

inline bool
operator>=( const csv::String &         i_s1,
            const char *                i_s2 )
{ return csv::compare( i_s1, i_s2 ) >= 0; }


inline bool
operator==( const char *                i_s1,
            const csv::String &         i_s2 )
{ return csv::compare( i_s1, i_s2 ) == 0; }

inline bool
operator!=( const char *                i_s1,
            const csv::String &         i_s2 )
{ return csv::compare( i_s1, i_s2 ) != 0; }

inline bool
operator<( const char *                i_s1,
           const csv::String &         i_s2 )
{ return csv::compare( i_s1, i_s2 ) < 0; }

inline bool
operator>( const char *                i_s1,
           const csv::String &         i_s2 )
{ return csv::compare( i_s1, i_s2 ) > 0; }

inline bool
operator<=( const char *                i_s1,
            const csv::String &         i_s2 )
{ return csv::compare( i_s1, i_s2 ) <= 0; }

inline bool
operator>=( const char *                i_s1,
            const csv::String &         i_s2 )
{ return csv::compare( i_s1, i_s2 ) >= 0; }


    //************    global stream operators   **************//


inline csv::ostream &
operator<<( csv::ostream &        o_rOut,
            const csv::String &   i_rSrc )
    { o_rOut << i_rSrc.c_str(); return o_rOut; }


//******************    typedefs   *********************//

namespace csv
{

typedef std::vector<String>   StringVector;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
