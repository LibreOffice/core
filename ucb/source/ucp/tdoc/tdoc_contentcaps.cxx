/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <sal/macros.h>
#include <tdoc_content.hxx>

namespace com { namespace sun { namespace star { namespace embed {
    class XStorage;
} } } }

using namespace com::sun::star;
using namespace tdoc_ucp;


//

//


#define MAKEPROPSEQUENCE( a ) \
    uno::Sequence< beans::Property >( a, (sizeof (a) / sizeof (a[0])) )

#define MAKECMDSEQUENCE( a ) \
    uno::Sequence< ucb::CommandInfo >( a, (sizeof (a) / sizeof (a[0])) )


//


//



uno::Sequence< beans::Property > Content::getProperties(
            const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_aProps.getType() == STREAM )
    {
        
        //
        
        //
        

        static const beans::Property aStreamPropertyInfoTable[] =
        {
            
            
            
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
            
            
            
            beans::Property(
                OUString(
                    "CreatableContentsInfo" ),
                -1,
                getCppuType( static_cast<
                    const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            )
            
            
            
        };
        return MAKEPROPSEQUENCE( aStreamPropertyInfoTable );
    }
    else if ( m_aProps.getType() == FOLDER )
    {
        
        //
        
        //
        

        static const beans::Property aFolderPropertyInfoTable[] =
        {
            
            
            
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
            
            
            
            beans::Property(
                OUString(
                    "CreatableContentsInfo" ),
                -1,
                getCppuType( static_cast<
                    const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            
            
            
            beans::Property(
                OUString( "Storage" ),
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
        
        //
        
        //
        

        static const beans::Property aDocPropertyInfoTable[] =
        {
            
            
            
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
            
            
            
            beans::Property(
                OUString(
                    "CreatableContentsInfo" ),
                -1,
                getCppuType( static_cast<
                    const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            ),
            
            
            
            beans::Property(
                OUString( "DocumentModel" ),
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
        
        //
        
        //
        

        OSL_ENSURE( m_aProps.getType() == ROOT, "Wrong content type!" );

        static const beans::Property aRootPropertyInfoTable[] =
        {
            
            
            
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
            
            
            
            beans::Property(
                OUString(
                    "CreatableContentsInfo" ),
                -1,
                getCppuType( static_cast<
                    const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY
            )
            
            
            
        };
        return MAKEPROPSEQUENCE( aRootPropertyInfoTable );
    }
}



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
            
            //
            
            //
            

            static const ucb::CommandInfo aStreamCommandInfoTable1[] =
            {
                
                
                
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
                
                
                
                ucb::CommandInfo(
                    OUString( "delete" ),
                    -1,
                    getCppuBooleanType()
                ),
                ucb::CommandInfo(
                    OUString( "open" ),
                    -1,
                    getCppuType(
                        static_cast< ucb::OpenCommandArgument2 * >( 0 ) )
                )
                
                
                
            };
            return MAKECMDSEQUENCE( aStreamCommandInfoTable1 );
        }
#endif
        
        //
        
        //
        

        static const ucb::CommandInfo aStreamCommandInfoTable[] =
        {
            
            
            
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
            
            
            
        };
        return MAKECMDSEQUENCE( aStreamCommandInfoTable );
    }
    else if ( m_aProps.getType() == FOLDER )
    {
        
        //
        
        //
        

        static const ucb::CommandInfo aFolderCommandInfoTable[] =
        {
            
            
            
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
                getCppuType( static_cast< ucb::TransferInfo * >( 0 ) )
            ),
            ucb::CommandInfo(
                OUString( "createNewContent" ),
                -1,
                getCppuType( static_cast< ucb::ContentInfo * >( 0 ) )
            )
            
            
            
        };
        return MAKECMDSEQUENCE( aFolderCommandInfoTable );
    }
    else if ( m_aProps.getType() == DOCUMENT )
    {
        
        //
        
        //
        

        static const ucb::CommandInfo aDocCommandInfoTable[] =
        {
            
            
            
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
            
            
            
            ucb::CommandInfo(
                OUString( "open" ),
                -1,
                getCppuType(
                    static_cast< ucb::OpenCommandArgument2 * >( 0 ) )
            ),
            ucb::CommandInfo(
                OUString( "transfer" ),
                -1,
                getCppuType( static_cast< ucb::TransferInfo * >( 0 ) )
            ),
            ucb::CommandInfo(
                OUString( "createNewContent" ),
                -1,
                getCppuType( static_cast< ucb::ContentInfo * >( 0 ) )
            )
            
            
            
        };
        return MAKECMDSEQUENCE( aDocCommandInfoTable );
    }
    else
    {
        
        //
        
        //
        

        OSL_ENSURE( m_aProps.getType() == ROOT, "Wrong content type!" );

        static const ucb::CommandInfo aRootCommandInfoTable[] =
        {
            
            
            
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
            
            
            
            ucb::CommandInfo(
                OUString( "open" ),
                -1,
                getCppuType(
                    static_cast< ucb::OpenCommandArgument2 * >( 0 ) )
            )
            
            
            
        };
        return MAKECMDSEQUENCE( aRootCommandInfoTable );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
