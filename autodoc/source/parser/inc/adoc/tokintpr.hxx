/*************************************************************************
 *
 *  $RCSfile: tokintpr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:39:08 $
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

