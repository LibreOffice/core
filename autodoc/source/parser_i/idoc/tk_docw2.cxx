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
#include <s2_dsapi/tk_docw2.hxx>


// NOT FULLY DEFINED SERVICES
#include <s2_dsapi/tokintpr.hxx>



namespace csi
{
namespace dsapi
{

void
Tok_Word::Trigger( TokenInterpreter &	io_rInterpreter ) const
{
    io_rInterpreter.Process_Word(*this);
}

const char *
Tok_Word::Text() const
{
    return sText;
}

void
Tok_Comma::Trigger( TokenInterpreter &	io_rInterpreter ) const
{
    io_rInterpreter.Process_Comma();
}

const char *
Tok_Comma::Text() const
{
    return ",";
}

void
Tok_DocuEnd::Trigger( TokenInterpreter &	io_rInterpreter ) const
{
    io_rInterpreter.Process_DocuEnd();
}

const char *
Tok_DocuEnd::Text() const
{
    return "*/";
}

void
Tok_EOL::Trigger( TokenInterpreter &	io_rInterpreter ) const
{
    io_rInterpreter.Process_EOL();
}

const char *
Tok_EOL::Text() const
{
    return "\r\n";
}

void
Tok_EOF::Trigger( TokenInterpreter & ) const
{
    csv_assert(false);
}

const char *
Tok_EOF::Text() const
{
    return "";
}

void
Tok_White::Trigger( TokenInterpreter &	io_rInterpreter ) const
{
    io_rInterpreter.Process_White();
}

const char *
Tok_White::Text() const
{
    return " ";
}




}   // namespace dsapi
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
