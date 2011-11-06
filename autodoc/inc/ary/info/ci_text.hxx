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



#ifndef ARY_INFO_CI_TEXT_HXX
#define ARY_INFO_CI_TEXT_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace info
{

class DocuToken;
class DocuDisplay;


class DocuText
{
  public:
    typedef std::vector< DocuToken * >  TokenList;

                        DocuText();
                        ~DocuText();

    void                Set_HtmlUse(
                            bool                i_bUseIt )
                                                { bUsesHtml = i_bUseIt; }
    void                Add_Token(
                            DYN DocuToken &     let_drToken )
                                                { aTokens.push_back(&let_drToken); }
    const TokenList &   Tokens() const          { return aTokens; }
    void                StoreAt(
                            DocuDisplay &       o_rDisplay ) const;
    bool                IsNoHtml() const        { return NOT bUsesHtml; }
    bool                IsEmpty() const         { return aTokens.size() == 0; }

  private:
    TokenList           aTokens;
    bool                bUsesHtml;
};






// IMPLEMENTATION


}
}

#endif

