/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tk_docw2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:54:39 $
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
#include <s2_dsapi/tk_docw2.hxx>


// NOT FULLY DEFINED SERVICES
#include <s2_dsapi/tokintpr.hxx>



namespace csi
{
namespace dsapi
{

void
Tok_Word::Trigger( TokenInterpreter &   io_rInterpreter ) const
{
    io_rInterpreter.Process_Word(*this);
}

const char *
Tok_Word::Text() const
{
    return sText;
}

void
Tok_Comma::Trigger( TokenInterpreter &  io_rInterpreter ) const
{
    io_rInterpreter.Process_Comma();
}

const char *
Tok_Comma::Text() const
{
    return ",";
}

void
Tok_DocuEnd::Trigger( TokenInterpreter &    io_rInterpreter ) const
{
    io_rInterpreter.Process_DocuEnd();
}

const char *
Tok_DocuEnd::Text() const
{
    return "*/";
}

void
Tok_EOL::Trigger( TokenInterpreter &    io_rInterpreter ) const
{
    io_rInterpreter.Process_EOL();
}

const char *
Tok_EOL::Text() const
{
    return "\r\n";
}

void
Tok_EOF::Trigger( TokenInterpreter &    io_rInterpreter ) const
{
    csv_assert(false);
}

const char *
Tok_EOF::Text() const
{
    return "";
}



}   // namespace dsapi
}   // namespace csi

