/*************************************************************************
 *
 *  $RCSfile: ldapuserprofilelayer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 14:39:07 $
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
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERCONTENTDESCIBER_HPP_
#include <com/sun/star/configuration/backend/XLayerContentDescriber.hpp>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

//==============================================================================
namespace extensions { namespace config { namespace ldap {


LdapUserProfileLayer::LdapUserProfileLayer(
    const uno::Reference<lang::XMultiServiceFactory>& xFactory,
    const rtl::OUString& aUser,
    const LdapUserProfileMap& aUserProfileMap,
    LdapConnection& aConnection,
    const rtl::OUString& aTimeStamp)
  : mFactory(xFactory),
    mUserName(aUser),
    mConnection(aConnection),
    mUserProfileMap(aUserProfileMap),
    mTimeStamp(aTimeStamp)
{


}
//------------------------------------------------------------------------------
static const sal_Unicode kPathSeparator = '/' ;

void SAL_CALL LdapUserProfileLayer::readData(
    const uno::Reference<backend::XLayerHandler>& xHandler)
    throw ( backend::MalformedDataException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{
    try
    {
        LdapUserProfile aUserProfile;
        mConnection.getUserProfile(mUserName,
                                   mUserProfileMap,
                                   aUserProfile);


        std::vector<LdapUserProfile::ProfileEntry>::const_iterator entry ;


        rtl::OUStringBuffer sComponentNameBuffer(mUserProfileMap.getComponentName());
        sComponentNameBuffer.append(kPathSeparator);
        sComponentNameBuffer.append (mUserProfileMap.getGroupName());
        sComponentNameBuffer.append(kPathSeparator);
        rtl::OUString sComponentName = sComponentNameBuffer.makeStringAndClear();

        std::vector<backend::PropertyInfo> aPropList;
        backend::PropertyInfo aPropInfo;
        for (entry = aUserProfile.mProfile.begin() ;
            entry != aUserProfile.mProfile.end() ; ++ entry)
        {
            if ((*entry).mAttribute.getLength()==0) { continue ; }
            if ((*entry).mValue.getLength()==0) { continue ; }
            aPropInfo.Name = sComponentName + entry->mAttribute;
            aPropInfo.Type = rtl::OUString::createFromAscii("string");
            aPropInfo.Value <<= entry->mValue;
            aPropInfo.Protected = sal_False;
            aPropList.push_back(aPropInfo);
        }
        if ( !aPropList.empty())
        {
            //Describe UserProfileLayer (the list of properties) to the XHandler
            //Object using com.sun.star.comp.backend.LayerContentDescriber Service
            uno::Sequence<backend::PropertyInfo> aPropInfoList(aPropList.size());
            for( sal_uInt32 i = 0; i < aPropList.size(); i++)
            {
                aPropInfoList[i] = aPropList[i];
            }
            rtl::OUString const k_sLayerDescriberService (
                RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.comp.configuration.backend.LayerDescriber"));

            typedef uno::Reference<backend::XLayerContentDescriber> LayerDescriber;
            LayerDescriber xLayerDescriber = LayerDescriber::query(
                mFactory->createInstance(k_sLayerDescriberService));

            if (xLayerDescriber.is())
            {
                xLayerDescriber->describeLayer(xHandler, aPropInfoList);
            }
            else
            {
                OSL_TRACE("LdapUserProfileLayer::readData- cannot create com.sun.star.configuration.backend.LayerContentDescriber Service");
            }
        }
    }
    catch(uno::Exception e)
    {
        OSL_TRACE("LdapUserProfileLayer::readData- cannot read LDAP User Profile Layer");
    }
}
//------------------------------------------------------------------------------
}}}
//------------------------------------------------------------------------------
