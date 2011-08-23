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
#include <adoc/tk_docw.hxx>


// NOT FULLY DEFINED SERVICES
#include <adoc/tokintpr.hxx>



namespace adoc {


    static const char C_sSpace[300] =
        "                                         "
        "                                         "
        "                                         "
        "                                         "
        "                                         "
        "                                        ";


//***********************   Tok_DocWord     ******************//

void
Tok_DocWord::Trigger( TokenInterpreter & io_rInterpreter ) const
{
    io_rInterpreter.Hdl_DocWord(*this);
}

const char *
Tok_DocWord::Text() const
{
    return sText;
}

//***********************   Tok_Whitespace     ******************//


void
Tok_Whitespace::Trigger( TokenInterpreter & io_rInterpreter ) const
{
    io_rInterpreter.Hdl_Whitespace(*this);
}

const char *
Tok_Whitespace::Text() const
{
    return C_sSpace + 299 - nSize;
}



//***********************   Tok_LineStart     ******************//


void
Tok_LineStart::Trigger( TokenInterpreter &	io_rInterpreter ) const
{
    io_rInterpreter.Hdl_LineStart(*this);
}

const char *
Tok_LineStart::Text() const
{
    return C_sSpace + 299 - nSize;
}


//***********************   Tok_Eol     ******************//

void
Tok_Eol::Trigger( TokenInterpreter & io_rInterpreter ) const
{
    io_rInterpreter.Hdl_Eol(*this);
}

const char *
Tok_Eol::Text() const
{
    return "\n";
}



//***********************   Tok_EoDocu     ******************//

void
Tok_EoDocu::Trigger( TokenInterpreter & io_rInterpreter ) const
{
    io_rInterpreter.Hdl_EoDocu(*this);
}

const char *
Tok_EoDocu::Text() const
{
    return "*/";
}

}   // namespace adoc


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
