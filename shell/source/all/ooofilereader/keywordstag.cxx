/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: keywordstag.cxx,v $
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

