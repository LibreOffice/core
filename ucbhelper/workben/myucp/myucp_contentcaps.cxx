/*************************************************************************
 *
 *  $RCSfile: myucp_contentcaps.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kso $ $Date: 2001-03-27 14:04:33 $
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
/*
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
*/
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

// @@@ Adjust multi-include-protection-ifdef and header file name.
#ifndef _MYUCP_CONTENT_HXX
#include "myucp_content.hxx"
#endif

using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace rtl;

// @@@ Adjust namespace name.
using namespace myucp;

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
Sequence< Property > Content::getProperties(
                            const Reference< XCommandEnvironment > & xEnv )
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

    #define PROPERTY_COUNT 4

    static Property aPropertyInfoTable[] =
    {
        ///////////////////////////////////////////////////////////////
        // Required properties
        ///////////////////////////////////////////////////////////////
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
            -1,
            getCppuType( static_cast< const OUString * >( 0 ) ),
            PropertyAttribute::BOUND | PropertyAttribute::READONLY
        ),
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
            -1,
            getCppuBooleanType(),
            PropertyAttribute::BOUND | PropertyAttribute::READONLY
        ),
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
            -1,
            getCppuBooleanType(),
            PropertyAttribute::BOUND | PropertyAttribute::READONLY
        ),
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
            -1,
            getCppuType( static_cast< const OUString * >( 0 ) ),
            PropertyAttribute::BOUND
        )
        ///////////////////////////////////////////////////////////////
        // Optional standard properties
        ///////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////
        // New properties
        ///////////////////////////////////////////////////////////////
    };
    return Sequence< Property >( aPropertyInfoTable, PROPERTY_COUNT );
}

//=========================================================================
// virtual
Sequence< CommandInfo > Content::getCommands(
                            const Reference< XCommandEnvironment > & xEnv )
{
    // @@@ Add additional commands...

//  osl::Guard< osl::Mutex > aGuard( m_aMutex );

    //=================================================================
    //
    // Supported commands
    //
    //=================================================================

    #define COMMAND_COUNT 4

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
        )
        ///////////////////////////////////////////////////////////////
        // Optional standard commands
        ///////////////////////////////////////////////////////////////
/*
        CommandInfo(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
            -1,
            getCppuBooleanType()
        ),
        CommandInfo(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
            -1,
            getCppuType( static_cast< InsertCommandArgument * >( 0 ) )
        ),
        CommandInfo(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
            -1,
            getCppuType( static_cast< OpenCommandArgument2 * >( 0 ) )
        )
*/
        ///////////////////////////////////////////////////////////////
        // New commands
        ///////////////////////////////////////////////////////////////
    };

    return Sequence< CommandInfo >( aCommandInfoTable, COMMAND_COUNT );
}

