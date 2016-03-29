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

 **************************************************************************

    Props/Commands:

                        root  document folder  folder  stream  stream
                                                (new)          (new)
    ----------------------------------------------------------------
    ContentType         r       r       r       r       r       r
    IsDocument          r       r       r       r       r       r
    IsFolder            r       r       r       r       r       r
    Title               r       r       w       w       w       w
    CreatableContentsInfo r     r       r       r       r       r
    Storage             -       -       r       r       -       -
    DocumentModel       -       r       -       -       -       -

    getCommandInfo      x       x       x       x       x       x
    getPropertySetInfo  x       x       x       x       x       x
    getPropertyValues   x       x       x       x       x       x
    setPropertyValues   x       x       x       x       x       x
    insert              -       -       x       x       x(*)    x(*)
    delete              -       -       x       -       x       -
    open                x       x       x       -       x       -
    transfer            -       x       x       -       -       -
    createNewContent    -       x       x       -       -       -

#ifdef NO_STREAM_CREATION_WITHIN_DOCUMENT_ROOT
 (*) not supported by streams that are direct children of document
#endif

 *************************************************************************/

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/ucb/CommandInfo.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <osl/diagnose.h>
#include <sal/macros.h>
#include <tdoc_content.hxx>

namespace com { namespace sun { namespace star { namespace embed {
    class XStorage;
} } } }

using namespace com::sun::star;
using namespace tdoc_ucp;


// Content implementation.


#define MAKEPROPSEQUENCE( a ) \
    uno::Sequence< beans::Property >( a, SAL_N_ELEMENTS(a) )

#define MAKECMDSEQUENCE( a ) \
    uno::Sequence< ucb::CommandInfo >( a, SAL_N_ELEMENTS(a) )


// IMPORTANT: If any property data ( name / type / ... ) are changed, then
//            Content::getPropertyValues(...) must be adapted too!


// virtual
uno::Sequence< beans::Property > Content::getProperties(
            const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_aProps.getType() == STREAM )
    {


        // Stream: Supported properties


        static const beans::Property aStreamPropertyInfoTable[] =
        {

            // Mandatory properties

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
                OUString(
                    "CreatableContentsInfo" ),
                -1,
                cppu::UnoType<uno::Sequence< ucb::ContentInfo >>::get(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            )

            // New properties

        };
        return MAKEPROPSEQUENCE( aStreamPropertyInfoTable );
    }
    else if ( m_aProps.getType() == FOLDER )
    {


        // Folder: Supported properties


        static const beans::Property aFolderPropertyInfoTable[] =
        {

            // Mandatory properties

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
                OUString(
                    "CreatableContentsInfo" ),
                -1,
                cppu::UnoType<uno::Sequence< ucb::ContentInfo >>::get(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),

            // New properties

            beans::Property(
                OUString( "Storage" ),
                -1,
                cppu::UnoType<embed::XStorage>::get(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            )
        };
        return MAKEPROPSEQUENCE( aFolderPropertyInfoTable );
    }
    else if ( m_aProps.getType() == DOCUMENT )
    {


        // Document: Supported properties


        static const beans::Property aDocPropertyInfoTable[] =
        {

            // Mandatory properties

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
                OUString(
                    "CreatableContentsInfo" ),
                -1,
                cppu::UnoType<uno::Sequence< ucb::ContentInfo >>::get(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),

            // New properties

            beans::Property(
                OUString( "DocumentModel" ),
                -1,
                cppu::UnoType<frame::XModel>::get(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            )
        };
        return MAKEPROPSEQUENCE( aDocPropertyInfoTable );
    }
    else
    {


        // Root: Supported properties


        OSL_ENSURE( m_aProps.getType() == ROOT, "Wrong content type!" );

        static const beans::Property aRootPropertyInfoTable[] =
        {

            // Mandatory properties

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
                OUString(
                    "CreatableContentsInfo" ),
                -1,
                cppu::UnoType<uno::Sequence< ucb::ContentInfo >>::get(),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            )

            // New properties

        };
        return MAKEPROPSEQUENCE( aRootPropertyInfoTable );
    }
}


// virtual
uno::Sequence< ucb::CommandInfo > Content::getCommands(
            const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_aProps.getType() == STREAM )
    {
#ifdef NO_STREAM_CREATION_WITHIN_DOCUMENT_ROOT
        Uri aUri( m_xIdentifier->getContentIdentifier() );
        Uri aParentUri( aUri.getParentUri() );

        if ( aParentUri.isDocument() )
        {


            // Stream, that is a child of a document: Supported commands


            static const ucb::CommandInfo aStreamCommandInfoTable1[] =
            {

                // Mandatory commands

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
                    OUString( "open" ),
                    -1,
                    cppu::UnoType<ucb::OpenCommandArgument2>::get()
                )

                // New commands

            };
            return MAKECMDSEQUENCE( aStreamCommandInfoTable1 );
        }
#endif


        // Stream: Supported commands


        static const ucb::CommandInfo aStreamCommandInfoTable[] =
        {

            // Mandatory commands

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
                cppu::UnoType< uno::Sequence< beans::Property >>::get()
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
    else if ( m_aProps.getType() == FOLDER )
    {


        // Folder: Supported commands


        static const ucb::CommandInfo aFolderCommandInfoTable[] =
        {

            // Mandatory commands

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
            )

            // New commands

        };
        return MAKECMDSEQUENCE( aFolderCommandInfoTable );
    }
    else if ( m_aProps.getType() == DOCUMENT )
    {


        // Document: Supported commands


        static const ucb::CommandInfo aDocCommandInfoTable[] =
        {

            // Mandatory commands

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
            )

            // New commands

        };
        return MAKECMDSEQUENCE( aDocCommandInfoTable );
    }
    else
    {


        // Root: Supported commands


        OSL_ENSURE( m_aProps.getType() == ROOT, "Wrong content type!" );

        static const ucb::CommandInfo aRootCommandInfoTable[] =
        {

            // Mandatory commands

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
            )

            // New commands

        };
        return MAKECMDSEQUENCE( aRootCommandInfoTable );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
