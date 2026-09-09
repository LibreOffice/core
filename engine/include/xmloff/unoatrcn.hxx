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

#ifndef INCLUDED_XMLOFF_UNOATRCN_HXX
#define INCLUDED_XMLOFF_UNOATRCN_HXX

#include <sal/config.h>

#include <memory>

#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <xmloff/xmlcnimp.hxx>

#include <cppuhelper/implbase.hxx>

extern css::uno::Reference< cpo::uno::XInterface >  SvUnoAttributeContainer_CreateInstance();

class XMLOFF_DLLPUBLIC SvUnoAttributeContainer final :
    public ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::container::XNameContainer >
{
private:
    std::unique_ptr<SvXMLAttrContainerData> mpContainer;

    SAL_DLLPRIVATE sal_uInt16 getIndexByName(std::u16string_view aName )
        const;

public:
    SvUnoAttributeContainer( std::unique_ptr<SvXMLAttrContainerData> pContainer = nullptr );
    SvXMLAttrContainerData* GetContainerImpl() const { return mpContainer.get(); }

    // css::container::XElementAccess
    virtual cpo::uno::Type  getElementType() override;
    virtual bool hasElements() override;

    // css::container::XNameAccess
    virtual cpo::uno::Any getByName(const OUString& aName) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName(const OUString& aName) override;

    // css::container::XNameReplace
    virtual void replaceByName(const OUString& aName, const cpo::uno::Any& aElement) override;

    // css::container::XNameContainer
    virtual void insertByName(const OUString& aName, const cpo::uno::Any& aElement) override;
    virtual void removeByName(const OUString& Name) override;

    // css::lang::XServiceInfo
    virtual OUString getImplementationName() override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
    virtual bool supportsService(const OUString& ServiceName) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
