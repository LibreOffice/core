/*************************************************************************
 *
 *  $RCSfile: pe_ignor.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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





