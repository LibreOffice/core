/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftpcontentcaps.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:23:24 $
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

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ucb/CommandInfo.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include "ftpcontent.hxx"

using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::util;
using namespace rtl;

using namespace ftp;

// virtual
Sequence< Property > FTPContent::getProperties(
    const Reference< XCommandEnvironment > & /*xEnv*/
)
{
    Sequence< Property > props(7);

    props[0] =
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
            -1,
            getCppuType( static_cast< const OUString * >( 0 ) ),
            PropertyAttribute::BOUND | PropertyAttribute::READONLY );

    props[1] =
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
            -1,
            getCppuBooleanType(),
            PropertyAttribute::BOUND | PropertyAttribute::READONLY );

    props[2] =
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
            -1,
            getCppuBooleanType(),
            PropertyAttribute::BOUND | PropertyAttribute::READONLY );

    props[3] =
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
            -1,
            getCppuType( static_cast< const OUString * >( 0 ) ),
            PropertyAttribute::BOUND );//  | PropertyAttribute::READONLY );

    props[4] =
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Size" ) ),
            -1,
            getCppuType( static_cast< const sal_Int64 * >( 0 ) ),
            PropertyAttribute::BOUND  | PropertyAttribute::READONLY );

    props[5] =
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "DateCreated" ) ),
            -1,
            getCppuType( static_cast< DateTime* >( 0 ) ),
            PropertyAttribute::BOUND  | PropertyAttribute::READONLY );

    props[6] =
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "IsReadOnly" ) ),
            -1,
            getCppuBooleanType(),
            PropertyAttribute::BOUND | PropertyAttribute::READONLY );

    return props;
}



//=========================================================================
// virtual
Sequence< CommandInfo > FTPContent::getCommands(
    const Reference< XCommandEnvironment > & /*xEnv*/ )
{
//  osl::MutexGuard aGuard( m_aMutex );

    //=================================================================
    //
    // Supported commands
    //
    //=================================================================

#define COMMAND_COUNT 7

    static CommandInfo aCommandInfoTable[] =
    {
        ///////////////////////////////////////////////////////////////
        // Required commands
        ///////////////////////////////////////////////////////////////
        CommandInfo(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
            -1,
            getCppuVoidType()
        ),
        CommandInfo(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
            -1,
            getCppuVoidType()
        ),
        CommandInfo(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
            -1,
            getCppuType( static_cast< Sequence< Property > * >( 0 ) )
        ),
        CommandInfo(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
            -1,
            getCppuType( static_cast< Sequence< PropertyValue > * >( 0 ) )
        ),
        CommandInfo(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
            -1,
            getCppuType( static_cast< OpenCommandArgument2 * >( 0 ) )
        ),
        CommandInfo(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
            -1,
            getCppuType( static_cast< InsertCommandArgument * >( 0 ) )
        ),
        CommandInfo(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
            -1,
            getCppuType( static_cast< sal_Bool * >( 0 ) )
        )
    };

    return Sequence<CommandInfo>(aCommandInfoTable,COMMAND_COUNT);
}

