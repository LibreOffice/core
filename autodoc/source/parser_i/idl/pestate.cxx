/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pestate.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:10:17 $
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
#include <s2_luidl/pestate.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/parsenv2.hxx>




namespace csi
{
namespace uidl
{

void
ParseEnvState::Process_Identifier( const TokIdentifier & )
{
    Process_Default();
}

void
ParseEnvState::Process_NameSeparator()
{
    Process_Default();
}

void
ParseEnvState::Process_Punctuation( const TokPunctuation & )
{
    Process_Default();
}

void
ParseEnvState::Process_BuiltInType( const TokBuiltInType & )
{
    Process_Default();
}

void
ParseEnvState::Process_TypeModifier( const TokTypeModifier & )
{
    Process_Default();
}

void
ParseEnvState::Process_MetaType( const TokMetaType &    )
{
    Process_Default();
}

void
ParseEnvState::Process_Stereotype( const TokStereotype & )
{
    Process_Default();
}

void
ParseEnvState::Process_ParameterHandling( const TokParameterHandling & )
{
    Process_Default();
}

void
ParseEnvState::Process_Raises()
{
    Process_Default();
}

void
ParseEnvState::Process_Needs()
{
    Process_Default();
}

void
ParseEnvState::Process_Observes()
{
    Process_Default();
}

void
ParseEnvState::Process_Assignment( const TokAssignment & )
{
    Process_Default();
}

void
ParseEnvState::Process_EOL()
{
    MyPE().SetResult(done,stay);
}


void
ParseEnvState::On_SubPE_Left()
{
}

void
ParseEnvState::Process_Default()
{
    if (bDefaultIsError)
        MyPE().SetResult(not_done, pop_failure);
    else    // ignore:
        MyPE().SetResult(done, stay);
}


}   // namespace uidl
}   // namespace csi

