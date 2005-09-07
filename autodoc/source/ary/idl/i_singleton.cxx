/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_singleton.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:45:31 $
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
#include <ary/idl/i_singleton.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/idl/ihost_ce.hxx>
#include <ary/idl/ik_singleton.hxx>
#include <sci_impl.hxx>


namespace ary
{
namespace idl
{

Singleton::Singleton( const String &      i_sName,
                  Ce_id               i_nOwner )
    :   sName(i_sName),
        nOwner(i_nOwner),
        nService()
{
}

Singleton::~Singleton()
{
}

void
Singleton::do_Visit_CeHost( CeHost & o_rHost ) const
{
    o_rHost.Do_Singleton(*this);
}

RCid
Singleton::inq_ClassId() const
{
    return class_id;
}

const String &
Singleton::inq_LocalName() const
{
    return sName;
}

Ce_id
Singleton::inq_NameRoom() const
{
    return nOwner;
}

Ce_id
Singleton::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Singleton::inq_SightLevel() const
{
    return sl_File;
}


namespace ifc_singleton
{

inline const Singleton &
singleton_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.ClassId() == Singleton::class_id );
    return static_cast< const Singleton& >(i_ce);
}

Type_id
attr::AssociatedService( const CodeEntity & i_ce )
{
    return singleton_cast(i_ce).nService;
}

} // namespace ifc_singleton


}   //  namespace   idl
}   //  namespace   ary

