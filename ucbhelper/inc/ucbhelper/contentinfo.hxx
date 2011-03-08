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

#ifndef _UCBHELPER_CONTENTINFO_HXX
#define _UCBHELPER_CONTENTINFO_HXX

#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <cppuhelper/weak.hxx>

#include "osl/mutex.hxx"
#include <ucbhelper/macros.hxx>

namespace ucbhelper {

//============================================================================
//
// class PropertySetInfo.
//
//============================================================================

class ContentImplHelper;

/**
  * This class provides a propertyset info ( the complete implementation of
  * the interface XPropertySetInfo ) for an object derived from class
  * ucb::ContentImplHelper. The implementation takes care about Additional
  * Core Properties that may have been added to the content.
  */
class PropertySetInfo :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::beans::XPropertySetInfo
{
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
                                m_xSMgr;
    com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >
                                m_xEnv;
    com::sun::star::uno::Sequence< com::sun::star::beans::Property >*
                                m_pProps;
    osl::Mutex                  m_aMutex;
    ContentImplHelper*          m_pContent;

private:
    sal_Bool queryProperty( const rtl::OUString& rName,
                            com::sun::star::beans::Property& rProp );

public:
    PropertySetInfo( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                     const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XCommandEnvironment >& rxEnv,
                     ContentImplHelper* pContent );
    virtual ~PropertySetInfo();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XPropertySetInfo
    virtual com::sun::star::uno::Sequence<
                com::sun::star::beans::Property > SAL_CALL
    getProperties()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::beans::Property SAL_CALL
    getPropertyByName( const rtl::OUString& aName )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    hasPropertyByName( const rtl::OUString& Name )
        throw( com::sun::star::uno::RuntimeException );

    // Non-Interface methods.
    void reset();
};

//============================================================================
//
// class CommandProcessorInfo.
//
//============================================================================

/**
  * This class provides a command info ( the complete implementation of
  * the interface XCommandInfo ) for an object derived from class
  * ucb::ContentImplHelper.
  */
class CommandProcessorInfo :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::ucb::XCommandInfo
{
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
                                m_xSMgr;
    com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >
                                m_xEnv;
    com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >*
                                m_pCommands;
    osl::Mutex                  m_aMutex;
    ContentImplHelper*          m_pContent;

private:
    sal_Bool queryCommand( const rtl::OUString& rName,
                           com::sun::star::ucb::CommandInfo& rCommand );
    sal_Bool queryCommand( sal_Int32 nHandle,
                           com::sun::star::ucb::CommandInfo& rCommand );

public:
    CommandProcessorInfo( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                           const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XCommandEnvironment >& rxEnv,
                         ContentImplHelper* pContent );
    virtual ~CommandProcessorInfo();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XCommandInfo
    virtual com::sun::star::uno::Sequence<
                com::sun::star::ucb::CommandInfo > SAL_CALL
    getCommands()
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::ucb::CommandInfo SAL_CALL
    getCommandInfoByName( const rtl::OUString& Name )
        throw( com::sun::star::ucb::UnsupportedCommandException,
        com::sun::star::uno::RuntimeException );
    virtual com::sun::star::ucb::CommandInfo SAL_CALL
    getCommandInfoByHandle( sal_Int32 Handle )
        throw( com::sun::star::ucb::UnsupportedCommandException,
        com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    hasCommandByName( const rtl::OUString& Name )
        throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    hasCommandByHandle( sal_Int32 Handle )
        throw( com::sun::star::uno::RuntimeException );

    // Non-Interface methods.
    void reset();
};

} // namespace ucbhelper

#endif /* !_UCBHELPER_CONTENTINFO_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
