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
#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <cppuhelper/implbase3.hxx>

extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SvUnoAttributeContainer_CreateInstance();

class SvXMLAttrContainerData;

class XMLOFF_DLLPUBLIC SvUnoAttributeContainer:
    public ::cppu::WeakAggImplHelper3<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XUnoTunnel,
        ::com::sun::star::container::XNameContainer >
{
private:
    SvXMLAttrContainerData* mpContainer;

    SAL_DLLPRIVATE sal_uInt16 getIndexByName(const OUString& aName )
        const;

public:
    SvUnoAttributeContainer( SvXMLAttrContainerData* pContainer = NULL );
    virtual ~SvUnoAttributeContainer();

    SvXMLAttrContainerData* GetContainerImpl() const { return mpContainer; }

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type  SAL_CALL getElementType() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasElements() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // ::com::sun::star::container::XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const OUString& aName) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& aName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // ::com::sun::star::container::XNameReplace
    virtual void SAL_CALL replaceByName(const OUString& aName, const ::com::sun::star::uno::Any& aElement) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // ::com::sun::star::container::XNameContainer
    virtual void SAL_CALL insertByName(const OUString& aName, const ::com::sun::star::uno::Any& aElement) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeByName(const OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    friend  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SvUnoAttributeContainer_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr ) throw( ::com::sun::star::uno::Exception );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
