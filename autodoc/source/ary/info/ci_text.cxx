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
#include <ary/info/ci_text.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/info/all_dts.hxx>


namespace ary
{
namespace info
{

DocuText::DocuText()
    :   bUsesHtml(false)
{
}

DocuText::~DocuText()
{
    for ( TokenList::iterator iter = aTokens.begin();
          iter != aTokens.end();
          ++iter )
    {
        delete (*iter);
    }
}

void
DocuText::StoreAt( DocuDisplay & o_rDisplay ) const
{
    ary::info::DocuText::TokenList::const_iterator itEnd = aTokens.end();
    for ( ary::info::DocuText::TokenList::const_iterator it = aTokens.begin();
          it != itEnd;
          ++it )
    {
        (*it)->StoreAt(o_rDisplay);
    }
}

}   // namespace info
}   // namespace ary


