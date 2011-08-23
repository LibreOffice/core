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
    virtual void		Hdl_##token( \
                            const Tok_##token & i_rTok ) = 0



class TokenInterpreter
{
  public:
    virtual				~TokenInterpreter() {}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
