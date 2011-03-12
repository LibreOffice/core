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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <framework/menuconfiguration.hxx>

#include <framework/bmkmenu.hxx>
#include <framework/addonmenu.hxx>
#include <xml/menudocumenthandler.hxx>
#include <xml/saxnamespacefilter.hxx>
#include <services.h>

#include <uielement/rootitemcontainer.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/frame/XFrame.hpp>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;

namespace framework
{

sal_Bool MenuConfiguration::IsPickListItemId( sal_uInt16 nId )
{
    return (( START_ITEMID_PICKLIST <= nId ) && ( nId <= END_ITEMID_PICKLIST ));
}

sal_Bool MenuConfiguration::IsWindowListItemId( sal_uInt16 nId )
{
    return (( START_ITEMID_WINDOWLIST <= nId ) && ( nId <= END_ITEMID_WINDOWLIST ));
}


MenuConfiguration::MenuConfiguration(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rServiceManager )
:   m_rxServiceManager( rServiceManager )
{
}


MenuConfiguration::~MenuConfiguration()
{
}


Reference< XIndexAccess > MenuConfiguration::CreateMenuBarConfigurationFromXML(
    Reference< XInputStream >& rInputStream )
throw ( WrappedTargetException )
{
    Reference< XParser > xParser( m_rxServiceManager->createInstance(SERVICENAME_SAXPARSER),UNO_QUERY);

    // connect stream to input stream to the parser
    InputSource aInputSource;

    aInputSource.aInputStream = rInputStream;


    // create menu bar
    Reference< XIndexContainer > xItemContainer( static_cast< cppu::OWeakObject *>( new RootItemContainer()), UNO_QUERY );

    // create namespace filter and set menudocument handler inside to support xml namespaces

    // #110897# Reference< XDocumentHandler > xDocHandler( new OReadMenuDocumentHandler( xItemContainer ));
    Reference< XDocumentHandler > xDocHandler( new OReadMenuDocumentHandler( m_rxServiceManager, xItemContainer ));

    Reference< XDocumentHandler > xFilter( new SaxNamespaceFilter( xDocHandler ));

    // connect parser and filter
    xParser->setDocumentHandler( xFilter );

    try
    {
        xParser->parseStream( aInputSource );
        return Reference< XIndexAccess >( xItemContainer, UNO_QUERY );
    }
    catch ( RuntimeException& e )
    {
        throw WrappedTargetException( e.Message, Reference< XInterface >(), Any() );
    }
    catch( SAXException& e )
    {
        SAXException aWrappedSAXException;

        if ( !( e.WrappedException >>= aWrappedSAXException ))
            throw WrappedTargetException( e.Message, Reference< XInterface >(), Any() );
        else
            throw WrappedTargetException( aWrappedSAXException.Message, Reference< XInterface >(), Any() );
    }
    catch( ::com::sun::star::io::IOException& e )
    {
        throw WrappedTargetException( e.Message, Reference< XInterface >(), Any() );
    }
}

PopupMenu* MenuConfiguration::CreateBookmarkMenu(
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    const ::rtl::OUString& aURL )
throw ( ::com::sun::star::lang::WrappedTargetException )
{
    if ( aURL == BOOKMARK_NEWMENU )
        return new BmkMenu( rFrame, BmkMenu::BMK_NEWMENU );
    else if ( aURL == BOOKMARK_WIZARDMENU )
        return new BmkMenu( rFrame, BmkMenu::BMK_WIZARDMENU );
    else
        return NULL;
}

void MenuConfiguration::StoreMenuBarConfigurationToXML(
    Reference< XIndexAccess >& rMenuBarConfiguration,
    Reference< XOutputStream >& rOutputStream )
throw ( WrappedTargetException )
{
    Reference< XDocumentHandler > xWriter;

    xWriter = Reference< XDocumentHandler >( m_rxServiceManager->createInstance(
            SERVICENAME_SAXWRITER), UNO_QUERY) ;

    Reference< XActiveDataSource> xDataSource( xWriter , UNO_QUERY );
    xDataSource->setOutputStream( rOutputStream );

    try
    {
        OWriteMenuDocumentHandler aWriteMenuDocumentHandler( rMenuBarConfiguration, xWriter );
        aWriteMenuDocumentHandler.WriteMenuDocument();
    }
    catch ( RuntimeException& e )
    {
        throw WrappedTargetException( e.Message, Reference< XInterface >(), Any() );
    }
    catch ( SAXException& e )
    {
        throw WrappedTargetException( e.Message, Reference< XInterface >(), Any() );
    }
    catch ( ::com::sun::star::io::IOException& e )
    {
        throw WrappedTargetException( e.Message, Reference< XInterface >(), Any() );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
