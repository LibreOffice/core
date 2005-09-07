/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tk_attag.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:37:05 $
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

#ifndef ADC_ADOC_TK_ATTAG_HXX
#define ADC_ADOC_TK_ATTAG_HXX

// USED SERVICES
    // BASE CLASSES
#include <adoc/adoc_tok.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/info/inftypes.hxx>

namespace adoc {

typedef ary::info::E_AtTagId E_AtTagId;


class Tok_at_std : public Token
{
  public:
                        Tok_at_std(
                            E_AtTagId           i_nId )
                                                : eId(i_nId) {}
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    virtual const char *
                        Text() const;
    E_AtTagId           Id() const              { return eId; }

  private:
    E_AtTagId           eId;
};


#define DECL_TOKEN_CLASS(name) \
class Tok_##name : public Token \
{ public: \
    virtual void        Trigger( \
                            TokenInterpreter &  io_rInterpreter ) const; \
    virtual const char * \
                        Text() const; \
}


DECL_TOKEN_CLASS(at_base);
DECL_TOKEN_CLASS(at_exception);
DECL_TOKEN_CLASS(at_impl);
DECL_TOKEN_CLASS(at_key);
DECL_TOKEN_CLASS(at_param);
DECL_TOKEN_CLASS(at_see);
DECL_TOKEN_CLASS(at_template);
DECL_TOKEN_CLASS(at_interface);
DECL_TOKEN_CLASS(at_internal);
DECL_TOKEN_CLASS(at_obsolete);
DECL_TOKEN_CLASS(at_module);
DECL_TOKEN_CLASS(at_file);
DECL_TOKEN_CLASS(at_gloss);
DECL_TOKEN_CLASS(at_global);
DECL_TOKEN_CLASS(at_include);
DECL_TOKEN_CLASS(at_label);
DECL_TOKEN_CLASS(at_HTML);
DECL_TOKEN_CLASS(at_NOHTML);
DECL_TOKEN_CLASS(at_since);


#undef DECL_TOKEN_CLASS



}   // namespace adoc

#endif

