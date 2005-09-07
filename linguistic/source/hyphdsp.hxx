/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hyphdsp.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:52:14 $
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

#ifndef _LINGUISTIC_HYPHDSP_HXX_
#define _LINGUISTIC_HYPHDSP_HXX_


#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEDISPLAYNAME_HPP_
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#endif
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XPossibleHyphens.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>

#ifndef _TOOLS_TABLE_HXX
#include <tools/table.hxx>
#endif

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations


#include "lngopt.hxx"
#include "misc.hxx"
#include "defs.hxx"

class LngSvcMgr;

///////////////////////////////////////////////////////////////////////////

class LangSvcEntry_Hyph
{
    friend class HyphenatorDispatcher;

    ::rtl::OUString                                     aSvcImplName;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >    aSvcRef;

//  INT16           nLang;     //used as key in the table
    SvcFlags        aFlags;

public:
    LangSvcEntry_Hyph() {}
    LangSvcEntry_Hyph( const ::rtl::OUString &rSvcImplName );
    ~LangSvcEntry_Hyph();

    BOOL    IsAlreadyWarned() const     { return aFlags.bAlreadyWarned != 0; }
    void    SetAlreadyWarned(BOOL bVal) { aFlags.bAlreadyWarned = 0 != bVal; }
    BOOL    IsDoWarnAgain() const       { return aFlags.bDoWarnAgain != 0; }
    void    SetDoWarnAgain(BOOL bVal)   { aFlags.bDoWarnAgain = 0 != bVal; }
};

DECLARE_TABLE( HyphSvcList, LangSvcEntry_Hyph * );


class HyphenatorDispatcher :
    public cppu::WeakImplHelper1
    <
        ::com::sun::star::linguistic2::XHyphenator
    >,
    public LinguDispatcher
{
    HyphSvcList     aSvcList;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >                     xPropSet;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSearchableDictionaryList >  xDicList;

    LngSvcMgr      &rMgr;

    // disallow copy-constructor and assignment-operator for now
    HyphenatorDispatcher(const HyphenatorDispatcher &);
    HyphenatorDispatcher & operator = (const HyphenatorDispatcher &);

    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
            GetPropSet();
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSearchableDictionaryList >
            GetDicList();

    void    ClearSvcList();

    com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenatedWord>
            buildHyphWord( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XDictionaryEntry> &xEntry,
                INT16 nLang, INT16 nMaxLeading );

    com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XPossibleHyphens >
            buildPossHyphens( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XDictionaryEntry > &xEntry,
                    INT16 nLanguage );

public:
    HyphenatorDispatcher( LngSvcMgr &rLngSvcMgr );
    virtual ~HyphenatorDispatcher();

    // XSupportedLocales
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::lang::Locale > SAL_CALL
        getLocales()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        hasLocale( const ::com::sun::star::lang::Locale& aLocale )
            throw(::com::sun::star::uno::RuntimeException);

    // XHyphenator
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XHyphenatedWord > SAL_CALL
        hyphenate( const ::rtl::OUString& aWord,
                const ::com::sun::star::lang::Locale& aLocale,
                sal_Int16 nMaxLeading,
                const ::com::sun::star::beans::PropertyValues& aProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XHyphenatedWord > SAL_CALL
        queryAlternativeSpelling( const ::rtl::OUString& aWord,
                const ::com::sun::star::lang::Locale& aLocale,
                sal_Int16 nIndex,
                const ::com::sun::star::beans::PropertyValues& aProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XPossibleHyphens > SAL_CALL
        createPossibleHyphens(
                const ::rtl::OUString& aWord,
                const ::com::sun::star::lang::Locale& aLocale,
                const ::com::sun::star::beans::PropertyValues& aProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);

    // LinguDispatcher
    virtual void
        SetServiceList( const ::com::sun::star::lang::Locale &rLocale,
                const ::com::sun::star::uno::Sequence<
                    rtl::OUString > &rSvcImplNames );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString >
        GetServiceList( const ::com::sun::star::lang::Locale &rLocale ) const;
    virtual DspType
        GetDspType() const;
};


inline ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet >
        HyphenatorDispatcher::GetPropSet()
{
    return xPropSet.is() ?
                xPropSet : xPropSet = ::linguistic::GetLinguProperties();
}


inline ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XSearchableDictionaryList >
        HyphenatorDispatcher::GetDicList()
{
    return xDicList.is() ?
                xDicList : xDicList = ::linguistic::GetSearchableDictionaryList();
}


///////////////////////////////////////////////////////////////////////////


#endif

