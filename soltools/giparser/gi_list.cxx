/*************************************************************************
 *
 *  $RCSfile: gi_list.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2001-06-11 16:04:51 $
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
                     sNextPathSegment -
                        ( *sNextPathSegment == 0 ? 0 : 1)
                        - i_sKeyPath );

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

    return true;
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
    Simstr sKey( i_sKeyPath, (o_sNextPathSegment == 0 ? strlen(i_sKeyPath) : o_sNextPathSegment++ - i_sKeyPath) );
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

