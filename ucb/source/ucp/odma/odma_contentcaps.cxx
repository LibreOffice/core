/*************************************************************************
 *
 *  $RCSfile: odma_contentcaps.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2003-08-25 13:00:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

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
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef ODMA_CONTENT_HXX
#include "odma_content.hxx"
#endif

using namespace com::sun;
using namespace com::sun::star;
using namespace odma;

//=========================================================================
//
// Content implementation.
//
//=========================================================================

//=========================================================================
//
// IMPORTENT: If any property data ( name / type / ... ) are changed, then
//            Content::getPropertyValues(...) must be adapted too!
//
//=========================================================================

// virtual
uno::Sequence< beans::Property > Content::getProperties(
             const uno::Reference< star::ucb::XCommandEnvironment > & xEnv )
{
    // @@@ Add additional properties...

    // @@@ Note: If your data source supports adding/removing properties,
    //           you should implement the interface XPropertyContainer
    //           by yourself and supply your own logic here. The base class
    //           uses the service "com.sun.star.ucb.Store" to maintain
    //           Additional Core properties. But using server functionality
    //           is preferred! In fact you should return a table conatining
    //           even that dynamicly added properties.

//  osl::Guard< osl::Mutex > aGuard( m_aMutex );

    //=================================================================
    //
    // Supported properties
    //
    //=================================================================

    #define PROPERTY_COUNT 10

    static beans::Property aPropertyInfoTable[] =
    {
        ///////////////////////////////////////////////////////////////
        // Required properties
        ///////////////////////////////////////////////////////////////
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
            -1,
            getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
            -1,
            getCppuBooleanType(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
            -1,
            getCppuBooleanType(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
            -1,
            getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
            beans::PropertyAttribute::BOUND
        ),
        ///////////////////////////////////////////////////////////////
        // Optional standard properties
        ///////////////////////////////////////////////////////////////
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateCreated" ) ),
            -1,
            getCppuType(static_cast< const com::sun::star::util::DateTime * >( 0 ) ),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateModified" ) ),
            -1,
            getCppuType(static_cast< const com::sun::star::util::DateTime * >( 0 ) ),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY
        ),
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsReadOnly" ) ),
            -1,
            getCppuBooleanType(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY
        ),
        ///////////////////////////////////////////////////////////////
        // New properties
        ///////////////////////////////////////////////////////////////
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Author" ) ),
            -1,
            getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
            beans::PropertyAttribute::BOUND
        ),
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Subject" ) ),
            -1,
            getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
            beans::PropertyAttribute::BOUND
        ),
        beans::Property(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Keywords" ) ),
            -1,
            getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
            beans::PropertyAttribute::BOUND
        )
    };
    return uno::Sequence<
            beans::Property >( aPropertyInfoTable, PROPERTY_COUNT );
}

//=========================================================================
// virtual
uno::Sequence< star::ucb::CommandInfo > Content::getCommands(
            const uno::Reference< star::ucb::XCommandEnvironment > & xEnv )
{
    // @@@ Add additional commands...

//  osl::Guard< osl::Mutex > aGuard( m_aMutex );

    //=================================================================
    //
    // Supported commands
    //
    //=================================================================

    #define COMMAND_COUNT 8

    static star::ucb::CommandInfo aCommandInfoTable[] =
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
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
            -1,
            getCppuVoidType()
        ),
        star::ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
            -1,
            getCppuType(
                static_cast< uno::Sequence< beans::Property > * >( 0 ) )
        ),
        star::ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
            -1,
            getCppuType(
                static_cast< uno::Sequence< beans::PropertyValue > * >( 0 ) )
        ),
        ///////////////////////////////////////////////////////////////
        // Optional standard commands
        ///////////////////////////////////////////////////////////////
/*
        star::ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
            -1,
            getCppuBooleanType()
        ),
*/
        star::ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
            -1,
            getCppuType(
                static_cast< star::ucb::InsertCommandArgument * >( 0 ) )
        ),
        star::ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
            -1,
            getCppuType( static_cast< star::ucb::OpenCommandArgument2 * >( 0 ) )
        ),
        star::ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "close" ) ),
            -1,
            getCppuVoidType( )
        ),
        star::ucb::CommandInfo(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "transfer" ) ),
            -1,
            getCppuType( static_cast< star::ucb::TransferInfo * >( 0 ) )
        )

        ///////////////////////////////////////////////////////////////
        // New commands
        ///////////////////////////////////////////////////////////////
    };

    return uno::Sequence<
            star::ucb::CommandInfo >( aCommandInfoTable, COMMAND_COUNT );
}

