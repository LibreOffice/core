/*************************************************************************
 *
 *  $RCSfile: ucbprops.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kso $ $Date: 2001-04-05 09:49:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CROSSREFERENCE_HPP_
#include <com/sun/star/ucb/CrossReference.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_DOCUMENTHEADERFIELD_HPP_
#include <com/sun/star/ucb/DocumentHeaderField.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_RECIPIENTINFO_HPP_
#include <com/sun/star/ucb/RecipientInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_RULESET_HPP_
#include <com/sun/star/ucb/RuleSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_SENDINFO_HPP_
#include <com/sun/star/ucb/SendInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_SENDMEDIATYPES_HPP_
#include <com/sun/star/ucb/SendMediaTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XDATACONTAINER_HPP_
#include <com/sun/star/ucb/XDataContainer.hpp>
#endif

#ifndef _UCBPROPS_HXX
#include "ucbprops.hxx"
#endif

using namespace rtl;
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
    return getCppuType( static_cast< const rtl::OUString * >( 0 ) );
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
    { "Account",            -1 /* WID_ACCOUNT */,               ATTR_DEFAULT,   &OUString_getCppuType },
    { "AutoUpdateInterval", -1 /* WID_AUTOUPDATE_INTERVAL */,   ATTR_DEFAULT,   &sal_uInt32_getCppuType },
    { "ConfirmEmpty",       -1 /* WID_TRASHCAN_FLAG_CONFIRMEMPTY */,
                                                                ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "ConnectionLimit",    -1 /* WID_HTTP_CONNECTION_LIMIT */, ATTR_DEFAULT,   &byte_getCppuType },
    { "ConnectionMode",     -1 /* WID_CONNECTION_MODE */,       ATTR_DEFAULT,   &enum_getCppuType },
    { "ContentCountLimit",  -1 /* WID_SHOW_MSGS_TIMELIMIT */,   ATTR_DEFAULT,   &sal_uInt16_getCppuType },
    { "ContentType",        -1 /* WID_CONTENT_TYPE */,          ATTR_DEFAULT,   &OUString_getCppuType },
    { "Cookie",             -1 /* WID_HTTP_COOKIE */,           ATTR_DEFAULT,   &OUString_getCppuType },
    { "CrossReferences",    -1 /* WID_NEWS_XREFLIST */,         ATTR_DEFAULT,   &Sequence_CrossReference_getCppuType },
    { "DateCreated",        -1 /* WID_DATE_CREATED */,          ATTR_DEFAULT,   &DateTime_getCppuType },
    { "DateModified",       -1 /* WID_DATE_MODIFIED */,         ATTR_DEFAULT,   &DateTime_getCppuType },
    { "DeleteOnServer",     -1 /* WID_DELETE_ON_SERVER */,      ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "DocumentBody",       -1 /* WID_DOCUMENT_BODY */,         ATTR_DEFAULT,   &Sequence_byte_getCppuType },
    { "DocumentCount",      -1 /* WID_TOTALCONTENTCOUNT */,     ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_uInt32_getCppuType },
    { "DocumentCountMarked",
                            -1 /* WID_MARKED_DOCUMENT_COUNT */, ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_uInt32_getCppuType },
    { "DocumentHeader",     -1 /* WID_DOCUMENT_HEADER */,       ATTR_DEFAULT,   &Sequence_DocumentHeaderField_getCppuType },
    { "DocumentStoreMode",  -1 /* WID_MESSAGE_STOREMODE */,     ATTR_DEFAULT,   &enum_getCppuType },
    { "DocumentViewMode",   -1 /* WID_MESSAGEVIEW_MODE */,      ATTR_DEFAULT,   &enum_getCppuType },
    { "FTPAccount",         -1 /* WID_FTP_ACCOUNT */,           ATTR_DEFAULT,   &OUString_getCppuType },
    { "Flags",              -1 /* WID_FSYS_FLAGS */,            ATTR_DEFAULT,   &sal_uInt32_getCppuType },
    { "FolderCount",        -1 /* WID_FOLDER_COUNT */,          ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_uInt32_getCppuType },
    { "FolderViewMode",     -1 /* WID_FOLDERVIEW_MODE */,       ATTR_DEFAULT,   &enum_getCppuType },
    { "FreeSpace",          -1 /* WID_FSYS_DISKSPACE_LEFT */,   ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_uInt64_getCppuType },
    { "HasDocuments",       -1 /* WID_FLAG_HAS_MESSAGES */,     ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_Bool_getCppuType },
    { "HasFolders",         -1 /* WID_FLAG_HAS_FOLDER */,       ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_Bool_getCppuType },
    { "IsAutoDelete",       -1 /* WID_TRASHCAN_FLAG_AUTODELETE */,
                                                                ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "IsAutoUpdate",       -1 /* WID_UPDATE_ENABLED */,        ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "IsDocument",         -1 /* WID_FLAG_IS_MESSAGE */,       ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_Bool_getCppuType },
    { "IsFolder",           -1 /* WID_FLAG_IS_FOLDER */,        ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_Bool_getCppuType },
    { "IsKeepExpired",      -1 /* WID_HTTP_KEEP_EXPIRED */,     ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "IsLimitedContentCount",
                            -1 /* WID_SHOW_MSGS_HAS_TIMELIMIT */,
                                                                ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "IsMarked",           -1 /* WID_IS_MARKED */,             ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "IsRead",             -1 /* WID_IS_READ */,               ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "IsReadOnly",         -1 /* WID_FLAG_READONLY */,         ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "IsSubscribed",       -1 /* WID_FLAG_SUBSCRIBED */,       ATTR_DEFAULT,   &sal_Bool_getCppuType },
