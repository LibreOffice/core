/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contentcaps.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:20:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlhelp.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDINFO_HPP_
#include <com/sun/star/ucb/CommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

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
//  osl::MutexGuard aGuard( m_aMutex );

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

