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
#include <s2_dsapi/tk_atag2.hxx>


// NOT FULLY DEFINED SERVICES
#include <s2_dsapi/tokintpr.hxx>



using csi::dsapi::Tok_AtTag;

lux::EnumValueMap	G_aTokAtTag_EV_TokenId_Values;
Tok_AtTag::EV_TokenId ev_none2(Tok_AtTag::e_none,"");
Tok_AtTag::EV_TokenId ev_author(Tok_AtTag::author,"@author");
Tok_AtTag::EV_TokenId ev_see(Tok_AtTag::see,"@see");
Tok_AtTag::EV_TokenId ev_param(Tok_AtTag::param,"@param");
Tok_AtTag::EV_TokenId ev_e_return(Tok_AtTag::e_return,"@return");
Tok_AtTag::EV_TokenId ev_e_throw(Tok_AtTag::e_throw,"@throws");
Tok_AtTag::EV_TokenId ev_example(Tok_AtTag::example,"@example");
Tok_AtTag::EV_TokenId ev_deprecated(Tok_AtTag::deprecated,"@deprecated");
Tok_AtTag::EV_TokenId ev_suspicious(Tok_AtTag::suspicious,"@suspicious");
Tok_AtTag::EV_TokenId ev_missing(Tok_AtTag::missing,"@missing");
Tok_AtTag::EV_TokenId ev_incomplete(Tok_AtTag::incomplete,"@incomplete");
Tok_AtTag::EV_TokenId ev_version(Tok_AtTag::version,"@version");
Tok_AtTag::EV_TokenId ev_guarantees(Tok_AtTag::guarantees,"@guarantees");
Tok_AtTag::EV_TokenId ev_exception(Tok_AtTag::exception,"@exception");
Tok_AtTag::EV_TokenId ev_since(Tok_AtTag::since,"@since");


namespace lux
{
template<> EnumValueMap &
Tok_AtTag::EV_TokenId::Values_()	{ return G_aTokAtTag_EV_TokenId_Values; }
}


namespace csi
{
namespace dsapi
{

void
Tok_AtTag::Trigger( TokenInterpreter &	io_rInterpreter ) const
{
    io_rInterpreter.Process_AtTag(*this);
}

const char *
Tok_AtTag::Text() const
{
    return eTag.Text();
}

}   // namespace dsapi
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
