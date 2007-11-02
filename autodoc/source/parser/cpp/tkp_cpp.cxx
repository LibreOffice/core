/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tkp_cpp.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:01:53 $
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
#include "tkp_cpp.hxx"

// NOT FULLY DECLARED SERVICES
#include "cx_c_std.hxx"
#include "c_dealer.hxx"


namespace cpp {




TokenParser_Cpp::TokenParser_Cpp( DYN autodoc::TkpDocuContext & let_drDocuContext )
    :   pBaseContext( new Context_CppStd( let_drDocuContext ) ),
        pCurContext(0),
        pDealer(0),
        pCharacterSource(0)
{
    SetStartContext();
}

TokenParser_Cpp::~TokenParser_Cpp()
{
}

void
TokenParser_Cpp::AssignPartners( CharacterSource &   io_rCharacterSource,
                                 cpp::Distributor &  o_rDealer )
{
    pDealer = &o_rDealer;
    pBaseContext->AssignDealer(o_rDealer);
    pCharacterSource = &io_rCharacterSource;
}

void
TokenParser_Cpp::StartNewFile( const csv::ploc::Path & i_file )
{
    csv_assert(pDealer != 0);
    pDealer->StartNewFile(i_file);

    csv_assert(pCharacterSource != 0);
    Start(*pCharacterSource);
}

void
TokenParser_Cpp::SetStartContext()
{
    pCurContext = pBaseContext.Ptr();
}

void
TokenParser_Cpp::SetCurrentContext( TkpContext & io_rContext )
{
    pCurContext = &io_rContext;
}

TkpContext &
TokenParser_Cpp::CurrentContext()
{
    return *pCurContext;
}

}   // namespace cpp

