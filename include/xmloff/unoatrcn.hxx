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

#ifndef INCLUDED_XMLOFF_UNOATRCN_HXX
#define INCLUDED_XMLOFF_UNOATRCN_HXX

#include <sal/config.h>

#include <memory>

#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <xmloff/xmlcnimp.hxx>

#include <cppuhelper/implbase.hxx>

extern css::uno::Reference< css::uno::XInterface >  SvUnoAttributeContainer_CreateInstance();

class XMLOFF_DLLPUBLIC SvUnoAttributeContainer:
    public ::cppu::WeakAggImplHelper <
        css::lang::XServiceInfo,
        css::lang::XUnoTunnel,
        css::container::XNameContainer >
{
private:
    std::unique_ptr<SvXMLAttrContainerData> mpContainer;

    SAL_DLLPRIVATE sal_uInt16 getIndexByName(const OUString& aName )
        const;

public:
    SvUnoAttributeContainer( std::unique_ptr<SvXMLAttrContainerData> pContainer = nullptr );
    SvXMLAttrContainerData* GetContainerImpl() const { return mpContainer.get(); }

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    // css::container::XElementAccess
    virtual css::uno::Type  SAL_CALL getElementType() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasElements() throw( css::uno::RuntimeException, std::exception ) override;

    // css::container::XNameAccess
    virtual css::uno::Any SAL_CALL getByName(const OUString& aName) throw( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& aName) throw( css::uno::RuntimeException, std::exception ) override;

    // css::container::XNameReplace
    virtual void SAL_CALL replaceByName(const OUString& aName, const css::uno::Any& aElement) throw( css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

    // css::container::XNameContainer
    virtual void SAL_CALL insertByName(const OUString& aName, const css::uno::Any& aElement) throw( css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeByName(const OUString& Name) throw( css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

    // css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;

    friend  css::uno::Reference< css::uno::XInterface > SvUnoAttributeContainer_CreateInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr ) throw( css::uno::Exception );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
