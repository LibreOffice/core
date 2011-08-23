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
#include <ary/cpp/c_funct.hxx>



// NOT FULLY DECLARED SERVICES
#include <algorithm>
#include <ary/cpp/c_funct.hxx>





namespace
{
using namespace ::ary::cpp;


class Parameter_2_NonTypeParamInfo
{
  public:
    String              operator()(
                            const S_Parameter & i_rParam ) const;
};

class Parameter_2_Type
{
  public:
    Type_id             operator()(
                            const S_Parameter & i_rParam ) const
                                                { return i_rParam.nType; }
};

/** @return
    A vector with Strings like this:
        "ParamName" or "ParamName[ArraySize]" or "ParamName = InitValue".
*/
StringVector        Create_NonTypeParameterInfos(
                        const std::vector<S_Parameter> &
                                            i_rParameters );
/** @return
    A vector of the parameters' type ids.
*/
std::vector<Type_id>
                    Create_ParameterTypeList(
                        const std::vector<S_Parameter> &
                                            i_rParameters );

}   // namspace anonymous


namespace ary
{
namespace cpp
{

Function::Function( const String  &             i_sLocalName,
                    Ce_id                       i_nOwner,
                    E_Protection                i_eProtection,
                    Lid                         i_nFile,
                    Type_id                     i_nReturnType,
                    const std::vector<S_Parameter> &
                                                i_parameters,
                    E_ConVol                    i_conVol,
                    E_Virtuality                i_eVirtuality,
                    FunctionFlags               i_aFlags,
                    bool                        i_bThrowExists,
                    const std::vector<Type_id> &
                                                i_rExceptions )
    :   aEssentials( i_sLocalName,
                     i_nOwner,
                     i_nFile ),
        aTemplateParameterTypes(),
        aSignature( Create_ParameterTypeList(i_parameters),
                    i_conVol ),
        nReturnType(i_nReturnType),
        eProtection(i_eProtection),
        eVirtuality(i_eVirtuality),
        aFlags(i_aFlags),
        aParameterInfos( Create_NonTypeParameterInfos(i_parameters) ),
        pExceptions( i_bThrowExists ? new ExceptionTypeList(i_rExceptions) : 0 )
{
}

Function::~Function()
{
}

bool
Function::IsIdentical( const Function & i_f ) const
{
    return
        LocalName() == i_f.LocalName()
        AND
        Owner() == i_f.Owner()
        AND
        aSignature == i_f.aSignature
        AND
        nReturnType == i_f.nReturnType
        AND
        eProtection == i_f.eProtection
        AND
        eVirtuality == i_f.eVirtuality
        AND
        aFlags == i_f.aFlags
        AND
        ( ( NOT pExceptions AND NOT i_f.pExceptions )
          OR
          ( pExceptions AND i_f.pExceptions
                ?   *pExceptions == *i_f.pExceptions
                :   false )
        )
        AND
        aTemplateParameterTypes.size() == i_f.aTemplateParameterTypes.size();
}

void
Function::Add_TemplateParameterType( const String  &    i_sLocalName,
                                     Type_id            i_nIdAsType )
{
    aTemplateParameterTypes.push_back(
        List_TplParam::value_type(i_sLocalName, i_nIdAsType) );
}


const String  &
Function::inq_LocalName() const
{
    return aEssentials.LocalName();
}

Cid
Function::inq_Owner() const
{
    return aEssentials.Owner();
}

Lid
Function::inq_Location() const
{
    return aEssentials.Location();
}

void
Function::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ClassId
Function::get_AryClass() const
{
    return class_id;
}



}   // namespace cpp
}   // namespace ary



namespace
{

String
Parameter_2_NonTypeParamInfo::operator()( const ary::cpp::S_Parameter & i_rParam ) const
{
    static StreamStr aParamName_(1020);
    aParamName_.seekp(0);

    aParamName_ << i_rParam.sName;
    if ( i_rParam.sSizeExpression.length() > 0 )
    {
        aParamName_ << '['
                    << i_rParam.sSizeExpression
                    << ']';
    }
    if ( i_rParam.sInitExpression.length() > 0 )
    {
        aParamName_ << " = "
                    << i_rParam.sInitExpression;
    }

    return aParamName_.c_str();
}


StringVector
Create_NonTypeParameterInfos( const std::vector<S_Parameter> & i_rParameters )
{
    static Parameter_2_NonTypeParamInfo
        aTransformFunction_;

    StringVector
        ret(i_rParameters.size(), String::Null_());
    std::transform( i_rParameters.begin(), i_rParameters.end(),
                    ret.begin(),
                    aTransformFunction_ );
    return ret;
}

std::vector<Type_id>
Create_ParameterTypeList( const std::vector<S_Parameter> & i_rParameters )
{
    static Parameter_2_Type
        aTransformFunction_;

    std::vector<Type_id>
        ret(i_rParameters.size(), Type_id(0));
    std::transform( i_rParameters.begin(), i_rParameters.end(),
                    ret.begin(),
                    aTransformFunction_ );
    return ret;
}




}   // namespace anonymous

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