//  { "IsThreaded",         -1 /* WID_THREADED */,              ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "IsTimeLimitedStore", -1 /* WID_STORE_MSGS_HAS_TIMELIMIT */,
                                                                ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "Keywords",           -1 /* WID_KEYWORDS */,              ATTR_DEFAULT,   &OUString_getCppuType },
    { "LocalBase",          -1 /* WID_LOCALBASE */,             ATTR_DEFAULT,   &OUString_getCppuType },
    { "MessageBCC",         -1 /* WID_BCC */,                   ATTR_DEFAULT,   &OUString_getCppuType },
    { "MessageBody",        -1 /* WID_MESSAGEBODY */,           ATTR_DEFAULT,   &XDataContainer_getCppuType },
    { "MessageCC",          -1 /* WID_CC */,                    ATTR_DEFAULT,   &OUString_getCppuType },
    { "MessageFrom",        -1 /* WID_FROM */,                  ATTR_DEFAULT,   &OUString_getCppuType },
    { "MessageId",          -1 /* WID_MESSAGE_ID */,            ATTR_DEFAULT,   &OUString_getCppuType },
    { "MessageInReplyTo",   -1 /* WID_IN_REPLY_TO */,           ATTR_DEFAULT,   &OUString_getCppuType },
    { "MessageReplyTo",     -1 /* WID_REPLY_TO */,              ATTR_DEFAULT,   &OUString_getCppuType },
    { "MessageTo",          -1 /* WID_TO */,                    ATTR_DEFAULT,   &OUString_getCppuType },
    { "NewsGroups",         -1 /* WID_NEWSGROUPS */,            ATTR_DEFAULT,   &OUString_getCppuType },
    { "NoCacheList",        -1 /* WID_HTTP_NOCACHE_LIST */,     ATTR_DEFAULT,   &OUString_getCppuType },
    { "Origin",             -1 /* WID_TRASH_ORIGIN */,          ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &OUString_getCppuType },
    { "OutgoingMessageRecipients",
                            -1 /* WID_RECIPIENTLIST */,         ATTR_DEFAULT,   &Sequence_RecipientInfo_getCppuType },
    { "OutgoingMessageState",
                            -1 /* WID_OUTMSGINTERNALSTATE */,   ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &enum_getCppuType },
    { "OutgoingMessageViewMode",
                            -1 /* WID_SENTMESSAGEVIEW_MODE */,
                                                                ATTR_DEFAULT,   &enum_getCppuType },
//  { "OwnURL",             -1 /* WID_OWN_URL */,               ATTR_DEFAULT,   &OUString_getCppuType },
    { "Password",           -1 /* WID_PASSWORD */,              ATTR_DEFAULT,   &OUString_getCppuType },
