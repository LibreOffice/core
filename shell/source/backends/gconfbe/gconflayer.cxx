/*************************************************************************
 *
 *  $RCSfile: gconflayer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-03-30 15:05:29 $
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

#ifndef GCONFLAYER_HXX_
#include "gconflayer.hxx"
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

GconfLayer::GconfLayer(
    const rtl::OUString& sComponent,
    const KeyMappingTable& aKeyMap,
    const rtl::OUString& sTimestamp,
    const TSMappingTable& aTSMap,
    const uno::Reference<lang::XMultiServiceFactory>& xFactory)
  : mComponent(sComponent), mKeyMap(aKeyMap),
    mTimestamp(sTimestamp), mTSMap(aTSMap),
    mFactory(xFactory)
{

}
//------------------------------------------------------------------------------
void GconfLayer::convertGconfValue(
    const GConfValue* aValue,
    uno::Any* aOOValue,
    const rtl::OUString& aOOType)
{
    sal_Bool bCorrectType = sal_True;
    switch (aValue->type)
    {
        case GCONF_VALUE_STRING:
        {
            if(aOOType != rtl::OUString::createFromAscii("string"))
            {
                bCorrectType = sal_False;
            }
            else
            {
                rtl::OUString aVal(
                    rtl::OUString::createFromAscii(gconf_value_get_string(aValue)));
                *aOOValue <<= aVal;
            }
            break;
        }
        case GCONF_VALUE_INT:
        {
            if(aOOType != rtl::OUString::createFromAscii("int") &&
                aOOType != rtl::OUString::createFromAscii("integer"))
            {
                bCorrectType = sal_False;
            }
            else
            {
                sal_Int32 aVal = gconf_value_get_int(aValue);
                 *aOOValue <<= aVal;
            }
            break;
        }
        case GCONF_VALUE_FLOAT:
        {
            if(aOOType != rtl::OUString::createFromAscii("double") )
            {
                bCorrectType = sal_False;
            }
            else
            {
                double aVal = gconf_value_get_float(aValue);
                 *aOOValue <<= aVal;
            }
            break;
        }
        case GCONF_VALUE_BOOL:
        {
            if(aOOType != rtl::OUString::createFromAscii("boolean"))
            {
                bCorrectType = sal_False;
            }
            else
            {
                sal_Bool aVal = gconf_value_get_bool(aValue);
                 *aOOValue <<= aVal;
            }
            break;
        }
        case GCONF_VALUE_LIST:
            //TODO
            break;
        case GCONF_VALUE_PAIR:
            //TODO
            break;
        case GCONF_VALUE_INVALID:
            break;

        default:
            break;
    }
    if (!bCorrectType)
     throw backend::MalformedDataException(
        rtl::OUString::createFromAscii("Gconfbe:GconfLayer: GconfType does not match StarOffice Type"),
            *this, uno::Any());


}
//------------------------------------------------------------------------------

void SAL_CALL GconfLayer::readData(
    const uno::Reference<backend::XLayerHandler>& xHandler)
    throw ( backend::MalformedDataException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{
    std::vector<backend::PropertyInfo> aPropList;
    //Look up in map all keys asociated with requested OOffice Component
    //and retrieve values from Gconf
    typedef KeyMappingTable::const_iterator BFIter;
    typedef std::pair<BFIter, BFIter> BFRange;



    BFRange aRange = mKeyMap.equal_range(mComponent);
    GError* aError = NULL;
    GConfClient* aClient = NULL;
    if  (aRange.first != mKeyMap.end())
    {
        aClient = GconfBackend::getGconfClient();
    }

    while (aRange.first != aRange.second)
    {
        BFIter cur = aRange.first++;

        //Need to read data using Gconf api here

        GConfValue* aGconfValue = NULL;
        rtl::OString sKeyValue = rtl::OUStringToOString(cur->second.mGconfName, RTL_TEXTENCODING_ASCII_US);
        aGconfValue = gconf_client_get( aClient, sKeyValue.getStr(),&aError);

        if (aError == NULL)
        {

            //Fill in the ProperyInfo Struct
            backend::PropertyInfo aPropInfo;

            aPropInfo.Name = cur->second.mOOName;
            aPropInfo.Type = cur->second.mOOType;
            aPropInfo.Protected = cur->second.mbProtected;
            convertGconfValue(aGconfValue, &aPropInfo.Value,aPropInfo.Type );


            aPropList.push_back(aPropInfo);
            gconf_value_free(aGconfValue);
        }
        else
        {
            rtl::OString aErrorStr(aError->message);
            //Do nothing for now - real impl can decide whether to throw exception
            OSL_TRACE("GconfLayer cannot read data for Gconfkey:%s with message:%s",
                sKeyValue.getStr(), aError->message);
        }

    }
    if ( !aPropList.empty())
    {

        uno::Sequence<backend::PropertyInfo> aPropInfoList(aPropList.size());
        for( sal_Int32 i = 0; i < aPropList.size(); i++)
        {
            aPropInfoList[i] = aPropList[i];
        }

        //Create instance of LayerContentDescriber Service
        rtl::OUString const k_sLayerDescriberService (RTL_CONSTASCII_USTRINGPARAM(
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
            OSL_TRACE("Could not create com.sun.star.configuration.backend.LayerContentDescriber Service");
        }
    }

}
//------------------------------------------------------------------------------

 rtl::OUString SAL_CALL GconfLayer::getTimestamp(void)
            throw (uno::RuntimeException)
{
    TSMappingTable::const_iterator aTSIter;
    aTSIter = mTSMap.find(mComponent);
    if (aTSIter != mTSMap.end())
    {
        mTimestamp= aTSIter->second;
    }

    return mTimestamp;
}
//------------------------------------------------------------------------------
