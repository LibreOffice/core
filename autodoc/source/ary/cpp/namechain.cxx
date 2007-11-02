/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: namechain.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:34:37 $
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
