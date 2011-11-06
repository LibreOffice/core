/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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



