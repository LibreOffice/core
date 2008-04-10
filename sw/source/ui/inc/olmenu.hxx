/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: olmenu.hxx,v $
 * $Revision: 1.12 $
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

#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#include <map>

class SwWrtShell;

class SwSpellPopup : public PopupMenu
{
    SwWrtShell* pSh;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >  >     aDics;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellAlternatives > xSpellAlt;

    LanguageType                nGuessLangWord;
    LanguageType                nGuessLangPara;

    USHORT nNumLanguageTextEntries;
    USHORT nNumLanguageParaEntries;
    USHORT nNumLanguageDocEntries;
    std::map< sal_Int16, ::rtl::OUString > aLangTable_Text;
    std::map< sal_Int16, ::rtl::OUString > aLangTable_Paragraph;
    std::map< sal_Int16, ::rtl::OUString > aLangTable_Document;
    USHORT fillLangPopupMenu( PopupMenu *pPopupMenu , USHORT Lang_Start, ::com::sun::star::uno::Sequence< ::rtl::OUString > aSeq,SwWrtShell* pWrtSh, USHORT nLangTable);

    using PopupMenu::Execute;

public:
    SwSpellPopup( SwWrtShell*,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XSpellAlternatives >  &xAlt,
            const String & rParaText );

    sal_uInt16  Execute( const Rectangle& rPopupPos, Window* pWin );
    void Execute( USHORT nId );

};

#endif

