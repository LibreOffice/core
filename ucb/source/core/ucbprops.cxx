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
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>

#include "ucbprops.hxx"

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;

#define ATTR_DEFAULT ( PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID | PropertyAttribute::MAYBEDEFAULT )

UcbPropertiesManager::UcbPropertiesManager()
: m_pProps({
    { u"Account"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"AutoUpdateInterval"_ustr, -1, cppu::UnoType<sal_Int32>::get(), ATTR_DEFAULT },
    { u"ConfirmEmpty"_ustr, -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { u"ConnectionLimit"_ustr, -1, cppu::UnoType<sal_Int16>::get(), ATTR_DEFAULT },
    { u"ConnectionMode"_ustr, -1, cppu::UnoType<sal_Int16>::get(), ATTR_DEFAULT },
    { u"ContentCountLimit"_ustr, -1, cppu::UnoType<sal_Int16>::get(), ATTR_DEFAULT },
    { u"ContentType"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"Cookie"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"CrossReferences"_ustr, -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::CrossReference>>::get(),
      ATTR_DEFAULT },
    { u"DateCreated"_ustr, -1, cppu::UnoType<css::util::DateTime>::get(),
      ATTR_DEFAULT },
    { u"DateModified"_ustr, -1, cppu::UnoType<css::util::DateTime>::get(),
      ATTR_DEFAULT },
    { u"DeleteOnServer"_ustr, -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { u"DocumentBody"_ustr, -1, cppu::UnoType<css::uno::Sequence<sal_Int8>>::get(),
      ATTR_DEFAULT },
    { u"DocumentCount"_ustr, -1, cppu::UnoType<sal_Int32>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { u"DocumentCountMarked"_ustr, -1, cppu::UnoType<sal_Int32>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { u"DocumentHeader"_ustr, -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::DocumentHeaderField>>::get(),
      ATTR_DEFAULT },
    { u"DocumentStoreMode"_ustr, -1,
      cppu::UnoType<css::ucb::DocumentStoreMode>::get(), ATTR_DEFAULT },
    { u"DocumentViewMode"_ustr, -1, cppu::UnoType<sal_Int16>::get(), ATTR_DEFAULT },
    { u"FTPAccount"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"Flags"_ustr, -1, cppu::UnoType<sal_Int32>::get(), ATTR_DEFAULT },
    { u"FolderCount"_ustr, -1, cppu::UnoType<sal_Int32>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { u"FolderViewMode"_ustr, -1, cppu::UnoType<sal_Int16>::get(), ATTR_DEFAULT },
    { u"FreeSpace"_ustr, -1, cppu::UnoType<sal_Int64>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { u"HasDocuments"_ustr, -1, cppu::UnoType<bool>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { u"HasFolders"_ustr, -1, cppu::UnoType<bool>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { u"IsAutoDelete"_ustr, -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { u"IsAutoUpdate"_ustr, -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { u"IsDocument"_ustr, -1, cppu::UnoType<bool>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { u"IsFolder"_ustr, -1, cppu::UnoType<bool>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { u"IsKeepExpired"_ustr, -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { u"IsLimitedContentCount"_ustr, -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { u"IsMarked"_ustr, -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { u"IsRead"_ustr, -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { u"IsReadOnly"_ustr, -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { u"IsSubscribed"_ustr, -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { u"IsTimeLimitedStore"_ustr, -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { u"Keywords"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"LocalBase"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"MessageBCC"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"MessageBody"_ustr, -1, cppu::UnoType<css::ucb::XDataContainer>::get(),
      ATTR_DEFAULT },
    { u"MessageCC"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"MessageFrom"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"MessageId"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"MessageInReplyTo"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"MessageReplyTo"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"MessageTo"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"NewsGroups"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"NoCacheList"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"Origin"_ustr, -1, cppu::UnoType<OUString>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { u"OutgoingMessageRecipients"_ustr, -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::RecipientInfo>>::get(),
      ATTR_DEFAULT },
    { u"OutgoingMessageState"_ustr, -1,
      cppu::UnoType<css::ucb::OutgoingMessageState>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { u"OutgoingMessageViewMode"_ustr, -1, cppu::UnoType<sal_Int16>::get(),
      ATTR_DEFAULT },
    { u"Password"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"Priority"_ustr, -1, cppu::UnoType<css::ucb::Priority>::get(), ATTR_DEFAULT },
    { u"References"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"Referer"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"Rules"_ustr, -1, cppu::UnoType<css::ucb::RuleSet>::get(), ATTR_DEFAULT },
    { u"SearchCriteria"_ustr, -1, cppu::UnoType<css::ucb::RuleSet>::get(),
      ATTR_DEFAULT },
    { u"SearchIndirections"_ustr, -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { u"SearchLocations"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"SearchRecursive"_ustr, -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { u"SeenCount"_ustr, -1, cppu::UnoType<sal_Int32>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { u"SendCopyTarget"_ustr, -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::SendInfo>>::get(),
      ATTR_DEFAULT },
    { u"SendFormats"_ustr, -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::SendMediaTypes>>::get(),
      ATTR_DEFAULT },
    { u"SendFroms"_ustr, -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::SendInfo>>::get(),
      ATTR_DEFAULT },
    { u"SendPasswords"_ustr, -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::SendInfo>>::get(),
      ATTR_DEFAULT },
    { u"SendProtocolPrivate"_ustr, -1, cppu::UnoType<sal_Int16>::get(),
      ATTR_DEFAULT },
    { u"SendProtocolPublic"_ustr, -1, cppu::UnoType<sal_Int16>::get(), ATTR_DEFAULT },
    { u"SendReplyTos"_ustr, -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::SendInfo>>::get(),
      ATTR_DEFAULT },
    { u"SendServerNames"_ustr, -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::SendInfo>>::get(),
      ATTR_DEFAULT },
    { u"SendUserNames"_ustr, -1,
      cppu::UnoType<css::uno::Sequence<css::ucb::SendInfo>>::get(),
      ATTR_DEFAULT },
    { u"SendVIMPostOfficePath"_ustr, -1, cppu::UnoType<OUString>::get(),
      ATTR_DEFAULT },
    { u"ServerBase"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"ServerName"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"ServerPort"_ustr, -1, cppu::UnoType<sal_Int16>::get(), ATTR_DEFAULT },
    { u"Size"_ustr, -1, cppu::UnoType<sal_Int64>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { u"SizeLimit"_ustr, -1, cppu::UnoType<sal_Int64>::get(), ATTR_DEFAULT },
    { u"SubscribedCount"_ustr, -1, cppu::UnoType<sal_Int32>::get(),
      ATTR_DEFAULT | PropertyAttribute::READONLY },
    { u"SynchronizePolicy"_ustr, -1,
      cppu::UnoType<css::ucb::SynchronizePolicy>::get(), ATTR_DEFAULT },
    { u"TargetFrames"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"TargetURL"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"TimeLimitStore"_ustr, -1, cppu::UnoType<sal_Int16>::get(), ATTR_DEFAULT },
    { u"Title"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"UpdateOnOpen"_ustr, -1, cppu::UnoType<bool>::get(), ATTR_DEFAULT },
    { u"UseOutBoxPrivateProtocolSettings"_ustr, -1, cppu::UnoType<bool>::get(),
      ATTR_DEFAULT },
    { u"UseOutBoxPublicProtocolSettings"_ustr, -1, cppu::UnoType<bool>::get(),
      ATTR_DEFAULT },
    { u"UserName"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"UserSortCriterium"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"VIMPostOfficePath"_ustr, -1, cppu::UnoType<OUString>::get(), ATTR_DEFAULT },
    { u"VerificationMode"_ustr, -1, cppu::UnoType<css::ucb::VerificationMode>::get(),
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
    return u"com.sun.star.comp.ucb.UcbPropertiesManager"_ustr;
}
sal_Bool SAL_CALL UcbPropertiesManager::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}
css::uno::Sequence< OUString > SAL_CALL UcbPropertiesManager::getSupportedServiceNames()
{
    return { u"com.sun.star.ucb.PropertiesManager"_ustr };
}



// Service factory implementation.

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_UcbPropertiesManager_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new UcbPropertiesManager());
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
    auto pProp = std::find_if(std::cbegin(m_pProps), std::cend(m_pProps),
        [&rName](const Property& rCurrProp) { return rCurrProp.Name == rName; });
    if (pProp != std::cend(m_pProps))
    {
        rProp = *pProp;
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
