/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cpp_pe.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:48:55 $
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
#include "cpp_pe.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/doc/d_oldcppdocu.hxx>
#include "cpp_tok.hxx"




namespace cpp {

void
Cpp_PE::SetTokenResult( E_TokenDone         i_eDone,
                        E_EnvStackAction    i_eWhat2DoWithEnvStack,
                        ParseEnvironment *  i_pParseEnv2Push )
{
    rMyEnv.SetTokenResult(      i_eDone,
                                i_eWhat2DoWithEnvStack,
                                i_pParseEnv2Push );
}

Cpp_PE::Cpp_PE( Cpp_PE * io_pParent )
    :   ParseEnvironment( io_pParent ),
        rMyEnv( io_pParent->Env() )
{
    csv_assert(io_pParent != 0);
}

Cpp_PE::Cpp_PE( EnvData & i_rEnv )
    :   ParseEnvironment(0),
        rMyEnv(i_rEnv)
{
}

void
Cpp_PE::StdHandlingOfSyntaxError( const char * )
{
//  GlobalHandlingOfSyntaxError(i_sText, Env().CurFileName(), Env().LineCount());

    SetTokenResult(not_done, pop_failure);
}


Cpp_PE *
Cpp_PE::Handle_ChildFailure()
{
     return 0;
}

}   // namespace cpp

