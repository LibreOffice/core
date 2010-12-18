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

#ifndef _HYP_HXX
#define _HYP_HXX

#include <tools/link.hxx>
#include <editeng/splwrap.hxx>
#include <com/sun/star/linguistic2/XHyphenator.hpp>

class SwView;

class SwHyphWrapper : public SvxSpellWrapper {
private:
    SwView* pView;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >  xHyph;
    Link            aOldLnk;
    LanguageType        nLangError;     // nicht vorhandene Sprache
    sal_uInt16          nPageCount;     // Seitenanzahl fuer Progressanzeige
    sal_uInt16          nPageStart;     // 1. gepruefte Seite
    sal_Bool            bInSelection : 1; // Trennen von selektiertem Text
    sal_Bool            bShowError : 1; // nicht vorhandene Sprache melden
    sal_Bool            bAutomatic : 1; // Trennstellen ohne Rueckfrage einfuegen
    sal_Bool            bInfoBox : 1;   // display info-box when ending
    DECL_LINK( SpellError, LanguageType * );

protected:
    virtual void SpellStart( SvxSpellArea eSpell );
    virtual sal_Bool SpellContinue();
    virtual void SpellEnd( );
    virtual sal_Bool SpellMore();
    virtual void InsertHyphen( const sal_uInt16 nPos ); // Hyphen einfuegen

public:
    SwHyphWrapper( SwView* pVw,
                   ::com::sun::star::uno::Reference<
                        ::com::sun::star::linguistic2::XHyphenator >  &rxHyph,
                   sal_Bool bStart, sal_Bool bOther, sal_Bool bSelect );
    ~SwHyphWrapper();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
