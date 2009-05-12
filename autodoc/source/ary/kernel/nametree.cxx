/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nametree.cxx,v $
 * $Revision: 1.7 $
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

#include <precomp.h>
#include <nametree.hxx>


// NOT FULLY DECLARED SERVICES



namespace ary
{


NameTree::NameTree()
{
}

NameTree::~NameTree()
{
}

const InstanceList &
NameTree::operator[]( const String  & i_rName ) const
{
    static InstanceList aNull_;

    const_iterator itFound = aNames.find(i_rName);
    if ( itFound != aNames.end() )
        return (*itFound).second;
    return aNull_;
}

void
NameTree::insert( const String  &     i_rName,
                  ary::Rid            i_nId )
{
    aNames[i_rName].push_back(i_nId);
}

NameTree::const_iterator
NameTree::find( const String  & i_rName )
{
    return aNames.find( i_rName );
}

NameTree::const_iterator
NameTree::lower_bound( const String  & i_rName ) const
{
     return aNames.lower_bound(i_rName);
}

NameTree::const_iterator
NameTree::begin() const
{
    return aNames.begin();
}

NameTree::const_iterator
NameTree::end() const
{
    return aNames.end();
}

NameTree::iterator
NameTree::begin()
{
    return aNames.begin();
}

NameTree::iterator
NameTree::end()
{
    return aNames.end();
}


}   // namespace ary
