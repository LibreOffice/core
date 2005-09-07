/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tk_docw.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:13:45 $
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
Tok_LineStart::Trigger( TokenInterpreter &  io_rInterpreter ) const
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


