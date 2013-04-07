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
#ifndef _OLMENU_HXX
#define _OLMENU_HXX

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

class SwSpellPopup : public PopupMenu
{
    SwWrtShell* pSh;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >  >     aDics;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellAlternatives > xSpellAlt;

    ::com::sun::star::linguistic2::ProofreadingResult xGrammarResult;
    sal_Int32 nGrammarError;

    ::com::sun::star::uno::Sequence< OUString >  aSuggestions;
    OUString sExplanationLink;

    LanguageType                nCheckedLanguage;
    LanguageType                nGuessLangWord;
    LanguageType                nGuessLangPara;

    std::map< sal_Int16, OUString > aLangTable_Text;
    std::map< sal_Int16, OUString > aLangTable_Paragraph;
//    std::map< sal_Int16, OUString > aLangTable_Document;

    String  aDicNameSingle;
    bool    bGrammarResults;    // show grammar results? Or show spellcheck results?

    Image   aInfo16;

    void fillLangPopupMenu( PopupMenu *pPopupMenu, sal_uInt16 nLangStart,
            ::com::sun::star::uno::Sequence< OUString > aSeq, SwWrtShell* pWrtSh,
            std::map< sal_Int16, OUString > &rLangTable );

    using PopupMenu::Execute;

public:
    SwSpellPopup( SwWrtShell *pWrtSh,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XSpellAlternatives >  &xAlt,
            const String & rParaText );

    SwSpellPopup( SwWrtShell *pWrtSh,
            const ::com::sun::star::linguistic2::ProofreadingResult &rResult,
            sal_Int32 nErrorInResult,
            const ::com::sun::star::uno::Sequence< OUString > &rSuggestions,
            const String & rParaText );

    sal_uInt16  Execute( const Rectangle& rPopupPos, Window* pWin );
    void Execute( sal_uInt16 nId );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
