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

#include "root.hxx"

#include <cassert>

#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/XMessageBox.hpp>
#include <com/sun/star/awt/MessageBoxButtons.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/awt/XMessageBoxFactory.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>

#include "helper.hxx"
#include "import.hxx"
#include "timer.hxx"
#include "translate.hxx"

namespace layoutimpl
{

using namespace css;
using ::rtl::OUString;

LayoutRoot::LayoutRoot( const uno::Reference< lang::XMultiServiceFactory >& xFactory )
    : mbDisposed( sal_False )
    , mxFactory( xFactory )
    , mpListeners( NULL )
    , mpToplevel( NULL )
{
    if ( !xFactory.is() )
        throw uno::RuntimeException();
    mxLayoutUnit = uno::Reference< awt::XLayoutUnit >( new LayoutUnit() );
}

LayoutRoot::~LayoutRoot()
{
// TODO: we want to delete the top level LayoutWidget...
    ::osl::MutexGuard aGuard( maMutex );
    if ( !mbDisposed )
    {
        try
        {
            m_refCount++; // inhibit multiple destruction
            dispose();
        }
        catch( uno::Exception& )
        {
        }
    }
}

void ShowMessageBox( uno::Reference< lang::XMultiServiceFactory > const& xFactory, uno::Reference< awt::XToolkit > xToolkit, OUString const& aTitle, OUString const& aMessage )
{
    uno::Reference< uno::XInterface > iDesktop = xFactory->createInstance
        ( OUString::createFromAscii( "com.sun.star.frame.Desktop" ) );
    uno::Reference< frame::XDesktop > xDesktop ( iDesktop, uno::UNO_QUERY );
    uno::Reference< frame::XFrame > xFrame ( xDesktop->getCurrentFrame() );
    uno::Reference< awt::XWindow > xContainerWindow( xFrame->getContainerWindow() );
    uno::Reference< awt::XWindowPeer > xWindowPeer( xContainerWindow, uno::UNO_QUERY_THROW );
    uno::Reference< awt::XMessageBoxFactory > xMessageBoxFactory( xToolkit, uno::UNO_QUERY );

    awt::Rectangle aRectangle;
    uno::Reference< awt::XMessageBox > xMessageBox
        = xMessageBoxFactory->createMessageBox
        ( xWindowPeer, aRectangle, OUString::createFromAscii( "errorbox" ),
          awt::MessageBoxButtons::BUTTONS_OK, aTitle, aMessage );

    if ( xMessageBox.is() )
        xMessageBox->execute();
    //FIXME: exceptions not caught and printed at top level??
    //else
    //printf( "%s\n", OUSTRING_CSTR( aMessage ) );
}

void LayoutRoot::error( OUString const& message )
{
    OSL_TRACE( "%s\n", OUSTRING_CSTR( message ) );
    ShowMessageBox( mxFactory, mxToolkit,
                    OUString::createFromAscii( "Fatal error" ),
                    message );
    throw uno::RuntimeException( message, uno::Reference< uno::XInterface >() );
}

// XInitialization
void SAL_CALL LayoutRoot::initialize( const uno::Sequence< uno::Any >& aArguments )
    throw ( uno::Exception,
            uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( mbDisposed )
        throw lang::DisposedException();

    if ( mxContainer.is() ) // only 1 init ...
        throw uno::Exception();

    if ( !aArguments.getLength() )
        throw lang::IllegalArgumentException();

    OSL_ENSURE( aArguments.getLength() == 1, "Wrong arg count\n" );

    OUString aXMLName;
    if ( !( aArguments[0] >>= aXMLName ) )
        throw lang::IllegalArgumentException();

    uno::Reference< xml::sax::XParser > xParser
        ( mxFactory->createInstance(
            OUString::createFromAscii( "com.sun.star.xml.sax.Parser" ) ),
          uno::UNO_QUERY );
    OSL_ASSERT( xParser.is() );
    if (! xParser.is())
    {
        throw uno::RuntimeException(
            OUString::createFromAscii( "cannot create sax-parser component" ),
            uno::Reference< uno::XInterface >() );
    }

    // FIXME: quite possibly we want to pass this in ...
    uno::Reference< awt::XToolkit > xToolkit;

    mxToolkit = uno::Reference< awt::XToolkit >(
        mxFactory->createInstance(
            OUString::createFromAscii( "com.sun.star.awt.Toolkit" ) ),
        uno::UNO_QUERY );

    if ( !mxToolkit.is() )
        throw uno::RuntimeException(
            OUString::createFromAscii( "failed to create toolkit!" ),
            uno::Reference< uno::XInterface >() );

    OUString aXMLFile = readRightTranslation( aXMLName );
    uno::Reference< io::XInputStream > xStream = getFileAsStream( aXMLFile );
    if (! xStream.is() )
        error( OUString::createFromAscii( "Installation problem: cannot find XML file:" ) + aXMLName );

    // error handler, entity resolver omitted

    ImportContext *pCtx = new ImportContext( *this );

    uno::Reference< xml::input::XRoot > xRoot( pCtx );
    uno::Sequence < uno::Any > aArgs( 1 );
    aArgs[0] <<= xRoot;
    uno::Reference< xml::sax::XDocumentHandler > xDocHandler
        (mxFactory->createInstanceWithArguments
         ( OUString::createFromAscii( "com.sun.star.xml.input.SaxDocumentHandler" ),
          aArgs ), uno::UNO_QUERY );

    if (! xDocHandler.is() )
        error( OUString::createFromAscii( "cannot find SAx handler for document type of:") + aXMLName );

    xParser->setDocumentHandler( xDocHandler );

    xml::sax::InputSource source;
    source.aInputStream = xStream;
    source.sSystemId = OUString::createFromAscii( "virtual file" );

    try
    {
        xParser->parseStream( source );
    }
    catch ( xml::sax::SAXParseException& e )
    {
        OUString c = OUString::createFromAscii( ":" );
        error( aXMLName
               + c + OUString::valueOf( e.LineNumber )
               + c + OUString::valueOf( e.ColumnNumber )
               + c + OUString::createFromAscii( "Sax parse error" ) );
    }
}

// XLayoutContainer
uno::Reference< awt::XLayoutContainer > LayoutRoot::getLayoutContainer() throw (uno::RuntimeException)
{
    return uno::Reference< awt::XLayoutContainer >();
}

// local helper ...
void LayoutRoot::addItem( const OUString &rName,
                          const uno::Reference< awt::XLayoutConstrains > &xRef )
{
    maItems[ rName ] = xRef;
}

// XNameAccess
uno::Any SAL_CALL LayoutRoot::getByName( const OUString &rName )
    throw ( container::NoSuchElementException,
            lang::WrappedTargetException,
            uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbDisposed )
        throw lang::DisposedException();

