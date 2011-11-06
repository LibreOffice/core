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
#include "autostyletag.hxx"

/***********************   CAutoStyleTag  ***********************/

CAutoStyleTag::CAutoStyleTag( const XmlTagAttributes_t& attributes ):
        m_CurrentStyleLocalePair( EMPTY_STYLELOCALE_PAIR )
{
    addAttributes( attributes);
};

void CAutoStyleTag::startTag()
{
}

void CAutoStyleTag::endTag()
{
}

void CAutoStyleTag::addCharacters(const std::wstring&)
{
}

void CAutoStyleTag::addAttributes(const XmlTagAttributes_t& attributes)
{
    if ( EMPTY_STYLELOCALE_PAIR == m_CurrentStyleLocalePair )
    {
        // the style-locale pair should be empty when entering STYLE_STYLE
        // tag, and otherwise should be STYLE_PROPERTIES.

        XmlTagAttributes_t::const_iterator iter = attributes.find(CONTENT_STYLE_STYLE_NAME);

        if  ( iter != attributes.end())
            setStyle( iter->second );
    }
    else
    {
        // tag STYLE_PROPERTIES entered.

        XmlTagAttributes_t::const_iterator iter_lan = attributes.find(CONTENT_STYLE_PROPERTIES_LANGUAGE);
        XmlTagAttributes_t::const_iterator iter_con = attributes.find(CONTENT_STYLE_PROPERTIES_COUNTRY);
        XmlTagAttributes_t::const_iterator iter_lan_asain = attributes.find(CONTENT_STYLE_PROPERTIES_LANGUAGEASIAN);
        XmlTagAttributes_t::const_iterator iter_con_asain = attributes.find(CONTENT_STYLE_PROPERTIES_COUNTRYASIAN);

        // if style:properties | fo:language or style:language-asian is exist,
        // set the locale field, otherwise clear the style-locale pair;
        if ( ( iter_lan!= attributes.end() ) && ( iter_con != attributes.end() ) )
            setLocale( ::std::make_pair( iter_lan->second,iter_con->second ) );
        else if ( ( iter_lan_asain!= attributes.end() ) && ( iter_con_asain != attributes.end() ) )
                setLocale( ::std::make_pair( iter_lan_asain->second,iter_con_asain->second ) );
        else
            clearStyleLocalePair();
    }

}

void CAutoStyleTag::setStyle( ::std::wstring const & Style )
{
    m_CurrentStyleLocalePair.first = Style;
}

void CAutoStyleTag::setLocale( LocaleSet_t Locale )
{
    m_CurrentStyleLocalePair.second = Locale;
}

void CAutoStyleTag::clearStyleLocalePair(  )
{
    m_CurrentStyleLocalePair  = EMPTY_STYLELOCALE_PAIR;
}


