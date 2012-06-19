/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
