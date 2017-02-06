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

#include <framework/menuconfiguration.hxx>

#include <framework/addonmenu.hxx>
#include <xml/menudocumenthandler.hxx>
#include <xml/saxnamespacefilter.hxx>
#include <services.h>

#include <uielement/rootitemcontainer.hxx>

#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;

namespace framework
{

MenuConfiguration::MenuConfiguration(
    const css::uno::Reference< css::uno::XComponentContext >& rxContext )
:   m_xContext( rxContext )
{
}

MenuConfiguration::~MenuConfiguration()
{
}

Reference< XIndexAccess > MenuConfiguration::CreateMenuBarConfigurationFromXML(
    Reference< XInputStream >& rInputStream )
{
    Reference< XParser > xParser = Parser::create( m_xContext );

    // connect stream to input stream to the parser
    InputSource aInputSource;

    aInputSource.aInputStream = rInputStream;

    // create menu bar
    Reference< XIndexContainer > xItemContainer( static_cast< cppu::OWeakObject *>( new RootItemContainer()), UNO_QUERY );

    // create namespace filter and set menudocument handler inside to support xml namespaces

    Reference< XDocumentHandler > xDocHandler( new OReadMenuDocumentHandler( xItemContainer ));

    Reference< XDocumentHandler > xFilter( new SaxNamespaceFilter( xDocHandler ));

    // connect parser and filter
    xParser->setDocumentHandler( xFilter );

    try
    {
        xParser->parseStream( aInputSource );
        return xItemContainer;
    }
    catch ( const RuntimeException& e )
    {
        throw WrappedTargetException( e.Message, Reference< XInterface >(), Any() );
    }
    catch( const SAXException& e )
    {
        SAXException aWrappedSAXException;

        if ( !( e.WrappedException >>= aWrappedSAXException ))
            throw WrappedTargetException( e.Message, Reference< XInterface >(), Any() );
        else
            throw WrappedTargetException( aWrappedSAXException.Message, Reference< XInterface >(), Any() );
    }
    catch( const css::io::IOException& e )
    {
        throw WrappedTargetException( e.Message, Reference< XInterface >(), Any() );
    }
}

void MenuConfiguration::StoreMenuBarConfigurationToXML(
    Reference< XIndexAccess >& rMenuBarConfiguration,
    Reference< XOutputStream >& rOutputStream, bool bIsMenuBar )
{
    Reference< XWriter > xWriter = Writer::create(m_xContext);
    xWriter->setOutputStream( rOutputStream );

    try
    {
        OWriteMenuDocumentHandler aWriteMenuDocumentHandler( rMenuBarConfiguration, xWriter, bIsMenuBar );
        aWriteMenuDocumentHandler.WriteMenuDocument();
    }
    catch ( const RuntimeException& e )
    {
        throw WrappedTargetException( e.Message, Reference< XInterface >(), Any() );
    }
    catch ( const SAXException& e )
    {
        throw WrappedTargetException( e.Message, Reference< XInterface >(), Any() );
    }
    catch ( const css::io::IOException& e )
    {
        throw WrappedTargetException( e.Message, Reference< XInterface >(), Any() );
    }
}

sal_uIntPtr MenuAttributes::CreateAttribute(const OUString& rFrame, const OUString& rImageIdStr)
{
    MenuAttributes* pAttributes = new MenuAttributes(rFrame, rImageIdStr);
    pAttributes->acquire();
    return reinterpret_cast<sal_uIntPtr>(pAttributes);
}

sal_uIntPtr MenuAttributes::CreateAttribute(const css::uno::WeakReference<css::frame::XDispatchProvider>& rDispatchProvider)
{
    MenuAttributes* pAttributes = new MenuAttributes(rDispatchProvider);
    pAttributes->acquire();
    return reinterpret_cast<sal_uIntPtr>(pAttributes);
}

void MenuAttributes::ReleaseAttribute(sal_uIntPtr nAttributePtr)
{
    if (!nAttributePtr)
        return;
    MenuAttributes* pAttributes = reinterpret_cast<MenuAttributes*>(nAttributePtr);
    pAttributes->release();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
