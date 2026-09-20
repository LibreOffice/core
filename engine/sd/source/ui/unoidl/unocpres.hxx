/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#pragma once

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/interfacecontainer4.hxx>
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
                                                                css::lang::XComponent,
                                                                css::lang::XServiceInfo >
{
private:
    SdCustomShow*       mpSdCustomShow;
    SdXImpressDocument* mpModel;

    // for xComponent
    std::mutex aDisposeContainerMutex;
    ::comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> aDisposeListeners;
    bool bDisposing;

public:
    SdXCustomPresentation() noexcept;
    explicit SdXCustomPresentation( SdCustomShow* mpSdCustomShow ) noexcept;
    virtual ~SdXCustomPresentation() noexcept override;

    // internal
    SdCustomShow* GetSdCustomShow() const noexcept { return mpSdCustomShow; }
    void SetSdCustomShow( SdCustomShow* pShow ) noexcept { mpSdCustomShow = pShow; }
    SdXImpressDocument* GetModel() const noexcept { return mpModel; }

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XIndexContainer
    virtual void insertByIndex( sal_Int32 Index, const cpo::uno::Any& Element ) override;
    virtual void removeByIndex( sal_Int32 Index ) override;

    // XIndexReplace
    virtual void replaceByIndex( sal_Int32 Index, const cpo::uno::Any& Element ) override;

    // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    // XIndexAccess
    virtual sal_Int32 getCount() override ;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

    // XNamed
    virtual OUString getName(  ) override;
    virtual void setName( const OUString& aName ) override;

    // XComponent
    virtual void dispose(  ) override;
    virtual void addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& aListener ) override;
};

class SdXCustomPresentationAccess final : public ::cppu::WeakImplHelper< css::container::XNameContainer,
                                                                    css::lang::XSingleServiceFactory,
                                                                    css::lang::XServiceInfo >
{
private:
    SdXImpressDocument& mrModel;

    // internal
    inline SdCustomShowList* GetCustomShowList() const noexcept;
    SdCustomShow * getSdCustomShow( std::u16string_view Name ) const noexcept;

public:
    explicit SdXCustomPresentationAccess(SdXImpressDocument& rMyModel) noexcept;
    virtual ~SdXCustomPresentationAccess() noexcept override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XSingleServiceFactory
    virtual cpo::uno::Reference< cpo::uno::XInterface > createInstance(  ) override;
    virtual cpo::uno::Reference< cpo::uno::XInterface > createInstanceWithArguments( const cpo::uno::Sequence< cpo::uno::Any >& aArguments ) override;

    // XNameContainer
    virtual void insertByName( const OUString& aName, const cpo::uno::Any& aElement ) override;
    virtual void removeByName( const OUString& Name ) override;

    // XNameReplace
    virtual void replaceByName( const OUString& aName, const cpo::uno::Any& aElement ) override;

    // XNameAccess
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

    // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;
};

inline SdCustomShowList* SdXCustomPresentationAccess::GetCustomShowList() const noexcept
{
    if(mrModel.GetDoc())
        return mrModel.GetDoc()->GetCustomShowList();
    else
        return nullptr;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