    uno::Reference< awt::XLayoutConstrains > xItem;
    ItemHash::iterator i = maItems.find( rName );
    if ( i != maItems.end() )
        xItem = i->second;
    return uno::makeAny( xItem );
}

sal_Bool SAL_CALL LayoutRoot::hasByName( const OUString &rName )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbDisposed ) throw lang::DisposedException();

    ItemHash::iterator i = maItems.find( rName );
    return i != maItems.end();
}

uno::Sequence< OUString > SAL_CALL LayoutRoot::getElementNames()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbDisposed ) throw lang::DisposedException();

    uno::Sequence< OUString > aNames( maItems.size() );
    sal_Int32 nPos = 0;

    for ( ItemHash::const_iterator it = maItems.begin();
          it != maItems.end(); it++ )
        aNames[ nPos++ ] = it->first;

    return aNames;
}

uno::Type SAL_CALL LayoutRoot::getElementType()
    throw ( uno::RuntimeException )
{
    return getCppuType( ( const uno::Reference< awt::XLayoutConstrains >* )NULL );
}

sal_Bool SAL_CALL LayoutRoot::hasElements()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( mbDisposed ) throw lang::DisposedException();

    return maItems.size() > 0;
}

// XComponent
void SAL_CALL LayoutRoot::dispose()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( mbDisposed ) throw lang::DisposedException();

    if ( mpListeners )
    {

        lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );
        mpListeners->disposeAndClear( aSource );
        delete mpListeners;
        mpListeners = NULL;
    }

    maItems.clear();
    mbDisposed = sal_True;
}

