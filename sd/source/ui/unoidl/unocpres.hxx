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
#ifndef INCLUDED_SD_SOURCE_UI_UNOIDL_UNOCPRES_HXX
#define INCLUDED_SD_SOURCE_UI_UNOIDL_UNOCPRES_HXX

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/interfacecontainer2.hxx>
#include <osl/mutex.hxx>

#include <cppuhelper/implbase.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <unomodel.hxx>
#include <drawdoc.hxx>


class SdCustomShow;

class SdXCustomPresentation :   public ::cppu::WeakImplHelper< css::container::XIndexContainer,
                                                                css::container::XNamed,
                                                                css::lang::XUnoTunnel,
                                                                css::lang::XComponent,
                                                                css::lang::XServiceInfo >
{
private:
    SdCustomShow*       mpSdCustomShow;
    SdXImpressDocument* mpModel;

    // for xComponent
    ::osl::Mutex aDisposeContainerMutex;
    ::comphelper::OInterfaceContainerHelper2 aDisposeListeners;
    bool bDisposing;

public:
    SdXCustomPresentation() throw();
    explicit SdXCustomPresentation( SdCustomShow* mpSdCustomShow ) throw();
    virtual ~SdXCustomPresentation() throw() override;

    // internal
    SdCustomShow* GetSdCustomShow() const throw() { return mpSdCustomShow; }
    void SetSdCustomShow( SdCustomShow* pShow ) throw() { mpSdCustomShow = pShow; }
    SdXImpressDocument* GetModel() const throw() { return mpModel; }

    // uno helper
    UNO3_GETIMPLEMENTATION_DECL(SdXCustomPresentation)

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XIndexContainer
    virtual void SAL_CALL insertByIndex( sal_Int32 Index, const css::uno::Any& Element ) override;
    virtual void SAL_CALL removeByIndex( sal_Int32 Index ) override;

    // XIndexReplace
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const css::uno::Any& Element ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XNamed
    virtual OUString SAL_CALL getName(  ) override;
    virtual void SAL_CALL setName( const OUString& aName ) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;
};

class SdXCustomPresentationAccess : public ::cppu::WeakImplHelper< css::container::XNameContainer,
                                                                    css::lang::XSingleServiceFactory,
                                                                    css::lang::XServiceInfo >
{
private:
    SdXImpressDocument& mrModel;

    // intern
    inline SdCustomShowList* GetCustomShowList() const throw();
    SdCustomShow * getSdCustomShow( const OUString& Name ) const throw();

public:
    explicit SdXCustomPresentationAccess(SdXImpressDocument& rMyModel) throw();
    virtual ~SdXCustomPresentationAccess() throw() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XSingleServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(  ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const OUString& Name ) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;
};

inline SdCustomShowList* SdXCustomPresentationAccess::GetCustomShowList() const throw()
{
    if(mrModel.GetDoc())
        return mrModel.GetDoc()->GetCustomShowList();
    else
        return nullptr;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
