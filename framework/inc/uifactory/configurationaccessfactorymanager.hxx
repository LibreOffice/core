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

#ifndef INCLUDED_FRAMEWORK_INC_UIFACTORY_UIELEMENTFACTORYMANAGER_HXX
#define INCLUDED_FRAMEWORK_INC_UIFACTORY_UIELEMENTFACTORYMANAGER_HXX

#include <sal/config.h>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase1.hxx>
#include <rtl/ustring.hxx>

#include <boost/unordered_map.hpp>

namespace framework {

    class ConfigurationAccess_FactoryManager : public ::cppu::WeakImplHelper1< ::com::sun::star::container::XContainerListener>
{
    public:
                      ConfigurationAccess_FactoryManager( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& rxContext, const OUString& _sRoot );
        virtual       ~ConfigurationAccess_FactoryManager();

        void          readConfigurationData();

        OUString                           getFactorySpecifierFromTypeNameModule( const OUString& rType, const OUString& rName, const OUString& rModule ) const;
        void                                    addFactorySpecifierToTypeNameModule( const OUString& rType, const OUString& rName, const OUString& rModule, const OUString& aServiceSpecifier );
        void                                    removeFactorySpecifierFromTypeNameModule( const OUString& rType, const OUString& rName, const OUString& rModule );
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >   getFactoriesDescription() const;

        // container.XContainerListener
    virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // lang.XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    private:
        class FactoryManagerMap : public boost::unordered_map< OUString,
                                                     OUString,
                                                     OUStringHash,
                                                     ::std::equal_to< OUString > >
        {
            inline void free()
            {
                FactoryManagerMap().swap( *this );
            }
        };

        sal_Bool impl_getElementProps( const ::com::sun::star::uno::Any& rElement, OUString& rType, OUString& rName, OUString& rModule, OUString& rServiceSpecifier ) const;

        mutable osl::Mutex m_aMutex;
        OUString                     m_aPropType;
        OUString                     m_aPropName;
        OUString                     m_aPropModule;
        OUString                     m_aPropFactory;
        OUString                   m_sRoot;
        FactoryManagerMap                 m_aFactoryManagerMap;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xConfigProvider;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >     m_xConfigAccess;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener > m_xConfigListener;
        sal_Bool                          m_bConfigAccessInitialized;
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_UIFACTORY_UIELEMENTFACTORYMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
