/*************************************************************************
 *
 *  $RCSfile: dirchain.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:25:38 $
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
 *  Copyright 2000 by Sun Microsystems, Inc.
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#include <cosv/template/tpltools.hxx>



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



