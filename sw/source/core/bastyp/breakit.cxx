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

#include <breakit.hxx>
#include <swtypes.hxx>

#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <svl/languageoptions.hxx>
#include <unicode/uchar.h>
#include <unotools/localedatawrapper.hxx>
#include <algorithm>
#include <utility>

using namespace com::sun::star;

SwBreakIt* g_pBreakIt = nullptr;

void SwBreakIt::Create_( const uno::Reference<uno::XComponentContext> & rxContext )
{
    delete g_pBreakIt;
    g_pBreakIt = new SwBreakIt( rxContext );
}

void SwBreakIt::Delete_()
{
    delete g_pBreakIt;
    g_pBreakIt = nullptr;
}

SwBreakIt * SwBreakIt::Get()
{
    return g_pBreakIt;
}

SwBreakIt::SwBreakIt( uno::Reference<uno::XComponentContext> xContext )
    : m_xContext(std::move(xContext))
    , m_xBreak(i18n::BreakIterator::create(m_xContext))
    , m_aForbiddenLang(LANGUAGE_DONTKNOW)
{
}

void SwBreakIt::GetLocale_( const LanguageType aLang )
{
    if (m_xLanguageTag)
        m_xLanguageTag->reset(aLang);
    else
        m_xLanguageTag.reset(new LanguageTag(aLang));
}

void SwBreakIt::GetLocale_( const LanguageTag& rLanguageTag )
{
    if (m_xLanguageTag)
        *m_xLanguageTag = rLanguageTag;
    else
        m_xLanguageTag.reset(new LanguageTag(rLanguageTag));
}

void SwBreakIt::GetForbidden_( const LanguageType aLang )
{
    const LocaleDataWrapper* pWrap = LocaleDataWrapper::get(GetLanguageTag(aLang));

    m_aForbiddenLang = aLang;
    m_oForbidden.emplace(pWrap->getForbiddenCharacters());
}

sal_uInt16 SwBreakIt::GetRealScriptOfText( const OUString& rText, sal_Int32 nPos ) const
{
    sal_uInt16 nScript = i18n::ScriptType::WEAK;
    if (!rText.isEmpty())
    {
        if( nPos && nPos == rText.getLength() )
            --nPos;
        else if( nPos < 0)
            nPos = 0;

        nScript = m_xBreak->getScriptType(rText, nPos);
        sal_Int32 nChgPos = 0;
        if (i18n::ScriptType::WEAK == nScript && nPos >= 0 && nPos + 1 < rText.getLength())
        {
            // A weak character followed by a mark may be meant to combine with
            // the mark, so prefer the following character's script
            switch (u_charType(rText[nPos + 1]))
            {
                case U_NON_SPACING_MARK:
                case U_ENCLOSING_MARK:
                case U_COMBINING_SPACING_MARK:
                    nScript = m_xBreak->getScriptType(rText, nPos+1);
                    break;
            }
        }
        if( i18n::ScriptType::WEAK == nScript && nPos )
        {
            nChgPos = m_xBreak->beginOfScript(rText, nPos, nScript);
            if( 0 < nChgPos )
                nScript = m_xBreak->getScriptType(rText, nChgPos-1);
        }

        if( i18n::ScriptType::WEAK == nScript )
        {
            nChgPos = m_xBreak->endOfScript(rText, nPos, nScript);
            if( rText.getLength() > nChgPos && 0 <= nChgPos )
                nScript = m_xBreak->getScriptType(rText, nChgPos);
        }
    }
    if( i18n::ScriptType::WEAK == nScript )
        nScript = SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() );
    return nScript;
}

SvtScriptType SwBreakIt::GetAllScriptsOfText( const OUString& rText ) const
{
    const SvtScriptType coAllScripts = SvtScriptType::LATIN |
                                       SvtScriptType::ASIAN |
                                       SvtScriptType::COMPLEX;
    SvtScriptType nRet = SvtScriptType::NONE;
    sal_uInt16 nScript = 0;
    if (!rText.isEmpty())
    {
        for( sal_Int32 n = 0, nEnd = rText.getLength(); n < nEnd;
                n = m_xBreak->endOfScript(rText, n, nScript) )
        {
            nScript = m_xBreak->getScriptType(rText, n);
            switch( nScript )
            {
            case i18n::ScriptType::LATIN:   nRet |= SvtScriptType::LATIN;   break;
            case i18n::ScriptType::ASIAN:   nRet |= SvtScriptType::ASIAN;   break;
            case i18n::ScriptType::COMPLEX: nRet |= SvtScriptType::COMPLEX; break;
            case i18n::ScriptType::WEAK:
                    if( nRet == SvtScriptType::NONE )
                        nRet |= coAllScripts;
                    break;
            }
            if( coAllScripts == nRet )
                break;
        }
    }
    return nRet;
}

sal_Int32 SwBreakIt::getGraphemeCount(const OUString& rText,
                                      sal_Int32 nStart, sal_Int32 nEnd) const
{
    sal_Int32 nGraphemeCount = 0;

    sal_Int32 nCurPos = std::max(static_cast<sal_Int32>(0), nStart);
    while (nCurPos < nEnd)
    {
        // fdo#49208 cheat and assume that nothing can combine with a space
        // to form a single grapheme
        if (rText[nCurPos] == ' ')
        {
            ++nCurPos;
        }
        else
        {
            sal_Int32 nCount2 = 1;
            nCurPos = m_xBreak->nextCharacters(rText, nCurPos, lang::Locale(),
                i18n::CharacterIteratorMode::SKIPCELL, nCount2, nCount2);
        }
        ++nGraphemeCount;
    }

    return nGraphemeCount;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
