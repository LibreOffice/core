/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hierarchycontentcaps.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:28:38 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 **************************************************************************

    Props/Commands:

                        root    folder  folder  link    link
                                        (new)           (new)
    ----------------------------------------------------------------
    ContentType         x       x       x       x       x
    IsDocument          x       x       x       x       x
    IsFolder            x       x       x       x       x
    Title               x       x       x       x       x
    TargetURL                                   x       x

    getCommandInfo      x       x       x       x       x
    getPropertySetInfo  x       x       x       x       x
    getPropertyValues   x       x       x       x       x
    setPropertyValues   x       x       x       x       x
    insert                              x               x
    delete                      x               x
    open                x       x
    transfer            x       x

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
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _HIERARCHYCONTENT_HXX
#include "hierarchycontent.hxx"
#endif

using namespace com::sun;
using namespace com::sun::star;
using namespace hierarchy_ucp;

//=========================================================================
//
// HierarchyContent implementation.
//
//=========================================================================

//=========================================================================
//
// IMPORTENT: If any property data ( name / type / ... ) are changed, then
//            HierarchyContent::getPropertyValues(...) must be adapted too!
//
//=========================================================================

// virtual
uno::Sequence< beans::Property > HierarchyContent::getProperties(
            const uno::Reference< star::ucb::XCommandEnvironment > & /*xEnv*/ )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_eKind == LINK )
    {
        //=================================================================
        //
        // Link: Supported properties
        //
        //=================================================================

        if ( isReadOnly() )
        {
            static beans::Property aLinkPropertyInfoTable[] =
            {
                ///////////////////////////////////////////////////////////
                // Required properties
                ///////////////////////////////////////////////////////////
                beans::Property(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
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
                        | beans::PropertyAttribute::READONLY
                ),
                ///////////////////////////////////////////////////////////
                // Optional standard properties
                ///////////////////////////////////////////////////////////
                beans::Property(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TargetURL" ) ),
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                )
                ///////////////////////////////////////////////////////////
                // New properties
                ///////////////////////////////////////////////////////////
            };
            return uno::Sequence<
                            beans::Property >( aLinkPropertyInfoTable, 5 );
        }
        else
        {
            static beans::Property aLinkPropertyInfoTable[] =
            {
                ///////////////////////////////////////////////////////////
                // Required properties
                ///////////////////////////////////////////////////////////
                beans::Property(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
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
                ),
                ///////////////////////////////////////////////////////////
                // Optional standard properties
                ///////////////////////////////////////////////////////////
                beans::Property(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TargetURL" ) ),
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                )
                ///////////////////////////////////////////////////////////
                // New properties
                ///////////////////////////////////////////////////////////
            };
            return uno::Sequence<
                            beans::Property >( aLinkPropertyInfoTable, 5 );
        }
    }
    else if ( m_eKind == FOLDER )
    {
        //=================================================================
        //
        // Folder: Supported properties
        //
        //=================================================================

        if ( isReadOnly() )
        {
            static beans::Property aFolderPropertyInfoTable[] =
            {
                ///////////////////////////////////////////////////////////
                // Required properties
                ///////////////////////////////////////////////////////////
                beans::Property(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
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
                        | beans::PropertyAttribute::READONLY
                )
                ///////////////////////////////////////////////////////////
                // Optional standard properties
                ///////////////////////////////////////////////////////////
                ///////////////////////////////////////////////////////////
                // New properties
                ///////////////////////////////////////////////////////////
            };
            return uno::Sequence<
                            beans::Property >( aFolderPropertyInfoTable, 4 );
        }
        else
        {
            static beans::Property aFolderPropertyInfoTable[] =
            {
                ///////////////////////////////////////////////////////////
                // Required properties
                ///////////////////////////////////////////////////////////
                beans::Property(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
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
                )
                ///////////////////////////////////////////////////////////
                // Optional standard properties
                ///////////////////////////////////////////////////////////
                ///////////////////////////////////////////////////////////
                // New properties
                ///////////////////////////////////////////////////////////
            };
            return uno::Sequence<
                            beans::Property >( aFolderPropertyInfoTable, 4 );
        }
    }
    else
    {
        //=================================================================
        //
        // Root Folder: Supported properties
        //
        //=================================================================

        static beans::Property aRootFolderPropertyInfoTable[] =
        {
            ///////////////////////////////////////////////////////////////
            // Required properties
            ///////////////////////////////////////////////////////////////
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
                    | beans::PropertyAttribute::READONLY
            )
            ///////////////////////////////////////////////////////////////
            // Optional standard properties
            ///////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////
            // New properties
            ///////////////////////////////////////////////////////////////
        };
        return uno::Sequence<
                beans::Property >( aRootFolderPropertyInfoTable, 4 );
    }
}

