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

#ifndef INCLUDED_TOOLKIT_CONTROLS_EVENTCONTAINER_HXX
#define INCLUDED_TOOLKIT_CONTROLS_EVENTCONTAINER_HXX

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>

#include <toolkit/helper/listenermultiplexer.hxx>

#include <cppuhelper/implbase.hxx>
#include <unordered_map>

namespace toolkit
{

class ScriptEventContainer : public ::cppu::WeakImplHelper<
                                        css::container::XNameContainer,
                                        css::container::XContainer >
{
    std::unordered_map< OUString, css::uno::Any>
                   mHashMap;
    css::uno::Type const mType;

    ContainerListenerMultiplexer maContainerListeners;

public:
    ScriptEventContainer();

    // Methods XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    // Methods XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) override;

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const OUString& Name ) override;

    // Methods XContainer
    void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
    void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
};

}   // namespace toolkit_namecontainer

#endif // INCLUDED_TOOLKIT_CONTROLS_EVENTCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
