/*************************************************************************
 *
 *  $RCSfile: tk_attag.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:38:15 $
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
    // KORR
    return "Ein Tag";

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

