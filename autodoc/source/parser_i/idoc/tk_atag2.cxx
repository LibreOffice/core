/*************************************************************************
 *
 *  $RCSfile: tk_atag2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: np $ $Date: 2002-05-14 09:02:21 $
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
#include <s2_dsapi/tk_atag2.hxx>


// NOT FULLY DEFINED SERVICES
#include <s2_dsapi/tokintpr.hxx>



using csi::dsapi::Tok_AtTag;

lux::EnumValueMap   G_aTokAtTag_EV_TokenId_Values;
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


namespace lux
{
EnumValueMap &
Tok_AtTag::EV_TokenId::Values_()    { return G_aTokAtTag_EV_TokenId_Values; }
}


namespace csi
{
namespace dsapi
{

void
Tok_AtTag::Trigger( TokenInterpreter &  io_rInterpreter ) const
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

