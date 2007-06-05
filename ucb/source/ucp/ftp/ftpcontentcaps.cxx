/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftpcontentcaps.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2007-06-05 17:59:26 $
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
#include "precompiled_ucb.hxx"

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
    #define PROPS_COUNT 7

    static const beans::Property aPropsInfoTable[] =
    {
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
            -1,
            getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
            -1,
            getCppuBooleanType(),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
            -1,
            getCppuBooleanType(),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
            -1,
            getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
            beans::PropertyAttribute::BOUND
            //  | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Size" ) ),
            -1,
            getCppuType( static_cast< const sal_Int64 * >( 0 ) ),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateCreated" ) ),
            -1,
            getCppuType( static_cast< util::DateTime* >( 0 ) ),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsReadOnly" ) ),
            -1,
            getCppuBooleanType(),
            beans::PropertyAttribute::BOUND
            | beans::PropertyAttribute::READONLY
        )
    };

    return uno::Sequence< beans::Property >( aPropsInfoTable,PROPS_COUNT);
}

//=========================================================================
// virtual
uno::Sequence< ucb::CommandInfo > FTPContent::getCommands(
    const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
//  osl::MutexGuard aGuard( m_aMutex );

    //=================================================================
    //
    // Supported commands
    //
    //=================================================================

    #define COMMAND_COUNT 7

    static const ucb::CommandInfo aCommandInfoTable[] =
    {
        ///////////////////////////////////////////////////////////////
        // Required commands
        ///////////////////////////////////////////////////////////////
        ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
            -1,
            getCppuVoidType()
        ),
        ucb::CommandInfo(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
            -1,
            getCppuVoidType()
        ),
        ucb::CommandInfo(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
            -1,
            getCppuType(
                static_cast< uno::Sequence< beans::Property > * >( 0 ) )
        ),
        ucb::CommandInfo(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
            -1,
            getCppuType(
                static_cast< uno::Sequence< beans::PropertyValue > * >( 0 ) )
        ),
        ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
            -1,
            getCppuType(
                static_cast< ucb::OpenCommandArgument2 * >( 0 ) )
        ),
        ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
            -1,
            getCppuType(
                static_cast< ucb::InsertCommandArgument * >( 0 ) )
        ),
        ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
            -1,
            getCppuType( static_cast< sal_Bool * >( 0 ) )
        )
    };

    return uno::Sequence<ucb::CommandInfo>(aCommandInfoTable,COMMAND_COUNT);
}

