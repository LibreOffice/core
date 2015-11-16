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

#include <ManifestWriter.hxx>
#include <ManifestExport.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>

#include <osl/diagnose.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::packages;
using namespace ::com::sun::star::xml::sax;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

ManifestWriter::ManifestWriter( const Reference < XComponentContext > & xContext )
: m_xContext ( xContext )
{
}
ManifestWriter::~ManifestWriter()
{
}

// XManifestWriter methods
void SAL_CALL ManifestWriter::writeManifestSequence( const Reference< XOutputStream >& rStream, const Sequence< Sequence< PropertyValue > >& rSequence )
        throw (RuntimeException, std::exception)
{
    Reference < XWriter > xSource = Writer::create( m_xContext );
    xSource->setOutputStream ( rStream );
    try {
        ManifestExport( xSource, rSequence);
    }
    catch( SAXException& )
    {
        throw RuntimeException( THROW_WHERE );
    }
}

// Component methods
Reference < XInterface > SAL_CALL ManifestWriter_createInstance( Reference< XMultiServiceFactory > const & rServiceFactory )
{
    return *new ManifestWriter( comphelper::getComponentContext(rServiceFactory) );
}

OUString ManifestWriter::static_getImplementationName()
{
    return OUString ( "com.sun.star.packages.manifest.comp.ManifestWriter" );
}

Sequence < OUString > ManifestWriter::static_getSupportedServiceNames()
{
    Sequence<OUString> aNames { "com.sun.star.packages.manifest.ManifestWriter" };
    return aNames;
}

OUString ManifestWriter::getImplementationName()
    throw (RuntimeException, std::exception)
{
    return static_getImplementationName();
}

sal_Bool SAL_CALL ManifestWriter::supportsService(OUString const & rServiceName)
    throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}
Sequence < OUString > ManifestWriter::getSupportedServiceNames()
    throw (RuntimeException, std::exception)
{
    return static_getSupportedServiceNames();
}
Reference < XSingleServiceFactory > ManifestWriter::createServiceFactory( Reference < XMultiServiceFactory > const & rServiceFactory )
{
    return cppu::createSingleFactory (rServiceFactory,
                                           static_getImplementationName(),
                                           ManifestWriter_createInstance,
                                           static_getSupportedServiceNames());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
