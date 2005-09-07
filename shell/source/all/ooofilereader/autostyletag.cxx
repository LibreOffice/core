/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: autostyletag.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:42:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef AUTOSTYLETAG_HXX_INCLUDED
#include "autostyletag.hxx"
#endif

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

void CAutoStyleTag::addCharacters(const std::wstring& characters)
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


