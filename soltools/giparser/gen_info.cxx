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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_soltools.hxx"

#include <gen_info.hxx>
#include <gi_list.hxx>


GenericInfo::GenericInfo( const Simstr &      i_sKey,
                          const Simstr &      i_sValue,
                          const Simstr &      i_sComment )
    :   sKey(i_sKey),
        sValue(i_sValue),
        sComment(i_sComment),
        dpSubList(0)
{
}

GenericInfo::GenericInfo( const GenericInfo & i_rInfo )
    :   sKey(i_rInfo.sKey),
        sValue(i_rInfo.sValue),
        sComment(i_rInfo.sComment),
        dpSubList(0)
{
    if ( i_rInfo.HasSubList() )
    {
        dpSubList = new List_GenericInfo(i_rInfo.SubList());
    }
}

GenericInfo::~GenericInfo()
{
    if ( dpSubList != 0 )
        delete dpSubList;
}

GenericInfo &
GenericInfo::operator=( const GenericInfo & i_rInfo )
{
    sKey = i_rInfo.sKey;
    sValue = i_rInfo.sValue;
    sComment = i_rInfo.sComment;

    if ( dpSubList != 0 )
        delete dpSubList;
    if ( i_rInfo.HasSubList() )
    {
        dpSubList = new List_GenericInfo(i_rInfo.SubList());
    }
    else
        dpSubList = 0;

    return *this;
}

List_GenericInfo &
GenericInfo::CreateMyList() const
{
    return * ( const_cast<GenericInfo&>(*this).dpSubList = new List_GenericInfo);

}

