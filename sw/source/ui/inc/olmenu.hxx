/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: olmenu.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 16:24:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _OLMENU_HXX
#define _OLMENU_HXX
#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARY_HPP_
#include <com/sun/star/linguistic2/XDictionary.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSPELLALTERNATIVES_HPP_
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#endif

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

