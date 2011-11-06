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




#ifndef AUTOSTYLETAG_HXX_INCLUDED
#define AUTOSTYLETAG_HXX_INCLUDED

#include "itag.hxx"

/***************************   CAutoStyleTag tag readers   ***************************/

/** Implements the ITag interface for
    building a Style-Locale list

    Usage sample:

    LocaleSet_t locale = meta_info_accessor.getDefaultLocale();
    CContentReader content( m_szFileName, locale );
    CStyleMap map = content.getStyleMap();
*/
class CAutoStyleTag : public ITag
{
    public:
        CAutoStyleTag():m_CurrentStyleLocalePair( EMPTY_STYLELOCALE_PAIR ){};
        CAutoStyleTag( const XmlTagAttributes_t& attributes );

        virtual void startTag();
        virtual void endTag();
        virtual void addCharacters(const std::wstring& characters);
        virtual void addAttributes(const XmlTagAttributes_t& attributes);
        virtual std::wstring getTagContent(){ return EMPTY_STRING; };
        virtual ::std::wstring const getTagAttribute( ::std::wstring  const & /*attrname*/ ){ return ::std::wstring() ; };

        void setStyle( ::std::wstring const & Style );
        void setLocale( LocaleSet_t Locale );
        void clearStyleLocalePair( void );
        inline StyleLocalePair_t getStyleLocalePair() const{ return m_CurrentStyleLocalePair; };
        inline bool isFull() const
        {
            return (( m_CurrentStyleLocalePair.first != EMPTY_STRING )&&
                   ( m_CurrentStyleLocalePair.second != EMPTY_LOCALE));
        };

    private:
        StyleLocalePair_t m_CurrentStyleLocalePair;
};

#endif
