/*************************************************************************
 *
 *  $RCSfile: gconflayer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-10-18 14:41:59 $
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

#include <stdio.h>

//==============================================================================

GconfLayer::GconfLayer(
    const rtl::OUString& aComponent,
    const rtl::OUString& aTimestamp,
    const uno::Reference<uno::XComponentContext>& xContext)
  : m_aComponent(aComponent), m_aTimestamp(aTimestamp)
{
    //Create instance of LayerContentDescriber Service
    rtl::OUString const k_sLayerDescriberService(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.configuration.backend.LayerDescriber"));

    typedef uno::Reference<backend::XLayerContentDescriber> LayerDescriber;
    uno::Reference< lang::XMultiComponentFactory > xServiceManager = xContext->getServiceManager();
    if( xServiceManager.is() )
    {
        m_xLayerContentDescriber = LayerDescriber::query(
            xServiceManager->createInstanceWithContext(k_sLayerDescriberService, xContext));
    }
    else
    {
        OSL_TRACE("Could not retrieve ServiceManager");
    }
}

//------------------------------------------------------------------------------

/*
uno::Any GconfLayer::convertGconfValue(const GConfValue* aValue, const rtl::OUString& aOOType)
{
    uno::Any aRetVal;

    sal_Bool bCorrectType = sal_True;
    switch (aValue->type)
    {
        case GCONF_VALUE_STRING:
        {
            if(aOOType != rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("string")))
            {
                bCorrectType = sal_False;
            }
            else
            {
                rtl::OString aVal(gconf_value_get_string(aValue));
                aRetVal <<= OStringToOUString(aVal, RTL_TEXTENCODING_UTF-8);
            }
            break;
        }
        case GCONF_VALUE_INT:
        {
            if( aOOType != rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("int") ) &&
                aOOType != rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("integer") ) )
            {
                bCorrectType = sal_False;
            }
            else
            {
                sal_Int32 aVal = gconf_value_get_int(aValue);
                aRetVal <<= aVal;
            }
            break;
        }
        case GCONF_VALUE_FLOAT:
        {
            if(aOOType != rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("double") ) )
            {
                bCorrectType = sal_False;
            }
            else
            {
                double aVal = gconf_value_get_float(aValue);
                aRetVal <<= aVal;
            }
            break;
        }
        case GCONF_VALUE_BOOL:
        {
            if(aOOType != rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("boolean") ) )
            {
                bCorrectType = sal_False;
            }
            else
            {
                sal_Bool aVal = gconf_value_get_bool(aValue);
                aRetVal <<= aVal;
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
        throw backend::MalformedDataException( rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("Gconfbe:GconfLayer: GconfType does not match StarOffice Type")
            ), *this, uno::Any());


}

*/

//------------------------------------------------------------------------------

