/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: thessubmenu.hxx,v $
 * $Revision: 1.0 $
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


#ifndef _THESSUBMENU_HXX_
#define _THESSUBMENU_HXX_

#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager.hpp>

#include <vcl/menu.hxx>
#include <sfx2/mnuitem.hxx>

class SfxBindings;
class PopupMenu;
class Menu;

namespace css = ::com::sun::star;

////////////////////////////////////////////////////////////

class SfxThesSubMenuControl : public SfxMenuControl
{
    PopupMenu*          pMenu;
    Menu&               rParent;

private:
    virtual void    StateChanged( sal_uInt16, SfxItemState, const SfxPoolItem* pState );
    DECL_LINK( MenuSelect, Menu * );

public:
    SfxThesSubMenuControl(sal_uInt16, Menu&, SfxBindings&);
    ~SfxThesSubMenuControl();

    virtual PopupMenu*  GetPopup() const;

    SFX_DECL_MENU_CONTROL();
};

////////////////////////////////////////////////////////////

class SfxThesSubMenuHelper
{
    css::uno::Reference< css::linguistic2::XLinguServiceManager >   m_xLngMgr;
    css::uno::Reference< css::linguistic2::XThesaurus >             m_xThesarus;

private:

    // don't use copy c-tor and assignment operator
    SfxThesSubMenuHelper( const SfxThesSubMenuHelper & );
    SfxThesSubMenuHelper & operator = ( const SfxThesSubMenuHelper & );

public:
    SfxThesSubMenuHelper();
    ~SfxThesSubMenuHelper();

    static ::rtl::OUString     GetText( const String &rLookUpString, xub_StrLen nDelimPos );

    // returns the Locale to be used for the selected text when the thesaurus is to be called
    static void GetLocale( css::lang::Locale /*out */ &rLocale, const String &rLookUpString, xub_StrLen nDelimPos );

    // returns true if the locale is upported by the theasaurus
    bool    IsSupportedLocale( const css::lang::Locale & rLocale ) const;

    // get the first nMax Synonym entries, even if different meanings need to be evaluated
    bool    GetMeanings( std::vector< ::rtl::OUString > & rSynonyms, const ::rtl::OUString & rWord, const css::lang::Locale & rLocale, sal_Int16 nMaxSynonms );

    String  GetThesImplName( const css::lang::Locale &rLocale ) const;
};

////////////////////////////////////////////////////////////


#endif


