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

#ifndef _LINGUISTIC_DEFS_HXX_
#define _LINGUISTIC_DEFS_HXX_

#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>

#include <boost/shared_ptr.hpp>

class SvStream;


///////////////////////////////////////////////////////////////////////////

typedef boost::shared_ptr< SvStream > SvStreamPtr;

namespace css = ::com::sun::star;

///////////////////////////////////////////////////////////////////////////

struct LangSvcEntries
{
    css::uno::Sequence< ::rtl::OUString >   aSvcImplNames;

    sal_Int16   nLastTriedSvcIndex;
    bool        bAlreadyWarned;
    bool        bDoWarnAgain;

    LangSvcEntries() : nLastTriedSvcIndex(-1), bAlreadyWarned(false), bDoWarnAgain(false) {}

    inline LangSvcEntries( const css::uno::Sequence< ::rtl::OUString > &rSvcImplNames ) :
        aSvcImplNames(rSvcImplNames),
        nLastTriedSvcIndex(-1), bAlreadyWarned(false), bDoWarnAgain(false)
    {
    }

    inline LangSvcEntries( const ::rtl::OUString &rSvcImplName ) :
        nLastTriedSvcIndex(-1), bAlreadyWarned(false), bDoWarnAgain(false)
    {
        aSvcImplNames.realloc(1);
        aSvcImplNames[0] = rSvcImplName;
    }

    bool    IsAlreadyWarned() const         { return bAlreadyWarned != 0; }
    void    SetAlreadyWarned( bool bVal )   { bAlreadyWarned = 0 != bVal; }
    bool    IsDoWarnAgain() const           { return bDoWarnAgain != 0; }
    void    SetDoWarnAgain( bool bVal )     { bDoWarnAgain = 0 != bVal; }

    inline void Clear()
    {
        aSvcImplNames.realloc(0);
        nLastTriedSvcIndex  = -1;
        bAlreadyWarned      = false;
        bDoWarnAgain        = false;
    }
};

struct LangSvcEntries_Spell : public LangSvcEntries
{
    css::uno::Sequence< css::uno::Reference< css::linguistic2::XSpellChecker > >  aSvcRefs;

    LangSvcEntries_Spell() : LangSvcEntries() {}
    LangSvcEntries_Spell( const css::uno::Sequence< ::rtl::OUString > &rSvcImplNames ) : LangSvcEntries( rSvcImplNames ) {}
};

struct LangSvcEntries_Grammar : public LangSvcEntries
{
    css::uno::Sequence< css::uno::Reference< css::linguistic2::XProofreader > >  aSvcRefs;

    LangSvcEntries_Grammar() : LangSvcEntries() {}
    LangSvcEntries_Grammar( const ::rtl::OUString &rSvcImplName ) : LangSvcEntries( rSvcImplName ) {}
};

struct LangSvcEntries_Hyph : public LangSvcEntries
{
    css::uno::Sequence< css::uno::Reference< css::linguistic2::XHyphenator > >  aSvcRefs;

    LangSvcEntries_Hyph() : LangSvcEntries() {}
    LangSvcEntries_Hyph( const ::rtl::OUString &rSvcImplName ) : LangSvcEntries( rSvcImplName ) {}
};

struct LangSvcEntries_Thes : public LangSvcEntries
{
    css::uno::Sequence< css::uno::Reference< css::linguistic2::XThesaurus > >  aSvcRefs;

    LangSvcEntries_Thes() : LangSvcEntries() {}
    LangSvcEntries_Thes( const css::uno::Sequence< ::rtl::OUString > &rSvcImplNames ) : LangSvcEntries( rSvcImplNames ) {}
};

///////////////////////////////////////////////////////////////////////////

// virtual base class for the different dispatchers
class LinguDispatcher
{
public:
    enum DspType    { DSP_SPELL, DSP_HYPH, DSP_THES, DSP_GRAMMAR };

    virtual void SetServiceList( const css::lang::Locale &rLocale, const css::uno::Sequence< rtl::OUString > &rSvcImplNames ) = 0;
    virtual css::uno::Sequence< rtl::OUString > GetServiceList( const css::lang::Locale &rLocale ) const = 0;
    virtual DspType GetDspType() const = 0;
};

///////////////////////////////////////////////////////////////////////////

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
