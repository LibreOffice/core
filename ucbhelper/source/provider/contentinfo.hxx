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

#include <optional>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <cppuhelper/implbase.hxx>

#include <mutex>

namespace com::sun::star::ucb { class XCommandEnvironment; }

namespace ucbhelper {




class ContentImplHelper;

/**
  * This class provides a propertyset info ( the complete implementation of
  * the interface XPropertySetInfo ) for an object derived from class
  * ucb::ContentImplHelper. The implementation takes care about Additional
  * Core Properties that may have been added to the content.
  */
class PropertySetInfo :
                public cppu::WeakImplHelper<css::beans::XPropertySetInfo>
{
    css::uno::Reference< css::ucb::XCommandEnvironment >
                                m_xEnv;
    std::optional<css::uno::Sequence< css::beans::Property >>
                                m_xProps;
    std::mutex                  m_aMutex;
    ContentImplHelper*          m_pContent;

private:
    bool queryProperty( std::u16string_view rName,
                            css::beans::Property& rProp );
    const css::uno::Sequence< css::beans::Property > & getPropertiesImpl();

public:
    PropertySetInfo( css::uno::Reference< css::ucb::XCommandEnvironment > xEnv,
                     ContentImplHelper* pContent );
    virtual ~PropertySetInfo() override;

    // XPropertySetInfo
    virtual css::uno::Sequence< css::beans::Property > SAL_CALL getProperties() override;
    virtual css::beans::Property SAL_CALL getPropertyByName( const OUString& aName ) override;
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) override;

    // Non-Interface methods.
    void reset();
};




/**
  * This class provides a command info ( the complete implementation of
  * the interface XCommandInfo ) for an object derived from class
  * ucb::ContentImplHelper.
  */
class CommandProcessorInfo :
                public cppu::WeakImplHelper<css::ucb::XCommandInfo>
{
    css::uno::Reference< css::ucb::XCommandEnvironment >
                                m_xEnv;
    std::optional<css::uno::Sequence< css::ucb::CommandInfo >>
                                m_xCommands;
    std::mutex                  m_aMutex;
    ContentImplHelper*          m_pContent;

private:
    bool queryCommand( std::u16string_view rName,
                           css::ucb::CommandInfo& rCommand );
    bool queryCommand( sal_Int32 nHandle,
                           css::ucb::CommandInfo& rCommand );
    const css::uno::Sequence< css::ucb::CommandInfo > & getCommandsImpl();

public:
    CommandProcessorInfo( css::uno::Reference< css::ucb::XCommandEnvironment > xEnv,
                         ContentImplHelper* pContent );
    virtual ~CommandProcessorInfo() override;

    // XCommandInfo
    virtual css::uno::Sequence< css::ucb::CommandInfo > SAL_CALL getCommands() override;
    virtual css::ucb::CommandInfo SAL_CALL getCommandInfoByName( const OUString& Name ) override;
    virtual css::ucb::CommandInfo SAL_CALL getCommandInfoByHandle( sal_Int32 Handle ) override;
    virtual sal_Bool SAL_CALL hasCommandByName( const OUString& Name ) override;
    virtual sal_Bool SAL_CALL hasCommandByHandle( sal_Int32 Handle ) override;

    // Non-Interface methods.
    void reset();
};

} // namespace ucbhelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
