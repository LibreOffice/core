/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

