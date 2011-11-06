/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <framework/menuconfiguration.hxx>

#ifndef __FRAMEWORK_CLASSES_BMKMENU_HXX_
#include <framework/bmkmenu.hxx>
#endif
#include <framework/addonmenu.hxx>
#include <xml/menudocumenthandler.hxx>
#include <xml/saxnamespacefilter.hxx>
#include <services.h>

#ifndef _FRAMEWORK_UIELEMENT_ROOTITEMCONTAINER_HXX_
#include <uielement/rootitemcontainer.hxx>
#endif

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

