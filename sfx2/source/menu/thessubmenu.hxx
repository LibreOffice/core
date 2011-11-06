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


