/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tk_attag.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:47:16 $
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
#include <adoc/tk_attag.hxx>


// NOT FULLY DEFINED SERVICES
#include <adoc/tokintpr.hxx>



namespace adoc {

#if 0
#define  EV_AtTagId( val, tex ) ENUM_VALUE(E_AtTagId, eATTAGID_##val, val, tex )

EV_AtTagId(atid_ATT, "ATTENTION" );
EV_AtTagId(atid_author, "Author" );
EV_AtTagId(atid_change, "Change" );
EV_AtTagId(atid_collab, "Collaborators" );
EV_AtTagId(atid_contact, "Contact" );
EV_AtTagId(atid_copyright, "Copyright (c)" );
EV_AtTagId(atid_descr, "Description" );
EV_AtTagId(atid_devstat, "Development State" );
EV_AtTagId(atid_docdate, "Date of Documentation" );
EV_AtTagId(atid_derive, "How to Derive from this class" );
EV_AtTagId(atid_instance, "Instances" );
EV_AtTagId(atid_life, "Lifecycle" );
EV_AtTagId(atid_multi, "Multiplicity" );
EV_AtTagId(atid_onerror, "On Error" );
EV_AtTagId(atid_persist, "Persistence" );
EV_AtTagId(atid_postcond, "Postcondition" );
EV_AtTagId(atid_precond, "Precondition" );
EV_AtTagId(atid_return, "Return" );
EV_AtTagId(atid_short, "Summary" );
EV_AtTagId(atid_since, "Valid Since" );
EV_AtTagId(atid_todo, "Todo" );
EV_AtTagId(atid_version, "Version" );
#endif // 0

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

