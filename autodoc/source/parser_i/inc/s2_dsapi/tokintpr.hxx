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



#ifndef ADC_DSAPI_TOKINTPR_HXX
#define ADC_DSAPI_TOKINTPR_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS

namespace csi
{
namespace dsapi
{


class Tok_AtTag;
class Tok_XmlConst;
class Tok_XmlLink_BeginTag;
class Tok_XmlLink_EndTag;
class Tok_XmlFormat_BeginTag;
class Tok_XmlFormat_EndTag;
class Tok_Word;
class Tok_HtmlTag;

class TokenInterpreter
{
  public:
    virtual             ~TokenInterpreter() {}

    virtual void        Process_AtTag(
                            const Tok_AtTag &   i_rToken ) = 0;
    virtual void        Process_HtmlTag(
                            const Tok_HtmlTag & i_rToken ) = 0;
    virtual void        Process_XmlConst(
                            const Tok_XmlConst &
                                                i_rToken ) = 0;
    virtual void        Process_XmlLink_BeginTag(
                            const Tok_XmlLink_BeginTag &
                                                i_rToken ) = 0;
    virtual void        Process_XmlLink_EndTag(
                            const Tok_XmlLink_EndTag &
                                                i_rToken ) = 0;
    virtual void        Process_XmlFormat_BeginTag(
                            const Tok_XmlFormat_BeginTag &
                                                i_rToken ) = 0;
    virtual void        Process_XmlFormat_EndTag(
                            const Tok_XmlFormat_EndTag &
                                                i_rToken ) = 0;
    virtual void        Process_Word(
                            const Tok_Word &    i_rToken ) = 0;
    virtual void        Process_Comma() = 0;
    virtual void        Process_DocuEnd() = 0;
    virtual void        Process_EOL() = 0;
    virtual void        Process_White() = 0;
};



// IMPLEMENTATION


}   // namespace dsapi
}   // namespace csi

#endif

