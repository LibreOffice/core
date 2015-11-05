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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_OLMENU_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_OLMENU_HXX

#include <com/sun/star/linguistic2/XDictionary.hpp>
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#include <com/sun/star/linguistic2/ProofreadingResult.hpp>
#include <com/sun/star/uno/Sequence.h>

#include <rtl/ustring.hxx>
#include <vcl/image.hxx>
#include <vcl/menu.hxx>

#include <map>
#include <vector>

class SwWrtShell;

class SW_DLLPUBLIC SwSpellPopup : public PopupMenu
{
    SwWrtShell* m_pSh;
    css::uno::Sequence< css::uno::Reference<
        css::linguistic2::XDictionary >  >     m_aDics;
    css::uno::Reference<
        css::linguistic2::XSpellAlternatives > m_xSpellAlt;

    css::linguistic2::ProofreadingResult m_xGrammarResult;
    sal_Int32 m_nGrammarError;

    css::uno::Sequence< OUString >  m_aSuggestions;
    OUString m_sExplanationLink;

    LanguageType                m_nCheckedLanguage;
    LanguageType                m_nGuessLangWord;
    LanguageType                m_nGuessLangPara;

    std::map< sal_Int16, OUString > m_aLangTable_Text;
    std::map< sal_Int16, OUString > m_aLangTable_Paragraph;
//    std::map< sal_Int16, OUString > aLangTable_Document;

    OUString  m_aDicNameSingle;
    bool      m_bGrammarResults;    // show grammar results? Or show spellcheck results?

    Image     m_aInfo16;

    static void fillLangPopupMenu( PopupMenu *pPopupMenu, sal_uInt16 nLangStart,
            const css::uno::Sequence< OUString >& aSeq, SwWrtShell* pWrtSh,
            std::map< sal_Int16, OUString > &rLangTable );

    using PopupMenu::Execute;

    /// Checks if any of the redline menu items should be hidden.
    void checkRedline();

public:
    SwSpellPopup( SwWrtShell *pWrtSh,
            const css::uno::Reference<
                css::linguistic2::XSpellAlternatives >  &xAlt,
            const OUString & rParaText );

    SwSpellPopup( SwWrtShell *pWrtSh,
            const css::linguistic2::ProofreadingResult &rResult,
            sal_Int32 nErrorInResult,
            const css::uno::Sequence< OUString > &rSuggestions,
            const OUString & rParaText );

    sal_uInt16  Execute( const Rectangle& rPopupPos, vcl::Window* pWin );
    void Execute( sal_uInt16 nId );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
