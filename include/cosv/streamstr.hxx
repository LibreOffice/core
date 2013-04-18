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

#ifndef CSV_STREAMSTR_HXX
#define CSV_STREAMSTR_HXX

#include "sal/config.h"

#include "sal/types.h"

// BASE CLASSES
#include <cosv/bstream.hxx>
// USED SERVICES
#include <cosv/str_types.hxx>
#include <string.h>




namespace csv
{
    class String;


void c_str();   // Dummy needed for StreamStr::operator<<(StreamStr::F_CSTR);


/** A string buffer class for all kinds of string manipulation.
*/
class StreamStr : public bostream
{
   public:
    typedef StreamStr       self;

    typedef str::size           size_type;
    typedef str::position       position_type;
    typedef intt                seek_type;
    typedef str::insert_mode    insert_mode;

    typedef const char *        const_iterator;
    typedef char *              iterator;

    typedef void (*F_CSTR)();


    /** Represents an area within a string.
    */
    struct Area
    {
        typedef str::size           size_type;

                        Area(
                            const char *        i_str = "",
                            size_type           i_nLength = str::maxsize )
                                                :   sStr(i_str),
                                                    nLength( i_nLength == str::maxsize
                                                                ?   strlen(i_str)
                                                                :   i_nLength ) {}
        const char *    sStr;
        size_type       nLength;
    };

    // LIFECYCLE
                        StreamStr(
                            size_type           i_nCapacity );
    /// Copies also insert_mode and current position.
                        StreamStr(
                            const self &        i_rOther );
                        ~StreamStr();

    // OPERATORS
    /// Copies also insert_mode and current position.
    self &              operator=(
                            const self &        i_rOther );

    self &              operator<<(
                            const char *        i_s );
    self &              operator<<(
                            const String &      i_s );
    self &              operator<<(
                            char                i_c );
    self &              operator<<(
                            unsigned char       i_c );
    self &              operator<<(
                            signed char         i_c );

    self &              operator<<(
                            short               i_n );
    self &              operator<<(
                            unsigned short      i_n );
    self &              operator<<(
                            int                 i_n );
    self &              operator<<(
                            unsigned int        i_n );
    self &              operator<<(
                            long                i_n );
    self &              operator<<(
                            unsigned long       i_n );

    self &              operator<<(
                            float               i_n );
    self &              operator<<(
                            double              i_n );

    /** This operator is used to finish a sequence of streaming
        oeprators by returning the c-string of the complete string.

        @return The same as ->c_str().

        @example
        csv::StreamStr  s(100);
        const char *
            fullname =  s << qualifier() << "::" << name() << csv::c_str;
    */
    const char *        operator<<(
                            F_CSTR              i_f );

    const char &        operator[](
                            position_type       i_nPosition ) const;
    char &              operator[](
                            position_type       i_nPosition );

    //  OPERATIONS
    void                resize(
                            size_type           i_nMinimumCapacity );

    void                clear();

    /** Sets start point for the next operator<<() call.
        if the intended position is not reachable, nothing happens.
    */
    self &              seekp(
                            seek_type           i_nCount,
                            seek_dir            i_eDirection = ::csv::beg );
    self &              reset()                 { return seekp(0); }
    /** Sets the insertion mode of all and only the operator<<() calls.

        Default is str::overwrite:
        str::overwrite: seekp() always sets the cur end of the string.
                        operator<<() calls push the end of the string forward.
        str::insert:    seekp() only sets the insertion point.
                        operator<<() calls insert their text at the tellp()
                        position and keep the rest of the string. tellp() is
                        then after the inserted text, on the beginning of the
                        rest of the string.
    */
    self &              set_insert_mode(
                            insert_mode         i_eMode );

    void                pop_front(
                            size_type           i_nCount );
    void                pop_back(
                            size_type           i_nCount );

    /// Works like operator<<(). Does the same as Perl's join().
    self &              operator_join(
                            std::vector<String>::const_iterator
                                                i_rBegin,
                            std::vector<String>::const_iterator
                                                i_rEnd,
                            const char *        i_sLink );
    /// Works like operator<<()
    self &              operator_add_substr(
                            const char *        i_sText,
                            size_type           i_nLength );
    /// Works like operator<<()
    self &              operator_add_token(
                            const char *        i_sText,
                            char                i_cDelimiter );
    /// Works like operator<<()
    self &              operator_read_line(
                            bstream &           i_src );

    void                strip_front_whitespace();    /// removes space, tab and crlf.
    void                strip_back_whitespace();
    void                strip_frontback_whitespace();

    void                replace_all(
                            char                i_cCarToSearch,
                            char                i_cReplacement );

    // INQUIRY
    const char *        c_str() const;
    const char *        data() const;

    bool                empty() const;
    size_type           size() const;
    size_type           length() const;

    size_type           capacity() const;

    position_type       tellp() const;

    const_iterator      begin() const;
    const_iterator      cur() const;
    const_iterator      end() const;

    // ACCESS
    iterator            begin();
    iterator            cur();
    iterator            end();

   private:
    // Interface bostream
    virtual UINT32      do_write(
                            const void *    i_pSrc,
                            UINT32          i_nNrofBytes);
    // Locals
    void                ProvideAddingSize(
                            size_type           i_nSize2Add );
    /// Resizes with the factor 2.0 (under 128), 1.5 or until i_nMinimumCapacity, whatever is bigger.
    void                Resize(
                            size_type           i_nMinimumCapacity = 0 );
    void                Advance(
                            size_type           i_nAddedSize );
    void                MoveData(
                            char *              i_pStart,
                            char *              i_pEnd,
                            seek_type           i_nDiff );
    // DATA
    size_type           nCapacity1;     /// Capacity of characters to contain + 1 for terminating 0.
    DYN char *          dpData;
    char *              pEnd;
    char *              pCur;
    insert_mode         eMode;
};



class StreamStrLock
{
  public:
                        StreamStrLock(
                            uintt               i_nMinimalSize );
                        ~StreamStrLock();

    StreamStr &         operator()()            { return *pStr; }

  private:
    StreamStr *         pStr;
};

// IMPLEMENTATION

inline const char *
StreamStr::operator<<( SAL_UNUSED_PARAMETER F_CSTR )
    { return dpData; }
inline void
StreamStr::clear()
    { pEnd = pCur = dpData; *pEnd = '\0'; }
inline const char *
StreamStr::c_str() const
    { return dpData; }
inline const char *
StreamStr::data() const
    { return dpData; }
inline bool
StreamStr::empty() const
    { return dpData == pEnd; }
inline StreamStr::size_type
StreamStr::size() const
    { return pEnd - dpData; }
inline StreamStr::size_type
StreamStr::length() const
    { return size(); }
inline StreamStr::size_type
StreamStr::capacity() const
    { return nCapacity1-1; }
inline StreamStr::position_type
StreamStr::tellp() const
    { return size_type(pCur-dpData); }
inline StreamStr::const_iterator
StreamStr::begin() const
    { return dpData; }
inline StreamStr::const_iterator
StreamStr::cur() const
    { return pCur; }
inline StreamStr::const_iterator
StreamStr::end() const
    { return pEnd; }
inline StreamStr::iterator
StreamStr::begin()
    { return dpData; }
inline StreamStr::iterator
StreamStr::cur()
    { return pCur; }
inline StreamStr::iterator
StreamStr::end()
    { return pEnd; }

}   // namespace csv
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
