/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include <precomp.h>
#include <ary/idl/i_typedef.hxx>
#include <ary/idl/ik_typedef.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <sci_impl.hxx>
#include "i2s_calculator.hxx"


namespace ary
{
namespace idl
{


Typedef::Typedef( const String &      i_sName,
                  Ce_id               i_nOwner,
                  Type_id             i_nDefiningType )
    :   sName(i_sName),
        nOwner(i_nOwner),
        nDefiningType(i_nDefiningType)
{
}

Typedef::~Typedef()
{
}

void
Typedef::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
Typedef::get_AryClass() const
{
    return class_id;
}

const String &
Typedef::inq_LocalName() const
{
    return sName;
}

Ce_id
Typedef::inq_NameRoom() const
{
    return nOwner;
}

Ce_id
Typedef::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Typedef::inq_SightLevel() const
{
    return sl_File;
}


namespace ifc_typedef
{

inline const Typedef &
typedef_cast( const CodeEntity & i_ce )
{
    csv_assert( i_ce.AryClass() == Typedef::class_id );
    return static_cast< const Typedef& >(i_ce);
}

Type_id
attr::DefiningType( const CodeEntity & i_ce )
{
    return typedef_cast(i_ce).nDefiningType;
}


void
xref::Get_SynonymTypedefs( Dyn_CeIterator &    o_result,
                           const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(typedef_2s_SynonymTypedefs));
}

void
xref::Get_AsReturns( Dyn_CeIterator &    o_result,
                     const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(typedef_2s_AsReturns));
}

void
xref::Get_AsParameters( Dyn_CeIterator &    o_result,
                        const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(typedef_2s_AsParameters));
}

void
xref::Get_AsDataTypes( Dyn_CeIterator &    o_result,
                       const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(typedef_2s_AsDataTypes));
}

} // namespace ifc_typedef



}   //  namespace   idl
}   //  namespace   ary
