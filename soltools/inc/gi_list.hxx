/*************************************************************************
 *
 *  $RCSfile: gi_list.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2001-06-11 16:05:17 $
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

#ifndef SOLTOOLS_GI_LIST_HXX
#define SOLTOOLS_GI_LIST_HXX


#include "st_list.hxx"


class GenericInfo;

/** Holds set of generic informations in a sorted list.

    At different places, methods of this class have a parameter,
    whose name includes "path". Those are paths like this:

        src370/drives/o:

    which are used to access GenericInfo keys in deep search through
    the lists and their sublists.
*/
class List_GenericInfo
{
  public:
    // TYPES
    class const_iterator
    {
      public:
        const GenericInfo & operator*() const;
        const_iterator &    operator++();
        bool                operator==( const const_iterator & ) const;
        bool                operator!=( const const_iterator & ) const;

                            const_iterator( const DynamicList< GenericInfo >::const_iterator & );
      private: DynamicList< GenericInfo >::const_iterator it;
    };
    class iterator
    { public:
        GenericInfo &       operator*() const;
        iterator &          operator++();
        bool                operator==( const iterator & ) const;
        bool                operator!=( const iterator & ) const;

                            iterator( const DynamicList< GenericInfo >::iterator & );
      private: DynamicList< GenericInfo >::iterator it;
    };

    typedef const char *    KeyPath;

    // LIFECYCLE
                        List_GenericInfo();
                        List_GenericInfo(
                            const List_GenericInfo &
                                                i_rList );
                        ~List_GenericInfo();

    // OPERATORS
    List_GenericInfo &  operator=(
                            const List_GenericInfo &
                                                i_rList );
    const GenericInfo * operator[](
                            KeyPath             i_sKeyPath ) const;
    GenericInfo *       operator[](
                            KeyPath             i_sKeyPath );

    // OPERATIONS
    bool                InsertInfo(
                            GenericInfo *       let_dpInfo,    /// Will be owned by this object.
                            bool                i_bOverwrite = true );
    bool                InsertInfoByPath(
                            GenericInfo *       let_dpInfo,    /// Will be owned by this object.
                            KeyPath             i_sKeyPath,
                            bool                i_bCreatePath,
                            bool                i_bOverwrite = true );

    GenericInfo *       ReleaseInfo(            /// Removes the GenericInfo from its parent.
                            KeyPath             i_sKeyPath );

    void                DeleteInfo(
                            KeyPath             i_sKeyPath );

    // INFO
    unsigned            Size() const;

    const_iterator      Begin() const;
    const_iterator      End() const;

    // ACCESS
    iterator            Begin();
    iterator            End();

  private:
    typedef DynamicList< GenericInfo >::iterator  sub_iterator;

    sub_iterator        lower_bound(
                            bool &              o_bExists,
                            const char * &      o_sNextPathSegment,
                            KeyPath             i_sKeyPath );

    DynamicList< GenericInfo >
                        aChildren;
};


// IMPLEMENTATION


inline const GenericInfo &
List_GenericInfo::
const_iterator::operator*() const
    { return *(*it); }

inline List_GenericInfo::const_iterator &
List_GenericInfo::
const_iterator::operator++()
    { ++it; return *this; }

inline bool
List_GenericInfo::
const_iterator::operator==( const const_iterator & i_rIter ) const
    { return it == i_rIter.it; }

inline bool
List_GenericInfo::
const_iterator::operator!=( const const_iterator & i_rIter ) const
    { return it != i_rIter.it; }

inline List_GenericInfo::
const_iterator::const_iterator( const DynamicList< GenericInfo >::const_iterator & i_rDynListIter )
    :   it(i_rDynListIter) { }


inline GenericInfo &
List_GenericInfo::
iterator::operator*() const
    { return *(*it); }

inline List_GenericInfo::iterator &
List_GenericInfo::
iterator::operator++()
    { ++it; return *this; }

inline bool
List_GenericInfo::
iterator::operator==( const iterator & i_rIter ) const
    { return it == i_rIter.it; }

inline bool
List_GenericInfo::
iterator::operator!=( const iterator & i_rIter ) const
    { return it != i_rIter.it; }

inline List_GenericInfo::
iterator::iterator( const DynamicList< GenericInfo >::iterator & i_rDynListIter )
    :   it(i_rDynListIter) { }

inline unsigned
List_GenericInfo::Size() const
    { return aChildren.size(); }

inline List_GenericInfo::const_iterator
List_GenericInfo::Begin() const
    { return aChildren.begin(); }

inline List_GenericInfo::const_iterator
List_GenericInfo::End() const
    { return aChildren.end(); }

inline List_GenericInfo::iterator
List_GenericInfo::Begin()
    { return aChildren.begin(); }

inline List_GenericInfo::iterator
List_GenericInfo::End()
    { return aChildren.end(); }



#endif

