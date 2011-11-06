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




#ifndef KEYWORDSTAG_HXX_INCLUDED
#define KEYWORDSTAG_HXX_INCLUDED

#include "itag.hxx"

/***************************   metainfo tag readers   ***************************/

/** Implements the ITag interface for
    building a keyword list
*/
class CKeywordsTag : public ITag
{
    public:
        virtual void startTag();
        virtual void endTag();
        virtual void addCharacters(const std::wstring& characters);
        virtual void addAttributes(const XmlTagAttributes_t& attributes);
        virtual std::wstring getTagContent();
        virtual ::std::wstring const getTagAttribute( ::std::wstring  const & /*attrname*/ ){ return ::std::wstring(); };

    private:
        StringList_t m_slKeywords;
        std::wstring m_sCurrentKeyword;
};

#endif
