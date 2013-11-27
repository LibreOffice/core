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
#include <ary/cpp/namechain.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/usedtype.hxx>
#include <ary/cpp/c_gate.hxx>
#include "tplparam.hxx"



namespace ary
{
namespace cpp
{
namespace ut
{


//*********************     NameSegment     ******************//

NameSegment::NameSegment( const char * i_sName )
    :   sName( i_sName )
        // pTemplate
{
}

NameSegment::NameSegment( const NameSegment & i_rSeg )
    :   sName(i_rSeg.sName)
        // pTemplate
{
    // KORR_FUTURE :   Handling of copying of templates.
//  csv_assert( NOT i_rSeg.pTemplate );
}

NameSegment& NameSegment::operator=(const NameSegment & i_rSeg)
{
    sName = i_rSeg.sName;
    return *this;
}

NameSegment::~NameSegment()
{
}

List_TplParameter &
NameSegment::AddTemplate()
{
    return * (pTemplate = new List_TplParameter);
}

intt
NameSegment::Compare( const NameSegment & i_rOther ) const
{
    intt nResult = strcmp( sName.c_str(), i_rOther.sName.c_str() );
    if (nResult != 0)
        return nResult;
    if ( bool(pTemplate) != bool(i_rOther.pTemplate) )
    {
        if ( NOT pTemplate )
            return -1;
        else
            return +1;
    }
    else if ( NOT pTemplate )
        return 0;
    else
        return pTemplate->Compare( *i_rOther.pTemplate );
}

void
NameSegment::Get_Text_AsScope( StreamStr &   o_rOut,
                               const Gate &  i_rGate ) const
{
    o_rOut << sName;
    if ( pTemplate )
        pTemplate->Get_Text( o_rOut, i_rGate );
}

void
NameSegment::Get_Text_AsMainType( StreamStr &   o_rName,
                                  StreamStr &   o_rPostName,
                                  const Gate &  i_rGate ) const
{
    o_rName << sName;
    if ( pTemplate )
        pTemplate->Get_Text( o_rPostName, i_rGate );
}


//*********************     NameChain     ******************//

NameChain::NameChain()
//  :   aSegments
{
}

NameChain::~NameChain()
{
}

void
NameChain::Add_Segment( const char * i_sSeg )
{
    aSegments.push_back( NameSegment(i_sSeg) );
}

List_TplParameter &
NameChain::Templatize_LastSegment()
{
    csv_assert( ! aSegments.empty() );

    return aSegments.back().AddTemplate();
}

intt
NameChain::Compare( const NameChain & i_rChain ) const
{
    intt nResult = intt(aSegments.size()) - intt(i_rChain.aSegments.size());
    if (nResult != 0)
        return nResult;

    std::vector< NameSegment >::const_iterator it1 = aSegments.begin();
    std::vector< NameSegment >::const_iterator it1End = aSegments.end();
    std::vector< NameSegment >::const_iterator it2 = i_rChain.aSegments.begin();

    for ( ; it1 != it1End; ++it1, ++it2 )
    {
        nResult = (*it1).Compare(*it2);
        if (nResult != 0)
            return nResult;
    }

    return 0;
}

const String  &
NameChain::LastSegment() const
{
    if ( ! aSegments.empty() )
        return aSegments.back().Name();
    return String::Null_();
}

void
NameChain::Get_Text( StreamStr &    o_rPreName,
                     StreamStr &    o_rName,
                     StreamStr &    o_rPostName,
                     const Gate &   i_rGate ) const
{
    std::vector< NameSegment >::const_iterator it = aSegments.begin();
    std::vector< NameSegment >::const_iterator itEnd = aSegments.end();

    if ( it == itEnd )
        return;

    for ( --itEnd; it != itEnd; ++it )
    {
        (*it).Get_Text_AsScope( o_rPreName, i_rGate );
        o_rPreName << "::";
    }
    (*it).Get_Text_AsMainType( o_rName, o_rPostName, i_rGate );
}



}   // namespace ut
}   // namespace cpp
}   // namespace ary
