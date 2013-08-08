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

#include <osl/diagnose.h>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/ucb/CrossReference.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/ucb/DocumentHeaderField.hpp>
#include <com/sun/star/ucb/RecipientInfo.hpp>
#include <com/sun/star/ucb/RuleSet.hpp>
#include <com/sun/star/ucb/SendInfo.hpp>
#include <com/sun/star/ucb/SendMediaTypes.hpp>
#include <com/sun/star/ucb/XDataContainer.hpp>
#include "ucbprops.hxx"

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;


//=========================================================================
//
// struct PropertyTableEntry
//
//=========================================================================

struct PropertyTableEntry
{
    const char* pName;
    sal_Int32   nHandle;
    sal_Int16   nAttributes;
    const com::sun::star::uno::Type& (*pGetCppuType)();
};

//////////////////////////////////////////////////////////////////////////
//
// CPPU type access functions.
//
//////////////////////////////////////////////////////////////////////////

static const com::sun::star::uno::Type& OUString_getCppuType()
{
    return getCppuType( static_cast< const OUString * >( 0 ) );
}

static const com::sun::star::uno::Type& sal_uInt16_getCppuType()
{
    // ! uInt -> Int, because of Java !!!
    return getCppuType( static_cast< const sal_Int16 * >( 0 ) );
}

static const com::sun::star::uno::Type& sal_uInt32_getCppuType()
{
    // ! uInt -> Int, because of Java !!!
    return getCppuType( static_cast< const sal_Int32 * >( 0 ) );
}

static const com::sun::star::uno::Type& sal_uInt64_getCppuType()
{
    // ! uInt -> Int, because of Java !!!
    return getCppuType( static_cast< const sal_Int64 * >( 0 ) );
}

static const com::sun::star::uno::Type& enum_getCppuType()
{
    // ! enum -> Int, because of Java !!!
    return getCppuType( static_cast< const sal_Int16 * >( 0 ) );
}

static const com::sun::star::uno::Type& sal_Bool_getCppuType()
{
    return getCppuBooleanType();
}

static const com::sun::star::uno::Type& byte_getCppuType()
{
    return getCppuType( static_cast< const sal_Int8 * >( 0 ) );
}

static const com::sun::star::uno::Type& Sequence_CrossReference_getCppuType()
{
    return getCppuType(
        static_cast< com::sun::star::uno::Sequence<
                        com::sun::star::ucb::CrossReference > * >( 0 ) );
}

static const com::sun::star::uno::Type& DateTime_getCppuType()
{
    return getCppuType(
        static_cast< com::sun::star::util::DateTime * >( 0 ) );
}

static const com::sun::star::uno::Type& Sequence_byte_getCppuType()
{
    return getCppuType(
        static_cast< com::sun::star::uno::Sequence< sal_Int8 > * >( 0 ) );
}

static const com::sun::star::uno::Type& Sequence_DocumentHeaderField_getCppuType()
{
    return getCppuType(
        static_cast< com::sun::star::uno::Sequence<
                        com::sun::star::ucb::DocumentHeaderField > * >( 0 ) );
}

static const com::sun::star::uno::Type& XDataContainer_getCppuType()
{
    return getCppuType(
        static_cast< com::sun::star::uno::Reference<
            com::sun::star::ucb::XDataContainer > * >( 0 ) );
}

static const com::sun::star::uno::Type& Sequence_RecipientInfo_getCppuType()
{
    return getCppuType(
        static_cast< com::sun::star::uno::Sequence<
                        com::sun::star::ucb::RecipientInfo > * >( 0 ) );
}

static const com::sun::star::uno::Type& RuleSet_getCppuType()
{
    return getCppuType(
        static_cast< com::sun::star::ucb::RuleSet * >( 0 ) );
}

static const com::sun::star::uno::Type& Sequence_SendInfo_getCppuType()
{
    return getCppuType(
        static_cast< com::sun::star::uno::Sequence<
                        com::sun::star::ucb::SendInfo > * >( 0 ) );
}

