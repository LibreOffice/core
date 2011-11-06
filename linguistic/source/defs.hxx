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



#ifndef _LINGUISTIC_DEFS_HXX_
#define _LINGUISTIC_DEFS_HXX_

#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>

#include <boost/shared_ptr.hpp>

class SvStream;


///////////////////////////////////////////////////////////////////////////

#define A2OU(x) ::rtl::OUString::createFromAscii( x )

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

