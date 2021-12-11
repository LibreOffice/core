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
#pragma once

#include <com/sun/star/linguistic2/XDictionary.hpp>
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#include <com/sun/star/linguistic2/ProofreadingResult.hpp>
#include <com/sun/star/uno/Sequence.h>

#include <rtl/ustring.hxx>
#include <vcl/builder.hxx>
#include <vcl/menu.hxx>

#include <map>
#include <vector>

#include <swdllapi.h>

//! Don't change these values. You may break context menu modifying extensions!
#define MN_IGNORE_SELECTION             201
#define MN_SHORT_COMMENT                208
#define MN_EXPLANATION_LINK             209

// id range for dictionaries sub menu
#define MN_DICTIONARIES_START               300
#define MN_DICTIONARIES_END                 (MN_DICTIONARIES_START + 99)

// id range for suggestions from spell and grammar checker
#define MN_SUGGESTION_START                 500
#define MN_SUGGESTION_END                   (MN_SUGGESTION_START + MN_MAX_NUM_LANG)

// id range for auto correction sub menu entries
#define MN_AUTOCORR_START                   700
#define MN_AUTOCORR_END                     (MN_AUTOCORR_START + MN_MAX_NUM_LANG)

// max number of language entries sub menus
#define MN_MAX_NUM_LANG                 99

#define MN_NONE_OFFSET                  (MN_MAX_NUM_LANG + 1)
#define MN_RESET_OFFSET                 (MN_MAX_NUM_LANG + 2)
#define MN_MORE_OFFSET                  (MN_MAX_NUM_LANG + 3)

// id range for 'set language for selection' sub menu entries
#define MN_SET_LANGUAGE_SELECTION_START     900
#define MN_SET_LANGUAGE_SELECTION_END       (MN_SET_LANGUAGE_SELECTION_START + MN_MAX_NUM_LANG)
#define MN_SET_SELECTION_NONE       (MN_SET_LANGUAGE_SELECTION_START + MN_NONE_OFFSET)
#define MN_SET_SELECTION_RESET      (MN_SET_LANGUAGE_SELECTION_START + MN_RESET_OFFSET)
#define MN_SET_SELECTION_MORE       (MN_SET_LANGUAGE_SELECTION_START + MN_MORE_OFFSET)

// id range for 'set language for paragraph' sub menu entries
#define MN_SET_LANGUAGE_PARAGRAPH_START     1100
#define MN_SET_LANGUAGE_PARAGRAPH_END       (MN_SET_LANGUAGE_PARAGRAPH_START + MN_MAX_NUM_LANG)
#define MN_SET_PARA_NONE            (MN_SET_LANGUAGE_PARAGRAPH_START + MN_NONE_OFFSET)
#define MN_SET_PARA_RESET           (MN_SET_LANGUAGE_PARAGRAPH_START + MN_RESET_OFFSET)
#define MN_SET_PARA_MORE            (MN_SET_LANGUAGE_PARAGRAPH_START + MN_MORE_OFFSET)

class SwWrtShell;

class SW_DLLPUBLIC SwSpellPopup
{
    VclBuilder m_aBuilder;
    VclPtr<PopupMenu> m_xPopupMenu;
    sal_uInt16 m_nIgnoreWordId;
    sal_uInt16 m_nAddMenuId;
    sal_uInt16 m_nAddId;
    sal_uInt16 m_nSpellDialogId;
    sal_uInt16 m_nCorrectMenuId;
    sal_uInt16 m_nCorrectDialogId;
    sal_uInt16 m_nLangSelectionMenuId;
    sal_uInt16 m_nLangParaMenuId;
    sal_uInt16 m_nRedlineAcceptId;
    sal_uInt16 m_nRedlineRejectId;
    sal_uInt16 m_nRedlineNextId;
    sal_uInt16 m_nRedlinePrevId;
    SwWrtShell* m_pSh;
    css::uno::Sequence< css::uno::Reference< css::linguistic2::XDictionary >  >     m_aDics;
    css::uno::Reference< css::linguistic2::XSpellAlternatives > m_xSpellAlt;

    OUString m_sExplanationLink;

    LanguageType                m_nCheckedLanguage;

    std::map< sal_Int16, OUString > m_aLangTable_Text;
    std::map< sal_Int16, OUString > m_aLangTable_Paragraph;

    OUString  m_aDicNameSingle;
    bool      m_bGrammarResults;    // show grammar results? Or show spellcheck results?

    static void fillLangPopupMenu( PopupMenu *pPopupMenu, sal_uInt16 nLangStart,
            const css::uno::Sequence< OUString >& aSeq, SwWrtShell* pWrtSh,
            std::map< sal_Int16, OUString > &rLangTable );

    /// Checks if any of the redline menu items should be hidden.
    void checkRedline();

public:
    SwSpellPopup( SwWrtShell *pWrtSh,
            const css::uno::Reference< css::linguistic2::XSpellAlternatives >  &xAlt,
            const OUString & rParaText );

    SwSpellPopup( SwWrtShell *pWrtSh,
            const css::linguistic2::ProofreadingResult &rResult,
            sal_Int32 nErrorInResult,
            const css::uno::Sequence< OUString > &rSuggestions,
            const OUString & rParaText );

    ~SwSpellPopup();

    void InitItemCommands(const css::uno::Sequence< OUString >& aSuggestions);

    PopupMenu& GetMenu()
    {
        return *m_xPopupMenu;
    }

    css::uno::Reference<css::awt::XPopupMenu> CreateMenuInterface() { return m_xPopupMenu->CreateMenuInterface(); }

    void Execute( const tools::Rectangle& rPopupPos, vcl::Window* pWin );
    void Execute( sal_uInt16 nId );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
