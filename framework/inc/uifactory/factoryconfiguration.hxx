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

#ifndef INCLUDED_FRAMEWORK_INC_UIFACTORY_FACTORYCONFIGURATION_HXX
#define INCLUDED_FRAMEWORK_INC_UIFACTORY_FACTORYCONFIGURATION_HXX
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <unordered_map>

//  Namespace

namespace framework
{

//  Configuration access class for PopupMenuControllerFactory implementation

class ConfigurationAccess_ControllerFactory : // interfaces
                                                    public  ::cppu::WeakImplHelper< ::com::sun::star::container::XContainerListener>
{
public:
                    ConfigurationAccess_ControllerFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext, const OUString& _sRoot );
    virtual       ~ConfigurationAccess_ControllerFactory();

    void          readConfigurationData();
    void          updateConfigurationData();

    OUString getServiceFromCommandModule( const OUString& rCommandURL, const OUString& rModule ) const;
    OUString getValueFromCommandModule( const OUString& rCommandURL, const OUString& rModule ) const;
    void          addServiceToCommandModule( const OUString& rCommandURL, const OUString& rModule, const OUString& rServiceSpecifier );
    void          removeServiceFromCommandModule( const OUString& rCommandURL, const OUString& rModule );

    // container.XContainerListener
    virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // lang.XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    struct ControllerInfo
    {
        OUString m_aImplementationName;
        OUString m_aValue;
        ControllerInfo(const OUString& _aImplementationName,const OUString& _aValue) : m_aImplementationName(_aImplementationName),m_aValue(_aValue){}
        ControllerInfo(){}
    };
    class MenuControllerMap : public std::unordered_map< OUString,
                                                         ControllerInfo,
                                                         OUStringHash,
                                                         std::equal_to< OUString > >
    {
    };

    bool impl_getElementProps( const ::com::sun::star::uno::Any& aElement, OUString& aCommand, OUString& aModule, OUString& aServiceSpecifier,OUString& aValue ) const;

    mutable osl::Mutex m_mutex;
    OUString                     m_aPropCommand;
    OUString                     m_aPropModule;
    OUString                     m_aPropController;
    OUString                     m_aPropValue;
    OUString                     m_sRoot;
    MenuControllerMap                 m_aMenuControllerMap;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xConfigProvider;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >        m_xConfigAccess;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener > m_xConfigAccessListener;
    bool                          m_bConfigAccessInitialized;
};

} // namespace framework
#endif // INCLUDED_FRAMEWORK_INC_UIFACTORY_FACTORYCONFIGURATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
