/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_traits.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:11:40 $
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

#ifndef ARY_IDL_I_TRAITS_HXX
#define ARY_IDL_I_TRAITS_HXX

// USED SERVICES
#include <ary/idl/i_types4idl.hxx>




namespace ary
{
namespace idl
{


/** Basic traits for derivd ones of ->CodeEntity.
*/
struct Ce_Traits
{
    typedef CodeEntity          entity_base_type;
    typedef Ce_id               id_type;

    static entity_base_type &
                        EntityOf_(
                            id_type             i_id );
    static id_type      IdOf_(
                            const entity_base_type &
                                                i_entity );
};


/** An instance of COMPARE for ->::ary::SortedIds<>.

    @see ::ary::SortedIds<>
*/
struct Ce_Compare : public Ce_Traits
{
    typedef  String             key_type;

    static const key_type &
                        KeyOf_(
                            const entity_base_type &
                                                i_entity );
    static bool         Lesser_(
                            const key_type &    i_1,
                            const key_type &    i_2 );
};




}   // namespace idl
}   // namespace ary
#endif
