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

#include <osl/diagnose.h>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>

#include <toolkit/helper/listenermultiplexer.hxx>

#include <cppuhelper/implbase2.hxx>
#include <unordered_map>

typedef ::cppu::WeakImplHelper2< css::container::XNameContainer,
                                 css::container::XContainer > NameContainerHelper;


namespace toolkit
{

// Hashtable to optimize
typedef std::unordered_map
<
    OUString,
    sal_Int32,
    OUStringHash
>
NameContainerNameMap;


class NameContainer_Impl : public NameContainerHelper
{
    NameContainerNameMap mHashMap;
    css::uno::Sequence< OUString > mNames;
    css::uno::Sequence< css::uno::Any > mValues;
    sal_Int32 mnElementCount;
    css::uno::Type mType;

    ContainerListenerMultiplexer maContainerListeners;

public:
    NameContainer_Impl( css::uno::Type const & aType )
        : mnElementCount( 0 ),
          mType( aType ),
          maContainerListeners( *this )
    {
    }

    // Methods XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(css::uno::RuntimeException, std::exception) override;

    // Methods XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
        throw(css::container::NoSuchElementException,
              css::lang::WrappedTargetException,
              css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(css::uno::RuntimeException, std::exception) override;

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement )
        throw(css::lang::IllegalArgumentException,
              css::container::NoSuchElementException,
              css::lang::WrappedTargetException,
              css::uno::RuntimeException, std::exception) override;

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement )
        throw(css::lang::IllegalArgumentException,
              css::container::ElementExistException,
              css::lang::WrappedTargetException,
              css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByName( const OUString& Name )
        throw(css::container::NoSuchElementException,
              css::lang::WrappedTargetException,
              css::uno::RuntimeException, std::exception) override;

    // Methods XContainer
    void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener )
        throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener )
        throw(css::uno::RuntimeException, std::exception) override;
};

class ScriptEventContainer : public NameContainer_Impl
{
public:
    ScriptEventContainer();
};


}   // namespace toolkit_namecontainer

#endif // INCLUDED_TOOLKIT_CONTROLS_EVENTCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
