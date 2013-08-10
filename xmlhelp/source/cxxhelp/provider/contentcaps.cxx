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
#include <com/sun/star/uno/Sequence.hxx>

#include "content.hxx"

using namespace com::sun;
using namespace com::sun::star;

using namespace chelp;

// virtual
uno::Sequence< beans::Property > Content::getProperties(
    const uno::Reference< star::ucb::XCommandEnvironment > & /*xEnv*/ )
{
    bool withMediaType = m_aURLParameter.isFile() || m_aURLParameter.isRoot();
    bool isModule = m_aURLParameter.isModule();
    bool isFile = m_aURLParameter.isFile();

    sal_Int32 num = withMediaType ? 7 : 6;
    if( isModule ) num+=6;
    if( isFile )   num++;

    uno::Sequence< beans::Property > props(num);

    sal_Int32 idx = 0;
    props[idx++] =
        beans::Property(
            "ContentType",
            -1,
            getCppuType( static_cast< const OUString * >( 0 ) ),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    props[idx++] =
        beans::Property(
            "IsReadOnly",
            -1,
            getCppuBooleanType(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    props[idx++] =
        beans::Property(
            "IsErrorDocument",
            -1,
            getCppuBooleanType(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    props[idx++] =
        beans::Property(
            "IsDocument",
            -1,
            getCppuBooleanType(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    props[idx++] =
        beans::Property(
            "IsFolder",
            -1,
            getCppuBooleanType(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    props[idx++] =
        beans::Property(
            "Title",
            -1,
            getCppuType( static_cast< const OUString * >( 0 ) ),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    if( withMediaType )
        props[idx++] =
            beans::Property(
                "MediaType",
                -1,
                getCppuType( static_cast< const OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    if( isModule )
    {
        props[idx++] =
            beans::Property(
                "Order",
                -1,
                getCppuType( static_cast< sal_Int32* >( 0 ) ),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

        props[idx++] =
            beans::Property(
                "KeywordList",
                -1,
                getCppuType( static_cast< const uno::Sequence< OUString >* >( 0 ) ),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

        props[idx++] =
            beans::Property(
                "KeywordRef",
                -1,
                getCppuType( static_cast< const uno::Sequence< uno::Sequence< OUString > >* >( 0 ) ),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

        props[idx++] =
            beans::Property(
                "KeywordTitleForRef",
                -1,
                getCppuType( static_cast< const uno::Sequence< uno::Sequence< OUString > >* >( 0 ) ),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

        props[idx++] =
            beans::Property(
                "KeywordAnchorForRef",
                -1,
                getCppuType( static_cast< const uno::Sequence< uno::Sequence< OUString > >* >( 0 ) ),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

        props[idx++] =
            beans::Property(
                "SearchScopes",
                -1,
                getCppuType( static_cast< const uno::Sequence< OUString >* >( 0 ) ),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );
    }

    if( isFile )
    {
        props[idx++] =
            beans::Property(
                "AnchorName",
                -1,
                getCppuType( static_cast< const OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );
    }

    return props;
}

// virtual
uno::Sequence< star::ucb::CommandInfo > Content::getCommands(
    const uno::Reference< star::ucb::XCommandEnvironment > & /*xEnv*/ )
{
    // Supported commands

#define COMMAND_COUNT 5

    static const star::ucb::CommandInfo aCommandInfoTable[] =
    {
        // Required commands
        star::ucb::CommandInfo(
            "getCommandInfo",
            -1,
            getCppuVoidType()
        ),
        star::ucb::CommandInfo(
            "getPropertySetInfo",
            -1,
            getCppuVoidType()
        ),
        star::ucb::CommandInfo(
            "getPropertyValues",
            -1,
            getCppuType( static_cast< uno::Sequence< beans::Property > * >( 0 ) )
        ),
        star::ucb::CommandInfo(
            "setPropertyValues",
            -1,
            getCppuType( static_cast< uno::Sequence< beans::PropertyValue > * >( 0 ) )
        ),
        star::ucb::CommandInfo(
            "open",
            -1,
            getCppuType( static_cast< star::ucb::OpenCommandArgument2 * >( 0 ) )
        )
    };

    return uno::Sequence< star::ucb::CommandInfo >(
        aCommandInfoTable, COMMAND_COUNT );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
