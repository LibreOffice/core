/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

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
#include <sal/macros.h>
#include <tdoc_content.hxx>

namespace com { namespace sun { namespace star { namespace embed {
    class XStorage;
} } } }

using namespace com::sun::star;
using namespace tdoc_ucp;

//=========================================================================
//
// Content implementation.
//
//=========================================================================

#define MAKEPROPSEQUENCE( a ) \
    uno::Sequence< beans::Property >( a, SAL_N_ELEMENTS( a ) )

#define MAKECMDSEQUENCE( a ) \
    uno::Sequence< ucb::CommandInfo >( a, SAL_N_ELEMENTS( a ) )

//=========================================================================
//
// IMPORTENT: If any property data ( name / type / ... ) are changed, then
//            Content::getPropertyValues(...) must be adapted too!
//
//=========================================================================

// virtual
uno::Sequence< beans::Property > Content::getProperties(
            const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_aProps.getType() == STREAM )
    {
        //=================================================================
        //
        // Stream: Supported properties
        //
        //=================================================================

        static const beans::Property aStreamPropertyInfoTable[] =
        {
            ///////////////////////////////////////////////////////////
            // Mandatory properties
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
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "CreatableContentsInfo" ) ),
                -1,
                getCppuType( static_cast<
                    const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            )
            ///////////////////////////////////////////////////////////
            // New properties
            ///////////////////////////////////////////////////////////
        };
        return MAKEPROPSEQUENCE( aStreamPropertyInfoTable );
    }
    else if ( m_aProps.getType() == FOLDER )
    {
        //=================================================================
        //
        // Folder: Supported properties
        //
        //=================================================================

        static const beans::Property aFolderPropertyInfoTable[] =
        {
            ///////////////////////////////////////////////////////////
            // Mandatory properties
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
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "CreatableContentsInfo" ) ),
                -1,
                getCppuType( static_cast<
                    const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            ///////////////////////////////////////////////////////////
            // New properties
            ///////////////////////////////////////////////////////////
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Storage" ) ),
                -1,
                getCppuType( static_cast<
                    const uno::Reference< embed::XStorage > * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            )
        };
        return MAKEPROPSEQUENCE( aFolderPropertyInfoTable );
    }
    else if ( m_aProps.getType() == DOCUMENT )
    {
        //=================================================================
        //
        // Document: Supported properties
        //
        //=================================================================

        static const beans::Property aDocPropertyInfoTable[] =
        {
            ///////////////////////////////////////////////////////////
            // Mandatory properties
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
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "CreatableContentsInfo" ) ),
                -1,
                getCppuType( static_cast<
                    const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            ///////////////////////////////////////////////////////////
            // New properties
            ///////////////////////////////////////////////////////////
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DocumentModel" ) ),
                -1,
                getCppuType( static_cast<
                    const uno::Reference< frame::XModel > * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            )
        };
        return MAKEPROPSEQUENCE( aDocPropertyInfoTable );
    }
    else
    {
        //=================================================================
        //
        // Root: Supported properties
        //
        //=================================================================

        OSL_ENSURE( m_aProps.getType() == ROOT, "Wrong content type!" );

        static const beans::Property aRootPropertyInfoTable[] =
        {
            ///////////////////////////////////////////////////////////////
            // Mandatory properties
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
            ),
            ///////////////////////////////////////////////////////////////
            // Optional standard properties
            ///////////////////////////////////////////////////////////////
            beans::Property(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "CreatableContentsInfo" ) ),
                -1,
                getCppuType( static_cast<
                    const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            )
            ///////////////////////////////////////////////////////////////
            // New properties
            ///////////////////////////////////////////////////////////////
        };
        return MAKEPROPSEQUENCE( aRootPropertyInfoTable );
    }
}

//=========================================================================
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
            //=================================================================
            //
            // Stream, that is a child of a document: Supported commands
            //
            //=================================================================

            static const ucb::CommandInfo aStreamCommandInfoTable1[] =
            {
                ///////////////////////////////////////////////////////////
                // Mandatory commands
                ///////////////////////////////////////////////////////////
                ucb::CommandInfo(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
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
                        static_cast<
                            uno::Sequence< beans::PropertyValue > * >( 0 ) )
                ),
                ///////////////////////////////////////////////////////////
                // Optional standard commands
                ///////////////////////////////////////////////////////////
                ucb::CommandInfo(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
                    -1,
                    getCppuBooleanType()
                ),
                ucb::CommandInfo(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                    -1,
                    getCppuType(
                        static_cast< ucb::OpenCommandArgument2 * >( 0 ) )
                )
                ///////////////////////////////////////////////////////////
                // New commands
                ///////////////////////////////////////////////////////////
            };
            return MAKECMDSEQUENCE( aStreamCommandInfoTable1 );
        }
