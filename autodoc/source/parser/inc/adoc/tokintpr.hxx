/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tokintpr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:37:43 $
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

#ifndef ADC_ADOC_TOKINTPR_HXX
#define ADC_ADOC_TOKINTPR_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS

namespace adoc {


class Tok_at_std;
class Tok_at_base;
class Tok_at_exception;
class Tok_at_impl;
class Tok_at_key;
class Tok_at_param;
class Tok_at_see;
class Tok_at_template;
class Tok_at_interface;
class Tok_at_internal;
class Tok_at_obsolete;
class Tok_at_module;
class Tok_at_file;
class Tok_at_gloss;
class Tok_at_global;
class Tok_at_include;
class Tok_at_label;
class Tok_at_since;
class Tok_at_HTML;          // Sets default to: Use HTML in DocuText
class Tok_at_NOHTML;        // Sets default to: Don't use HTML in DocuText

class Tok_DocWord;
class Tok_LineStart;
class Tok_Whitespace;
class Tok_Eol;
class Tok_EoDocu;


#define DECL_TOK_HANDLER(token) \
    virtual void        Hdl_##token( \
                            const Tok_##token & i_rTok ) = 0



class TokenInterpreter
{
  public:
    virtual             ~TokenInterpreter() {}

                        DECL_TOK_HANDLER(at_std);
                        DECL_TOK_HANDLER(at_base);
                        DECL_TOK_HANDLER(at_exception);
                        DECL_TOK_HANDLER(at_impl);
                        DECL_TOK_HANDLER(at_key);
                        DECL_TOK_HANDLER(at_param);
                        DECL_TOK_HANDLER(at_see);
                        DECL_TOK_HANDLER(at_template);
                        DECL_TOK_HANDLER(at_interface);
                        DECL_TOK_HANDLER(at_internal);
                        DECL_TOK_HANDLER(at_obsolete);
                        DECL_TOK_HANDLER(at_module);
                        DECL_TOK_HANDLER(at_file);
                        DECL_TOK_HANDLER(at_gloss);
                        DECL_TOK_HANDLER(at_global);
                        DECL_TOK_HANDLER(at_include);
                        DECL_TOK_HANDLER(at_label);
                        DECL_TOK_HANDLER(at_since);
                        DECL_TOK_HANDLER(at_HTML);
                        DECL_TOK_HANDLER(at_NOHTML);
                        DECL_TOK_HANDLER(DocWord);
                        DECL_TOK_HANDLER(Whitespace);
                        DECL_TOK_HANDLER(LineStart);
                        DECL_TOK_HANDLER(Eol);
                        DECL_TOK_HANDLER(EoDocu);
};

#undef DECL_TOK_HANDLER


// IMPLEMENTATION


}   // namespace adoc


#endif

