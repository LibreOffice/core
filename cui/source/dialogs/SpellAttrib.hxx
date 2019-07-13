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

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>

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
    css::uno::Sequence< OUString >  aSuggestions;
    OUString                                     sRuleId;

    SpellErrorDescription( bool bGrammar,
                      const OUString& rText,
                      const css::lang::Locale& rLocale,
                      const css::uno::Sequence< OUString >& rSuggestions,
                      css::uno::Reference< css::linguistic2::XProofreader > const & rxGrammarChecker,
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

    SpellErrorDescription()
        : bIsGrammarError(false)
    {
    }

    bool operator==( const SpellErrorDescription& rDesc ) const
    {
        return bIsGrammarError == rDesc.bIsGrammarError &&
                sErrorText == rDesc.sErrorText &&
                aLocale.Language == rDesc.aLocale.Language &&
                aLocale.Country == rDesc.aLocale.Country &&
                aLocale.Variant == rDesc.aLocale.Variant &&
                aSuggestions == rDesc.aSuggestions &&
                xGrammarChecker == rDesc.xGrammarChecker &&
                sDialogTitle == rDesc.sDialogTitle &&
                sExplanation == rDesc.sExplanation &&
                sExplanationURL == rDesc.sExplanationURL &&
                sRuleId == rDesc.sRuleId;
    }

    css::uno::Sequence<css::uno::Any> toSequence() const
    {
        css::uno::Sequence<css::uno::Any> aEntries(9);
        aEntries[0] <<= bIsGrammarError;
        aEntries[1] <<= sErrorText;
        aEntries[2] <<= sDialogTitle;
        aEntries[3] <<= sExplanation;
        aEntries[4] <<= sExplanationURL;
        aEntries[5] <<= aLocale;
        aEntries[6] <<= xGrammarChecker;
        aEntries[7] <<= aSuggestions;
        aEntries[8] <<= sRuleId;
        return aEntries;
    }

    void fromSequence(const css::uno::Sequence<css::uno::Any>& rEntries)
    {
        rEntries[0] >>= bIsGrammarError;
        rEntries[1] >>= sErrorText;
        rEntries[2] >>= sDialogTitle;
        rEntries[3] >>= sExplanation;
        rEntries[4] >>= sExplanationURL;
        rEntries[5] >>= aLocale;
        rEntries[6] >>= xGrammarChecker;
        rEntries[7] >>= aSuggestions;
        rEntries[8] >>= sRuleId;
    }
};

}//namespace svx
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
