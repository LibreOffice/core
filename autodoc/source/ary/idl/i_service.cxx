/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_service.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:46:58 $
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
#include <ary/idl/i_service.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <ary/idl/ik_service.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <sci_impl.hxx>
#include "i2s_calculator.hxx"


namespace ary
{
namespace idl
{

Service::Service( const String &      i_sName,
                  Ce_id               i_nOwner )
    :   sName(i_sName),
        nOwner(i_nOwner),
        aIncludedServices(),
        aSupportedInterfaces(),
        aProperties()
{
}

Service::~Service()
{
    for ( RelationList::iterator it = aIncludedServices.begin();
          it != aIncludedServices.end();
          ++it )
    {
        delete (*it).Info();
    }

    for ( RelationList::iterator it = aSupportedInterfaces.begin();
          it != aSupportedInterfaces.end();
          ++it )
    {
        delete (*it).Info();
    }
}

void
Service::Get_SupportedInterfaces( Dyn_StdConstIterator<CommentedRelation> & o_rResult ) const
{
    o_rResult = new SCI_Vector<CommentedRelation>(aSupportedInterfaces);
}

void
Service::Get_IncludedServices( Dyn_StdConstIterator<CommentedRelation> & o_rResult ) const
{
    o_rResult = new SCI_Vector<CommentedRelation>(aIncludedServices);
}

void
Service::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
Service::get_AryClass() const
{
    return class_id;
}

const String &
Service::inq_LocalName() const
{
    return sName;
}

Ce_id
Service::inq_NameRoom() const
{
    return nOwner;
}

Ce_id
Service::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Service::inq_SightLevel() const
{
    return sl_File;
}


namespace ifc_service
{

inline const Service &
service_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.AryClass() == Service::class_id );
    return static_cast< const Service& >(i_ce);
}

void
attr::Get_IncludedServices( Dyn_StdConstIterator<CommentedRelation> & o_result,
                            const CodeEntity &                        i_ce )
{
    o_result = new SCI_Vector<CommentedRelation>( service_cast(i_ce).aIncludedServices );
}

void
attr::Get_ExportedInterfaces( Dyn_StdConstIterator<CommentedRelation> & o_result,
                              const CodeEntity &                            i_ce )
{
    o_result = new SCI_Vector<CommentedRelation>( service_cast(i_ce).aSupportedInterfaces );
}

void
attr::Get_Properties( Dyn_CeIterator &    o_result,
                      const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>( service_cast(i_ce).aProperties );
}

void
xref::Get_IncludingServices( Dyn_CeIterator &    o_result,
                             const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(service_2s_IncludingServices));
}

void
xref::Get_InstantiatingSingletons( Dyn_CeIterator &    o_result,
                                   const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(service_2s_InstantiatingSingletons));
}


} // namespace ifc_service


}   //  namespace   idl
}   //  namespace   ary
