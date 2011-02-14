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

#ifndef _BREAKIT_HXX
#define _BREAKIT_HXX

#include <tools/solar.h>
#include <i18npool/lang.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/XScriptTypeDetector.hpp>
#include <com/sun/star/i18n/ForbiddenCharacters.hdl>
#include <swdllapi.h>

class String;

/*************************************************************************
 *                      class SwBreakIt
 *************************************************************************/


class SW_DLLPUBLIC SwBreakIt
{
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > m_xMSF;
    mutable com::sun::star::uno::Reference< com::sun::star::i18n::XBreakIterator > xBreak;
    com::sun::star::uno::Reference< com::sun::star::i18n::XScriptTypeDetector > xCTLDetect;


    com::sun::star::lang::Locale * m_pLocale;
    com::sun::star::i18n::ForbiddenCharacters * m_pForbidden;

    LanguageType aLast;          // language of the current locale
    LanguageType aForbiddenLang; // language of the current forbiddenChar struct

    void _GetLocale( const LanguageType aLang );
    void _GetForbidden( const LanguageType  aLang );

    void createBreakIterator() const;
    void createScriptTypeDetector();

    // forbidden and not implemented.
    SwBreakIt();
    SwBreakIt( const SwBreakIt &);
    SwBreakIt & operator= ( const SwBreakIt &);

    // private (see @ _Create, _Delete).
    explicit SwBreakIt(
        const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rxMSF);
    ~SwBreakIt();

public:
    // private (see @ source/core/bastyp/init.cxx).
    static void _Create(
        const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rxMSF);
    static void _Delete();

public:
    static SwBreakIt * Get();

    com::sun::star::uno::Reference< com::sun::star::i18n::XBreakIterator > GetBreakIter()
    {
        createBreakIterator();
        return xBreak;
    }

    com::sun::star::uno::Reference< com::sun::star::i18n::XScriptTypeDetector > GetScriptTypeDetector()
    {
        createScriptTypeDetector();
        return xCTLDetect;
    }

    const com::sun::star::lang::Locale& GetLocale( const LanguageType aLang )
    {
        if( !m_pLocale || aLast != aLang )
            _GetLocale( aLang );
        return *m_pLocale;
    }

    const com::sun::star::i18n::ForbiddenCharacters& GetForbidden( const LanguageType aLang )
    {
        if( !m_pForbidden || aForbiddenLang != aLang )
            _GetForbidden( aLang );
        return *m_pForbidden;
    }

    sal_uInt16 GetRealScriptOfText( const String& rTxt, xub_StrLen nPos ) const;
       sal_uInt16 GetAllScriptsOfText( const String& rTxt ) const;
};

#define SW_BREAKITER()  SwBreakIt::Get()
#define SW_XBREAKITER() SW_BREAKITER()->GetBreakIter()

// @@@ backward compatibility @@@
SW_DLLPUBLIC extern SwBreakIt* pBreakIt;

#endif