static const com::sun::star::uno::Type& Sequence_SendMediaTypes_getCppuType()
{
    return getCppuType(
        static_cast< com::sun::star::uno::Sequence<
                        com::sun::star::ucb::SendMediaTypes > * >( 0 ) );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A table with all well-known UCB properties.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ATTR_DEFAULT ( PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID | PropertyAttribute::MAYBEDEFAULT )

static PropertyTableEntry __aPropertyTable[] =
{
    { "Account",            -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "AutoUpdateInterval", -1,                                 ATTR_DEFAULT,   &sal_uInt32_getCppuType },
    { "ConfirmEmpty",       -1,                                 ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "ConnectionLimit",    -1,                                 ATTR_DEFAULT,   &byte_getCppuType },
    { "ConnectionMode",     -1,                                 ATTR_DEFAULT,   &enum_getCppuType },
    { "ContentCountLimit",  -1,                                 ATTR_DEFAULT,   &sal_uInt16_getCppuType },
    { "ContentType",        -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "Cookie",             -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "CrossReferences",    -1,                                 ATTR_DEFAULT,   &Sequence_CrossReference_getCppuType },
    { "DateCreated",        -1,                                 ATTR_DEFAULT,   &DateTime_getCppuType },
    { "DateModified",       -1,                                 ATTR_DEFAULT,   &DateTime_getCppuType },
    { "DeleteOnServer",     -1,                                 ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "DocumentBody",       -1,                                 ATTR_DEFAULT,   &Sequence_byte_getCppuType },
    { "DocumentCount",      -1,                                 ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_uInt32_getCppuType },
    { "DocumentCountMarked",
                            -1,                                 ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_uInt32_getCppuType },
    { "DocumentHeader",     -1,                                 ATTR_DEFAULT,   &Sequence_DocumentHeaderField_getCppuType },
    { "DocumentStoreMode",  -1,                                 ATTR_DEFAULT,   &enum_getCppuType },
    { "DocumentViewMode",   -1,                                 ATTR_DEFAULT,   &enum_getCppuType },
    { "FTPAccount",         -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "Flags",              -1,                                 ATTR_DEFAULT,   &sal_uInt32_getCppuType },
    { "FolderCount",        -1,                                 ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_uInt32_getCppuType },
    { "FolderViewMode",     -1,                                 ATTR_DEFAULT,   &enum_getCppuType },
    { "FreeSpace",          -1,                                 ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_uInt64_getCppuType },
    { "HasDocuments",       -1,                                 ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_Bool_getCppuType },
    { "HasFolders",         -1,                                 ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_Bool_getCppuType },
    { "IsAutoDelete",       -1,                                 ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "IsAutoUpdate",       -1,                                 ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "IsDocument",         -1,                                 ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_Bool_getCppuType },
    { "IsFolder",           -1,                                 ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_Bool_getCppuType },
    { "IsKeepExpired",      -1,                                 ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "IsLimitedContentCount",
                            -1,                                 ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "IsMarked",           -1,                                 ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "IsRead",             -1,                                 ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "IsReadOnly",         -1,                                 ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "IsSubscribed",       -1,                                 ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "IsTimeLimitedStore", -1,                                 ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "Keywords",           -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "LocalBase",          -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "MessageBCC",         -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "MessageBody",        -1,                                 ATTR_DEFAULT,   &XDataContainer_getCppuType },
    { "MessageCC",          -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "MessageFrom",        -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "MessageId",          -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "MessageInReplyTo",   -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "MessageReplyTo",     -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "MessageTo",          -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "NewsGroups",         -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "NoCacheList",        -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "Origin",             -1,                                 ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &OUString_getCppuType },
    { "OutgoingMessageRecipients",
                            -1,                                 ATTR_DEFAULT,   &Sequence_RecipientInfo_getCppuType },
    { "OutgoingMessageState",
                            -1,                                 ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &enum_getCppuType },
    { "OutgoingMessageViewMode",
                            -1,                                 ATTR_DEFAULT,   &enum_getCppuType },
    { "Password",           -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "Priority",           -1,                                 ATTR_DEFAULT,   &enum_getCppuType },
    { "References",         -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "Referer",            -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "Rules",              -1,                                 ATTR_DEFAULT,   &RuleSet_getCppuType },
    { "SearchCriteria",     -1,                                 ATTR_DEFAULT,   &RuleSet_getCppuType },
    { "SearchIndirections", -1,                                 ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "SearchLocations",    -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "SearchRecursive",    -1,                                 ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "SeenCount",          -1,                                 ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_uInt32_getCppuType },
    { "SendCopyTarget",     -1,                                 ATTR_DEFAULT,   &Sequence_SendInfo_getCppuType },
    { "SendFormats",        -1,                                 ATTR_DEFAULT,   &Sequence_SendMediaTypes_getCppuType },
    { "SendFroms",          -1,                                 ATTR_DEFAULT,   &Sequence_SendInfo_getCppuType },
    { "SendPasswords",      -1,                                 ATTR_DEFAULT,   &Sequence_SendInfo_getCppuType },
    { "SendProtocolPrivate",-1,                                 ATTR_DEFAULT,   &sal_uInt16_getCppuType },
    { "SendProtocolPublic", -1,                                 ATTR_DEFAULT,   &sal_uInt16_getCppuType },
    { "SendReplyTos",       -1,                                 ATTR_DEFAULT,   &Sequence_SendInfo_getCppuType },
    { "SendServerNames",    -1,                                 ATTR_DEFAULT,   &Sequence_SendInfo_getCppuType },
    { "SendUserNames",      -1,                                 ATTR_DEFAULT,   &Sequence_SendInfo_getCppuType },
    { "SendVIMPostOfficePath",
                            -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "ServerBase",         -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "ServerName",         -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "ServerPort",         -1,                                 ATTR_DEFAULT,   &sal_uInt16_getCppuType },
    { "Size",               -1,                                 ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_uInt64_getCppuType },
    { "SizeLimit",          -1,                                 ATTR_DEFAULT,   &sal_uInt64_getCppuType },
    { "SubscribedCount",    -1,                                 ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_uInt32_getCppuType },
    { "SynchronizePolicy",  -1,                                 ATTR_DEFAULT,   &enum_getCppuType },
    { "TargetFrames",       -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "TargetURL",          -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "TimeLimitStore",     -1,                                 ATTR_DEFAULT,   &sal_uInt16_getCppuType },
    { "Title",              -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "UpdateOnOpen",       -1,                                 ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "UseOutBoxPrivateProtocolSettings",
                            -1,                                 ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "UseOutBoxPublicProtocolSettings",
                            -1,                                 ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "UserName",           -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "UserSortCriterium",  -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "VIMPostOfficePath",  -1,                                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "VerificationMode",   -1,                                 ATTR_DEFAULT,   &enum_getCppuType },

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // EOT.
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    { 0, 0, 0, 0 }
};

//=========================================================================
//=========================================================================
//
// UcbPropertiesManager Implementation.
//
//=========================================================================
//=========================================================================

UcbPropertiesManager::UcbPropertiesManager(
                        const Reference< XMultiServiceFactory >& )
: m_pProps( 0 )
{
}

//=========================================================================
// virtual
UcbPropertiesManager::~UcbPropertiesManager()
{
    delete m_pProps;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_3( UcbPropertiesManager,
                   XTypeProvider,
                   XServiceInfo,
                   XPropertySetInfo );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_3( UcbPropertiesManager,
                          XTypeProvider,
                      XServiceInfo,
                          XPropertySetInfo );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_IMPL_1( UcbPropertiesManager,
                     OUString( "com.sun.star.comp.ucb.UcbPropertiesManager" ),
                     OUString( PROPERTIES_MANAGER_SERVICE_NAME ) );

//=========================================================================
//
// Service factory implementation.
//
//=========================================================================

ONE_INSTANCE_SERVICE_FACTORY_IMPL( UcbPropertiesManager );

//=========================================================================
//
// XPropertySetInfo methods.
//
//=========================================================================

// virtual
Sequence< Property > SAL_CALL UcbPropertiesManager::getProperties()
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_pProps )
    {
        m_pProps = new Sequence< Property >( 128 );
        Property* pProps = m_pProps->getArray();
        sal_Int32 nPos  = 0;
        sal_Int32 nSize = m_pProps->getLength();

        //////////////////////////////////////////////////////////////////
        // Get info for well-known properties.
        //////////////////////////////////////////////////////////////////

        const PropertyTableEntry* pCurr = &__aPropertyTable[ 0 ];
        while ( pCurr->pName )
        {
            if ( nSize <= nPos )
            {
                OSL_FAIL( "UcbPropertiesManager::getProperties - "
                            "Initial size of property sequence too small!" );

                m_pProps->realloc( 128 );
                nSize += 128;
            }

            Property& rProp = pProps[ nPos ];

            rProp.Name       = OUString::createFromAscii( pCurr->pName );
            rProp.Handle     = pCurr->nHandle;
            rProp.Type       = pCurr->pGetCppuType();
            rProp.Attributes = pCurr->nAttributes;

            nPos++;
            pCurr++;
        }

        if ( nPos > 0 )
        {
            m_pProps->realloc( nPos );
            nSize = m_pProps->getLength();
        }
    }
    return *m_pProps;
}

//=========================================================================
// virtual
Property SAL_CALL UcbPropertiesManager::getPropertyByName( const OUString& aName )
    throw( UnknownPropertyException, RuntimeException )
{
    Property aProp;
    if ( queryProperty( aName, aProp ) )
        return aProp;

    throw UnknownPropertyException();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL UcbPropertiesManager::hasPropertyByName( const OUString& Name )
    throw( RuntimeException )
{
    Property aProp;
    return queryProperty( Name, aProp );
}

//=========================================================================
//
// Non-Interface methods.
//
//=========================================================================

sal_Bool UcbPropertiesManager::queryProperty(
                                const OUString& rName, Property& rProp )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    getProperties();

    const Property* pProps = m_pProps->getConstArray();
    sal_Int32 nCount = m_pProps->getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const Property& rCurrProp = pProps[ n ];
        if ( rCurrProp.Name == rName )
        {
            rProp = rCurrProp;
            return sal_True;
        }
    }

    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
