/*************************************************************************
 *
 *  $RCSfile: ldapuserprofilelayer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-10-22 08:06:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards OOurce License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free OOftware; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free OOftware Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free OOftware
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards OOurce License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  OOurce License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  OOftware provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE OOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the OOftware.
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

#ifndef EXTENSIONS_CONFIG_LDAP_LADPUSERPROFILELAYER_HXX_
#include "ldapuserprofilelayer.hxx"
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_PROPERTYINFO_HPP_
#include <com/sun/star/configuration/backend/PropertyInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_CONNECTIONLOSTEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/ConnectionLostException.hpp>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

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
