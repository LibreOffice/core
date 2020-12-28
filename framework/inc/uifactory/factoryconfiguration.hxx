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

#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>

#include <string_view>
#include <unordered_map>

//  Namespace

namespace framework
{

//  Configuration access class for PopupMenuControllerFactory implementation

class ConfigurationAccess_ControllerFactory final : // interfaces
                                                    public  ::cppu::WeakImplHelper< css::container::XContainerListener>
{
public:
                    ConfigurationAccess_ControllerFactory( const css::uno::Reference< css::uno::XComponentContext >& rxContext, const OUString& _sRoot );
    virtual       ~ConfigurationAccess_ControllerFactory() override;

    void          readConfigurationData();
    void          updateConfigurationData();

    OUString getServiceFromCommandModule( std::u16string_view rCommandURL, std::u16string_view rModule ) const;
    OUString getValueFromCommandModule( std::u16string_view rCommandURL, std::u16string_view rModule ) const;
    void          addServiceToCommandModule( std::u16string_view rCommandURL, std::u16string_view rModule, const OUString& rServiceSpecifier );
    void          removeServiceFromCommandModule( std::u16string_view rCommandURL, std::u16string_view rModule );

    // container.XContainerListener
    virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) override;
    virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) override;
    virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) override;

    // lang.XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

private:
    struct ControllerInfo
    {
        OUString m_aImplementationName;
        OUString m_aValue;
        ControllerInfo(const OUString& _aImplementationName,const OUString& _aValue) : m_aImplementationName(_aImplementationName),m_aValue(_aValue){}
        ControllerInfo(){}
    };
    class MenuControllerMap : public std::unordered_map< OUString,
                                                         ControllerInfo >
    {
    };

    bool impl_getElementProps( const css::uno::Any& aElement, OUString& aCommand, OUString& aModule, OUString& aServiceSpecifier,OUString& aValue ) const;

    mutable osl::Mutex           m_mutex;
    OUString                     m_aPropCommand;
    OUString                     m_aPropModule;
    OUString                     m_aPropController;
    OUString                     m_aPropValue;
    OUString                     m_sRoot;
    MenuControllerMap            m_aMenuControllerMap;
    css::uno::Reference< css::lang::XMultiServiceFactory >    m_xConfigProvider;
    css::uno::Reference< css::container::XNameAccess >        m_xConfigAccess;
    css::uno::Reference< css::container::XContainerListener > m_xConfigAccessListener;
    bool                          m_bConfigAccessInitialized;
};

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
