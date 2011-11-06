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

