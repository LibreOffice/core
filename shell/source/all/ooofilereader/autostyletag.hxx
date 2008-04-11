/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: autostyletag.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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