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

                            const_iterator();
                            const_iterator( const DynamicList< GenericInfo >::const_iterator & );
      private: DynamicList< GenericInfo >::const_iterator it;
    };
    class iterator
    { public:
        GenericInfo &       operator*() const;
        iterator &          operator++();
        bool                operator==( const iterator & ) const;
        bool                operator!=( const iterator & ) const;

                            iterator();
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
const_iterator::const_iterator()
    :   it(0) { }

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
iterator::iterator()
    :   it(0) { }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