//=========================================================================
// virtual
uno::Sequence< star::ucb::CommandInfo > HierarchyContent::getCommands(
            const uno::Reference< star::ucb::XCommandEnvironment > & /*xEnv*/ )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_eKind == LINK )
    {
        //=================================================================
        //
        // Link: Supported commands
        //
        //=================================================================

        if ( isReadOnly() )
        {
            static star::ucb::CommandInfo aLinkCommandInfoTable[] =
            {
                ///////////////////////////////////////////////////////////
                // Required commands
                ///////////////////////////////////////////////////////////
                star::ucb::CommandInfo(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
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
                        static_cast<
                            uno::Sequence< beans::PropertyValue > * >( 0 ) )
                )
                ///////////////////////////////////////////////////////////
                // Optional standard commands
                ///////////////////////////////////////////////////////////

                ///////////////////////////////////////////////////////////
                // New commands
                ///////////////////////////////////////////////////////////
            };
            return uno::Sequence<
                    star::ucb::CommandInfo >( aLinkCommandInfoTable, 4 );
        }
        else
        {
            static star::ucb::CommandInfo aLinkCommandInfoTable[] =
            {
                ///////////////////////////////////////////////////////////
                // Required commands
                ///////////////////////////////////////////////////////////
                star::ucb::CommandInfo(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
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
                        static_cast<
                            uno::Sequence< beans::PropertyValue > * >( 0 ) )
                ),
                ///////////////////////////////////////////////////////////
                // Optional standard commands
                ///////////////////////////////////////////////////////////
                star::ucb::CommandInfo(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
                    -1,
                    getCppuBooleanType()
                ),
                star::ucb::CommandInfo(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
                    -1,
                    getCppuVoidType()
                )
                ///////////////////////////////////////////////////////////
                // New commands
                ///////////////////////////////////////////////////////////
            };
            return uno::Sequence<
                    star::ucb::CommandInfo >( aLinkCommandInfoTable, 6 );
        }
    }
    else if ( m_eKind == FOLDER )
    {
        //=================================================================
        //
        // Folder: Supported commands
        //
        //=================================================================

        if ( isReadOnly() )
        {
            static star::ucb::CommandInfo aFolderCommandInfoTable[] =
            {
                ///////////////////////////////////////////////////////////
                // Required commands
                ///////////////////////////////////////////////////////////
                star::ucb::CommandInfo(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
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
                        static_cast<
                            uno::Sequence< beans::PropertyValue > * >( 0 ) )
                ),
                ///////////////////////////////////////////////////////////
                // Optional standard commands
                ///////////////////////////////////////////////////////////
                star::ucb::CommandInfo(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                    -1,
                    getCppuType(
                        static_cast< star::ucb::OpenCommandArgument2 * >( 0 ) )
                )
                ///////////////////////////////////////////////////////////
                // New commands
                ///////////////////////////////////////////////////////////
            };
            return uno::Sequence<
                    star::ucb::CommandInfo >( aFolderCommandInfoTable, 5 );
        }
        else
        {
            static star::ucb::CommandInfo aFolderCommandInfoTable[] =
            {
                ///////////////////////////////////////////////////////////
                // Required commands
                ///////////////////////////////////////////////////////////
                star::ucb::CommandInfo(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
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
                        static_cast<
                            uno::Sequence< beans::PropertyValue > * >( 0 ) )
                ),
                ///////////////////////////////////////////////////////////
                // Optional standard commands
                ///////////////////////////////////////////////////////////
                star::ucb::CommandInfo(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
                    -1,
                    getCppuBooleanType()
                ),
                star::ucb::CommandInfo(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
                    -1,
                    getCppuVoidType()
                ),
                star::ucb::CommandInfo(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                    -1,
                    getCppuType(
                        static_cast< star::ucb::OpenCommandArgument2 * >( 0 ) )
                ),
                star::ucb::CommandInfo(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "transfer" ) ),
                    -1,
                    getCppuType( static_cast< star::ucb::TransferInfo * >( 0 ) )
                )
                ///////////////////////////////////////////////////////////
                // New commands
                ///////////////////////////////////////////////////////////
            };
            return uno::Sequence<
                    star::ucb::CommandInfo >( aFolderCommandInfoTable, 8 );
        }
    }
    else
    {
        //=================================================================
        //
        // Root Folder: Supported commands
        //
        //=================================================================

        if ( isReadOnly() )
        {
            static star::ucb::CommandInfo aRootFolderCommandInfoTable[] =
            {
                ///////////////////////////////////////////////////////////
                // Required commands
                ///////////////////////////////////////////////////////////
                star::ucb::CommandInfo(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
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
                        static_cast<
                            uno::Sequence< beans::PropertyValue > * >( 0 ) )
                ),
                ///////////////////////////////////////////////////////////
                // Optional standard commands
                ///////////////////////////////////////////////////////////
                star::ucb::CommandInfo(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                    -1,
                    getCppuType(
                        static_cast< star::ucb::OpenCommandArgument2 * >( 0 ) )
                )
                ///////////////////////////////////////////////////////////
                // New commands
                ///////////////////////////////////////////////////////////
            };
            return uno::Sequence<
                    star::ucb::CommandInfo >( aRootFolderCommandInfoTable, 5 );
        }
        else
        {
            static star::ucb::CommandInfo aRootFolderCommandInfoTable[] =
            {
                ///////////////////////////////////////////////////////////
                // Required commands
                ///////////////////////////////////////////////////////////
                star::ucb::CommandInfo(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
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
                        static_cast<
                            uno::Sequence< beans::PropertyValue > * >( 0 ) )
                ),
                ///////////////////////////////////////////////////////////
                // Optional standard commands
                ///////////////////////////////////////////////////////////
                star::ucb::CommandInfo(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                    -1,
                    getCppuType(
                        static_cast< star::ucb::OpenCommandArgument2 * >( 0 ) )
                ),
                star::ucb::CommandInfo(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "transfer" ) ),
                    -1,
                    getCppuType( static_cast< star::ucb::TransferInfo * >( 0 ) )
                )
                ///////////////////////////////////////////////////////////
                // New commands
                ///////////////////////////////////////////////////////////
            };
            return uno::Sequence<
                    star::ucb::CommandInfo >( aRootFolderCommandInfoTable, 6 );
        }
    }
}

