/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

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
Tok_AtTag::EV_TokenId ev_since(Tok_AtTag::since,"@since");


namespace lux
{
template<> EnumValueMap &
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

