/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pkgcontentcaps.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:30:18 $
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

    Props/Commands:

                        rootfolder folder  stream
    ---------------------------------------------
    ContentType           r         r         r
    IsDocument            r         r         r
    IsFolder              r         r         r
    MediaType            (w)       (w)        w
    Title                 r         w         w
    Size                  -         -         r
    Compressed            -         -         w
    Encrypted             -         -         w
    HasEncryptedEntries   r         -         -

    getCommandInfo        x         x         x
    getPropertySetInfo    x         x         x
    getPropertyValues     x         x         x
    setPropertyValues     x         x         x
    insert                -         x         x
    delete                -         x         x
    open                  x         x         x
    transfer              x         x         -
    flush                 x         x         -

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

#ifndef _PKGCONTENT_HXX
#include "pkgcontent.hxx"
#endif

using namespace com::sun;
using namespace com::sun::star;
using namespace package_ucp;

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
            const uno::Reference< star::ucb::XCommandEnvironment > & /*xEnv*/ )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( isFolder() )
    {
        if ( m_aUri.isRootFolder() )
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
                ///////////////////////////////////////////////////////////////
                // Optional standard properties
                ///////////////////////////////////////////////////////////////
                beans::Property(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ),
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                ),
                ///////////////////////////////////////////////////////////////
                // New properties
                ///////////////////////////////////////////////////////////////
                beans::Property(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "HasEncryptedEntries" ) ),
                    -1,
                    getCppuBooleanType(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                )
            };
            return uno::Sequence< beans::Property >(
                                            aRootFolderPropertyInfoTable, 6 );
        }
        else
        {
            //=================================================================
            //
            // Folder: Supported properties
            //
            //=================================================================

            static beans::Property aFolderPropertyInfoTable[] =
            {
                ///////////////////////////////////////////////////////////////
                // Required properties
                ///////////////////////////////////////////////////////////////
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
                ///////////////////////////////////////////////////////////////
                // Optional standard properties
                ///////////////////////////////////////////////////////////////
                beans::Property(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ),
                    -1,
                    getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                )
                ///////////////////////////////////////////////////////////////
                // New properties
                ///////////////////////////////////////////////////////////////
            };
            return uno::Sequence< beans::Property >(
                                            aFolderPropertyInfoTable, 5 );
        }
    }
    else
    {
        //=================================================================
        //
        // Stream: Supported properties
        //
        //=================================================================

        static beans::Property aStreamPropertyInfoTable[] =
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
            ),
            ///////////////////////////////////////////////////////////////
            // Optional standard properties
            ///////////////////////////////////////////////////////////////
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ),
                -1,
                getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND
            ),
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Size" ) ),
                -1,
                getCppuType( static_cast< const sal_Int64 * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            ///////////////////////////////////////////////////////////////
            // New properties
            ///////////////////////////////////////////////////////////////
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Compressed" ) ),
                -1,
                getCppuBooleanType(),
                beans::PropertyAttribute::BOUND
            ),
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Encrypted" ) ),
                -1,
                getCppuBooleanType(),
                beans::PropertyAttribute::BOUND
            )
        };
        return uno::Sequence< beans::Property >( aStreamPropertyInfoTable, 8 );
    }
}

//=========================================================================
// virtual
uno::Sequence< star::ucb::CommandInfo > Content::getCommands(
            const uno::Reference< star::ucb::XCommandEnvironment > & /*xEnv*/ )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( isFolder() )
    {
        if ( m_aUri.isRootFolder() )
        {
            //=================================================================
            //
            // Root Folder: Supported commands
            //
            //=================================================================

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
                        static_cast<
                            uno::Sequence< beans::Property > * >( 0 ) )
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
                    getCppuType(
                        static_cast< star::ucb::TransferInfo * >( 0 ) )
                ),
                ///////////////////////////////////////////////////////////
                // New commands
                ///////////////////////////////////////////////////////////
                star::ucb::CommandInfo(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "flush" ) ),
                    -1,
                    getCppuVoidType()
                )
            };

            return uno::Sequence<
                    star::ucb::CommandInfo >( aRootFolderCommandInfoTable, 7 );
        }
        else
        {
            //=================================================================
            //
            // Folder: Supported commands
            //
            //=================================================================

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
                        static_cast<
                            uno::Sequence< beans::Property > * >( 0 ) )
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
                    getCppuType(
                        static_cast< star::ucb::TransferInfo * >( 0 ) )
                ),
                ///////////////////////////////////////////////////////////
                // New commands
                ///////////////////////////////////////////////////////////
                star::ucb::CommandInfo(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "flush" ) ),
                    -1,
                    getCppuVoidType()
                )
            };

            return uno::Sequence<
                    star::ucb::CommandInfo >( aFolderCommandInfoTable, 9 );
        }
    }
    else
    {
        //=================================================================
        //
        // Stream: Supported commands
        //
        //=================================================================

        static star::ucb::CommandInfo aStreamCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////////
            // Required commands
            ///////////////////////////////////////////////////////////////
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
            ///////////////////////////////////////////////////////////////
            // Optional standard commands
            ///////////////////////////////////////////////////////////////
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
            )
            ///////////////////////////////////////////////////////////////
            // New commands
            ///////////////////////////////////////////////////////////////
        };

        return uno::Sequence< star::ucb::CommandInfo >(
                                            aStreamCommandInfoTable, 7 );
    }
}

