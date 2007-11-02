/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: d_namedlist.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:37:32 $
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
#include <ary/doc/d_namedlist.hxx>


// NOT FULLY DEFINED SERVICES
#include <algorithm>



namespace ary
{
namespace doc
{


typedef StringVector::const_iterator        name_const_iterator;
typedef StringVector::iterator              name_iterator;
typedef NodeList::iterator                  node_iterator;


NamedList::NamedList(nodetype::id i_id)
    :   Node(i_id),
        aNames(),
        aElements()
{
}

NamedList::~NamedList()
{
}

const Node *
NamedList::operator[](const String & i_name) const
{
    name_const_iterator
        itFound = std::find(aNames.begin(), aNames.end(), i_name);
    if (itFound != aNames.end())
        return & aElements[itFound - aNames.begin()];
    return 0;
}

void
NamedList::Add_Node( const String &          i_name,
                     DYN Node &              pass_Element,
                     E_HandleDuplicateNames  eHandleDuplicateNames )
{
    switch (eHandleDuplicateNames)
    {
        case hdn_normal:
                    AddNode_normal(i_name,pass_Element);
                    break;
        case hdn_put_together:
                    AddNode_put_together(i_name,pass_Element);
                    break;
        case hdn_keep_only_first:
                    AddNode_keep_only_first(i_name,pass_Element);
                    break;
        default:
                    csv_assert(false);
    }
}

void
NamedList::AddNode_normal( const String &  i_name,
                            DYN Node &      pass_Element )
{
    aNames.push_back(i_name);
    aElements.push_back(pass_Element);
}

void
NamedList::AddNode_put_together(   const String &  i_name,
                                    DYN Node &      pass_Element )
{
    name_iterator
        itFound = std::find(aNames.begin(), aNames.end(), i_name);
    if (itFound == aNames.end())
    {
        AddNode_normal(i_name, pass_Element);
        return;
    }

    // Find position after last equal name:
    for ( ++itFound;
          itFound != aNames.end()
                        ?   *itFound == i_name
                        :   false;
          ++itFound ) {}

    // Calculate same position in aElements:
    node_iterator
        itElem = aElements.begin() + (itFound - aNames.begin());

    aNames.insert(itFound, i_name);
    aElements.insert(itElem, pass_Element);
}

void
NamedList::AddNode_keep_only_first( const String &  i_name,
                                     DYN Node &      pass_Element )
{
    if (std::find(aNames.begin(), aNames.end(), i_name) == aNames.end())
        AddNode_normal(i_name, pass_Element);
    else
        delete &pass_Element;
}

void
NamedList::do_Accept(csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor,*this);
}




}   // namespace doc
}   // namespace ary
