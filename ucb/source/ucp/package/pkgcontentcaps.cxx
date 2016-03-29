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
    uno::Sequence< beans::Property >( a, SAL_N_ELEMENTS( a ) )

#define MAKECMDSEQUENCE( a ) \
    uno::Sequence< ucb::CommandInfo >( a, SAL_N_ELEMENTS( a ) )


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
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    OUString( "IsDocument" ),
                    -1,
                    cppu::UnoType<bool>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    OUString( "IsFolder" ),
                    -1,
                    cppu::UnoType<bool>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    OUString( "Title" ),
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),

                // Optional standard properties

                beans::Property(
                    OUString( "MediaType" ),
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                ),
                beans::Property(
                    OUString(
                        "CreatableContentsInfo" ),
                    -1,
                    cppu::UnoType<uno::Sequence< ucb::ContentInfo >>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),

                // New properties

                beans::Property(
                    OUString( "HasEncryptedEntries" ),
                    -1,
                    cppu::UnoType<bool>::get(),
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
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    OUString( "IsDocument" ),
                    -1,
                    cppu::UnoType<bool>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    OUString( "IsFolder" ),
                    -1,
                    cppu::UnoType<bool>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    OUString( "Title" ),
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                ),

                // Optional standard properties

                beans::Property(
                    OUString( "MediaType" ),
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                ),
                beans::Property(
                    OUString(
                        "CreatableContentsInfo" ),
                    -1,
                    cppu::UnoType<uno::Sequence< ucb::ContentInfo >>::get(),
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
                cppu::UnoType<OUString>::get(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                OUString( "IsDocument" ),
                -1,
                cppu::UnoType<bool>::get(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                OUString( "IsFolder" ),
                -1,
                cppu::UnoType<bool>::get(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                OUString( "Title" ),
                -1,
                cppu::UnoType<OUString>::get(),
                beans::PropertyAttribute::BOUND
            ),

            // Optional standard properties

            beans::Property(
                OUString( "MediaType" ),
                -1,
                cppu::UnoType<OUString>::get(),
                beans::PropertyAttribute::BOUND
            ),
            beans::Property(
                OUString( "Size" ),
                -1,
                cppu::UnoType<sal_Int64>::get(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                OUString(
                    "CreatableContentsInfo" ),
                -1,
                cppu::UnoType<uno::Sequence< ucb::ContentInfo >>::get(),
                beans::PropertyAttribute::BOUND
                | beans::PropertyAttribute::READONLY
            ),

            // New properties

            beans::Property(
                OUString( "Compressed" ),
                -1,
                cppu::UnoType<bool>::get(),
                beans::PropertyAttribute::BOUND
            ),
            beans::Property(
                OUString( "Encrypted" ),
                -1,
                cppu::UnoType<bool>::get(),
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
                    cppu::UnoType<void>::get()
                ),
                ucb::CommandInfo(
                    OUString( "getPropertySetInfo" ),
                    -1,
                    cppu::UnoType<void>::get()
                ),
                ucb::CommandInfo(
                    OUString( "getPropertyValues" ),
                    -1,
                    cppu::UnoType<uno::Sequence< beans::Property >>::get()
                ),
                ucb::CommandInfo(
                    OUString( "setPropertyValues" ),
                    -1,
                    cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get()
                ),

                // Optional standard commands

                ucb::CommandInfo(
                    OUString( "open" ),
                    -1,
                    cppu::UnoType<ucb::OpenCommandArgument2>::get()
                ),
                ucb::CommandInfo(
                    OUString( "transfer" ),
                    -1,
                    cppu::UnoType<ucb::TransferInfo>::get()
                ),
                ucb::CommandInfo(
                    OUString( "createNewContent" ),
                    -1,
                    cppu::UnoType<ucb::ContentInfo>::get()
                ),

                // New commands

                ucb::CommandInfo(
                    OUString( "flush" ),
                    -1,
                    cppu::UnoType<void>::get()
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
                    cppu::UnoType<void>::get()
                ),
                ucb::CommandInfo(
                    OUString( "getPropertySetInfo" ),
                    -1,
                    cppu::UnoType<void>::get()
                ),
                ucb::CommandInfo(
                    OUString( "getPropertyValues" ),
                    -1,
                    cppu::UnoType<uno::Sequence< beans::Property >>::get()
                ),
                ucb::CommandInfo(
                    OUString( "setPropertyValues" ),
                    -1,
                    cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get()
                ),

                // Optional standard commands

                ucb::CommandInfo(
                    OUString( "delete" ),
                    -1,
                    cppu::UnoType<bool>::get()
                ),
                ucb::CommandInfo(
                    OUString( "insert" ),
                    -1,
                    cppu::UnoType<void>::get()
                ),
                ucb::CommandInfo(
                    OUString( "open" ),
                    -1,
                    cppu::UnoType<ucb::OpenCommandArgument2>::get()
                ),
                ucb::CommandInfo(
                    OUString( "transfer" ),
                    -1,
                    cppu::UnoType<ucb::TransferInfo>::get()
                ),
                ucb::CommandInfo(
                    OUString( "createNewContent" ),
                    -1,
                    cppu::UnoType<ucb::ContentInfo>::get()
                ),

                // New commands

                ucb::CommandInfo(
                    OUString( "flush" ),
                    -1,
                    cppu::UnoType<void>::get()
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
                cppu::UnoType<void>::get()
            ),
            ucb::CommandInfo(
                OUString( "getPropertySetInfo" ),
                -1,
                cppu::UnoType<void>::get()
            ),
            ucb::CommandInfo(
                OUString( "getPropertyValues" ),
                -1,
                cppu::UnoType<uno::Sequence< beans::Property >>::get()
            ),
            ucb::CommandInfo(
                OUString( "setPropertyValues" ),
                -1,
                cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get()
            ),

            // Optional standard commands

            ucb::CommandInfo(
                OUString( "delete" ),
                -1,
                cppu::UnoType<bool>::get()
            ),
            ucb::CommandInfo(
                OUString( "insert" ),
                -1,
                cppu::UnoType<void>::get()
            ),
            ucb::CommandInfo(
                OUString( "open" ),
                -1,
                cppu::UnoType<ucb::OpenCommandArgument2>::get()
            )

            // New commands

        };

        return MAKECMDSEQUENCE( aStreamCommandInfoTable );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