#endif
        //=================================================================
        //
        // Stream: Supported commands
        //
        //=================================================================

        static const ucb::CommandInfo aStreamCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////
            // Mandatory commands
            ///////////////////////////////////////////////////////////
            ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
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
                    static_cast<
                        uno::Sequence< beans::PropertyValue > * >( 0 ) )
            ),
            ///////////////////////////////////////////////////////////
            // Optional standard commands
            ///////////////////////////////////////////////////////////
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
                -1,
                getCppuBooleanType()
            ),
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
                -1,
                getCppuVoidType()
            ),
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                -1,
                getCppuType(
                    static_cast< ucb::OpenCommandArgument2 * >( 0 ) )
            )
            ///////////////////////////////////////////////////////////
            // New commands
            ///////////////////////////////////////////////////////////
        };
        return MAKECMDSEQUENCE( aStreamCommandInfoTable );
    }
    else if ( m_aProps.getType() == FOLDER )
    {
        //=================================================================
        //
        // Folder: Supported commands
        //
        //=================================================================

        static const ucb::CommandInfo aFolderCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////
            // Mandatory commands
            ///////////////////////////////////////////////////////////
            ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
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
                    static_cast<
                        uno::Sequence< beans::PropertyValue > * >( 0 ) )
            ),
            ///////////////////////////////////////////////////////////
            // Optional standard commands
            ///////////////////////////////////////////////////////////
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
                -1,
                getCppuBooleanType()
            ),
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
                -1,
                getCppuVoidType()
            ),
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                -1,
                getCppuType(
                    static_cast< ucb::OpenCommandArgument2 * >( 0 ) )
            ),
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "transfer" ) ),
                -1,
                getCppuType( static_cast< ucb::TransferInfo * >( 0 ) )
            ),
            ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "createNewContent" ) ),
                -1,
                getCppuType( static_cast< ucb::ContentInfo * >( 0 ) )
            )
            ///////////////////////////////////////////////////////////
            // New commands
            ///////////////////////////////////////////////////////////
        };
        return MAKECMDSEQUENCE( aFolderCommandInfoTable );
    }
    else if ( m_aProps.getType() == DOCUMENT )
    {
        //=================================================================
        //
        // Document: Supported commands
        //
        //=================================================================

        static const ucb::CommandInfo aDocCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////
            // Mandatory commands
            ///////////////////////////////////////////////////////////
            ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
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
                    static_cast<
                        uno::Sequence< beans::PropertyValue > * >( 0 ) )
            ),
            ///////////////////////////////////////////////////////////
            // Optional standard commands
            ///////////////////////////////////////////////////////////
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                -1,
                getCppuType(
                    static_cast< ucb::OpenCommandArgument2 * >( 0 ) )
            ),
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "transfer" ) ),
                -1,
                getCppuType( static_cast< ucb::TransferInfo * >( 0 ) )
            ),
            ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "createNewContent" ) ),
                -1,
                getCppuType( static_cast< ucb::ContentInfo * >( 0 ) )
            )
            ///////////////////////////////////////////////////////////
            // New commands
            ///////////////////////////////////////////////////////////
        };
        return MAKECMDSEQUENCE( aDocCommandInfoTable );
    }
    else
    {
        //=================================================================
        //
        // Root: Supported commands
        //
        //=================================================================

        OSL_ENSURE( m_aProps.getType() == ROOT, "Wrong content type!" );

        static const ucb::CommandInfo aRootCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////
            // Mandatory commands
            ///////////////////////////////////////////////////////////
            ucb::CommandInfo(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
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
                    static_cast<
                        uno::Sequence< beans::PropertyValue > * >( 0 ) )
            ),
            ///////////////////////////////////////////////////////////
            // Optional standard commands
            ///////////////////////////////////////////////////////////
            ucb::CommandInfo(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                -1,
                getCppuType(
                    static_cast< ucb::OpenCommandArgument2 * >( 0 ) )
            )
            ///////////////////////////////////////////////////////////
            // New commands
            ///////////////////////////////////////////////////////////
        };
        return MAKECMDSEQUENCE( aRootCommandInfoTable );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
