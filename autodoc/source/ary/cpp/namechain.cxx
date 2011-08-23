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


//*********************		NameSegment		******************//

NameSegment::NameSegment( const char * i_sName )
    :	sName( i_sName )
        // pTemplate
{
}

NameSegment::NameSegment( const NameSegment & i_rSeg )
    :	sName(i_rSeg.sName)
        // pTemplate
{
    // KORR_FUTURE :   Handling of copying of templates.
//	csv_assert( NOT i_rSeg.pTemplate );
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


//*********************		NameChain     ******************//

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
    csv_assert( aSegments.size() > 0 );

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
    if ( aSegments.size() > 0 )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
