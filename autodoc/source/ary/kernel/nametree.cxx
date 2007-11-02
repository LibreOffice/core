/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nametree.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:13:52 $
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
