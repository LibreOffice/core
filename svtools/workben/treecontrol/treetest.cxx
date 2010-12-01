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

#include <stdio.h>
#include <wchar.h>

#include <cppuhelper/bootstrap.hxx>

#include <osl/file.hxx>
#include <osl/process.h>

#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/tree/XMutableTreeDataModel.hpp>
#include <com/sun/star/awt/tree/XTreeControl.hpp>
#include <com/sun/star/awt/tree/XTreeExpansionListener.hpp>
#include <com/sun/star/awt/tree/XTreeEditListener.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/view/SelectionType.hpp>

#include <tools/urlobj.hxx>
#include <vcl/image.hxx>
#include <vcl/graph.hxx>

#include <cppuhelper/implbase2.hxx>

#include <string.h>
#include <rtl/ref.hxx>

#include "imagemgr.hxx"

using rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::bridge;
using namespace com::sun::star::frame;
using namespace com::sun::star::registry;
using namespace com::sun::star::awt;
using namespace com::sun::star::awt::tree;
using namespace com::sun::star::container;
using namespace com::sun::star::view;
using namespace com::sun::star::util;

class DirectoryTree : public ::cppu::WeakImplHelper2< XTreeExpansionListener, XTreeEditListener >
{
public:
    DirectoryTree( const Reference< XComponentContext >& xComponentContext );
    virtual ~DirectoryTree();

    void fillNode( const Reference< XMutableTreeNode >& xNode );
    void display( const OUString& rURL );

    // XTreeExpansionListener
    virtual void SAL_CALL requestChildNodes( const TreeExpansionEvent& Event ) throw (RuntimeException);
    virtual void SAL_CALL treeExpanding( const TreeExpansionEvent& Event ) throw (ExpandVetoException, RuntimeException);
    virtual void SAL_CALL treeCollapsing( const TreeExpansionEvent& Event ) throw (ExpandVetoException, RuntimeException);
    virtual void SAL_CALL treeExpanded( const TreeExpansionEvent& Event ) throw (RuntimeException);
    virtual void SAL_CALL treeCollapsed( const TreeExpansionEvent& Event ) throw (RuntimeException);

