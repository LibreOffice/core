/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: menuconfiguration.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 14:30:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_MENUCONFIGURATION_HXX_
#include <xml/menuconfiguration.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_BMKMENU_HXX_
#include <classes/bmkmenu.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_ADDONMENU_HXX_
#include <classes/addonmenu.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_MENUDOCUMENTHANDLER_HXX_
#include <xml/menudocumenthandler.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_SAXNAMESPACEFILTER_HXX_
#include <xml/saxnamespacefilter.hxx>
#endif

#ifndef _FRAMEWORK_SERVICES_LAYOUTMANAGER_HXX_
#include <services/layoutmanager.hxx>
#endif

#ifndef _FRAMEWORK_UIELEMENT_ROOTITEMCONTAINER_HXX_
#include <uielement/rootitemcontainer.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;

namespace framework
{

BOOL MenuConfiguration::IsPickListItemId( USHORT nId )
{
    return (( START_ITEMID_PICKLIST <= nId ) && ( nId <= END_ITEMID_PICKLIST ));
}

BOOL MenuConfiguration::IsWindowListItemId( USHORT nId )
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
    Reference< XParser > xParser( m_rxServiceManager->createInstance(
                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser" ))),
                                  UNO_QUERY);

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
            ::rtl::OUString::createFromAscii( "com.sun.star.xml.sax.Writer" )), UNO_QUERY) ;

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
