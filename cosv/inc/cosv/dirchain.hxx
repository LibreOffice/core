/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dirchain.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:39:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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



