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

using namespace com::sun::star;

using namespace chelp;

// virtual
uno::Sequence< beans::Property > Content::getProperties(
    const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
    bool withMediaType = m_aURLParameter.isFile() || m_aURLParameter.isRoot();
    bool isModule = m_aURLParameter.isModule();
    bool isFile = m_aURLParameter.isFile();

    sal_Int32 num = withMediaType ? 7 : 6;
    if( isModule ) num+=6;
    if( isFile )   num++;

    uno::Sequence< beans::Property > props(num);
    auto pprops = props.getArray();

    sal_Int32 idx = 0;
    pprops[idx++] =
        beans::Property(
            "ContentType",
            -1,
            cppu::UnoType<OUString>::get(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    pprops[idx++] =
        beans::Property(
            "IsReadOnly",
            -1,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    pprops[idx++] =
        beans::Property(
            "IsErrorDocument",
            -1,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    pprops[idx++] =
        beans::Property(
            "IsDocument",
            -1,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    pprops[idx++] =
        beans::Property(
            "IsFolder",
            -1,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    pprops[idx++] =
        beans::Property(
            "Title",
            -1,
            cppu::UnoType<OUString>::get(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    if( withMediaType )
        pprops[idx++] =
            beans::Property(
                "MediaType",
                -1,
                cppu::UnoType<OUString>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    if( isModule )
    {
        pprops[idx++] =
            beans::Property(
                "Order",
                -1,
                cppu::UnoType<sal_Int32>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

        pprops[idx++] =
            beans::Property(
                "KeywordList",
                -1,
                cppu::UnoType<uno::Sequence< OUString >>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

        pprops[idx++] =
            beans::Property(
                "KeywordRef",
                -1,
                cppu::UnoType<uno::Sequence< uno::Sequence< OUString > >>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

        pprops[idx++] =
            beans::Property(
                "KeywordTitleForRef",
                -1,
                cppu::UnoType<uno::Sequence< uno::Sequence< OUString > >>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

        pprops[idx++] =
            beans::Property(
                "KeywordAnchorForRef",
                -1,
                cppu::UnoType<uno::Sequence< uno::Sequence< OUString > >>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

        pprops[idx++] =
            beans::Property(
                "SearchScopes",
                -1,
                cppu::UnoType<uno::Sequence< OUString >>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );
    }

    if( isFile )
    {
        pprops[idx++] =
            beans::Property(
                "AnchorName",
                -1,
                cppu::UnoType<OUString>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );
    }

    return props;
}

// virtual
uno::Sequence< ucb::CommandInfo > Content::getCommands(
    const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
    // Supported commands

#define COMMAND_COUNT 5

    static const ucb::CommandInfo aCommandInfoTable[] =
    {
        // Required commands
        ucb::CommandInfo(
            "getCommandInfo",
            -1,
            cppu::UnoType<void>::get()
        ),
        ucb::CommandInfo(
            "getPropertySetInfo",
            -1,
            cppu::UnoType<void>::get()
        ),
        ucb::CommandInfo(
            "getPropertyValues",
            -1,
            cppu::UnoType<uno::Sequence< beans::Property >>::get()
        ),
        ucb::CommandInfo(
            "setPropertyValues",
            -1,
            cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get()
        ),
        ucb::CommandInfo(
            "open",
            -1,
            cppu::UnoType<ucb::OpenCommandArgument2>::get()
        )
    };

    return uno::Sequence< ucb::CommandInfo >(
        aCommandInfoTable, COMMAND_COUNT );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
