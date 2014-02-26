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

#ifndef _UCBPROPS_HXX
#define _UCBPROPS_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
#include <ucbhelper/macros.hxx>



#define PROPERTIES_MANAGER_SERVICE_NAME "com.sun.star.ucb.PropertiesManager"



// class UcbPropertiesManager.



class UcbPropertiesManager :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::beans::XPropertySetInfo
{
    com::sun::star::uno::Sequence< com::sun::star::beans::Property >*
                                m_pProps;
    osl::Mutex                  m_aMutex;

private:
    sal_Bool queryProperty( const OUString& rName,
                            com::sun::star::beans::Property& rProp );

public:
    UcbPropertiesManager( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XMultiServiceFactory >&
                                rxSMgr );
    virtual ~UcbPropertiesManager();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XPropertySetInfo
    virtual com::sun::star::uno::Sequence<
                com::sun::star::beans::Property > SAL_CALL
    getProperties()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::beans::Property SAL_CALL
    getPropertyByName( const OUString& aName )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    hasPropertyByName( const OUString& Name )
        throw( com::sun::star::uno::RuntimeException );
};

#endif /* !_UCBPROPS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
