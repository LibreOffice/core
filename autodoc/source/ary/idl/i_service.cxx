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
                            const CodeEntity &						  i_ce )
{
    o_result = new SCI_Vector<CommentedRelation>( service_cast(i_ce).aIncludedServices );
}

void
attr::Get_ExportedInterfaces( Dyn_StdConstIterator<CommentedRelation> & o_result,
                              const CodeEntity &							i_ce )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