void SAL_CALL LayoutRoot::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( mbDisposed ) throw lang::DisposedException();

    if ( !mpListeners )
        mpListeners = new ::cppu::OInterfaceContainerHelper( maMutex );
    mpListeners->addInterface( xListener );
}

void SAL_CALL LayoutRoot::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( mbDisposed ) throw lang::DisposedException();

    if ( mpListeners )
        mpListeners->removeInterface( xListener );
}

// builder

LayoutWidget *LayoutRoot::create( OUString id, const OUString unoName, long attrbs,uno::Reference< awt::XLayoutContainer > xParent )
{
    LayoutWidget *pWidget = new LayoutWidget( mxToolkit, xParent, unoName, attrbs );
    if ( !mpToplevel )
    {
        mpToplevel = pWidget;
        mxWindow = uno::Reference< awt::XWindow >( pWidget->getPeer(), uno::UNO_QUERY );
        mxContainer = pWidget->mxContainer;
    }
    if ( pWidget->mxContainer.is() )
        pWidget->mxContainer->setLayoutUnit( mxLayoutUnit );
    if ( id.getLength() )
        maItems[ id ] = pWidget->getPeer();
    return pWidget;
}

#if 0
uno::Reference< awt::XLayoutConstrains > LayoutRoot::getToplevel()
{
    if ( mpToplevel )
        return mpToplevel->getPeer();
    return uno::Reference< awt::XLayoutConstrains > ();
}

uno::Reference< awt::XLayoutConstrains > LayoutRoot::getById( OUString id )
{
    uno::Reference< awt::XLayoutConstrains > rRef = 0;
    ItemHash::iterator it = maItems.find( id );
    if ( it != maItems.end() )
        rRef = it->second;
    return rRef;
}
#endif

LayoutWidget::LayoutWidget( uno::Reference< awt::XToolkit > xToolkit,
                            uno::Reference< awt::XLayoutContainer > xParent,
                            OUString unoName, long attrbs )
{
    while ( xParent.is() && !uno::Reference< awt::XWindow >( xParent, uno::UNO_QUERY ).is() )
    {
        uno::Reference< awt::XLayoutContainer > xContainer( xParent, uno::UNO_QUERY );
        assert( xContainer.is() );
        xParent = uno::Reference< awt::XLayoutContainer >( xContainer->getParent(), uno::UNO_QUERY );
    }

    mxWidget = WidgetFactory::createWidget( xToolkit, xParent, unoName, attrbs );
    assert( mxWidget.is() );
    mxContainer = uno::Reference< awt::XLayoutContainer >( mxWidget, uno::UNO_QUERY );
}

LayoutWidget::~LayoutWidget()
{
    /* should we dispose of the references...? */
    // at least of its children... Or should root?
}

bool LayoutWidget::addChild( LayoutWidget *pChild )
{
    if ( !mxContainer.is() )
        return false;

    try
    {
        mxContainer->addChild( pChild->mxWidget );
    }
    catch( awt::MaxChildrenException ex )
    {
        return false;
    }
    return true;
}

void LayoutWidget::setProperties( PropList const& rProps )
{
    ::layoutimpl::setProperties( mxWidget, rProps );
}

void LayoutWidget::setProperty( OUString const& attr, OUString const& value )
{
    ::layoutimpl::setProperty( mxWidget, attr, value );
}

void LayoutWidget::setChildProperties( LayoutWidget *pChild,
                                       PropList const& rProps )
{
    uno::Reference< beans::XPropertySet > xChildPeer;
    xChildPeer = mxContainer->getChildProperties( pChild->mxWidget );

    if ( xChildPeer.is() )
        ::layoutimpl::setProperties( xChildPeer, rProps );
}

} // namespace layoutimpl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
