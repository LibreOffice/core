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

    ::com::sun::star::uno::Sequence< rtl::OUString >  aSuggestions;

    LanguageType                nCheckedLanguage;
    LanguageType                nGuessLangWord;
    LanguageType                nGuessLangPara;

    std::map< sal_Int16, ::rtl::OUString > aLangTable_Text;
    std::map< sal_Int16, ::rtl::OUString > aLangTable_Paragraph;
//    std::map< sal_Int16, ::rtl::OUString > aLangTable_Document;

    bool    bGrammarResults;    // show grammar results? Or show spellcheck results?

    Image   aInfo16;

    void fillLangPopupMenu( PopupMenu *pPopupMenu, sal_uInt16 nLangStart,
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aSeq, SwWrtShell* pWrtSh,
            std::map< sal_Int16, ::rtl::OUString > &rLangTable );

    using PopupMenu::Execute;

public:
    SwSpellPopup( SwWrtShell *pWrtSh,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XSpellAlternatives >  &xAlt,
            const String & rParaText );

    SwSpellPopup( SwWrtShell *pWrtSh,
            const ::com::sun::star::linguistic2::ProofreadingResult &rResult,
            sal_Int32 nErrorInResult,
            const ::com::sun::star::uno::Sequence< rtl::OUString > &rSuggestions,
            const String & rParaText );

    sal_uInt16  Execute( const Rectangle& rPopupPos, Window* pWin );
    void Execute( sal_uInt16 nId );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
