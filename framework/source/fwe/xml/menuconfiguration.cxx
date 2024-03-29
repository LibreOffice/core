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

#include <menuconfiguration.hxx>

#include <addonmenu.hxx>
#include <utility>
#include <xml/menudocumenthandler.hxx>
#include <xml/saxnamespacefilter.hxx>

#include <uielement/rootitemcontainer.hxx>

#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <cppuhelper/exc_hlp.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;

namespace framework
{

MenuConfiguration::MenuConfiguration( css::uno::Reference< css::uno::XComponentContext > xContext )
:   m_xContext(std::move( xContext ))
{
}

MenuConfiguration::~MenuConfiguration()
{
}

Reference< XIndexAccess > MenuConfiguration::CreateMenuBarConfigurationFromXML(
    Reference< XInputStream > const & rInputStream )
{
    Reference< XParser > xParser = Parser::create( m_xContext );

    // connect stream to input stream to the parser
    InputSource aInputSource;

    aInputSource.aInputStream = rInputStream;

    // create menu bar
    Reference< XIndexContainer > xItemContainer( new RootItemContainer() );

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
        css::uno::Any anyEx = cppu::getCaughtException();
        throw WrappedTargetException( e.Message, Reference< XInterface >(), anyEx );
    }
    catch( const SAXException& e )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        SAXException aWrappedSAXException;

        if ( !( e.WrappedException >>= aWrappedSAXException ))
            throw WrappedTargetException( e.Message, Reference< XInterface >(), anyEx );
        else
            throw WrappedTargetException( aWrappedSAXException.Message, Reference< XInterface >(), e.WrappedException );
    }
    catch( const css::io::IOException& e )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw WrappedTargetException( e.Message, Reference< XInterface >(), anyEx );
    }
}

void MenuConfiguration::StoreMenuBarConfigurationToXML(
    Reference< XIndexAccess > const & rMenuBarConfiguration,
    Reference< XOutputStream > const & rOutputStream, bool bIsMenuBar )
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
        css::uno::Any anyEx = cppu::getCaughtException();
        throw WrappedTargetException( e.Message, Reference< XInterface >(), anyEx );
    }
    catch ( const SAXException& e )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw WrappedTargetException( e.Message, Reference< XInterface >(), anyEx );
    }
    catch ( const css::io::IOException& e )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw WrappedTargetException( e.Message, Reference< XInterface >(), anyEx );
    }
}

void* MenuAttributes::CreateAttribute(const OUString& rFrame, const OUString& rImageIdStr)
{
    MenuAttributes* pAttributes = new MenuAttributes(rFrame, rImageIdStr);
    pAttributes->acquire();
    return pAttributes;
}

void* MenuAttributes::CreateAttribute(const css::uno::WeakReference<css::frame::XDispatchProvider>& rDispatchProvider)
{
    MenuAttributes* pAttributes = new MenuAttributes(rDispatchProvider);
    pAttributes->acquire();
    return pAttributes;
}

void MenuAttributes::ReleaseAttribute(void* nAttributePtr)
{
    if (!nAttributePtr)
        return;
    MenuAttributes* pAttributes = static_cast<MenuAttributes*>(nAttributePtr);
    pAttributes->release();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
