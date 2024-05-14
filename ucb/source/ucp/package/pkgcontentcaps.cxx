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
                    u"ContentType"_ustr,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    u"IsDocument"_ustr,
                    -1,
                    cppu::UnoType<bool>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    u"IsFolder"_ustr,
                    -1,
                    cppu::UnoType<bool>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    u"Title"_ustr,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),

                // Optional standard properties

                beans::Property(
                    u"MediaType"_ustr,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                ),
                beans::Property(
                    u"CreatableContentsInfo"_ustr,
                    -1,
                    cppu::UnoType<uno::Sequence< ucb::ContentInfo >>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),

                // New properties

                beans::Property(
                    u"HasEncryptedEntries"_ustr,
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
                    u"ContentType"_ustr,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    u"IsDocument"_ustr,
                    -1,
                    cppu::UnoType<bool>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    u"IsFolder"_ustr,
                    -1,
                    cppu::UnoType<bool>::get(),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY
                ),
                beans::Property(
                    u"Title"_ustr,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                ),

                // Optional standard properties

                beans::Property(
                    u"MediaType"_ustr,
                    -1,
                    cppu::UnoType<OUString>::get(),
                    beans::PropertyAttribute::BOUND
                ),
                beans::Property(
                    u"CreatableContentsInfo"_ustr,
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
                u"ContentType"_ustr,
                -1,
                cppu::UnoType<OUString>::get(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                u"IsDocument"_ustr,
                -1,
                cppu::UnoType<bool>::get(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                u"IsFolder"_ustr,
                -1,
                cppu::UnoType<bool>::get(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                u"Title"_ustr,
                -1,
                cppu::UnoType<OUString>::get(),
                beans::PropertyAttribute::BOUND
            ),

            // Optional standard properties

            beans::Property(
                u"MediaType"_ustr,
                -1,
                cppu::UnoType<OUString>::get(),
                beans::PropertyAttribute::BOUND
            ),
            beans::Property(
                u"Size"_ustr,
                -1,
                cppu::UnoType<sal_Int64>::get(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            beans::Property(
                u"CreatableContentsInfo"_ustr,
                -1,
                cppu::UnoType<uno::Sequence< ucb::ContentInfo >>::get(),
                beans::PropertyAttribute::BOUND
                | beans::PropertyAttribute::READONLY
            ),

            // New properties

            beans::Property(
                u"Compressed"_ustr,
                -1,
                cppu::UnoType<bool>::get(),
                beans::PropertyAttribute::BOUND
            ),
            beans::Property(
                u"Encrypted"_ustr,
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
                    u"getCommandInfo"_ustr,
                    -1,
                    cppu::UnoType<void>::get()
                ),
                ucb::CommandInfo(
                    u"getPropertySetInfo"_ustr,
                    -1,
                    cppu::UnoType<void>::get()
                ),
                ucb::CommandInfo(
                    u"getPropertyValues"_ustr,
                    -1,
                    cppu::UnoType<uno::Sequence< beans::Property >>::get()
                ),
                ucb::CommandInfo(
                    u"setPropertyValues"_ustr,
                    -1,
                    cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get()
                ),

                // Optional standard commands

                ucb::CommandInfo(
                    u"open"_ustr,
                    -1,
                    cppu::UnoType<ucb::OpenCommandArgument2>::get()
                ),
                ucb::CommandInfo(
                    u"transfer"_ustr,
                    -1,
                    cppu::UnoType<ucb::TransferInfo>::get()
                ),
                ucb::CommandInfo(
                    u"createNewContent"_ustr,
                    -1,
                    cppu::UnoType<ucb::ContentInfo>::get()
                ),

                // New commands

                ucb::CommandInfo(
                    u"flush"_ustr,
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
                    u"getCommandInfo"_ustr,
                    -1,
                    cppu::UnoType<void>::get()
                ),
                ucb::CommandInfo(
                    u"getPropertySetInfo"_ustr,
                    -1,
                    cppu::UnoType<void>::get()
                ),
                ucb::CommandInfo(
                    u"getPropertyValues"_ustr,
                    -1,
                    cppu::UnoType<uno::Sequence< beans::Property >>::get()
                ),
                ucb::CommandInfo(
                    u"setPropertyValues"_ustr,
                    -1,
                    cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get()
                ),

                // Optional standard commands

                ucb::CommandInfo(
                    u"delete"_ustr,
                    -1,
                    cppu::UnoType<bool>::get()
                ),
                ucb::CommandInfo(
                    u"insert"_ustr,
                    -1,
                    cppu::UnoType<void>::get()
                ),
                ucb::CommandInfo(
                    u"open"_ustr,
                    -1,
                    cppu::UnoType<ucb::OpenCommandArgument2>::get()
                ),
                ucb::CommandInfo(
                    u"transfer"_ustr,
                    -1,
                    cppu::UnoType<ucb::TransferInfo>::get()
                ),
                ucb::CommandInfo(
                    u"createNewContent"_ustr,
                    -1,
                    cppu::UnoType<ucb::ContentInfo>::get()
                ),

                // New commands

                ucb::CommandInfo(
                    u"flush"_ustr,
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
                u"getCommandInfo"_ustr,
                -1,
                cppu::UnoType<void>::get()
            ),
            ucb::CommandInfo(
                u"getPropertySetInfo"_ustr,
                -1,
                cppu::UnoType<void>::get()
            ),
            ucb::CommandInfo(
                u"getPropertyValues"_ustr,
                -1,
                cppu::UnoType<uno::Sequence< beans::Property >>::get()
            ),
            ucb::CommandInfo(
                u"setPropertyValues"_ustr,
                -1,
                cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get()
            ),

            // Optional standard commands

            ucb::CommandInfo(
                u"delete"_ustr,
                -1,
                cppu::UnoType<bool>::get()
            ),
            ucb::CommandInfo(
                u"insert"_ustr,
                -1,
                cppu::UnoType<void>::get()
            ),
            ucb::CommandInfo(
                u"open"_ustr,
                -1,
                cppu::UnoType<ucb::OpenCommandArgument2>::get()
            )

            // New commands

        };

        return MAKECMDSEQUENCE( aStreamCommandInfoTable );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
