/*************************************************************************
 *
 *  $RCSfile: spelldsp.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-17 12:37:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _LINGUISTIC_SPELLDSP_HXX_
#define _LINGUISTIC_SPELLDSP_HXX_

#include "lngopt.hxx"
#include "misc.hxx"
#include "iprcache.hxx"

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <cppuhelper/implbase7.hxx> // helper for implementations

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEDISPLAYNAME_HPP_
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#endif
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>

#ifndef _TOOLS_TABLE_HXX
#include <tools/table.hxx>
#endif


class LngSvcMgr;

///////////////////////////////////////////////////////////////////////////

class SeqLangSvcEntry_Spell
{
    friend class SpellCheckerDispatcher;

    ::com::sun::star::uno::Sequence< ::rtl::OUString >          aSvcImplNames;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XSpellChecker > >    aSvcRefs;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XSpellChecker1 > >   aSvc1Refs;

//  INT16           nLang;     //used as key in the table
    SvcFlags        aFlags;

public:
    SeqLangSvcEntry_Spell() {}
    SeqLangSvcEntry_Spell( const ::com::sun::star::uno::Sequence<
            ::rtl::OUString > &rSvcImplNames );
    ~SeqLangSvcEntry_Spell();

    BOOL    IsAlreadyWarned() const     { return aFlags.bAlreadyWarned != 0; }
    void    SetAlreadyWarned(BOOL bVal) { aFlags.bAlreadyWarned = 0 != bVal; }
    BOOL    IsDoWarnAgain() const       { return aFlags.bDoWarnAgain != 0; }
    void    SetDoWarnAgain(BOOL bVal)   { aFlags.bDoWarnAgain = 0 != bVal; }
};

DECLARE_TABLE( SpellSvcList, SeqLangSvcEntry_Spell * );


class SpellCheckerDispatcher :
    public cppu::WeakImplHelper2
    <
        ::com::sun::star::linguistic2::XSpellChecker,
        ::com::sun::star::linguistic2::XSpellChecker1
    >,
    public LinguDispatcher
{
    SpellSvcList        aSvcList;
    LinguOptions        aOpt;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >                     xPropSet;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSearchableDictionaryList >  xDicList;

    LngSvcMgr                   &rMgr;
    linguistic::IPRSpellCache   *pExtCache; // SpellCache for external SpellCheckers
                                    // (usually those not called via XSpellChecker1)
                                    // One for all of them.

    // disallow copy-constructor and assignment-operator for now
    SpellCheckerDispatcher(const SpellCheckerDispatcher &);
    SpellCheckerDispatcher & operator = (const SpellCheckerDispatcher &);

    inline linguistic::IPRSpellCache &  GetExtCache() const;

    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
            GetPropSet();
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSearchableDictionaryList >
            GetDicList();

    void    ClearSvcList();

    BOOL    isValid_Impl(const ::rtl::OUString& aWord, INT16 nLanguage,
                    const ::com::sun::star::beans::PropertyValues& aProperties,
                    BOOL bCheckDics)
                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellAlternatives >
            spell_Impl(const ::rtl::OUString& aWord, INT16 nLanguage,
                    const ::com::sun::star::beans::PropertyValues& aProperties,
                    BOOL bCheckDics)
                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

    BOOL    isValidInAny(const ::rtl::OUString& aWord,
                         const ::com::sun::star::uno::Sequence< INT16 >& aLanguages,
                         const ::com::sun::star::beans::PropertyValues& aProperties)
                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

    com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellAlternatives >
            spellInAny(const ::rtl::OUString& aWord,
                       const ::com::sun::star::uno::Sequence< INT16 >& aLanguages,
                       const ::com::sun::star::beans::PropertyValues& aProperties,
                       INT16 nPreferredResultLang)
                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

public:
    SpellCheckerDispatcher( LngSvcMgr &rLngSvcMgr );
    virtual ~SpellCheckerDispatcher();

    // XSupportedLocales (for XSpellChecker)
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::lang::Locale > SAL_CALL
        getLocales()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        hasLocale( const ::com::sun::star::lang::Locale& aLocale )
            throw(::com::sun::star::uno::RuntimeException);

    // XSpellChecker
    virtual sal_Bool SAL_CALL
        isValid( const ::rtl::OUString& aWord,
                const ::com::sun::star::lang::Locale& aLocale,
                const ::com::sun::star::beans::PropertyValues& aProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XSpellAlternatives > SAL_CALL
        spell( const ::rtl::OUString& aWord,
                const ::com::sun::star::lang::Locale& aLocale,
                const ::com::sun::star::beans::PropertyValues& aProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);

    // XSupportedLanguages (for XSpellChecker1)
    virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL
        getLanguages()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        hasLanguage( sal_Int16 nLanguage )
            throw(::com::sun::star::uno::RuntimeException);

    // XSpellChecker1 (same as XSpellChecker but sal_Int16 for language)
    virtual sal_Bool SAL_CALL
        isValid( const ::rtl::OUString& aWord, sal_Int16 nLanguage,
                const ::com::sun::star::beans::PropertyValues& aProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XSpellAlternatives > SAL_CALL
        spell( const ::rtl::OUString& aWord, sal_Int16 nLanguage,
                const ::com::sun::star::beans::PropertyValues& aProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);

    // LinguDispatcher
    virtual void
        SetServiceList( const ::com::sun::star::lang::Locale &rLocale,
                const ::com::sun::star::uno::Sequence<
                    rtl::OUString > &rSvcImplNames );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString >
        GetServiceList( const ::com::sun::star::lang::Locale &rLocale );
};


inline linguistic::IPRSpellCache & SpellCheckerDispatcher::GetExtCache() const
{
    return pExtCache ? *pExtCache : * new linguistic::IPRSpellCache( 997 );
}


inline ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet >
        SpellCheckerDispatcher::GetPropSet()
{
    return xPropSet.is() ?
        xPropSet : xPropSet = linguistic::GetLinguProperties();
}


inline ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XSearchableDictionaryList >
        SpellCheckerDispatcher::GetDicList()
{
    return xDicList.is() ?
        xDicList : xDicList = linguistic::GetSearchableDictionaryList();
}


///////////////////////////////////////////////////////////////////////////

#endif

