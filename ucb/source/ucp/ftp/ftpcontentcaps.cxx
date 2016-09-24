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


#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ucb/CommandInfo.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include "ftpcontent.hxx"

using namespace com::sun::star;
using namespace ftp;

// virtual
uno::Sequence< beans::Property > FTPContent::getProperties(
    const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/)
{
    #define PROPS_COUNT 8

    static const beans::Property aPropsInfoTable[] =
    {
        beans::Property(
            OUString( "ContentType" ),
            -1,
            cppu::UnoType<OUString>::get(),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            OUString( "IsDocument" ),
            -1,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            OUString( "IsFolder" ),
            -1,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            OUString( "Title" ),
            -1,
            cppu::UnoType<OUString>::get(),
            beans::PropertyAttribute::BOUND
            //  | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            OUString( "Size" ),
            -1,
            cppu::UnoType<sal_Int64>::get(),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            OUString( "DateCreated" ),
            -1,
            cppu::UnoType<util::DateTime>::get(),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            OUString( "IsReadOnly" ),
            -1,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            OUString(
                "CreatableContentsInfo" ),
            -1,
            cppu::UnoType<uno::Sequence< ucb::ContentInfo >>::get(),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY
        )
    };

    return uno::Sequence< beans::Property >( aPropsInfoTable, PROPS_COUNT );
}


// virtual
uno::Sequence< ucb::CommandInfo > FTPContent::getCommands(
    const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
// osl::MutexGuard aGuard( m_aMutex );


    // Supported commands


    #define COMMAND_COUNT 8

    static const ucb::CommandInfo aCommandInfoTable[] =
    {

        // Required commands

        ucb::CommandInfo(
            OUString( "getCommandInfo" ),
            -1,
            cppu::UnoType<void>::get()
        ),
        ucb::CommandInfo(
            OUString( "getPropertySetInfo" ),
            -1,
            cppu::UnoType<void>::get()
        ),
        ucb::CommandInfo(
            OUString( "getPropertyValues" ),
            -1,
            cppu::UnoType<uno::Sequence< beans::Property >>::get()
        ),
        ucb::CommandInfo(
            OUString( "setPropertyValues" ),
            -1,
            cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get()
        ),
        ucb::CommandInfo(
            OUString( "open" ),
            -1,
            cppu::UnoType<ucb::OpenCommandArgument2>::get()
        ),
        ucb::CommandInfo(
            OUString( "insert" ),
            -1,
            cppu::UnoType<ucb::InsertCommandArgument>::get()
        ),
        ucb::CommandInfo(
            OUString( "delete" ),
            -1,
            cppu::UnoType<bool>::get()
        ),
        ucb::CommandInfo(
            OUString( "createNewContent" ),
            -1,
            cppu::UnoType<ucb::ContentInfo>::get()
        )
    };

    return uno::Sequence< ucb::CommandInfo >( aCommandInfoTable, COMMAND_COUNT );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
