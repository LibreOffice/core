/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_UIFACTORY_FACTORYCONFIGURATION_HXX_
#define __FRAMEWORK_UIFACTORY_FACTORYCONFIGURATION_HXX_
#include <threadhelp/threadhelpbase.hxx>
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
#include <com/sun/star/frame/XUIControllerRegistration.hpp>

#include <cppuhelper/implbase1.hxx>
#include <rtl/ustring.hxx>

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________

namespace framework
{

//*****************************************************************************************************************
//  Configuration access class for PopupMenuControllerFactory implementation
//*****************************************************************************************************************
class ConfigurationAccess_ControllerFactory : // interfaces
                                                    private ThreadHelpBase,
                                                    public  ::cppu::WeakImplHelper1< ::com::sun::star::container::XContainerListener>
{
public:
                    ConfigurationAccess_ControllerFactory( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rServiceManager,const ::rtl::OUString& _sRoot,bool _bAskValue = false );
    virtual       ~ConfigurationAccess_ControllerFactory();

    void          readConfigurationData();
    void          updateConfigurationData();

    rtl::OUString getServiceFromCommandModule( const rtl::OUString& rCommandURL, const rtl::OUString& rModule ) const;
    rtl::OUString getValueFromCommandModule( const rtl::OUString& rCommandURL, const rtl::OUString& rModule ) const;
    void          addServiceToCommandModule( const rtl::OUString& rCommandURL, const rtl::OUString& rModule, const rtl::OUString& rServiceSpecifier );
    void          removeServiceFromCommandModule( const rtl::OUString& rCommandURL, const rtl::OUString& rModule );
    inline bool   hasValue() const { return m_bAskValue; }

    // container.XContainerListener
    virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

    // lang.XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

private:
    struct ControllerInfo
    {
        rtl::OUString m_aImplementationName;
        rtl::OUString m_aValue;
        ControllerInfo(const ::rtl::OUString& _aImplementationName,const ::rtl::OUString& _aValue) : m_aImplementationName(_aImplementationName),m_aValue(_aValue){}
        ControllerInfo(){}
    };
    class MenuControllerMap : public boost::unordered_map< rtl::OUString,
                                                         ControllerInfo,
                                                         OUStringHashCode,
                                                         ::std::equal_to< ::rtl::OUString > >
    {
        inline void free()
        {
            MenuControllerMap().swap( *this );
        }
    };

    sal_Bool impl_getElementProps( const ::com::sun::star::uno::Any& aElement, rtl::OUString& aCommand, rtl::OUString& aModule, rtl::OUString& aServiceSpecifier,rtl::OUString& aValue ) const;

    rtl::OUString                     m_aPropCommand;
    rtl::OUString                     m_aPropModule;
    rtl::OUString                     m_aPropController;
    rtl::OUString                     m_aPropValue;
    rtl::OUString                     m_sRoot;
    MenuControllerMap                 m_aMenuControllerMap;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xConfigProvider;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >        m_xConfigAccess;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener > m_xConfigAccessListener;
    sal_Bool                          m_bConfigAccessInitialized;
    bool                              m_bAskValue;
};

} // namespace framework
#endif // __FRAMEWORK_UIFACTORY_FACTORYCONFIGURATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
