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



#ifndef ARY_CINFO_CI_TEXT2_HXX
#define ARY_CINFO_CI_TEXT2_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace inf
{


class DocumentationDisplay;


class DocuToken
{
  public:
    virtual             ~DocuToken() {}
    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const = 0;
    virtual bool        IsWhiteOnly() const = 0;
};


class DocuTex2
{
  public:
    typedef std::vector< DocuToken * >  TokenList;

                        DocuTex2();
    virtual             ~DocuTex2();

    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const;
    void                AddToken(
                            DYN DocuToken &     let_drToken );

    const TokenList &   Tokens() const          { return aTokens; }
    bool                IsEmpty() const;
    const String &      TextOfFirstToken() const;

    String &            Access_TextOfFirstToken();

  private:
    TokenList           aTokens;
};



// IMPLEMENTATION

}   // namespace inf
}   // namespace ary


#endif

