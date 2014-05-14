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

#ifndef INCLUDED_LINGUISTIC_SOURCE_DLISTIMP_HXX
#define INCLUDED_LINGUISTIC_SOURCE_DLISTIMP_HXX

#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <vector>

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
        virtual void    AtExit() SAL_OVERRIDE;
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

    bool    bDisposing;
    bool    bInCreation;

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

    void                SearchForDictionaries( DictionaryVec_t &rDicList,
                                            const OUString &rDicDir, bool bIsWritePath );
    sal_Int32           GetDicPos(const com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XDictionary > &xDic);

public:
    DicList();
    virtual ~DicList();

    // XDictionaryList
    virtual ::sal_Int16 SAL_CALL getCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary > > SAL_CALL getDictionaries(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary > SAL_CALL getDictionaryByName( const OUString& aDictionaryName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL addDictionary( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary >& xDictionary ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL removeDictionary( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary >& xDictionary ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL addDictionaryListEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionaryListEventListener >& xListener, sal_Bool bReceiveVerbose ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL removeDictionaryListEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionaryListEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int16 SAL_CALL beginCollectEvents(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int16 SAL_CALL endCollectEvents(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int16 SAL_CALL flushEvents(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary > SAL_CALL createDictionary( const OUString& aName, const ::com::sun::star::lang::Locale& aLocale, ::com::sun::star::linguistic2::DictionaryType eDicType, const OUString& aURL ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XSearchableDictionaryList
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionaryEntry > SAL_CALL queryDictionaryEntry( const OUString& aWord, const ::com::sun::star::lang::Locale& aLocale, sal_Bool bSearchPosDics, sal_Bool bSpellEntry ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


    static inline OUString getImplementationName_Static() throw();
    static com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static() throw();

    // non UNO-specific
    void    SaveDics();
};

inline OUString DicList::getImplementationName_Static() throw()
{
    return OUString( "com.sun.star.lingu2.DicList" );
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
