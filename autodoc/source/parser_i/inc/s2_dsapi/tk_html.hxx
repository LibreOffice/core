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



#ifndef DSAPI_TK_HTML_HXX
#define DSAPI_TK_HTML_HXX


// USED SERVICES
    // BASE CLASSES
#include <s2_dsapi/dsapitok.hxx>
    // COMPONENTS
    // PARAMETERS

namespace csi
{
namespace dsapi
{


class Tok_HtmlTag : public Token
{
  public:
    // Spring and Fall
                        Tok_HtmlTag(
                            const char *        i_sTag,
                            bool                i_bIsParagraphStarter )
                                                :   sTag(i_sTag),
                                                    bIsParagraphStarter(i_bIsParagraphStarter)
                                                {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char* Text() const;
    bool                IsParagraphStarter() const
                                                { return bIsParagraphStarter; }

  private:
    String              sTag;
    bool                bIsParagraphStarter;
};


}   // namespace dsapi
}   // namespace csi


#endif

