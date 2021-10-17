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

#ifndef INCLUDED_LINGUISTIC_SOURCE_DEFS_HXX
#define INCLUDED_LINGUISTIC_SOURCE_DEFS_HXX

#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>

#include <memory>

class SvStream;

typedef std::shared_ptr< SvStream > SvStreamPtr;

struct LangSvcEntries
{
    css::uno::Sequence< OUString >   aSvcImplNames;

    sal_Int16   nLastTriedSvcIndex;

    explicit LangSvcEntries( const css::uno::Sequence< OUString > &rSvcImplNames ) :
        aSvcImplNames(rSvcImplNames),
        nLastTriedSvcIndex(-1)
    {
    }

    explicit LangSvcEntries( const OUString &rSvcImplName ) :
        nLastTriedSvcIndex(-1)
    {
        aSvcImplNames = { rSvcImplName };
    }

    void Clear()
    {
        aSvcImplNames.realloc(0);
        nLastTriedSvcIndex  = -1;
    }
};

struct LangSvcEntries_Spell : public LangSvcEntries
{
    css::uno::Sequence< css::uno::Reference< css::linguistic2::XSpellChecker > >  aSvcRefs;

    explicit LangSvcEntries_Spell( const css::uno::Sequence< OUString > &rSvcImplNames ) : LangSvcEntries( rSvcImplNames ) {}
};

struct LangSvcEntries_Hyph : public LangSvcEntries
{
    css::uno::Sequence< css::uno::Reference< css::linguistic2::XHyphenator > >  aSvcRefs;

    explicit LangSvcEntries_Hyph( const OUString &rSvcImplName ) : LangSvcEntries( rSvcImplName ) {}
};

struct LangSvcEntries_Thes : public LangSvcEntries
{
    css::uno::Sequence< css::uno::Reference< css::linguistic2::XThesaurus > >  aSvcRefs;

    explicit LangSvcEntries_Thes( const css::uno::Sequence< OUString > &rSvcImplNames ) : LangSvcEntries( rSvcImplNames ) {}
};


// virtual base class for the different dispatchers
class LinguDispatcher
{
public:
    virtual void SetServiceList( const css::lang::Locale &rLocale, const css::uno::Sequence< OUString > &rSvcImplNames ) = 0;
    virtual css::uno::Sequence< OUString > GetServiceList( const css::lang::Locale &rLocale ) const = 0;

protected:
    ~LinguDispatcher() {}
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
