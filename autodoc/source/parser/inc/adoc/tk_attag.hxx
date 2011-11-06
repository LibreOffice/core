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

