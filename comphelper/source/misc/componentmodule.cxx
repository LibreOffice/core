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

#include <comphelper/componentmodule.hxx>

#include <vector>

namespace comphelper
{

    using namespace ::cppu;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;

    typedef std::vector< ComponentDescription >   ComponentDescriptions;

    /** implementation for <type>OModule</type>. not threadsafe, has to be guarded by its owner
    */
    class OModuleImpl
    {
    public:
        ComponentDescriptions                           m_aRegisteredComponents;

        OModuleImpl();
    };

    OModuleImpl::OModuleImpl()
    {
    }

    OModule::OModule()
        : m_pImpl(new OModuleImpl)
    {
    }

    OModule::~OModule()
    {
    }

    void OModule::registerImplementation( const ComponentDescription& _rComp )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_pImpl )
            throw RuntimeException();

        m_pImpl->m_aRegisteredComponents.push_back( _rComp );
    }

    void OModule::registerImplementation( const OUString& _rImplementationName, const css::uno::Sequence< OUString >& _rServiceNames,
        ::cppu::ComponentFactoryFunc _pCreateFunction )
    {
        ComponentDescription aComponent( _rImplementationName, _rServiceNames, _pCreateFunction, ::cppu::createSingleComponentFactory );
        registerImplementation( aComponent );
    }

    void* OModule::getComponentFactory( const sal_Char* _pImplementationName )
    {
        Reference< XInterface > xFactory( getComponentFactory(
            OUString::createFromAscii( _pImplementationName ) ) );
        return xFactory.get();
    }

    Reference< XInterface > OModule::getComponentFactory( const OUString& _rImplementationName )
    {
        Reference< XInterface > xReturn;

        for (const auto& rComponent : m_pImpl->m_aRegisteredComponents)
        {
            if ( rComponent.sImplementationName == _rImplementationName )
            {
                xReturn = rComponent.pFactoryCreationFunc(
                    rComponent.pComponentCreationFunc,
                    rComponent.sImplementationName,
                    rComponent.aSupportedServices,
                    nullptr
                );
                if ( xReturn.is() )
                {
                    xReturn->acquire();
                    return xReturn.get();
                }
            }
        }

        return nullptr;
    }

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
