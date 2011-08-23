/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _RTL_WSTRING
#include <rtl/ustring.h>
#endif

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>

#include <unotools/processfactory.hxx>

#include <transuno.hxx>

/*========================================================================
 *
 * SvBindingTransport_Impl internals.
 *
 *======================================================================*/
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;

namespace binfilter {

typedef com::sun::star::lang::XMultiServiceFactory factory_type;
typedef com::sun::star::uno::XInterface            interface_type;


/*
 * S2U.
 */
inline rtl::OUString S2U (const sal_Char *ascii)
{
    return rtl::OUString::createFromAscii (ascii);
}

/*========================================================================
 *
 * SvBindingTransport_Impl implementation.
 *
 *======================================================================*/
/*
 * getProcessServiceFactory.
 */
com::sun::star::uno::Reference<com::sun::star::uno::XInterface>
SvBindingTransport_Impl::getProcessServiceFactory (void)
{
    return com::sun::star::uno::Reference<com::sun::star::uno::XInterface>(
        utl::getProcessServiceFactory(), com::sun::star::uno::UNO_QUERY_THROW);
}

/*
 * createContent.
 */
com::sun::star::uno::Reference<XContent>
SvBindingTransport_Impl::createContent (const rtl::OUString &rUrl)
{
    com::sun::star::uno::Reference<factory_type>
        xFactory (getProcessServiceFactory(), com::sun::star::uno::UNO_QUERY);
    if (!xFactory.is())
        return com::sun::star::uno::Reference<XContent>();

    com::sun::star::uno::Reference<interface_type>
        xBroker (xFactory->createInstance (
            S2U ("com.sun.star.ucb.UniversalContentBroker")));
    if (!xBroker.is())
        return com::sun::star::uno::Reference<XContent>();

    com::sun::star::uno::Reference<XContentProvider>
        xProvider (xBroker, com::sun::star::uno::UNO_QUERY);
    if (!xProvider.is())
        return com::sun::star::uno::Reference<XContent>();

    com::sun::star::uno::Reference<XContentIdentifierFactory>
        xIdentProv (xProvider, com::sun::star::uno::UNO_QUERY);
    if (!xIdentProv.is())
        return com::sun::star::uno::Reference<XContent>();

    com::sun::star::uno::Reference<XContentIdentifier>
        xIdent (xIdentProv->createContentIdentifier (rUrl));
    if (!xIdent.is())
        return com::sun::star::uno::Reference<XContent>();

    return xProvider->queryContent (xIdent);
}

/*
 * getProperties.
 */
Any SvBindingTransport_Impl::getProperties ( const Reference<XCommandProcessor> &rxProcessor, const Sequence<Property> &rProperties)
{
    Any aResult;
    if (rxProcessor.is() && rProperties.getLength())
    {
        try
        {
            com::sun::star::uno::Reference<XCommandEnvironment> xEnvironment;
            Command aCommand;
            aCommand.Name     = ::rtl::OUString::createFromAscii("getPropertyValues");
            aCommand.Handle   = -1; /* unknown */
            aCommand.Argument <<= rProperties;
            aResult = rxProcessor->execute( aCommand, 0, xEnvironment );
        }
        catch (com::sun::star::uno::Exception &)
        {
        }
    }

    return aResult;
}

/*
 * setProperties.
 */
void SvBindingTransport_Impl::setProperties ( const Reference<XCommandProcessor> &rxProcessor, const Sequence<PropertyValue> &rProperties)
{
    if (rxProcessor.is() && rProperties.getLength())
    {
        try
        {
            Reference<XCommandEnvironment> xEnvironment;
            Command aCommand;
            aCommand.Name     = ::rtl::OUString::createFromAscii("setPropertyValues");
            aCommand.Handle   = -1; /* unknown */
            aCommand.Argument <<= rProperties;
            rxProcessor->execute( aCommand, 0, xEnvironment );
        }
        catch (com::sun::star::uno::Exception &)
        {
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