//  { "PresentationURL",    -1 /* WID_REAL_URL */,              ATTR_DEFAULT | PropertyAttribute::READONLY,
//                                                                                          &OUString_getCppuType },
    { "Priority",           -1 /* WID_PRIORITY */,              ATTR_DEFAULT,   &enum_getCppuType },
    { "References",         -1 /* WID_REFERENCES */,            ATTR_DEFAULT,   &OUString_getCppuType },
    { "Referer",            -1 /* WID_HTTP_REFERER */,          ATTR_DEFAULT,   &OUString_getCppuType },
    { "Rules",              -1 /* WID_RULES */,                 ATTR_DEFAULT,   &RuleSet_getCppuType },
    { "SearchCriteria",     -1 /* WID_SEARCH_CRITERIA */,       ATTR_DEFAULT,   &RuleSet_getCppuType },
    { "SearchIndirections", -1 /* WID_SEARCH_INDIRECTIONS */,   ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "SearchLocations",    -1 /* WID_SEARCH_LOCATIONS */,      ATTR_DEFAULT,   &OUString_getCppuType },
    { "SearchRecursive",    -1 /* WID_SEARCH_RECURSIVE */,      ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "SeenCount",          -1 /* WID_SEENCONTENTCOUNT */,      ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_uInt32_getCppuType },
    { "SendCopyTarget",     -1 /* WID_SEND_COPY_TARGET */,      ATTR_DEFAULT,   &Sequence_SendInfo_getCppuType },
    { "SendFormats",        -1 /* WID_SEND_FORMATS */,          ATTR_DEFAULT,   &Sequence_SendMediaTypes_getCppuType },
    { "SendFroms",          -1 /* WID_SEND_FROM_DEFAULT */,     ATTR_DEFAULT,   &Sequence_SendInfo_getCppuType },
    { "SendPasswords",      -1 /* WID_SEND_PASSWORD */,         ATTR_DEFAULT,   &Sequence_SendInfo_getCppuType },
    { "SendProtocolPrivate",-1 /* WID_SEND_PRIVATE_PROT_ID */,  ATTR_DEFAULT,   &sal_uInt16_getCppuType },
    { "SendProtocolPublic", -1 /* WID_SEND_PUBLIC_PROT_ID */,   ATTR_DEFAULT,   &sal_uInt16_getCppuType },
    { "SendReplyTos",       -1 /* WID_SEND_REPLY_TO_DEFAULT */, ATTR_DEFAULT,   &Sequence_SendInfo_getCppuType },
    { "SendServerNames",    -1 /* WID_SEND_SERVERNAME */,       ATTR_DEFAULT,   &Sequence_SendInfo_getCppuType },
    { "SendUserNames",      -1 /* WID_SEND_USERNAME */,         ATTR_DEFAULT,   &Sequence_SendInfo_getCppuType },
    { "SendVIMPostOfficePath",
                            -1 /* WID_SEND_VIM_POPATH */,       ATTR_DEFAULT,   &OUString_getCppuType },
    { "ServerBase",         -1 /* WID_SERVERBASE */,            ATTR_DEFAULT,   &OUString_getCppuType },
    { "ServerName",         -1 /* WID_SERVERNAME */,            ATTR_DEFAULT,   &OUString_getCppuType },
    { "ServerPort",         -1 /* WID_SERVERPORT */,            ATTR_DEFAULT,   &sal_uInt16_getCppuType },
    { "Size",               -1 /* WID_DOCUMENT_SIZE */,         ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_uInt64_getCppuType },
    { "SizeLimit",          -1 /* WID_SIZE_LIMIT */,            ATTR_DEFAULT,   &sal_uInt64_getCppuType },
    { "SubscribedCount",    -1 /* WID_SUBSCRNEWSGROUPCOUNT */,  ATTR_DEFAULT | PropertyAttribute::READONLY,
                                                                                            &sal_uInt32_getCppuType },
    { "SynchronizePolicy",  -1 /* WID_WHO_IS_MASTER */,         ATTR_DEFAULT,   &enum_getCppuType },
    { "TargetFrames",       -1 /* WID_TARGET_FRAMES */,         ATTR_DEFAULT,   &OUString_getCppuType },
    { "TargetURL",          -1 /* WID_TARGET_URL */,            ATTR_DEFAULT,   &OUString_getCppuType },
//  { "ThreadingInfo",      -1 /* WID_THREADING */,             ATTR_DEFAULT,   &Sequence_ThreadingInfo_getCppuType },
    { "TimeLimitStore",     -1 /* WID_STORE_MSGS_TIMELIMIT */,  ATTR_DEFAULT,   &sal_uInt16_getCppuType },
    { "Title",              -1 /* WID_TITLE */,                 ATTR_DEFAULT,   &OUString_getCppuType },
    { "UpdateOnOpen",       -1 /* WID_FLAG_UPDATE_ON_OPEN */,   ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "UseOutBoxPrivateProtocolSettings",
                            -1 /* WID_SEND_PRIVATE_OUTBOXPROPS */,
                                                                ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "UseOutBoxPublicProtocolSettings",
                            -1 /* WID_SEND_PUBLIC_OUTBOXPROPS */,
                                                                ATTR_DEFAULT,   &sal_Bool_getCppuType },
    { "UserName",           -1 /* WID_USERNAME */,              ATTR_DEFAULT,   &OUString_getCppuType },
    { "UserSortCriterium",  -1 /* WID_USER_SORT_CRITERIUM */,   ATTR_DEFAULT,   &OUString_getCppuType },
    { "VIMPostOfficePath",  -1 /* WID_VIM_POPATH */,            ATTR_DEFAULT,   &OUString_getCppuType },
    { "VerificationMode",   -1 /* WID_HTTP_VERIFY_MODE */,      ATTR_DEFAULT,   &enum_getCppuType },

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
                        const Reference< XMultiServiceFactory >& rxSMgr )
: /*m_xSMgr( rxSMgr ),*/
  m_pProps( 0 )
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
                     OUString::createFromAscii(
                         "com.sun.star.comp.ucb.UcbPropertiesManager" ),
                     OUString::createFromAscii(
                         PROPERTIES_MANAGER_SERVICE_NAME ) );

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
                OSL_ENSURE( sal_False,
                            "UcbPropertiesManager::getProperties - "
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

