/*************************************************************************
 *
 *  $RCSfile: i_namelookup.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:15:27 $
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

#include <precomp.h>
#include <ary/idl/i_namelookup.hxx>

// NOT FULLY DECLARED SERVICES
#include <sci_impl.hxx>

namespace ary
{
namespace idl
{

NameLookup::NameLookup()
    :   aNames()
{
}

NameLookup::~NameLookup()
{
}

void
NameLookup::Add_Name( const String &      i_name,
                      Ce_id               i_id,
                      RCid                i_class,
                      Ce_id               i_owner )
{
    aNames.insert( std::pair< const String, NameProperties>(
                                   i_name,
                                   NameProperties( i_id,
                                                   i_class,
                                                   i_owner )));
//    aNames.insert( std::make_pair( i_name,
//                                   NameProperties( i_id,
//                                                   i_class,
//                                                   i_owner )));
}

bool
NameLookup::Has_Name( const String &      i_name,
                      RCid                i_class,
                      Ce_id               i_owner ) const
{
    IteratorRange<Map_Names::const_iterator>
        aResult( aNames.equal_range(i_name) );

    for ( ; aResult.operator bool(); ++aResult )
    {
        if ( (i_class == 0
                OR (*aResult.cur()).second.nClass == i_class)
             AND
             ((*aResult.cur()).second.nOwner == i_owner
                OR NOT i_owner.IsValid()) )
        {
            return true;
        }
    }   // end for
    return false;
}

void
NameLookup::Get_Names( Dyn_StdConstIterator<Map_Names::value_type> & o_rResult,
                       const String &                         i_name ) const
{
    IteratorRange<Map_Names::const_iterator>
        aResult( aNames.equal_range(i_name) );
    o_rResult = new SCI_MultiMap<String, NameProperties>(aResult.cur(), aResult.end());
}


}   //  namespace   idl
}   //  namespace   ary
