/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: keywordstag.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:44:13 $
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

#ifndef KEYWORDSTAG_HXX_INCLUDED
#include "keywordstag.hxx"
#endif

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

