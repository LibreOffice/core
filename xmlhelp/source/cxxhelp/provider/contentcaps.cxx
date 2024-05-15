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
            u"ContentType"_ustr,
            -1,
            cppu::UnoType<OUString>::get(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    pprops[idx++] =
        beans::Property(
            u"IsReadOnly"_ustr,
            -1,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    pprops[idx++] =
        beans::Property(
            u"IsErrorDocument"_ustr,
            -1,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    pprops[idx++] =
        beans::Property(
            u"IsDocument"_ustr,
            -1,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    pprops[idx++] =
        beans::Property(
            u"IsFolder"_ustr,
            -1,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    pprops[idx++] =
        beans::Property(
            u"Title"_ustr,
            -1,
            cppu::UnoType<OUString>::get(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    if( withMediaType )
        pprops[idx++] =
            beans::Property(
                u"MediaType"_ustr,
                -1,
                cppu::UnoType<OUString>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

    if( isModule )
    {
        pprops[idx++] =
            beans::Property(
                u"Order"_ustr,
                -1,
                cppu::UnoType<sal_Int32>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

        pprops[idx++] =
            beans::Property(
                u"KeywordList"_ustr,
                -1,
                cppu::UnoType<uno::Sequence< OUString >>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

        pprops[idx++] =
            beans::Property(
                u"KeywordRef"_ustr,
                -1,
                cppu::UnoType<uno::Sequence< uno::Sequence< OUString > >>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

        pprops[idx++] =
            beans::Property(
                u"KeywordTitleForRef"_ustr,
                -1,
                cppu::UnoType<uno::Sequence< uno::Sequence< OUString > >>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

        pprops[idx++] =
            beans::Property(
                u"KeywordAnchorForRef"_ustr,
                -1,
                cppu::UnoType<uno::Sequence< uno::Sequence< OUString > >>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );

        pprops[idx++] =
            beans::Property(
                u"SearchScopes"_ustr,
                -1,
                cppu::UnoType<uno::Sequence< OUString >>::get(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY );
    }

    if( isFile )
    {
        pprops[idx++] =
            beans::Property(
                u"AnchorName"_ustr,
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
            u"getCommandInfo"_ustr,
            -1,
            cppu::UnoType<void>::get()
        ),
        ucb::CommandInfo(
            u"getPropertySetInfo"_ustr,
            -1,
            cppu::UnoType<void>::get()
        ),
        ucb::CommandInfo(
            u"getPropertyValues"_ustr,
            -1,
            cppu::UnoType<uno::Sequence< beans::Property >>::get()
        ),
        ucb::CommandInfo(
            u"setPropertyValues"_ustr,
            -1,
            cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get()
        ),
        ucb::CommandInfo(
            u"open"_ustr,
            -1,
            cppu::UnoType<ucb::OpenCommandArgument2>::get()
        )
    };

    return uno::Sequence< ucb::CommandInfo >(
        aCommandInfoTable, COMMAND_COUNT );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
