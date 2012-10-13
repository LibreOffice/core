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

#ifndef _LINGUISTIC_DLISTIMP_HXX_
#define _LINGUISTIC_DLISTIMP_HXX_

#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <cppuhelper/implbase3.hxx> // helper for implementations
#include <cppuhelper/interfacecontainer.h>

#include <vector>
#include <memory>

#include "linguistic/misc.hxx"
#include "lngopt.hxx"

class DicEvtListenerHelper;


class DicList :
    public cppu::WeakImplHelper3
    <
        ::com::sun::star::linguistic2::XSearchableDictionaryList,
        ::com::sun::star::lang::XComponent,
        ::com::sun::star::lang::XServiceInfo
    >
{
    class MyAppExitListener : public linguistic::AppExitListener
    {
        DicList & rMyDicList;

    public:
        MyAppExitListener( DicList &rDicList ) : rMyDicList( rDicList ) {}
        virtual void    AtExit();
    };

    LinguOptions    aOpt;

    ::cppu::OInterfaceContainerHelper       aEvtListeners;

    typedef std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary > >   DictionaryVec_t;
    DictionaryVec_t                          aDicList;

    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::
                XDictionaryEventListener >  xDicEvtLstnrHelper;
    DicEvtListenerHelper                    *pDicEvtLstnrHelper;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::
                XTerminateListener >        xExitListener;
    MyAppExitListener                       *pExitListener;

    sal_Bool    bDisposing;
    sal_Bool    bInCreation;

    // disallow copy-constructor and assignment-operator for now
    DicList( const DicList & );
    DicList & operator = (const DicList &);

    void                _CreateDicList();
    DictionaryVec_t &   GetOrCreateDicList()
                        {
                            if ( !bInCreation && aDicList.empty() )
                                _CreateDicList();
                            return aDicList;
                        }

    void            LaunchEvent(sal_Int16 nEvent, com::sun::star::uno::Sequence<
                            ::com::sun::star::linguistic2::XDictionary > xDic);
    void            SearchForDictionaries( DictionaryVec_t &rDicList,
                                            const String &rDicDir, sal_Bool bIsWritePath );
    sal_Int32           GetDicPos(const com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XDictionary > &xDic);

public:
    DicList();
    virtual ~DicList();

    // XDictionaryList
    virtual ::sal_Int16 SAL_CALL getCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary > > SAL_CALL getDictionaries(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary > SAL_CALL getDictionaryByName( const ::rtl::OUString& aDictionaryName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL addDictionary( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary >& xDictionary ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL removeDictionary( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary >& xDictionary ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL addDictionaryListEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionaryListEventListener >& xListener, ::sal_Bool bReceiveVerbose ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL removeDictionaryListEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionaryListEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL beginCollectEvents(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL endCollectEvents(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL flushEvents(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary > SAL_CALL createDictionary( const ::rtl::OUString& aName, const ::com::sun::star::lang::Locale& aLocale, ::com::sun::star::linguistic2::DictionaryType eDicType, const ::rtl::OUString& aURL ) throw (::com::sun::star::uno::RuntimeException);

    // XSearchableDictionaryList
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionaryEntry > SAL_CALL queryDictionaryEntry( const ::rtl::OUString& aWord, const ::com::sun::star::lang::Locale& aLocale, sal_Bool bSearchPosDics, sal_Bool bSpellEntry ) throw(::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);


    static inline ::rtl::OUString getImplementationName_Static() throw();
    static com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static() throw();

    // non UNO-specific
    void    SaveDics();
};

inline ::rtl::OUString DicList::getImplementationName_Static() throw()
{
    return A2OU( "com.sun.star.lingu2.DicList" );
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
