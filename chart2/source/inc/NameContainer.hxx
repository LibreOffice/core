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

#pragma once

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <cppuhelper/implbase.hxx>
#include "charttoolsdllapi.hxx"

#include <map>

namespace chart
{

OOO_DLLPUBLIC_CHARTTOOLS css::uno::Reference< css::container::XNameContainer > createNameContainer(
    const css::uno::Type& rType, const OUString& rServicename, const OUString& rImplementationName );

namespace impl
{
typedef ::cppu::WeakImplHelper<
        css::container::XNameContainer,
        css::lang::XServiceInfo,
        css::util::XCloneable >
    NameContainer_Base;
}

class NameContainer : public impl::NameContainer_Base
{
public:
    NameContainer() = delete;
    NameContainer( const css::uno::Type& rType, const OUString& rServicename, const OUString& rImplementationName );
    explicit NameContainer( const NameContainer & rOther );
    virtual ~NameContainer() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XNameContainer
    virtual void SAL_CALL insertByName( const  OUString& aName, const  css::uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const  OUString& Name ) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  OUString& aName, const  css::uno::Any& aElement ) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const  OUString& aName ) override;
    virtual css::uno::Sequence<  OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const  OUString& aName ) override;

    // XElementAccess
    virtual sal_Bool SAL_CALL hasElements(  ) override;
    virtual css::uno::Type SAL_CALL getElementType(  ) override;

    // XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

private: //member
    const css::uno::Type           m_aType;
    const OUString                 m_aServicename;
    const OUString                 m_aImplementationName;

    typedef std::map< OUString, css::uno::Any > tContentMap;

    tContentMap m_aMap;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
