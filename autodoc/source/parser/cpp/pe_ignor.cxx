/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_ignor.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:27:06 $
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
#include "pe_ignor.hxx"


// NOT FULLY DECLARED SERVICES


namespace cpp {



PE_Ignore::PE_Ignore( Cpp_PE * i_pParent )
    :   Cpp_PE(i_pParent),
        nBracketCounter(0),
        bBlockOpened(false)
{
    Setup_StatusFunctions();
}


PE_Ignore::~PE_Ignore()
{
}

void
PE_Ignore::Call_Handler( const cpp::Token & i_rTok )
{
    if ( NOT bBlockOpened )
    {
        switch (i_rTok.TypeId())
        {
            case Tid_SwBracket_Left:    SetTokenResult(done, stay);
                                        nBracketCounter++;
                                        bBlockOpened = true;
                                        break;
            case Tid_Semicolon:         SetTokenResult(done, pop_success);
                                        break;
            default:
                                        SetTokenResult(done, stay);
        }   // end switch
    }
    else if ( nBracketCounter > 0 )
    {
        SetTokenResult(done, stay);

        switch (i_rTok.TypeId())
        {
            case Tid_SwBracket_Left:    nBracketCounter++;
                                        break;
            case Tid_SwBracket_Right:   nBracketCounter--;
                                        break;
        }   // end switch
    }
    else if ( i_rTok.TypeId() == Tid_Semicolon )
    {
        SetTokenResult(done, pop_success);
    }
    else
    {
        SetTokenResult(not_done, pop_success);
    }
}

void
PE_Ignore::Setup_StatusFunctions()
{
    // Does nothing.
}

void
PE_Ignore::InitData()
{
    nBracketCounter = 0;
    bBlockOpened = false;
}

void
PE_Ignore::TransferData()
{
    // Does nothing.
}


}   // namespace cpp





