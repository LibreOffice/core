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

                           root    folder  folder  link    link
                                   (new)           (new)
    ----------------------------------------------------------------
    ContentType            x       x       x       x       x
    IsDocument             x       x       x       x       x
    IsFolder               x       x       x       x       x
    Title                  x       x       x       x       x
    TargetURL                                      x       x
    CreatableContentsInfo  x       x       x       x       x

    getCommandInfo         x       x       x       x       x
    getPropertySetInfo     x       x       x       x       x
    getPropertyValues      x       x       x       x       x
    setPropertyValues      x       x       x       x       x
    createNewContent       x               x
    insert                                 x               x
    delete                         x               x
    open                   x       x
    transfer               x       x

 *************************************************************************/

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ucb/CommandInfo.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <sal/macros.h>
#include "hierarchycontent.hxx"

using namespace com::sun::star;
using namespace hierarchy_ucp;


//

//


#define MAKEPROPSEQUENCE( a ) \
    uno::Sequence< beans::Property >( a, sizeof( a )  / sizeof( a[ 0 ] ) )

#define MAKECMDSEQUENCE( a ) \
    uno::Sequence< ucb::CommandInfo >( a, sizeof( a )  / sizeof( a[ 0 ] ) )


//


//



uno::Sequence< beans::Property > HierarchyContent::getProperties(
            const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_eKind == LINK )
    {
        
        //
        
        //
        

        if ( isReadOnly() )
        {
            static const beans::Property aLinkPropertyInfoTable[] =
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
                    OUString( "TargetURL" ),
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
                        | beans::PropertyAttribute::READONLY )
                
                
                
            };
            return MAKEPROPSEQUENCE( aLinkPropertyInfoTable );
        }
        else
        {
            static const beans::Property aLinkPropertyInfoTable[] =
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
                    OUString( "TargetURL" ),
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
                        | beans::PropertyAttribute::READONLY )
                
                
                
            };
            return MAKEPROPSEQUENCE( aLinkPropertyInfoTable );
        }
    }
    else if ( m_eKind == FOLDER )
    {
        
        //
        
        //
        

        if ( isReadOnly() )
        {
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
                        | beans::PropertyAttribute::READONLY
                ),
                
                
                
                beans::Property(
                    OUString(
                        "CreatableContentsInfo" ),
                    -1,
                    getCppuType( static_cast<
                        const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                    beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY )
                
                
                
            };
            return MAKEPROPSEQUENCE( aFolderPropertyInfoTable );
        }
        else
        {
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
                        | beans::PropertyAttribute::READONLY )
                
                
                
            };
            return MAKEPROPSEQUENCE( aFolderPropertyInfoTable );
        }
    }
    else
    {
        
        //
        
        //
        

        
        

        static const beans::Property aRootFolderPropertyInfoTable[] =
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
                        | beans::PropertyAttribute::READONLY )
            
            
            
        };
        return MAKEPROPSEQUENCE( aRootFolderPropertyInfoTable );
    }
}



uno::Sequence< ucb::CommandInfo > HierarchyContent::getCommands(
            const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_eKind == LINK )
    {
        
        //
        
        //
        

        if ( isReadOnly() )
        {
            static const ucb::CommandInfo aLinkCommandInfoTable[] =
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
                )
                
                
                

                
                
                
            };
            return MAKECMDSEQUENCE( aLinkCommandInfoTable );
        }
        else
        {
            static const ucb::CommandInfo aLinkCommandInfoTable[] =
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
                )
                
                
                
            };
            return MAKECMDSEQUENCE( aLinkCommandInfoTable );
        }
    }
    else if ( m_eKind == FOLDER )
    {
        
        //
        
        //
        

        if ( isReadOnly() )
        {
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
                    OUString( "open" ),
                    -1,
                    getCppuType(
                        static_cast< ucb::OpenCommandArgument2 * >( 0 ) )
                )
                
                
                
            };
            return MAKECMDSEQUENCE( aFolderCommandInfoTable );
        }
        else
        {
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
    }
    else
    {
        
        //
        
        //
        

        if ( isReadOnly() )
        {
            static const ucb::CommandInfo aRootFolderCommandInfoTable[] =
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
            return MAKECMDSEQUENCE( aRootFolderCommandInfoTable );
        }
        else
        {
            static const ucb::CommandInfo aRootFolderCommandInfoTable[] =
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
            return MAKECMDSEQUENCE( aRootFolderCommandInfoTable );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
