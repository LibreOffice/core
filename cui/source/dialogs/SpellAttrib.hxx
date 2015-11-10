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
#ifndef INCLUDED_CUI_SOURCE_DIALOGS_SPELLATTRIB_HXX
#define INCLUDED_CUI_SOURCE_DIALOGS_SPELLATTRIB_HXX

#include <vcl/txtattr.hxx>
#include <i18nlangtag/lang.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <tools/color.hxx>

#define TEXTATTR_SPELL_ERROR            (TEXTATTR_USER_START + 1)
#define TEXTATTR_SPELL_LANGUAGE         (TEXTATTR_USER_START + 2)
#define TEXTATTR_SPELL_BACKGROUND       (TEXTATTR_USER_START + 3)

namespace svx{
struct SpellErrorDescription
{
    bool                                                bIsGrammarError;
    OUString                                     sErrorText;
    OUString                                     sDialogTitle;
    OUString                                     sExplanation;
    OUString                                     sExplanationURL;
    css::lang::Locale                      aLocale;
    css::uno::Reference< css::linguistic2::XProofreader > xGrammarChecker;
    OUString                                     sServiceName; ///< service name of GrammarChecker/SpellChecker
    css::uno::Sequence< OUString >  aSuggestions;
    OUString                                     sRuleId;

    SpellErrorDescription() :
        bIsGrammarError( false ){}

    SpellErrorDescription( bool bGrammar,
                      const OUString& rText,
                      const css::lang::Locale& rLocale,
                      const css::uno::Sequence< OUString >& rSuggestions,
                      css::uno::Reference< css::linguistic2::XProofreader > rxGrammarChecker,
                      const OUString& rServiceName,
                      const OUString* pDialogTitle = nullptr,
                      const OUString* pExplanation = nullptr,
                      const OUString* pRuleId = nullptr,
                      const OUString* pExplanationURL = nullptr ) :
        bIsGrammarError( bGrammar ),
        sErrorText( rText ),
        sDialogTitle( ),
        sExplanation( ),
        sExplanationURL( ),
        aLocale( rLocale ),
        xGrammarChecker( rxGrammarChecker ),
        sServiceName( rServiceName ),
        aSuggestions( rSuggestions )
        {
            if( pDialogTitle )
                sDialogTitle = *pDialogTitle;
            if( pExplanation )
                sExplanation = *pExplanation;
            if( pExplanationURL )
                sExplanationURL = *pExplanationURL;
            if( pRuleId )
                sRuleId = *pRuleId;
        };

    bool operator==( const SpellErrorDescription& rDesc ) const
    {
        return bIsGrammarError == rDesc.bIsGrammarError &&
                sErrorText.equals( rDesc.sErrorText ) &&
                aLocale.Language.equals( rDesc.aLocale.Language ) &&
                aLocale.Country.equals( rDesc.aLocale.Country ) &&
                aLocale.Variant.equals( rDesc.aLocale.Variant ) &&
                aSuggestions == rDesc.aSuggestions &&
                xGrammarChecker == rDesc.xGrammarChecker &&
                sDialogTitle.equals( rDesc.sDialogTitle ) &&
                sExplanation.equals( rDesc.sExplanation ) &&
                sExplanationURL.equals( rDesc.sExplanationURL ) &&
                sRuleId == rDesc.sRuleId;
    }
};


class SpellErrorAttrib : public TextAttrib
{
public:

private:
    SpellErrorDescription        m_aSpellErrorDescription;

                            //not accessible
                            SpellErrorAttrib();
public:
                            SpellErrorAttrib( const SpellErrorDescription& );
                            SpellErrorAttrib( const SpellErrorAttrib& rAttr );
                            virtual ~SpellErrorAttrib();

    const SpellErrorDescription& GetErrorDescription() const { return m_aSpellErrorDescription; }


    virtual void            SetFont( vcl::Font& rFont ) const override;
    virtual TextAttrib*     Clone() const override;
    virtual bool            operator==( const TextAttrib& rAttr ) const override;
};


class SpellLanguageAttrib : public TextAttrib
{
    LanguageType m_eLanguage;

                            //not accessible
                            SpellLanguageAttrib();

public:
                            SpellLanguageAttrib(LanguageType eLanguage);
                            SpellLanguageAttrib( const SpellLanguageAttrib& rAttr );
                            virtual ~SpellLanguageAttrib();

    LanguageType            GetLanguage() const {return m_eLanguage;}

    virtual void            SetFont( vcl::Font& rFont ) const override;
    virtual TextAttrib*     Clone() const override;
    virtual bool            operator==( const TextAttrib& rAttr ) const override;
};


class SpellBackgroundAttrib : public TextAttrib
{
    Color   m_aBackgroundColor;

                            //not accessible
                            SpellBackgroundAttrib();

public:
                            SpellBackgroundAttrib(const Color& rCol);
                            SpellBackgroundAttrib( const SpellBackgroundAttrib& rAttr );
                            virtual ~SpellBackgroundAttrib();

    virtual void            SetFont( vcl::Font& rFont ) const override;
    virtual TextAttrib*     Clone() const override;
    virtual bool            operator==( const TextAttrib& rAttr ) const override;
};
}//namespace svx
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
