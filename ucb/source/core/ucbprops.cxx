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

#include <sal/config.h>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/ucb/CrossReference.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/ucb/DocumentHeaderField.hpp>
#include <com/sun/star/ucb/DocumentStoreMode.hpp>
#include <com/sun/star/ucb/Priority.hpp>
#include <com/sun/star/ucb/RecipientInfo.hpp>
#include <com/sun/star/ucb/RuleSet.hpp>
#include <com/sun/star/ucb/SendInfo.hpp>
#include <com/sun/star/ucb/SendMediaTypes.hpp>
#include <com/sun/star/ucb/SynchronizePolicy.hpp>
#include <com/sun/star/ucb/VerificationMode.hpp>
#include <com/sun/star/ucb/XDataContainer.hpp>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>

#include "ucbprops.hxx"

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;

#define ATTR_DEFAULT ( PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID | PropertyAttribute::MAYBEDEFAULT )

UcbPropertiesManager::UcbPropertiesManager()
: m_pProps({
    { "Account", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "AutoUpdateInterval", -1, cppu::UnoType<sal_Int32>::get(), ATTR_DEFAULT },
    { "ConfirmEmpty", -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { "ConnectionLimit", -1, cppu::UnoType<sal_Int16>::get(), ATTR_DEFAULT },
    { "ConnectionMode", -1, cppu::UnoType<sal_Int16>::get(), ATTR_DEFAULT },
    { "ContentCountLimit", -1, cppu::UnoType<sal_Int16>::get(), ATTR_DEFAULT },
    { "ContentType", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "Cookie", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "CrossReferences", -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::CrossReference>>::get(),
      ATTR_DEFAULT },
    { "DateCreated", -1, cppu::UnoType<css::util::DateTime>::get(),
      ATTR_DEFAULT },
    { "DateModified", -1, cppu::UnoType<css::util::DateTime>::get(),
      ATTR_DEFAULT },
    { "DeleteOnServer", -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { "DocumentBody", -1, cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(),
      ATTR_DEFAULT },
    { "DocumentCount", -1, cppu::UnoType<sal_Int32>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { "DocumentCountMarked", -1, cppu::UnoType<sal_Int32>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { "DocumentHeader", -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::DocumentHeaderField>>::get(),
      ATTR_DEFAULT },
    { "DocumentStoreMode", -1,
      cppu::UnoType<css::ucb::DocumentStoreMode>::get(), ATTR_DEFAULT },
    { "DocumentViewMode", -1, cppu::UnoType<sal_Int16>::get(), ATTR_DEFAULT },
    { "FTPAccount", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "Flags", -1, cppu::UnoType<sal_Int32>::get(), ATTR_DEFAULT },
    { "FolderCount", -1, cppu::UnoType<sal_Int32>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { "FolderViewMode", -1, cppu::UnoType<sal_Int16>::get(), ATTR_DEFAULT },
    { "FreeSpace", -1, cppu::UnoType<sal_Int64>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { "HasDocuments", -1, cppu::UnoType<bool>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { "HasFolders", -1, cppu::UnoType<bool>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { "IsAutoDelete", -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { "IsAutoUpdate", -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { "IsDocument", -1, cppu::UnoType<bool>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { "IsFolder", -1, cppu::UnoType<bool>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { "IsKeepExpired", -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { "IsLimitedContentCount", -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { "IsMarked", -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { "IsRead", -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { "IsReadOnly", -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { "IsSubscribed", -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { "IsTimeLimitedStore", -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { "Keywords", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "LocalBase", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "MessageBCC", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "MessageBody", -1, cppu::UnoType<css::ucb::XDataContainer>::get(),
      ATTR_DEFAULT },
    { "MessageCC", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "MessageFrom", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "MessageId", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "MessageInReplyTo", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "MessageReplyTo", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "MessageTo", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "NewsGroups", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "NoCacheList", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "Origin", -1, cppu::UnoType<OUString>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { "OutgoingMessageRecipients", -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::RecipientInfo>>::get(),
      ATTR_DEFAULT },
    { "OutgoingMessageState", -1,
      cppu::UnoType<css::ucb::OutgoingMessageState>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { "OutgoingMessageViewMode", -1, cppu::UnoType<sal_Int16>::get(),
      ATTR_DEFAULT },
    { "Password", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "Priority", -1, cppu::UnoType<css::ucb::Priority>::get(), ATTR_DEFAULT },
    { "References", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "Referer", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "Rules", -1, cppu::UnoType<css::ucb::RuleSet>::get(), ATTR_DEFAULT },
    { "SearchCriteria", -1, cppu::UnoType<css::ucb::RuleSet>::get(),
      ATTR_DEFAULT },
    { "SearchIndirections", -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { "SearchLocations", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "SearchRecursive", -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { "SeenCount", -1, cppu::UnoType<sal_Int32>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { "SendCopyTarget", -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::SendInfo>>::get(),
      ATTR_DEFAULT },
    { "SendFormats", -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::SendMediaTypes>>::get(),
      ATTR_DEFAULT },
    { "SendFroms", -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::SendInfo>>::get(),
      ATTR_DEFAULT },
    { "SendPasswords", -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::SendInfo>>::get(),
      ATTR_DEFAULT },
    { "SendProtocolPrivate", -1, cppu::UnoType<sal_Int16>::get(),
      ATTR_DEFAULT },
    { "SendProtocolPublic", -1, cppu::UnoType<sal_Int16>::get(), ATTR_DEFAULT },
    { "SendReplyTos", -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::SendInfo>>::get(),
      ATTR_DEFAULT },
    { "SendServerNames", -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::SendInfo>>::get(),
      ATTR_DEFAULT },
    { "SendUserNames", -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::SendInfo>>::get(),
      ATTR_DEFAULT },
    { "SendVIMPostOfficePath", -1, cppu::UnoType<OUString>::get(),
      ATTR_DEFAULT },
    { "ServerBase", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "ServerName", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "ServerPort", -1, cppu::UnoType<sal_Int16>::get(), ATTR_DEFAULT },
    { "Size", -1, cppu::UnoType<sal_Int64>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { "SizeLimit", -1, cppu::UnoType<sal_Int64>::get(), ATTR_DEFAULT },
    { "SubscribedCount", -1, cppu::UnoType<sal_Int32>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { "SynchronizePolicy", -1,
      cppu::UnoType<css::ucb::SynchronizePolicy>::get(), ATTR_DEFAULT },
    { "TargetFrames", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "TargetURL", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "TimeLimitStore", -1, cppu::UnoType<sal_Int16>::get(), ATTR_DEFAULT },
    { "Title", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "UpdateOnOpen", -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { "UseOutBoxPrivateProtocolSettings", -1, cppu::UnoType<bool>::get(),
      ATTR_DEFAULT },
    { "UseOutBoxPublicProtocolSettings", -1, cppu::UnoType<bool>::get(),
      ATTR_DEFAULT },
    { "UserName", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "UserSortCriterium", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "VIMPostOfficePath", -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { "VerificationMode", -1, cppu::UnoType<css::ucb::VerificationMode>::get(),
      ATTR_DEFAULT }})
{
}


// virtual
UcbPropertiesManager::~UcbPropertiesManager()
{
}

// XServiceInfo methods.

OUString SAL_CALL UcbPropertiesManager::getImplementationName()
{
    return "com.sun.star.comp.ucb.UcbPropertiesManager";
}
sal_Bool SAL_CALL UcbPropertiesManager::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}
css::uno::Sequence< OUString > SAL_CALL UcbPropertiesManager::getSupportedServiceNames()
{
    return { "com.sun.star.ucb.PropertiesManager" };
}



// Service factory implementation.

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_UcbPropertiesManager_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(static_cast<cppu::OWeakObject*>(new UcbPropertiesManager()));
}

// XPropertySetInfo methods.


// virtual
Sequence< Property > SAL_CALL UcbPropertiesManager::getProperties()
{
    return m_pProps;
}


// virtual
Property SAL_CALL UcbPropertiesManager::getPropertyByName( const OUString& aName )
{
    Property aProp;
    if ( queryProperty( aName, aProp ) )
        return aProp;

    throw UnknownPropertyException(aName);
}


// virtual
sal_Bool SAL_CALL UcbPropertiesManager::hasPropertyByName( const OUString& Name )
{
    Property aProp;
    return queryProperty( Name, aProp );
}


// Non-Interface methods.


bool UcbPropertiesManager::queryProperty(
                                const OUString& rName, Property& rProp )
{
    auto pProp = std::find_if(m_pProps.begin(), m_pProps.end(),
        [&rName](const Property& rCurrProp) { return rCurrProp.Name == rName; });
    if (pProp != m_pProps.end())
    {
        rProp = *pProp;
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
