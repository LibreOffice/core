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

#ifndef INCLUDED_UCBHELPER_CONTENTINFO_HXX
#define INCLUDED_UCBHELPER_CONTENTINFO_HXX

#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <cppuhelper/weak.hxx>

#include <osl/mutex.hxx>
#include <ucbhelper/macros.hxx>

namespace ucbhelper {



// class PropertySetInfo.



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
    com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >
                                m_xEnv;
    com::sun::star::uno::Sequence< com::sun::star::beans::Property >*
                                m_pProps;
    osl::Mutex                  m_aMutex;
    ContentImplHelper*          m_pContent;

private:
    bool queryProperty( const OUString& rName,
                            com::sun::star::beans::Property& rProp );

public:
    PropertySetInfo( const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XCommandEnvironment >& rxEnv,
                     ContentImplHelper* pContent );
    virtual ~PropertySetInfo();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    // XPropertySetInfo
    virtual com::sun::star::uno::Sequence<
                com::sun::star::beans::Property > SAL_CALL
    getProperties()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::beans::Property SAL_CALL
    getPropertyByName( const OUString& aName )
        throw( com::sun::star::beans::UnknownPropertyException,
               com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    hasPropertyByName( const OUString& Name )
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    // Non-Interface methods.
    void reset();
};



// class CommandProcessorInfo.



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
    com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >
                                m_xEnv;
    com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >*
                                m_pCommands;
    osl::Mutex                  m_aMutex;
    ContentImplHelper*          m_pContent;

private:
    bool queryCommand( const OUString& rName,
                           com::sun::star::ucb::CommandInfo& rCommand );
    bool queryCommand( sal_Int32 nHandle,
                           com::sun::star::ucb::CommandInfo& rCommand );

public:
    CommandProcessorInfo( const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XCommandEnvironment >& rxEnv,
                         ContentImplHelper* pContent );
    virtual ~CommandProcessorInfo();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    // XCommandInfo
    virtual com::sun::star::uno::Sequence<
                com::sun::star::ucb::CommandInfo > SAL_CALL
    getCommands()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::ucb::CommandInfo SAL_CALL
    getCommandInfoByName( const OUString& Name )
        throw( com::sun::star::ucb::UnsupportedCommandException,
        com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::ucb::CommandInfo SAL_CALL
    getCommandInfoByHandle( sal_Int32 Handle )
        throw( com::sun::star::ucb::UnsupportedCommandException,
        com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    hasCommandByName( const OUString& Name )
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    hasCommandByHandle( sal_Int32 Handle )
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    // Non-Interface methods.
    void reset();
};

} // namespace ucbhelper

#endif /* ! INCLUDED_UCBHELPER_CONTENTINFO_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
