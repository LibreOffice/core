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
#include <ary/cpp/c_builtintype.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_type.hxx>


namespace ary
{
namespace cpp
{




//**********************        Type        **************************//
Rid
Type::inq_RelatedCe() const
{
     return 0;
}


//**********************        BuiltInType        **************************//

BuiltInType::BuiltInType( const String  &       i_sName,
                          E_TypeSpecialisation  i_eSpecialisation )
    :   sName( i_sName ),
        eSpecialisation( i_eSpecialisation )
{
}

String
BuiltInType::SpecializedName_( const char *         i_sName,
                               E_TypeSpecialisation i_eTypeSpecialisation )
{
    StreamLock
        aStrLock(60);
    StreamStr &
        ret = aStrLock();

    switch ( i_eTypeSpecialisation )
    {
        case TYSP_unsigned:
                    ret << "u_";
                    break;
        case TYSP_signed:
                    if (strcmp(i_sName,"char") == 0)
                        ret << "s_";
                    break;
        default:
                    ;

    }   // end switch

    ret << i_sName;
    return String(ret.c_str());
}

void
BuiltInType::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ary::ClassId
BuiltInType::get_AryClass() const
{
    return class_id;
}

bool
BuiltInType::inq_IsConst() const
{
    return false;
}

void
BuiltInType::inq_Get_Text( StreamStr &      ,               // o_rPreName
                           StreamStr &      o_rName,
                           StreamStr &      ,               // o_rPostName
                           const Gate &     ) const         // i_rGate
{
    switch (eSpecialisation)
    {
        case TYSP_unsigned: o_rName << "unsigned "; break;
        case TYSP_signed:   o_rName << "signed ";   break;

        default:            // Does nothing.
                            ;
    }
    o_rName << sName;
}




}   // namespace cpp
}   // namespace ary
