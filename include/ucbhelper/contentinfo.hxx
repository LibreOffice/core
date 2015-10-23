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
                public css::lang::XTypeProvider,
                public css::beans::XPropertySetInfo
{
    css::uno::Reference< css::ucb::XCommandEnvironment >
                                m_xEnv;
    css::uno::Sequence< css::beans::Property >*
                                m_pProps;
    osl::Mutex                  m_aMutex;
    ContentImplHelper*          m_pContent;

private:
    bool queryProperty( const OUString& rName,
                            css::beans::Property& rProp );

public:
    PropertySetInfo( const css::uno::Reference< css::ucb::XCommandEnvironment >& rxEnv,
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
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XPropertySetInfo
    virtual css::uno::Sequence<
                css::beans::Property > SAL_CALL
    getProperties()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::beans::Property SAL_CALL
    getPropertyByName( const OUString& aName )
        throw( css::beans::UnknownPropertyException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    hasPropertyByName( const OUString& Name )
        throw( css::uno::RuntimeException, std::exception ) override;

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
                public css::lang::XTypeProvider,
                public css::ucb::XCommandInfo
{
    css::uno::Reference< css::ucb::XCommandEnvironment >
                                m_xEnv;
    css::uno::Sequence< css::ucb::CommandInfo >*
                                m_pCommands;
    osl::Mutex                  m_aMutex;
    ContentImplHelper*          m_pContent;

private:
    bool queryCommand( const OUString& rName,
                           css::ucb::CommandInfo& rCommand );
    bool queryCommand( sal_Int32 nHandle,
                           css::ucb::CommandInfo& rCommand );

public:
    CommandProcessorInfo( const css::uno::Reference< css::ucb::XCommandEnvironment >& rxEnv,
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
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XCommandInfo
    virtual css::uno::Sequence<
                css::ucb::CommandInfo > SAL_CALL
    getCommands()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::ucb::CommandInfo SAL_CALL
    getCommandInfoByName( const OUString& Name )
        throw( css::ucb::UnsupportedCommandException,
        css::uno::RuntimeException, std::exception ) override;
    virtual css::ucb::CommandInfo SAL_CALL
    getCommandInfoByHandle( sal_Int32 Handle )
        throw( css::ucb::UnsupportedCommandException,
        css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    hasCommandByName( const OUString& Name )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    hasCommandByHandle( sal_Int32 Handle )
        throw( css::uno::RuntimeException, std::exception ) override;

    // Non-Interface methods.
    void reset();
};

} // namespace ucbhelper

#endif /* ! INCLUDED_UCBHELPER_CONTENTINFO_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
