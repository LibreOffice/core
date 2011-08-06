/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
