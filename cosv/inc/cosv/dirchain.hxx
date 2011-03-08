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

#ifndef CSV_DIRCHAIN_HXX
#define CSV_DIRCHAIN_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <cosv/string.hxx>
    // PARAMETERS
#include <cosv/csv_ostream.hxx>

#include <cosv/persist.hxx>
#include <cosv/tpl/tpltools.hxx>



namespace csv
{
    class bostream;

namespace ploc
{


class DirectoryChain
{
  public:
                        DirectoryChain();
                        DirectoryChain(
                            const char *        i_sPath,
                            bool                i_bPathIsAlwaysDir = false,
                            const char *        i_sDelimiter = Delimiter() );
                        DirectoryChain(
                            const DirectoryChain &
                                                i_rDC );
                        ~DirectoryChain();

    // OPERATORS
    DirectoryChain &    operator=(
                            const DirectoryChain &
                                                i_rDC );
    DirectoryChain &    operator+=(
                            const String &      i_sName );
    DirectoryChain &    operator+=(
                            const DirectoryChain &
                                                i_rDC );
    DirectoryChain &    operator-=(
                            uintt               i_nLevelsUp );

    // OPERATIONS
    void                Set(
                            const char *        i_sPath,
                            bool                i_bPathIsAlwaysDir = false,
                            const char *        i_sDelimiter = Delimiter() );
    void                PushFront(
                            const String &      i_sName );
    void                PushFront(
                            const DirectoryChain &
                                                i_sPath );
    void                PushBack(
                            const String &      i_sName );
    void                PushBack(
                            const DirectoryChain &
                                                i_sPath );
    void                PopFront(
                            uintt               i_nCount = 1 );
    void                PopBack(
                            uintt               i_nCount = 1 );

    // INQUIRY
    uintt               Size() const;

    StringVector::const_iterator
                        Begin() const;
    StringVector::const_iterator
                        End() const;

    const String &      Front() const;
    const String &      Back() const;

    void                Get(
                            ostream &           o_rPath,
                            const char *        i_sDelimiter ) const;
    void                Get(
                            bostream &          o_rPath,
                            const char *        i_sDelimiter ) const;
  private:
    StringVector        aPath;
};


// IMPLEMENTATION
inline
DirectoryChain::DirectoryChain( const DirectoryChain & i_rDC )
    { PushBack(i_rDC); }

    // OPERATORS
inline DirectoryChain &
DirectoryChain::operator=( const DirectoryChain & i_rDC )
    { csv::erase_container(aPath); PushBack(i_rDC); return *this; }
inline DirectoryChain &
DirectoryChain::operator+=( const String & i_sName )
    { PushBack(i_sName); return *this;  }
inline DirectoryChain &
DirectoryChain::operator+=( const DirectoryChain & i_rDC )
    { PushBack(i_rDC); return *this;  }
inline DirectoryChain &
DirectoryChain::operator-=( uintt i_nLevelsUp )
    { PopBack(i_nLevelsUp); return *this;  }
inline uintt
DirectoryChain::Size() const
    { return aPath.size(); }

inline StringVector::const_iterator
DirectoryChain::Begin() const
    { return aPath.begin(); }
inline StringVector::const_iterator
DirectoryChain::End() const
    { return aPath.end(); }
inline const String &
DirectoryChain::Front() const
    { return aPath.empty() ? String::Null_() : aPath.front(); }
inline const String &
DirectoryChain::Back() const
    { return aPath.empty() ? String::Null_() : aPath.back(); }


}   // namespace ploc
}   // namespace csv


inline csv::ostream &
operator<<( csv::ostream &                     o_rOut,
            const csv::ploc::DirectoryChain &  i_rSubPath )
{
     i_rSubPath.Get(o_rOut, csv::ploc::Delimiter());
    return o_rOut;
}

inline csv::bostream &
operator<<( csv::bostream &                     o_rOut,
            const csv::ploc::DirectoryChain &   i_rSubPath )
{
     i_rSubPath.Get(o_rOut, csv::ploc::Delimiter());
    return o_rOut;
}



#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
