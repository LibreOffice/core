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
#include <ary/idl/i_singleton.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
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
Singleton::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
Singleton::get_AryClass() const
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
    csv_assert( i_ce.AryClass() == Singleton::class_id );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