    // XTreeEditListener
    virtual void SAL_CALL nodeEditing( const Reference< XTreeNode >& Node ) throw (VetoException, RuntimeException);
    virtual void SAL_CALL nodeEdited( const Reference< XTreeNode >& Node, const OUString& NewText ) throw (RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (RuntimeException);

    Reference< XMultiComponentFactory > mxMultiComponentFactoryServer;
    Reference< XComponentContext > mxComponentContext;
    Reference< XTreeControl > mxTreeControl;
    Reference< XMutableTreeDataModel > mxTreeDataModel;
};

DirectoryTree::DirectoryTree( const Reference< XComponentContext >& xComponentContext )
: mxComponentContext( xComponentContext )
, mxMultiComponentFactoryServer( xComponentContext->getServiceManager() )
{
}

DirectoryTree::~DirectoryTree()
{
}

void DirectoryTree::display( const OUString& rURL )
{
    // some property names for later use
    const OUString sPositionX( RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) );
    const OUString sPositionY( RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) );
    const OUString sWidth( RTL_CONSTASCII_USTRINGPARAM( "Width" ) );
    const OUString sHeight( RTL_CONSTASCII_USTRINGPARAM( "Height" ) );
    const OUString sDataModel( RTL_CONSTASCII_USTRINGPARAM( "DataModel" ) );
    const OUString sSelectionType( RTL_CONSTASCII_USTRINGPARAM( "SelectionType" ) );
    const OUString sShowsRootHandles( RTL_CONSTASCII_USTRINGPARAM( "ShowsRootHandles" ) );
    const OUString sShowsHandles( RTL_CONSTASCII_USTRINGPARAM( "ShowsHandles" ) );
    const OUString sRootDisplayed( RTL_CONSTASCII_USTRINGPARAM( "RootDisplayed" ) );
    const OUString sEditable( RTL_CONSTASCII_USTRINGPARAM( "Editable" ) );
    const OUString sTitle( RTL_CONSTASCII_USTRINGPARAM( "Title" ) );
    const OUString sRowHeight( RTL_CONSTASCII_USTRINGPARAM( "RowHeight" ) );

    // first create a data model for our tree control
    mxTreeDataModel = Reference< XMutableTreeDataModel >(
        mxMultiComponentFactoryServer->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.tree.MutableTreeDataModel" ) ),
            mxComponentContext ), UNO_QUERY_THROW );

    // now fill it with some sample data
    Reference< XMutableTreeNode > xNode( mxTreeDataModel->createNode( Any( rURL ), false ), UNO_QUERY_THROW );
    xNode->setDataValue( Any( rURL ) );
    xNode->setExpandedGraphicURL( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:graphicrepository/sd/res/triangle_down.png" ) ) );
    xNode->setCollapsedGraphicURL( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:graphicrepository/sd/res/triangle_right.png" ) ) );

    fillNode( xNode );
    mxTreeDataModel->setRoot( xNode );

    // now create the dialog
    Reference< XControlModel > xDialogModel(
        mxMultiComponentFactoryServer->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlDialogModel" ) ),
            mxComponentContext ), UNO_QUERY_THROW );

    Reference< XPropertySet > xDialogPropertySet( xDialogModel, UNO_QUERY_THROW );
    xDialogPropertySet->setPropertyValue( sPositionX,   Any( sal_Int32(50) ) );
    xDialogPropertySet->setPropertyValue( sPositionY,   Any( sal_Int32(50) ) );
    xDialogPropertySet->setPropertyValue( sWidth,       Any( sal_Int32(256) ) );
    xDialogPropertySet->setPropertyValue( sHeight,      Any( sal_Int32(256) ) );
    xDialogPropertySet->setPropertyValue( sTitle,       Any( OUString( RTL_CONSTASCII_USTRINGPARAM( "Tree Control Test" ) ) ) );

    Reference< XMultiServiceFactory > xDialogMSF( xDialogModel, UNO_QUERY_THROW );

    // now create our tree control
    Reference< XControlModel > xTreeControlModel(
        xDialogMSF->createInstance(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.tree.TreeControlModel" ) ) ),
            UNO_QUERY_THROW );

    Reference< XPropertySet > XTreeControlModelSet( xTreeControlModel, UNO_QUERY_THROW );

    XTreeControlModelSet->setPropertyValue( sSelectionType, Any( SelectionType_NONE ) );
    XTreeControlModelSet->setPropertyValue( sPositionX,     Any( sal_Int32(3) ) );
    XTreeControlModelSet->setPropertyValue( sPositionY,     Any( sal_Int32(3) ) );
    XTreeControlModelSet->setPropertyValue( sWidth,         Any( sal_Int32(253) ) );
    XTreeControlModelSet->setPropertyValue( sHeight,        Any( sal_Int32(253) ) );
    XTreeControlModelSet->setPropertyValue( sDataModel,     Any( mxTreeDataModel ) );
    XTreeControlModelSet->setPropertyValue( sShowsRootHandles,Any( sal_False ) );
    XTreeControlModelSet->setPropertyValue( sShowsHandles,  Any( sal_False ) );
    XTreeControlModelSet->setPropertyValue( sRootDisplayed, Any( sal_True ) );
    XTreeControlModelSet->setPropertyValue( sEditable,      Any( sal_True ) );
//  XTreeControlModelSet->setPropertyValue( sRowHeight,     Any( sal_Int32( 12 ) ) );

    Reference< XNameContainer > xDialogModelContainer( xDialogModel, UNO_QUERY_THROW );

    const OUString sTreeControlName( RTL_CONSTASCII_USTRINGPARAM( "tree" ) );

    xDialogModelContainer->insertByName( sTreeControlName, Any( xTreeControlModel ) );

    // now create the peers
    Reference< XControl > xDialogControl(
        mxMultiComponentFactoryServer->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlDialog" ) ),
            mxComponentContext ), UNO_QUERY_THROW );

    xDialogControl->setModel( xDialogModel );

    Reference< XToolkit > xToolkit(
        mxMultiComponentFactoryServer->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.Toolkit" ) ),
            mxComponentContext ), UNO_QUERY_THROW );
    xDialogControl->createPeer( xToolkit, 0 );

    // get the peers of the sub controls from the dialog peer container
    Reference< XControlContainer > xDialogContainer( xDialogControl, UNO_QUERY_THROW );
    mxTreeControl = Reference< XTreeControl >( xDialogContainer->getControl( sTreeControlName ), UNO_QUERY_THROW );

    Reference< XTreeExpansionListener > xListener( static_cast<XWeak*>(this), UNO_QUERY_THROW );
    mxTreeControl->addTreeExpansionListener( xListener );
    Reference< XDialog > xDialog( xDialogControl, UNO_QUERY_THROW );
    xDialog->execute();
}

