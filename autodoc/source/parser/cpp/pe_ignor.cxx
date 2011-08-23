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
            case Tid_SwBracket_Left:	SetTokenResult(done, stay);
                                        nBracketCounter++;
                                        bBlockOpened = true;
                                        break;
            case Tid_Semicolon:			SetTokenResult(done, pop_success);
                                        break;
            default:
                                        SetTokenResult(done, stay);
        }	// end switch
    }
    else if ( nBracketCounter > 0 )
    {
        SetTokenResult(done, stay);

        switch (i_rTok.TypeId())
        {
            case Tid_SwBracket_Left:	nBracketCounter++;
                                        break;
            case Tid_SwBracket_Right:	nBracketCounter--;
                                        break;
        }	// end switch
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





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
