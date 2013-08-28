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


#ifndef _THESSUBMENU_HXX_
#define _THESSUBMENU_HXX_

#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager2.hpp>

#include <vcl/menu.hxx>
#include <sfx2/mnuitem.hxx>

class SfxThesSubMenuHelper
{
    css::uno::Reference< css::linguistic2::XLinguServiceManager2 >  m_xLngMgr;
    css::uno::Reference< css::linguistic2::XThesaurus >             m_xThesarus;

private:

    // don't use copy constructor and assignment operator
    SfxThesSubMenuHelper( const SfxThesSubMenuHelper & );
    SfxThesSubMenuHelper & operator = ( const SfxThesSubMenuHelper & );

public:
    SfxThesSubMenuHelper();
    ~SfxThesSubMenuHelper();

    static OUString     GetText( const OUString &rLookUpString, sal_Int32 nDelimPos );

    // returns the Locale to be used for the selected text when the thesaurus is to be called
    static void GetLocale( css::lang::Locale /*out */ &rLocale, const OUString &rLookUpString, sal_Int32 nDelimPos );

    // returns true if the locale is upported by the theasaurus
    bool    IsSupportedLocale( const css::lang::Locale & rLocale ) const;

    // get the first nMax Synonym entries, even if different meanings need to be evaluated
    bool    GetMeanings( std::vector< OUString > & rSynonyms, const OUString & rWord, const css::lang::Locale & rLocale, sal_Int16 nMaxSynonms );

    OUString  GetThesImplName( const css::lang::Locale &rLocale ) const;
};



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
