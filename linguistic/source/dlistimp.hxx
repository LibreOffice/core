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

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <vector>

#include "linguistic/misc.hxx"
#include "lngopt.hxx"

class DicEvtListenerHelper;


class DicList :
    public cppu::WeakImplHelper
    <
        css::linguistic2::XSearchableDictionaryList,
        css::lang::XComponent,
        css::lang::XServiceInfo
    >
{
    class MyAppExitListener : public linguistic::AppExitListener
    {
        DicList & rMyDicList;

    public:
        explicit MyAppExitListener( DicList &rDicList ) : rMyDicList( rDicList ) {}
        virtual void    AtExit() override;
    };

    LinguOptions    aOpt;

    ::cppu::OInterfaceContainerHelper       aEvtListeners;

    typedef std::vector< css::uno::Reference< css::linguistic2::XDictionary > >   DictionaryVec_t;
    DictionaryVec_t                          aDicList;

    css::uno::Reference< css::linguistic2::
                XDictionaryEventListener >  xDicEvtLstnrHelper;
    DicEvtListenerHelper                    *pDicEvtLstnrHelper;

    css::uno::Reference< css::frame::
                XTerminateListener >        xExitListener;
    MyAppExitListener                       *pExitListener;

    bool    bDisposing;
    bool    bInCreation;

    DicList( const DicList & ) = delete;
    DicList & operator = (const DicList &) = delete;

    void                _CreateDicList();
    DictionaryVec_t &   GetOrCreateDicList()
                        {
                            if ( !bInCreation && aDicList.empty() )
                                _CreateDicList();
                            return aDicList;
                        }

    void                SearchForDictionaries( DictionaryVec_t &rDicList,
                                            const OUString &rDicDir, bool bIsWritePath );
    sal_Int32           GetDicPos(const css::uno::Reference<
                            css::linguistic2::XDictionary > &xDic);

public:
    DicList();
    virtual ~DicList();

    // XDictionaryList
    virtual ::sal_Int16 SAL_CALL getCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Reference< css::linguistic2::XDictionary > > SAL_CALL getDictionaries(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::linguistic2::XDictionary > SAL_CALL getDictionaryByName( const OUString& aDictionaryName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL addDictionary( const css::uno::Reference< css::linguistic2::XDictionary >& xDictionary ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL removeDictionary( const css::uno::Reference< css::linguistic2::XDictionary >& xDictionary ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL addDictionaryListEventListener( const css::uno::Reference< css::linguistic2::XDictionaryListEventListener >& xListener, sal_Bool bReceiveVerbose ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL removeDictionaryListEventListener( const css::uno::Reference< css::linguistic2::XDictionaryListEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int16 SAL_CALL beginCollectEvents(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int16 SAL_CALL endCollectEvents(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int16 SAL_CALL flushEvents(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::linguistic2::XDictionary > SAL_CALL createDictionary( const OUString& aName, const css::lang::Locale& aLocale, css::linguistic2::DictionaryType eDicType, const OUString& aURL ) throw (css::uno::RuntimeException, std::exception) override;

    // XSearchableDictionaryList
    virtual css::uno::Reference< css::linguistic2::XDictionaryEntry > SAL_CALL queryDictionaryEntry( const OUString& aWord, const css::lang::Locale& aLocale, sal_Bool bSearchPosDics, sal_Bool bSpellEntry ) throw(css::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;


    static inline OUString getImplementationName_Static() throw();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw();

    // non UNO-specific
    void    SaveDics();
};

inline OUString DicList::getImplementationName_Static() throw()
{
    return OUString( "com.sun.star.lingu2.DicList" );
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
