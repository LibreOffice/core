/*************************************************************************
 *
 *  $RCSfile: localelayer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-20 14:09:54 $
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

#ifndef _LOCALELAYER_HXX_
#include "localelayer.hxx"
#endif

//------------------------------------------------------------------------------

LocaleLayer::LocaleLayer( const uno::Sequence<backend::PropertyInfo>& aPropInfoList,
    const rtl::OUString& aTimestamp,
    const uno::Reference<uno::XComponentContext>& xContext)
  : m_aPropInfoList(aPropInfoList), m_aTimestamp(aTimestamp)
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

void SAL_CALL LocaleLayer::readData(
    const uno::Reference<backend::XLayerHandler>& xHandler)
    throw ( backend::MalformedDataException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{

    if (m_xLayerContentDescriber.is())
    {
        m_xLayerContentDescriber->describeLayer(xHandler, m_aPropInfoList);
    }
    else
    {
        OSL_TRACE("Could not create com.sun.star.configuration.backend.LayerContentDescriber Service");
    }
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL LocaleLayer::getTimestamp(void)
    throw (uno::RuntimeException)
{
    return m_aTimestamp;
}

//------------------------------------------------------------------------------
