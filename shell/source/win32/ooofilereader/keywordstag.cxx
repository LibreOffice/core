/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
