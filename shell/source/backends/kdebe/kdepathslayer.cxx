/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: kdepathslayer.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_shell.hxx"
#include "kdepathslayer.hxx"
#include <vcl/kde_headers.h>
#include <com/sun/star/configuration/backend/PropertyInfo.hpp>
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERCONTENTDESCIBER_HPP_
#include <com/sun/star/configuration/backend/XLayerContentDescriber.hpp>
#endif
#include <osl/security.hxx>
#include <osl/file.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#define SPACE      ' '

//==============================================================================

KDEPathsLayer::KDEPathsLayer(const uno::Reference<uno::XComponentContext>& xContext)
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

void SAL_CALL KDEPathsLayer::readData( const uno::Reference<backend::XLayerHandler>& xHandler)
    throw ( backend::MalformedDataException, lang::NullPointerException,
            lang::WrappedTargetException, uno::RuntimeException)
{
    if( ! m_xLayerContentDescriber.is() )
    {
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
            "Could not create com.sun.star.configuration.backend.LayerContentDescriber Service"
        ) ), static_cast < backend::XLayer * > (this) );
    }

    uno::Sequence<backend::PropertyInfo> aPropInfoList(1);
    sal_Int32 nProperties = 0;

    QString aDocumentsDir( "file:" );
    ::rtl::OUString sDocumentsDir;

    aDocumentsDir += KGlobalSettings::documentPath();
    if ( aDocumentsDir.endsWith(QChar('/')) )
        aDocumentsDir.truncate ( aDocumentsDir.length() - 1 );
    sDocumentsDir = (const sal_Unicode *) aDocumentsDir.ucs2();

    aPropInfoList[nProperties].Name = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Office.Paths/Variables/Work") );
    aPropInfoList[nProperties].Type = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM( "string" ) );
    aPropInfoList[nProperties].Protected = sal_False;
    aPropInfoList[nProperties++].Value = uno::makeAny( sDocumentsDir );

    if( nProperties > 0 )
    {
        aPropInfoList.realloc(nProperties);
        m_xLayerContentDescriber->describeLayer(xHandler, aPropInfoList);
    }
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL KDEPathsLayer::getTimestamp(void)
    throw (uno::RuntimeException)
{
    // Return the value as timestamp to avoid regenerating the binary cache
    // on each office launch.

    ::rtl::OUString sTimeStamp,
                    sep( RTL_CONSTASCII_USTRINGPARAM( "$" ) );

    QString aDocumentsDir;
    aDocumentsDir = KGlobalSettings::documentPath();

    sTimeStamp += (const sal_Unicode *) aDocumentsDir.ucs2();

    return sTimeStamp;
}