void SAL_CALL GconfLayer::readData( const uno::Reference<backend::XLayerHandler>& xHandler)
    throw ( backend::MalformedDataException, lang::NullPointerException,
            lang::WrappedTargetException, uno::RuntimeException)
{

    if( m_xLayerContentDescriber.is() )
    {
        uno::Sequence<backend::PropertyInfo> aPropInfoList(5);
        sal_Int32 nProperties = 0;

        GError* aError;
        GConfClient* aClient = GconfBackend::getGconfClient();
        GConfValue* aGconfValue;

        if( m_aComponent.equalsAscii("org.openoffice.Inet" ) )
        {
            aError = NULL;
            aGconfValue = gconf_client_get(aClient, "/system/proxy/mode" , &aError);

            if( aError == NULL && aGconfValue != NULL )
            {
                rtl::OString aMode(gconf_value_get_string(aGconfValue));

                if( aMode.equals("manual") )
                {
                    aPropInfoList[nProperties].Name = rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetProxyType") );
                    aPropInfoList[nProperties].Type = rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "int" ) );
                    aPropInfoList[nProperties].Protected = sal_False;
                    aPropInfoList[nProperties++].Value = uno::makeAny( (sal_Int32) 1 );

                    aError = NULL;
                    aGconfValue = gconf_client_get(aClient, "/system/http_proxy/host" , &aError);

                    if( aError == NULL && aGconfValue != NULL ) {
                        aPropInfoList[nProperties].Name = rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetHTTPProxyName") );
                        aPropInfoList[nProperties].Type = rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM( "string" ) );
                        aPropInfoList[nProperties].Protected = sal_False;
                        aPropInfoList[nProperties++].Value = uno::makeAny( OStringToOUString(
                            rtl::OString( gconf_value_get_string(aGconfValue) ),
                            RTL_TEXTENCODING_UTF8 ) );
                    }

                    aError = NULL;
                    aGconfValue = gconf_client_get(aClient, "/system/http_proxy/port" , &aError);

                    if( aError == NULL && aGconfValue != NULL ) {
                        aPropInfoList[nProperties].Name = rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetHTTPProxyPort") );
                        aPropInfoList[nProperties].Type = rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM( "int" ) );
                        aPropInfoList[nProperties].Protected = sal_False;
                        aPropInfoList[nProperties++].Value = uno::makeAny(
                            (sal_Int32) gconf_value_get_int(aGconfValue) );
                    }

                    aError = NULL;
                    aGconfValue = gconf_client_get(aClient, "/system/proxy/ftp_host" , &aError);

                    if( aError == NULL && aGconfValue != NULL ) {
                        aPropInfoList[nProperties].Name = rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetFTPProxyName") );
                        aPropInfoList[nProperties].Type = rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM( "string" ) );
                        aPropInfoList[nProperties].Protected = sal_False;
                        aPropInfoList[nProperties++].Value = uno::makeAny( OStringToOUString(
                            rtl::OString( gconf_value_get_string(aGconfValue) ),
                            RTL_TEXTENCODING_UTF8 ) );
                    }

                    aError = NULL;
                    aGconfValue = gconf_client_get(aClient, "/system/proxy/ftp_port" , &aError);

                    if( aError == NULL && aGconfValue != NULL ) {
                        aPropInfoList[nProperties].Name = rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetFTPProxyPort") );
                        aPropInfoList[nProperties].Type = rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM( "int" ) );
                        aPropInfoList[nProperties].Protected = sal_False;
                        aPropInfoList[nProperties++].Value = uno::makeAny(
                            (sal_Int32) gconf_value_get_int(aGconfValue) );
                    }
                }
                else if( aMode.equals("none") )
                {
                    aPropInfoList[nProperties].Name = rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetProxyType") );
                    aPropInfoList[nProperties].Type = rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "int" ) );
                    aPropInfoList[nProperties].Protected = sal_False;
                    aPropInfoList[nProperties++].Value = uno::makeAny( (sal_Int32) 0 );
                }
            }
        }
        else if( m_aComponent.equalsAscii("org.openoffice.Office.Common" ) )
        {
            aError = NULL;
            aGconfValue = gconf_client_get(aClient, "/desktop/gnome/url-handlers/mailto/command" , &aError);

            if( aError == NULL && aGconfValue != NULL ) {
                sal_Int32 nIndex = 0;
                aPropInfoList[nProperties].Name = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Office.Common/ExternalMailer/Program") );
                aPropInfoList[nProperties].Type = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "string" ) );
                aPropInfoList[nProperties].Protected = sal_False;
                aPropInfoList[nProperties++].Value = uno::makeAny( OStringToOUString(
                    rtl::OString( gconf_value_get_string(aGconfValue) ).getToken(0, ' ', nIndex),
                    RTL_TEXTENCODING_UTF8 ) );
            }
        }

        if( nProperties > 0 )
        {
            aPropInfoList.realloc(nProperties);
            m_xLayerContentDescriber->describeLayer(xHandler, aPropInfoList);
        }
    }
    else
    {
        OSL_TRACE("Could not create com.sun.star.configuration.backend.LayerContentDescriber Service");
    }
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL GconfLayer::getTimestamp(void)
    throw (uno::RuntimeException)
{
    return m_aTimestamp;
}
