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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_soltools.hxx"


#include <gi_list.hxx>


#include <gen_info.hxx>



const char C_cKeySeparator = '/';


List_GenericInfo::List_GenericInfo()
{
}

List_GenericInfo::List_GenericInfo( const List_GenericInfo & i_rList )
    :   aChildren(i_rList.aChildren)
{
}

List_GenericInfo::~List_GenericInfo()
{
}

List_GenericInfo &
List_GenericInfo::operator=( const List_GenericInfo & i_rList )
{
    aChildren = i_rList.aChildren;
    return *this;
}

const GenericInfo *
List_GenericInfo::operator[]( KeyPath i_sKeyPath ) const
{
    return const_cast< List_GenericInfo& >(*this)[i_sKeyPath];
}

GenericInfo *
List_GenericInfo::operator[]( KeyPath i_sKeyPath )
{
    bool bExists = false;
    const char * sNextPathSegment = 0;
    sub_iterator it = lower_bound(bExists, sNextPathSegment, i_sKeyPath);

    if ( bExists )
    {
        if ( sNextPathSegment == 0 )
            return (*it);
        else
            return (*it)->SubList()[sNextPathSegment];
    }
    else
    {
        return 0;
    }
}

bool
List_GenericInfo::InsertInfo( GenericInfo *       let_dpInfo,
                              bool                i_bOverwrite )
{
    if ( let_dpInfo == 0 )
        return false;

    bool bExists = false;
    const char * sNextPathSegment = 0;
    sub_iterator it = lower_bound(bExists, sNextPathSegment, let_dpInfo->Key() );

    if ( ! bExists )
    {
        aChildren.insert( it, let_dpInfo );
    }
    else if ( i_bOverwrite )
    {
        delete (*it);
        (*it) = let_dpInfo;
    }
    else
    {
         delete let_dpInfo;
        return false;
    }

    return true;
}

bool
List_GenericInfo::InsertInfoByPath( GenericInfo *       let_dpInfo,
                                    KeyPath             i_sKeyPath,
                                    bool                i_bCreatePath,
                                    bool                i_bOverwrite )
{
    if ( let_dpInfo == 0 )
        return false;

    if ( i_sKeyPath == 0 ? true : *i_sKeyPath == 0 )
         return InsertInfo(let_dpInfo, i_bOverwrite);

    bool bExists = false;
    const char * sNextPathSegment = 0;
    sub_iterator it = lower_bound(bExists, sNextPathSegment, i_sKeyPath);

    if ( bExists )
    {
        return (*it)->SubList().InsertInfoByPath(
                                    let_dpInfo,
                                    sNextPathSegment,
                                    i_bCreatePath,
                                    i_bOverwrite );
    }
    else if ( i_bCreatePath )
    {
        Simstr aKey( i_sKeyPath,
                     0,
                     (int)(sNextPathSegment -
                        ( *sNextPathSegment == 0 ? 0 : 1)
                           - i_sKeyPath ));

        GenericInfo * pNew = new GenericInfo(aKey);
        InsertInfo(pNew,false);

        return pNew->SubList().InsertInfoByPath(
                                    let_dpInfo,
                                    sNextPathSegment,
                                    i_bCreatePath,
                                    i_bOverwrite );
    }
    else
    {
         delete let_dpInfo;
        return false;
    }
}

GenericInfo *
List_GenericInfo::ReleaseInfo( KeyPath i_sKeyPath )
{
    bool bExists = false;
    const char * sNextPathSegment = 0;
    sub_iterator it = lower_bound(bExists, sNextPathSegment, i_sKeyPath );

    if ( bExists )
    {
        if ( *sNextPathSegment == 0 )
            return (*it);
        else
            return (*it)->SubList().ReleaseInfo(sNextPathSegment);
    }
    else
    {
         return 0;
    }
}

void
List_GenericInfo::DeleteInfo( KeyPath i_sKeyPath )
{
    bool bExists = false;
    const char * sNextPathSegment = 0;
    sub_iterator it = lower_bound(bExists, sNextPathSegment, i_sKeyPath );

    if ( bExists )
    {
        if ( *sNextPathSegment == 0 )
        {
            aChildren.remove(it);
        }
        else
        {
            (*it)->SubList().DeleteInfo(sNextPathSegment);
        }
    }
}

List_GenericInfo::sub_iterator
List_GenericInfo::lower_bound( bool &              o_bExists,
                               const char * &      o_sNextPathSegment,
                               KeyPath             i_sKeyPath )
{
    o_sNextPathSegment = strchr(i_sKeyPath, '/');
    Simstr sKey( i_sKeyPath, (int)(o_sNextPathSegment == 0 ? strlen(i_sKeyPath) : o_sNextPathSegment++ - i_sKeyPath) );
    GenericInfo aSearch(sKey);

    unsigned low = 0;
    unsigned high = aChildren.size();

    for ( unsigned cur = high / 2; high > low; cur = (low + high) / 2 )
    {
        if ( *aChildren[cur] < aSearch )
        {
            low = cur+1;
        }
        else
        {
            high = cur;
        }
    }   // end for

    o_bExists = low < aChildren.size()
                    ? !(aSearch < *aChildren[low] )
                    : false;
    return &aChildren[low];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
