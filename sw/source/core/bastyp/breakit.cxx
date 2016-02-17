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

#include "breakit.hxx"
#include "swtypes.hxx"

#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <editeng/unolingu.hxx>
#include <editeng/scripttypeitem.hxx>
#include <unicode/uchar.h>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>
#include <algorithm>

using namespace com::sun::star;

SwBreakIt* g_pBreakIt = nullptr;

void SwBreakIt::_Create( const uno::Reference<uno::XComponentContext> & rxContext )
{
    delete g_pBreakIt;
    g_pBreakIt = new SwBreakIt( rxContext );
}

void SwBreakIt::_Delete()
{
    delete g_pBreakIt;
    g_pBreakIt = nullptr;
}

SwBreakIt * SwBreakIt::Get()
{
    return g_pBreakIt;
}

SwBreakIt::SwBreakIt( const uno::Reference<uno::XComponentContext> & rxContext )
    : m_xContext( rxContext ),
      m_pLanguageTag( nullptr ),
      m_pForbidden( nullptr ),
      aForbiddenLang( LANGUAGE_DONTKNOW )
{
    OSL_ENSURE( m_xContext.is(), "SwBreakIt: no MultiServiceFactory" );
}

SwBreakIt::~SwBreakIt()
{
    delete m_pLanguageTag;
    delete m_pForbidden;
}

void SwBreakIt::createBreakIterator() const
{
    if ( m_xContext.is() && !xBreak.is() )
        xBreak.set( i18n::BreakIterator::create(m_xContext) );
}

void SwBreakIt::_GetLocale( const LanguageType aLang )
{
    if (m_pLanguageTag)
        m_pLanguageTag->reset( aLang );
    else
        m_pLanguageTag = new LanguageTag( aLang );
}

void SwBreakIt::_GetLocale( const LanguageTag& rLanguageTag )
{
    if (m_pLanguageTag)
        *m_pLanguageTag = rLanguageTag;
    else
        m_pLanguageTag = new LanguageTag( rLanguageTag );
}

void SwBreakIt::_GetForbidden( const LanguageType aLang )
{
    LocaleDataWrapper aWrap( m_xContext, GetLanguageTag( aLang ) );

    aForbiddenLang = aLang;
    delete m_pForbidden;
    m_pForbidden = new i18n::ForbiddenCharacters( aWrap.getForbiddenCharacters() );
}

sal_uInt16 SwBreakIt::GetRealScriptOfText( const OUString& rText, sal_Int32 nPos ) const
{
    createBreakIterator();
    sal_uInt16 nScript = i18n::ScriptType::WEAK;
    if( xBreak.is() && !rText.isEmpty() )
    {
        if( nPos && nPos == rText.getLength() )
            --nPos;
        else if( nPos < 0)
            nPos = 0;

        nScript = xBreak->getScriptType( rText, nPos );
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
                    nScript = xBreak->getScriptType( rText, nPos+1 );
                    break;
            }
        }
        if( i18n::ScriptType::WEAK == nScript &&
            nPos &&
            0 < ( nChgPos = xBreak->beginOfScript( rText, nPos, nScript ) ) )
        {
            nScript = xBreak->getScriptType( rText, nChgPos-1 );
        }

        if( i18n::ScriptType::WEAK == nScript &&
            rText.getLength() > ( nChgPos = xBreak->endOfScript( rText, nPos, nScript ) ) &&
            0 <= nChgPos )
        {
            nScript = xBreak->getScriptType( rText, nChgPos );
        }
    }
    if( i18n::ScriptType::WEAK == nScript )
        nScript = SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() );
    return nScript;
}

SvtScriptType SwBreakIt::GetAllScriptsOfText( const OUString& rText ) const
{
    const SvtScriptType coAllScripts = ( SvtScriptType::LATIN |
                                      SvtScriptType::ASIAN |
                                      SvtScriptType::COMPLEX );
    createBreakIterator();
    SvtScriptType nRet = SvtScriptType::NONE;
    sal_uInt16 nScript = 0;
    if( !xBreak.is() )
    {
        nRet = coAllScripts;
    }
    else if( !rText.isEmpty() )
    {
        for( sal_Int32 n = 0, nEnd = rText.getLength(); n < nEnd;
                n = xBreak->endOfScript(rText, n, nScript) )
        {
            switch( nScript = xBreak->getScriptType( rText, n ) )
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
            nCurPos = xBreak->nextCharacters(rText, nCurPos, lang::Locale(),
                i18n::CharacterIteratorMode::SKIPCELL, nCount2, nCount2);
        }
        ++nGraphemeCount;
    }

    return nGraphemeCount;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
