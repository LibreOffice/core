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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"
#include "keywordstag.hxx"

/***********************   CKeywordsTag  ***********************/

void CKeywordsTag::startTag()
{
    m_sCurrentKeyword.clear();
}

void CKeywordsTag::endTag()
{
    m_slKeywords.push_back(m_sCurrentKeyword);
}

void CKeywordsTag::addCharacters(const std::wstring& characters)
{
    m_sCurrentKeyword += characters;
}

void CKeywordsTag::addAttributes(const XmlTagAttributes_t& /*attributes*/)
{
    // there are no attributes for keywords
}

std::wstring CKeywordsTag::getTagContent( )
{
    StringList_t::const_iterator keywords_Iter= m_slKeywords.begin( );
    StringList_t::const_iterator keywords_Iter_end = m_slKeywords.end( );

    std::wstring ret_KeyWord_String = ( keywords_Iter != keywords_Iter_end) ? *keywords_Iter++ : L"";
    for ( ; keywords_Iter != keywords_Iter_end; ++keywords_Iter)
        ret_KeyWord_String += L"," + *keywords_Iter;
    return ret_KeyWord_String;
}