void DirectoryTree::fillNode( const Reference< XMutableTreeNode >& xNode )
{
    if( xNode->getChildCount() == 0 )
    {
        OUString sParentPath;
        xNode->getDataValue() >>= sParentPath;

        osl::Directory aDirectory( sParentPath );
        if( aDirectory.open() == osl::Directory::E_None )
        {
            osl::DirectoryItem aItem;
            osl::FileStatus fs( FileStatusMask_Type | FileStatusMask_FileURL);
            while( aDirectory.getNextItem( aItem, 0xffffffff ) == osl::Directory::E_None )
            {
                if (aItem.getFileStatus(fs) == osl::FileBase::E_None)
                {
                    bool bDirectory = fs.getFileType() == osl::FileStatus::Directory;
                    Reference< XMutableTreeNode > xChildNode( mxTreeDataModel->createNode( Any( fs.getFileName() ), bDirectory ), UNO_QUERY_THROW );
                    xChildNode->setDataValue( Any( fs.getFileURL() ) );
                    if( bDirectory )
                    {
                        xChildNode->setExpandedGraphicURL( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:graphicrepository/sd/res/triangle_down.png" ) ) );
                        xChildNode->setCollapsedGraphicURL( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:graphicrepository/sd/res/triangle_right.png" ) ) );
                    }
                    else
                    {
                        xChildNode->setNodeGraphicURL( OUString( RTL_CONSTASCII_USTRINGPARAM("private:graphicrepository/sw/imglst/nc20010.png") ) );
                    }
                    xNode->appendChild( xChildNode );
                }
            }
        }
    }
}

// XTreeExpansionListener
void SAL_CALL DirectoryTree::requestChildNodes( const TreeExpansionEvent& rEvent ) throw (RuntimeException)
{
    if( rEvent.Node.is() && rEvent.Node->hasChildsOnDemand() )
    {
        Reference< XMutableTreeNode > xNode( rEvent.Node, UNO_QUERY );
        if( xNode.is() )
        {
            fillNode( xNode );
            xNode->setHasChildsOnDemand( sal_False );
        }
    }
}

void SAL_CALL DirectoryTree::treeExpanding( const TreeExpansionEvent& /*rEvent*/ ) throw (ExpandVetoException, RuntimeException)
{
}

void SAL_CALL DirectoryTree::treeCollapsing( const TreeExpansionEvent&  ) throw (ExpandVetoException, RuntimeException)
{
}

void SAL_CALL DirectoryTree::treeExpanded( const TreeExpansionEvent&  ) throw (RuntimeException)
{
}

void SAL_CALL DirectoryTree::treeCollapsed( const TreeExpansionEvent& /*rEvent*/ ) throw (RuntimeException)
{
/*
    if( rEvent.Node != mxTreeDataModel->getRoot() )
    {
        Reference< XMutableTreeNode > xNode( rEvent.Node, UNO_QUERY );
        if( xNode.is() )
        {
            while( xNode->getChildCount() )
                xNode->removeChildByIndex(0);
            xNode->setHasChildsOnDemand( sal_True );
        }
    }
*/
}

// XTreeEditListener
void SAL_CALL DirectoryTree::nodeEditing( const Reference< XTreeNode >&  ) throw (VetoException, RuntimeException)
{
}

void SAL_CALL DirectoryTree::nodeEdited( const Reference< XTreeNode >& , const OUString&  ) throw (RuntimeException)
{
}

// XEventListener
void SAL_CALL DirectoryTree::disposing( const ::com::sun::star::lang::EventObject&  ) throw (RuntimeException)
{
}

//============================================================================
int SAL_CALL main( int argc, char **argv )
{
    OUString sConnectionString(RTL_CONSTASCII_USTRINGPARAM("uno:socket,host=localhost,port=5678;urp;StarOffice.ServiceManager"));

    if (argc < 2)
    {
        printf("using: treetest <directory> [<uno_connection_url>]\n\n"
#ifdef WNT
                "example: treetest  \"c:\" \"uno:socket,host=localhost,port=5678;urp;StarOffice.ServiceManager\"\n");
#else
                "example: treetest  \"/etc\" \"uno:socket,host=localhost,port=5678;urp;StarOffice.ServiceManager\"\n");
#endif
        exit(1);
    }
     if (argc == 3)
    {
        sConnectionString = OUString::createFromAscii(argv[2]);
    }

    // Creates a simple registry service instance.
    Reference< XSimpleRegistry > xSimpleRegistry(::cppu::createSimpleRegistry() );

    // Connects the registry to a persistent data source represented by an URL.
    xSimpleRegistry->open(
        OUString( RTL_CONSTASCII_USTRINGPARAM("treetest.rdb") ), sal_True, sal_False );

    /* Bootstraps an initial component context with service manager upon a given
       registry. This includes insertion of initial services:
       - (registry) service manager, shared lib loader,
       - simple registry, nested registry,
       - implementation registration
       - registry typedescription provider, typedescription manager (also
         installs it into cppu core)
    */
    Reference< XComponentContext > xComponentContext(
        ::cppu::bootstrap_InitialComponentContext( xSimpleRegistry ) );

    /* Gets the service manager instance to be used (or null). This method has
       been added for convenience, because the service manager is a often used
       object.
    */
    Reference< XMultiComponentFactory > xMultiComponentFactoryClient(
        xComponentContext->getServiceManager() );

    /* Creates an instance of a component which supports the services specified
       by the factory.
    */
    Reference< XInterface > xInterface =
        xMultiComponentFactoryClient->createInstanceWithContext(
            OUString::createFromAscii( "com.sun.star.bridge.UnoUrlResolver" ),
            xComponentContext );

    Reference< XUnoUrlResolver > resolver( xInterface, UNO_QUERY );

    // Resolves the component context from the office, on the uno URL given by argv[1].
    try
    {
        xInterface = Reference< XInterface >(
            resolver->resolve( sConnectionString ), UNO_QUERY );
    }
    catch ( Exception& e )
    {
        printf("Error: cannot establish a connection using '%s':\n       %s\n",
               OUStringToOString(sConnectionString, RTL_TEXTENCODING_ASCII_US).getStr(),
               OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        exit(1);
    }

    // gets the server component context as property of the office component factory
    Reference< XPropertySet > xPropSet( xInterface, UNO_QUERY );
    xPropSet->getPropertyValue( OUString::createFromAscii("DefaultContext") ) >>= xComponentContext;

    // gets the service manager from the office
    Reference< XMultiComponentFactory > xMultiComponentFactoryServer(
        xComponentContext->getServiceManager() );

    try
    {
        OUString ustrFileURL;
        osl::FileBase::getFileURLFromSystemPath( OUString::createFromAscii(argv[1]), ustrFileURL );

        rtl::Reference< DirectoryTree >(
            new DirectoryTree( xComponentContext ) )->display(ustrFileURL);
    }
    catch( Exception& e )
    {
        printf("Error: exception caught during test:\n       %s\n",
               OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        exit(1);
    }

    // dispose the local service manager
    Reference< XComponent >::query( xMultiComponentFactoryClient )->dispose();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
