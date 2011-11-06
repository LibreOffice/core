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



#ifndef ARY_CI_ATAG2_HXX
#define ARY_CI_ATAG2_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <ary_i/ci_text2.hxx>
    // PARAMETERS



namespace ary
{
namespace inf
{


class DocumentationDisplay;

class AtTag2
{
  public:
    virtual             ~AtTag2() {}

    virtual void        DisplayAt(
                            DocumentationDisplay &
                                                o_rDisplay ) const = 0;

    const char *        Title() const           { return sTitle; }
    const DocuTex2 &    Text() const            { return aText; }
    DocuTex2 &          Access_Text()           { return aText; }

  protected:
                        AtTag2(
                            const char *        i_sTitle)
                                                :   sTitle(i_sTitle) {}
    String              sTitle;
    DocuTex2            aText;
};



// IMPLEMENTATION

}   // namespace inf
}   // namespace ary


#endif

