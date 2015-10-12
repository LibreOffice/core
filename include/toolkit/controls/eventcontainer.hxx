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

typedef ::cppu::WeakImplHelper2< ::com::sun::star::container::XNameContainer,
                                 ::com::sun::star::container::XContainer > NameContainerHelper;


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
    ::com::sun::star::uno::Sequence< OUString > mNames;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > mValues;
    sal_Int32 mnElementCount;
    ::com::sun::star::uno::Type mType;

    ContainerListenerMultiplexer maContainerListeners;

public:
    NameContainer_Impl( ::com::sun::star::uno::Type const & aType )
        : mnElementCount( 0 ),
          mType( aType ),
          maContainerListeners( *this )
    {
    }

    // Methods XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // Methods XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // Methods XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException, std::exception) override;

    // Methods XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::ElementExistException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByName( const OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException, std::exception) override;

    // Methods XContainer
    void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener )
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener )
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

class ScriptEventContainer : public NameContainer_Impl
{
public:
    ScriptEventContainer();
};


}   // namespace toolkit_namecontainer

#endif // INCLUDED_TOOLKIT_CONTROLS_EVENTCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
