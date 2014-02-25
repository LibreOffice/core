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
    CreatableContentsInfo r         r         r
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
    createNewContent      x         x         -

 *************************************************************************/
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ucb/CommandInfo.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <sal/macros.h>
#include "pkgcontent.hxx"

using namespace com::sun::star;
using namespace package_ucp;



// Content implementation.



#define MAKEPROPSEQUENCE( a ) \
    uno::Sequence< beans::Property >( a, sizeof( a )  / sizeof( a[ 0 ] ) )

#define MAKECMDSEQUENCE( a ) \
    uno::Sequence< ucb::CommandInfo >( a, sizeof( a )  / sizeof( a[ 0 ] ) )



// IMPORTANT: If any property data ( name / type / ... ) are changed, then
//            Content::getPropertyValues(...) must be adapted too!



// virtual
uno::Sequence< beans::Property > Content::getProperties(
            const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( isFolder() )
    {
        if ( m_aUri.isRootFolder() )
        {


            // Root Folder: Supported properties



            static const beans::Property aRootFolderPropertyInfoTable[] =
            {

                // Required properties

                beans::Property(
                    OUString( "ContentType" ),
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    OUString( "IsDocument" ),
                    -1,
                    getCppuBooleanType(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    OUString( "IsFolder" ),
                    -1,
                    getCppuBooleanType(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    OUString( "Title" ),
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),

                // Optional standard properties

                beans::Property(
                    OUString( "MediaType" ),
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                ),
                beans::Property(
                    OUString(
                        "CreatableContentsInfo" ),
                    -1,
                    getCppuType( static_cast<
                        const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),

                // New properties

                beans::Property(
                    OUString( "HasEncryptedEntries" ),
                    -1,
                    getCppuBooleanType(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                )
            };
            return MAKEPROPSEQUENCE( aRootFolderPropertyInfoTable );
        }
        else
        {


            // Folder: Supported properties



            static const beans::Property aFolderPropertyInfoTable[] =
            {

                // Required properties

                beans::Property(
                    OUString( "ContentType" ),
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    OUString( "IsDocument" ),
                    -1,
                    getCppuBooleanType(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    OUString( "IsFolder" ),
                    -1,
                    getCppuBooleanType(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    OUString( "Title" ),
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                ),

                // Optional standard properties

                beans::Property(
                    OUString( "MediaType" ),
                    -1,
                    getCppuType( static_cast< const OUString * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                ),
                beans::Property(
                    OUString(
                        "CreatableContentsInfo" ),
                    -1,
                    getCppuType( static_cast<
                        const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                )

                // New properties

            };
            return MAKEPROPSEQUENCE( aFolderPropertyInfoTable );
        }
    }
    else
    {


        // Stream: Supported properties



        static const beans::Property aStreamPropertyInfoTable[] =
        {

            // Required properties

            beans::Property(
                OUString( "ContentType" ),
                -1,
                getCppuType( static_cast< const OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                OUString( "IsDocument" ),
                -1,
                getCppuBooleanType(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                OUString( "IsFolder" ),
                -1,
                getCppuBooleanType(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                OUString( "Title" ),
                -1,
                getCppuType( static_cast< const OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND
            ),

            // Optional standard properties

            beans::Property(
                OUString( "MediaType" ),
                -1,
                getCppuType( static_cast< const OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND
            ),
            beans::Property(
                OUString( "Size" ),
                -1,
                getCppuType( static_cast< const sal_Int64 * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                OUString(
                    "CreatableContentsInfo" ),
                -1,
                getCppuType( static_cast<
                    const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                | beans::PropertyAttribute::READONLY
            ),

            // New properties

            beans::Property(
                OUString( "Compressed" ),
                -1,
                getCppuBooleanType(),
                beans::PropertyAttribute::BOUND
            ),
            beans::Property(
                OUString( "Encrypted" ),
                -1,
                getCppuBooleanType(),
                beans::PropertyAttribute::BOUND
            )
        };
        return MAKEPROPSEQUENCE( aStreamPropertyInfoTable );
    }
}


// virtual
uno::Sequence< ucb::CommandInfo > Content::getCommands(
            const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( isFolder() )
    {
        if ( m_aUri.isRootFolder() )
        {


            // Root Folder: Supported commands



            static const ucb::CommandInfo aRootFolderCommandInfoTable[] =
            {

                // Required commands

                ucb::CommandInfo(
                    OUString( "getCommandInfo" ),
                    -1,
                    getCppuVoidType()
                ),
                ucb::CommandInfo(
                    OUString( "getPropertySetInfo" ),
                    -1,
                    getCppuVoidType()
                ),
                ucb::CommandInfo(
                    OUString( "getPropertyValues" ),
                    -1,
                    getCppuType(
                        static_cast<
                            uno::Sequence< beans::Property > * >( 0 ) )
                ),
                ucb::CommandInfo(
                    OUString( "setPropertyValues" ),
                    -1,
                    getCppuType(
                        static_cast<
                            uno::Sequence< beans::PropertyValue > * >( 0 ) )
                ),

                // Optional standard commands

                ucb::CommandInfo(
                    OUString( "open" ),
                    -1,
                    getCppuType(
                        static_cast< ucb::OpenCommandArgument2 * >( 0 ) )
                ),
                ucb::CommandInfo(
                    OUString( "transfer" ),
                    -1,
                    getCppuType(
                        static_cast< ucb::TransferInfo * >( 0 ) )
                ),
                ucb::CommandInfo(
                    OUString( "createNewContent" ),
                    -1,
                    getCppuType( static_cast< ucb::ContentInfo * >( 0 ) )
                ),

                // New commands

                ucb::CommandInfo(
                    OUString( "flush" ),
                    -1,
                    getCppuVoidType()
                )
            };

            return MAKECMDSEQUENCE( aRootFolderCommandInfoTable );
        }
        else
        {


            // Folder: Supported commands



            static const ucb::CommandInfo aFolderCommandInfoTable[] =
            {

                // Required commands

                ucb::CommandInfo(
                    OUString( "getCommandInfo" ),
                    -1,
                    getCppuVoidType()
                ),
                ucb::CommandInfo(
                    OUString( "getPropertySetInfo" ),
                    -1,
                    getCppuVoidType()
                ),
                ucb::CommandInfo(
                    OUString( "getPropertyValues" ),
                    -1,
                    getCppuType(
                        static_cast<
                            uno::Sequence< beans::Property > * >( 0 ) )
                ),
                ucb::CommandInfo(
                    OUString( "setPropertyValues" ),
                    -1,
                    getCppuType(
                        static_cast<
                            uno::Sequence< beans::PropertyValue > * >( 0 ) )
                ),

                // Optional standard commands

                ucb::CommandInfo(
                    OUString( "delete" ),
                    -1,
                    getCppuBooleanType()
                ),
                ucb::CommandInfo(
                    OUString( "insert" ),
                    -1,
                    getCppuVoidType()
                ),
                ucb::CommandInfo(
                    OUString( "open" ),
                    -1,
                    getCppuType(
                        static_cast< ucb::OpenCommandArgument2 * >( 0 ) )
                ),
                ucb::CommandInfo(
                    OUString( "transfer" ),
                    -1,
                    getCppuType(
                        static_cast< ucb::TransferInfo * >( 0 ) )
                ),
                ucb::CommandInfo(
                    OUString( "createNewContent" ),
                    -1,
                    getCppuType( static_cast< ucb::ContentInfo * >( 0 ) )
                ),

                // New commands

                ucb::CommandInfo(
                    OUString( "flush" ),
                    -1,
                    getCppuVoidType()
                )
            };

            return MAKECMDSEQUENCE( aFolderCommandInfoTable );
        }
    }
    else
    {


        // Stream: Supported commands



        static const ucb::CommandInfo aStreamCommandInfoTable[] =
        {

            // Required commands

            ucb::CommandInfo(
                OUString( "getCommandInfo" ),
                -1,
                getCppuVoidType()
            ),
            ucb::CommandInfo(
                OUString( "getPropertySetInfo" ),
                -1,
                getCppuVoidType()
            ),
            ucb::CommandInfo(
                OUString( "getPropertyValues" ),
                -1,
                getCppuType(
                    static_cast< uno::Sequence< beans::Property > * >( 0 ) )
            ),
            ucb::CommandInfo(
                OUString( "setPropertyValues" ),
                -1,
                getCppuType(
                    static_cast<
                        uno::Sequence< beans::PropertyValue > * >( 0 ) )
            ),

            // Optional standard commands

            ucb::CommandInfo(
                OUString( "delete" ),
                -1,
                getCppuBooleanType()
            ),
            ucb::CommandInfo(
                OUString( "insert" ),
                -1,
                getCppuVoidType()
            ),
            ucb::CommandInfo(
                OUString( "open" ),
                -1,
                getCppuType(
                    static_cast< ucb::OpenCommandArgument2 * >( 0 ) )
            )

            // New commands

        };

        return MAKECMDSEQUENCE( aStreamCommandInfoTable );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
