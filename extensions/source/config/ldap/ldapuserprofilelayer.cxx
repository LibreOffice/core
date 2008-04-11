/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ldapuserprofilelayer.cxx,v $
 * $Revision: 1.8 $
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
#include "precompiled_extensions.hxx"
#include "ldapuserprofilelayer.hxx"
#include <com/sun/star/configuration/backend/PropertyInfo.hpp>
#include <com/sun/star/configuration/backend/ConnectionLostException.hpp>
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
// on windows this is defined indirectly by <ldap.h>
#undef OPTIONAL
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Sequence.hxx>

//==============================================================================
namespace extensions { namespace config { namespace ldap {

static const sal_Unicode kPathSeparator = '/' ;

static
uno::Reference<backend::XLayerContentDescriber>
    newLayerDescriber(const uno::Reference<lang::XMultiServiceFactory>& xFactory)
{
    typedef uno::Reference<backend::XLayerContentDescriber> LayerDescriber;

    rtl::OUString const k_sLayerDescriberService (
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.configuration.backend.LayerDescriber"));

    LayerDescriber xResult(xFactory->createInstance(k_sLayerDescriberService), uno::UNO_QUERY_THROW);
    return xResult;
}
//------------------------------------------------------------------------------

void LdapUserProfileSource::getUserProfile(const rtl::OUString & aUser, LdapUserProfile & aProfile)
{
    mConnection.getUserProfile(aUser,
                               mProfileMap,
                               aProfile);
}

rtl::OUString LdapUserProfileSource::getComponentName() const
{
    return mProfileMap.getComponentName();
}

rtl::OUString LdapUserProfileSource::getConfigurationBasePath() const
{
    rtl::OUStringBuffer sComponentNameBuffer(mProfileMap.getComponentName());
    sComponentNameBuffer.append(kPathSeparator);
    sComponentNameBuffer.append (mProfileMap.getGroupName());
    sComponentNameBuffer.append(kPathSeparator);

    return sComponentNameBuffer.makeStringAndClear();
}
//------------------------------------------------------------------------------

struct LdapUserProfileLayer::ProfileData
{
    LdapUserProfile mProfile;
    rtl::OUString mBasePath;

    explicit ProfileData(LdapUserProfileSource & aSource, const rtl::OUString & aUser)
    {
        aSource.getUserProfile(aUser, mProfile);
        mBasePath = aSource.getConfigurationBasePath();
    }
};
//------------------------------------------------------------------------------

LdapUserProfileLayer::LdapUserProfileLayer(
    const uno::Reference<lang::XMultiServiceFactory>& xFactory,
    const rtl::OUString& aUser,
    const LdapUserProfileSourceRef & aUserProfileSource,
    const rtl::OUString& aTimestamp)
: mLayerDescriber( newLayerDescriber(xFactory) )
, mSource( aUserProfileSource )
, mUser(aUser)
, mTimestamp(aTimestamp)
, mProfile( 0 )
{
    OSL_ASSERT(mSource.is());
}

//------------------------------------------------------------------------------
LdapUserProfileLayer::~LdapUserProfileLayer()
{
    delete mProfile;
}
//------------------------------------------------------------------------------

#define PROPNAME( name ) rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( name ) )
#define PROPTYPE( type ) getCppuType( static_cast< type const *>( 0 ) )

const sal_Int32 LAYER_PROPERTY_URL = 1;

cppu::IPropertyArrayHelper * SAL_CALL LdapUserProfileLayer::newInfoHelper()
{
    using com::sun::star::beans::Property;
    using namespace com::sun::star::beans::PropertyAttribute;

    Property properties[] =
    {
        Property(PROPNAME("URL"), LAYER_PROPERTY_URL, PROPTYPE(rtl::OUString), READONLY)
    };

    return new cppu::OPropertyArrayHelper(properties, sizeof(properties)/sizeof(properties[0]));
}
//------------------------------------------------------------------------------

void SAL_CALL LdapUserProfileLayer::getFastPropertyValue( uno::Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
    case LAYER_PROPERTY_URL:
        {
            rtl::OUStringBuffer aURL;
            aURL.appendAscii("ldap-user-profile:");
            aURL.append(mUser);
            aURL.append(sal_Unicode('@'));
            if (mSource.is())
                aURL.append(mSource->getComponentName());
            else
                aURL.appendAscii("<NULL>");

            rValue <<= aURL.makeStringAndClear();
        }
        break;

    default:
        OSL_ENSURE(false, "Error: trying to get an UNKNOWN property");
        break;
    }
}
//------------------------------------------------------------------------------

bool LdapUserProfileLayer::readProfile()
{
    if (mSource.is())
    try
    {
        OSL_ASSERT(!mProfile);
        mProfile = new ProfileData(*mSource,mUser);

        mSource.clear();
    }
    catch (ldap::LdapConnectionException & e)
    {
        // without existing Ldap Connection we should never have gotten a timestamp
        OSL_ENSURE(false, "Unexpected: Have Ldap Backedn Layer but no vaild LDAP connection ?!");
        throw backend::ConnectionLostException(e.Message, *this, uno::makeAny(e) );
    }
    catch (ldap::LdapGenericException & e)
    {
        throw backend::BackendAccessException(e.Message, *this, uno::makeAny(e) );
    }
    OSL_ASSERT( !mSource.is() );
    OSL_ASSERT( mProfile != 0 );
    return mProfile != 0;
}
//------------------------------------------------------------------------------

void SAL_CALL LdapUserProfileLayer::readData(
    const uno::Reference<backend::XLayerHandler>& xHandler)
    throw ( backend::MalformedDataException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{
    std::vector<backend::PropertyInfo> aPropList;
#ifdef SUPPRESS_BACKEND_ERRORS
    try
#endif
    if ( readProfile() )
    {
        // initialize PropInfo members that are the same for all settings
        const rtl::OUString k_sTypeString(RTL_CONSTASCII_USTRINGPARAM("string"));

        backend::PropertyInfo aPropInfo;
        aPropInfo.Type = k_sTypeString;
        aPropInfo.Protected = sal_False;

        LdapUserProfile * pProfile = &mProfile->mProfile;
        aPropList.reserve(pProfile->mProfile.size());

        for (LdapUserProfile::Iterator entry = pProfile->mProfile.begin() ;
             entry != pProfile->mProfile.end() ; ++ entry)
        {
            if (entry->mAttribute.getLength()==0) { continue ; }
            if (entry->mValue.getLength()==0) { continue ; }

            aPropInfo.Name = mProfile->mBasePath + entry->mAttribute;
            aPropInfo.Value <<= entry->mValue;

            aPropList.push_back(aPropInfo);
        }
    }
#ifdef SUPPRESS_BACKEND_ERRORS
    catch (uno::Exception & e)
    {
        OSL_TRACE("LDAP Backend - Reading data from LDAP failed: %s\n",
                  rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
    }
#endif

    if ( !aPropList.empty())
    {
        //Describe UserProfileLayer (the list of properties) to the XHandler
        //Object using com.sun.star.comp.backend.LayerContentDescriber Service
        uno::Sequence<backend::PropertyInfo> aPropInfoList(&aPropList.front(),aPropList.size());

        mLayerDescriber->describeLayer(xHandler, aPropInfoList);
    }
    // else { check handler not NULL; xHandler->startLayer(); xHandler->endLayer(); }
}
//------------------------------------------------------------------------------
}}}
//------------------------------------------------------------------------------

