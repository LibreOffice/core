/*************************************************************************
 *
 *  $RCSfile: thesdsp.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-17 12:37:43 $
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

#ifndef _LINGUISTIC_THESDSP_HXX_
#define _LINGUISTIC_THESDSP_HXX_

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEDISPLAYNAME_HPP_
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#endif

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <cppuhelper/implbase5.hxx> // helper for implementations

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#include <vos/mutex.hxx>

#ifndef _TOOLS_TABLE_HXX
#include <tools/table.hxx>
#endif

#include "lngopt.hxx"


class LinguOptions;

namespace com { namespace sun { namespace star { namespace beans {
    class XPropertySet;
}}}};

///////////////////////////////////////////////////////////////////////////

class SeqLangSvcEntry_Thes
{
    friend class ThesaurusDispatcher;

    ::com::sun::star::uno::Sequence< ::rtl::OUString >      aSvcImplNames;
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XThesaurus > >   aSvcRefs;
//  INT16           nLang;     //used as key in the table
    SvcFlags        aFlags;

public:
    SeqLangSvcEntry_Thes() {}
    SeqLangSvcEntry_Thes( const ::com::sun::star::uno::Sequence<
            ::rtl::OUString > &rSvcImplNames );
    ~SeqLangSvcEntry_Thes();

    BOOL    IsAlreadyWarned() const     { return aFlags.bAlreadyWarned != 0; }
    void    SetAlreadyWarned(BOOL bVal) { aFlags.bAlreadyWarned = 0 != bVal; }
    BOOL    IsDoWarnAgain() const       { return aFlags.bDoWarnAgain != 0; }
    void    SetDoWarnAgain(BOOL bVal)   { aFlags.bDoWarnAgain = 0 != bVal; }
};



DECLARE_TABLE( ThesSvcList, SeqLangSvcEntry_Thes * );

class ThesaurusDispatcher :
    public cppu::WeakImplHelper1
    <
        ::com::sun::star::linguistic2::XThesaurus
    >,
    public LinguDispatcher
{
    ThesSvcList     aSvcList;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >     xPropSet;

    // disallow copy-constructor and assignment-operator for now
    ThesaurusDispatcher(const ThesaurusDispatcher &);
    ThesaurusDispatcher & operator = (const ThesaurusDispatcher &);

    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
            GetPropSet();

    void    ClearSvcList();

public:
    ThesaurusDispatcher();
    virtual ~ThesaurusDispatcher();

    // XSupportedLocales
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::lang::Locale > SAL_CALL
        getLocales()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        hasLocale( const ::com::sun::star::lang::Locale& aLocale )
            throw(::com::sun::star::uno::RuntimeException);

    // XThesaurus
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XMeaning > > SAL_CALL
        queryMeanings( const ::rtl::OUString& aTerm,
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
        GetServiceList( const ::com::sun::star::lang::Locale &rLocale );
};


inline ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet >
        ThesaurusDispatcher::GetPropSet()
{
    return xPropSet.is() ?
        xPropSet : xPropSet = linguistic::GetLinguProperties();
}


///////////////////////////////////////////////////////////////////////////

#endif

