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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlhelp.hxx"
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
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
            -1,
            getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY );

    props[idx++] =
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsReadOnly" ) ),
            -1,
            getCppuBooleanType(),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY );

    props[idx++] =
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsErrorDocument" ) ),
            -1,
            getCppuBooleanType(),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY );

    props[idx++] =
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
            -1,
            getCppuBooleanType(),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY );

    props[idx++] =
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
            -1,
            getCppuBooleanType(),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY );

    props[idx++] =
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
            -1,
            getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY );

    if( withMediaType )
        props[idx++] =
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                | beans::PropertyAttribute::READONLY );

    if( isModule )
    {
        props[idx++] =
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Order" ) ),
                -1,
                getCppuType( static_cast< sal_Int32* >( 0 ) ),
                beans::PropertyAttribute::BOUND
                | beans::PropertyAttribute::READONLY );

        props[idx++] =
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "KeywordList" ) ),
                -1,
                getCppuType(
                    static_cast< const uno::Sequence< rtl::OUString >* >(
                        0 ) ),
                beans::PropertyAttribute::BOUND
                | beans::PropertyAttribute::READONLY );

        props[idx++] =
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "KeywordRef" ) ),
                -1,
                getCppuType(
                    static_cast< const uno::Sequence< uno::Sequence< rtl::OUString > >* >( 0 ) ),
                beans::PropertyAttribute::BOUND
                | beans::PropertyAttribute::READONLY );

        props[idx++] =
            beans::Property(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "KeywordTitleForRef" ) ),
                -1,
                getCppuType(
                    static_cast< const uno::Sequence< uno::Sequence< rtl::OUString > >* >( 0 ) ),
                beans::PropertyAttribute::BOUND
                | beans::PropertyAttribute::READONLY );

        props[idx++] =
            beans::Property(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "KeywordAnchorForRef" ) ),
                -1,
                getCppuType(
                    static_cast< const uno::Sequence< uno::Sequence< rtl::OUString > >* >( 0 ) ),
                beans::PropertyAttribute::BOUND
                | beans::PropertyAttribute::READONLY );

        props[idx++] =
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SearchScopes" ) ),
                -1,
                getCppuType(
                    static_cast< const uno::Sequence< rtl::OUString >* >( 0 ) ),
                beans::PropertyAttribute::BOUND
                | beans::PropertyAttribute::READONLY );
    }

    if( isFile )
    {
        props[idx++] =
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AnchorName" ) ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                | beans::PropertyAttribute::READONLY );
    }

    return props;
}



//=========================================================================
// virtual
uno::Sequence< star::ucb::CommandInfo > Content::getCommands(
    const uno::Reference< star::ucb::XCommandEnvironment > & /*xEnv*/ )
{
    //=================================================================
    //
    // Supported commands
    //
    //=================================================================

#define COMMAND_COUNT 5

    static const star::ucb::CommandInfo aCommandInfoTable[] =
    {
        ///////////////////////////////////////////////////////////////
        // Required commands
        ///////////////////////////////////////////////////////////////
        star::ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
            -1,
            getCppuVoidType()
        ),
        star::ucb::CommandInfo(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
            -1,
            getCppuVoidType()
        ),
        star::ucb::CommandInfo(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
            -1,
            getCppuType(
                static_cast< uno::Sequence< beans::Property > * >( 0 ) )
        ),
        star::ucb::CommandInfo(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
            -1,
            getCppuType(
                static_cast< uno::Sequence< beans::PropertyValue > * >( 0 ) )
        ),
        star::ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
            -1,
            getCppuType(
                static_cast< star::ucb::OpenCommandArgument2 * >( 0 ) )
        )
    };

    return uno::Sequence< star::ucb::CommandInfo >(
        aCommandInfoTable, COMMAND_COUNT );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
