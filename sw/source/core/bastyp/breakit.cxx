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

using namespace com::sun::star;

SwBreakIt* pBreakIt = 0;

void SwBreakIt::_Create( const uno::Reference<lang::XMultiServiceFactory> & rxMSF )
{
    delete pBreakIt, pBreakIt = new SwBreakIt( rxMSF );
}

void SwBreakIt::_Delete()
{
    delete pBreakIt, pBreakIt = 0;
}

SwBreakIt * SwBreakIt::Get()
{
    return pBreakIt;
}

SwBreakIt::SwBreakIt( const uno::Reference<lang::XMultiServiceFactory> & rxMSF )
    : m_xMSF( rxMSF ),
      m_pLocale( NULL ),
      m_pForbidden( NULL ),
      aLast( LANGUAGE_DONTKNOW ),
      aForbiddenLang( LANGUAGE_DONTKNOW )
{
    OSL_ENSURE( m_xMSF.is(), "SwBreakIt: no MultiServiceFactory" );
}

SwBreakIt::~SwBreakIt()
{
    delete m_pLocale;
    delete m_pForbidden;
}

void SwBreakIt::createBreakIterator() const
{
    if ( m_xMSF.is() && !xBreak.is() )
        xBreak.set( i18n::BreakIterator::create(comphelper::getComponentContext(m_xMSF)) );
}

void SwBreakIt::_GetLocale( const LanguageType aLang )
{
    aLast = aLang;
    delete m_pLocale;
    m_pLocale = new lang::Locale( SvxCreateLocale( aLast ) );
}

void SwBreakIt::_GetForbidden( const LanguageType aLang )
{
    LocaleDataWrapper aWrap( m_xMSF, GetLocale( aLang ) );

    aForbiddenLang = aLang;
    delete m_pForbidden;
    m_pForbidden = new i18n::ForbiddenCharacters( aWrap.getForbiddenCharacters() );
}

sal_uInt16 SwBreakIt::GetRealScriptOfText( const rtl::OUString& rTxt, sal_Int32 nPos ) const
{
    createBreakIterator();
    sal_uInt16 nScript = i18n::ScriptType::WEAK;
    if( xBreak.is() && !rTxt.isEmpty() )
    {
        if( nPos && nPos == rTxt.getLength() )
            --nPos;
        nScript = xBreak->getScriptType( rTxt, nPos );
        sal_Int32 nChgPos = 0;
        if ( i18n::ScriptType::WEAK == nScript && nPos + 1 < rTxt.getLength() )
        {
            // A weak character followed by a mark may be meant to combine with
            // the mark, so prefer the following character's script
            switch (u_charType(rTxt[nPos + 1]))
            {
                case U_NON_SPACING_MARK:
                case U_ENCLOSING_MARK:
                case U_COMBINING_SPACING_MARK:
                    nScript = xBreak->getScriptType( rTxt, nPos+1 );
                    break;
            }
        }
        if( i18n::ScriptType::WEAK == nScript &&
            nPos &&
            0 < ( nChgPos = xBreak->beginOfScript( rTxt, nPos, nScript ) ) )
        {
            nScript = xBreak->getScriptType( rTxt, nChgPos-1 );
        }

        if( i18n::ScriptType::WEAK == nScript &&
            rTxt.getLength() > ( nChgPos = xBreak->endOfScript( rTxt, nPos, nScript ) ) &&
            0 <= nChgPos )
        {
            nScript = xBreak->getScriptType( rTxt, nChgPos );
        }
    }
    if( i18n::ScriptType::WEAK == nScript )
        nScript = GetI18NScriptTypeOfLanguage( (sal_uInt16)GetAppLanguage() );
    return nScript;
}

sal_uInt16 SwBreakIt::GetAllScriptsOfText( const rtl::OUString& rTxt ) const
{
    const sal_uInt16 coAllScripts = ( SCRIPTTYPE_LATIN |
                                      SCRIPTTYPE_ASIAN |
                                      SCRIPTTYPE_COMPLEX );
    createBreakIterator();
    sal_uInt16 nRet = 0, nScript;
    if( !xBreak.is() )
    {
        nRet = coAllScripts;
    }
    else if( !rTxt.isEmpty() )
    {
        for( sal_Int32 n = 0, nEnd = rTxt.getLength(); n < nEnd;
                n = xBreak->endOfScript(rTxt, n, nScript) )
        {
            switch( nScript = xBreak->getScriptType( rTxt, n ) )
            {
            case i18n::ScriptType::LATIN:   nRet |= SCRIPTTYPE_LATIN;   break;
            case i18n::ScriptType::ASIAN:   nRet |= SCRIPTTYPE_ASIAN;   break;
            case i18n::ScriptType::COMPLEX: nRet |= SCRIPTTYPE_COMPLEX; break;
            case i18n::ScriptType::WEAK:
                    if( !nRet )
                        nRet |= coAllScripts;
                    break;
            }
            if( coAllScripts == nRet )
                break;
        }
    }
    return nRet;
}

sal_Int32 SwBreakIt::getGraphemeCount(const rtl::OUString& rText,
                                      sal_Int32 nStart, sal_Int32 nEnd) const
{
    sal_Int32 nGraphemeCount = 0;

    sal_Int32 nCurPos = nStart;
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
