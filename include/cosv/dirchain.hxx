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
    void                PushBack(
                            const String &      i_sName );
    void                PushBack(
                            const DirectoryChain &
                                                i_sPath );
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

inline csv::bostream &
operator<<( csv::bostream &                     o_rOut,
            const csv::ploc::DirectoryChain &   i_rSubPath )
{
     i_rSubPath.Get(o_rOut, csv::ploc::Delimiter());
    return o_rOut;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
