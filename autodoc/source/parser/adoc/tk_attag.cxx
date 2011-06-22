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
#include <adoc/tk_attag.hxx>


// NOT FULLY DEFINED SERVICES
#include <adoc/tokintpr.hxx>



namespace adoc {

void
Tok_at_std::Trigger( TokenInterpreter & io_rInterpreter ) const
{
    io_rInterpreter.Hdl_at_std(*this);
}

const char *
Tok_at_std::Text() const
{
    // KORR_FUTURE
    return "A Tag";

//  return eId.Text();
}


#define DEFINE_TOKEN_CLASS(name, text) \
void \
Tok_##name::Trigger( TokenInterpreter & io_rInterpreter ) const \
{   io_rInterpreter.Hdl_##name(*this); } \
const char * \
Tok_##name::Text() const \
{   return text; }

DEFINE_TOKEN_CLASS(at_base,  "Base Classes")
DEFINE_TOKEN_CLASS(at_exception,  "Exceptions")
DEFINE_TOKEN_CLASS(at_impl,  "Implements")
DEFINE_TOKEN_CLASS(at_key,  "Keywords")
DEFINE_TOKEN_CLASS(at_param,  "Parameters")
DEFINE_TOKEN_CLASS(at_see,  "See Also")
DEFINE_TOKEN_CLASS(at_template,  "Template Parameters")
DEFINE_TOKEN_CLASS(at_interface, "Interface")
DEFINE_TOKEN_CLASS(at_internal,  "[ INTERNAL ]")
DEFINE_TOKEN_CLASS(at_obsolete,  "[ DEPRECATED ]")
DEFINE_TOKEN_CLASS(at_module,  "Module")
DEFINE_TOKEN_CLASS(at_file,  "File")
DEFINE_TOKEN_CLASS(at_gloss,  "Glossary")
DEFINE_TOKEN_CLASS(at_global,  "<global doc text>")
DEFINE_TOKEN_CLASS(at_include,  "<included text>")
DEFINE_TOKEN_CLASS(at_label,  "Label")
DEFINE_TOKEN_CLASS(at_HTML,  "")
DEFINE_TOKEN_CLASS(at_NOHTML,  "")
DEFINE_TOKEN_CLASS(at_since, "Since");

}   // namespace adoc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
