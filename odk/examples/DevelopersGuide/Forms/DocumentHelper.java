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

/**************************************************************************/
import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.drawing.*;
import com.sun.star.frame.*;
import com.sun.star.form.*;
import com.sun.star.beans.*;
import com.sun.star.container.*;

/**************************************************************************/
/** provides a small wrapper around a document
*/
public class DocumentHelper
{
    /// the remote office context
    protected XComponentContext         m_remoteContext;
    /// the remote service manager
    protected XMultiServiceFactory      m_orb;
    protected XComponent                m_documentComponent;

    /* ------------------------------------------------------------------ */
    public XComponent getDocument( )
    {
        return m_documentComponent;
    }

    /* ------------------------------------------------------------------ */
    public XComponentContext getContext( )
    {
        return m_remoteContext;
    }

    /* ------------------------------------------------------------------ */
    public XMultiServiceFactory getOrb( )
    {
        return m_orb;
    }

    /* ------------------------------------------------------------------ */
    public DocumentHelper( XComponentContext xContext, XComponent document )
    {
        m_remoteContext = xContext;
        m_orb = (XMultiServiceFactory)UnoRuntime.queryInterface(
            XMultiServiceFactory.class, m_remoteContext.getServiceManager());
        m_documentComponent = document;
    }

    /* ------------------------------------------------------------------ */
    protected static XComponent implCreateBlankDocument( XComponentContext xCtx, String factoryURL ) throws com.sun.star.uno.Exception
    {
        XComponentLoader aLoader = (XComponentLoader)UnoRuntime.queryInterface(
            XComponentLoader.class,
            xCtx.getServiceManager().createInstanceWithContext(
                "com.sun.star.frame.Desktop", xCtx ));

        return UNO.queryComponent(
            aLoader.loadComponentFromURL( factoryURL, "_blank", 0, new PropertyValue[ 0 ] )
        );
    }

    /* ------------------------------------------------------------------ */
    public static DocumentHelper blankTextDocument( XComponentContext xCtx ) throws com.sun.star.uno.Exception
    {
        return blankDocument( xCtx, DocumentType.WRITER );
    }

    /* ------------------------------------------------------------------ */
    public static DocumentHelper blankDocument( XComponentContext xCtx, DocumentType eType ) throws com.sun.star.uno.Exception
    {
        XComponent document = implCreateBlankDocument( xCtx, getDocumentFactoryURL( eType ) );
        if ( eType == DocumentType.CALC )
            return new SpreadsheetDocument( xCtx, document );

        return new DocumentHelper( xCtx, document );
    }

    /* ------------------------------------------------------------------ */
    /** retrieves the current view of the document
        @return
            the view component, queried for the interface described by aInterfaceClass
    */
    public DocumentViewHelper getCurrentView( )
    {
        // get the model interface for the document
        XModel xDocModel = (XModel)UnoRuntime.queryInterface(XModel.class, m_documentComponent );
        // get the current controller for the document - as a controller is tied to a view,
        // this gives us the currently active view for the document.
        XController xController = xDocModel.getCurrentController();

        if ( classify() == DocumentType.CALC )
            return new SpreadsheetView( m_orb, this, xController );

        return new DocumentViewHelper( m_orb, this, xController );
    }

    /* ------------------------------------------------------------------ */
    /** creates a new form which is a child of the given form components container

        @param xParentContainer
            The parent container for the new form
        @param sInitialName
            The initial name of the form. May be null, in this case the default (which
            is an implementation detail) applies.
    */
    protected XIndexContainer createSubForm( XIndexContainer xParentContainer, String sInitialName )
            throws com.sun.star.uno.Exception
    {
        // create a new form
        Object xNewForm = m_orb.createInstance( "com.sun.star.form.component.DataForm" );

        // insert
        xParentContainer.insertByIndex( xParentContainer.getCount(), xNewForm );

        // set the name if necessary
        if ( null != sInitialName )
        {
            XPropertySet xFormProps = UNO.queryPropertySet( xNewForm );
            xFormProps.setPropertyValue( "Name", sInitialName );
        }

        // outta here
        return (XIndexContainer)UnoRuntime.queryInterface( XIndexContainer.class, xNewForm );
    }

    /* ------------------------------------------------------------------ */
    /** creates a new form which is a child of the given form components container

        @param aParentContainer
            The parent container for the new form
        @param sInitialName
            The initial name of the form. May be null, in this case the default (which
            is an implementation detail) applies.
    */
    public XIndexContainer createSubForm( Object aParentContainer, String sInitialName )
        throws com.sun.star.uno.Exception
    {
        XIndexContainer xParentContainer = (XIndexContainer)UnoRuntime.queryInterface(
            XIndexContainer.class, aParentContainer );
        return createSubForm( xParentContainer, sInitialName );
    }

    /* ------------------------------------------------------------------ */
    /** creates a form which is a sibling of the given form
        @param aForm
            A sinbling of the to be created form.

        @param sInitialName
            The initial name of the form. May be null, in this case the default (which
            is an implementation detail) applies.
    */
    public XIndexContainer createSiblingForm( Object aForm, String sInitialName ) throws com.sun.star.uno.Exception
    {
        // get the parent
        XChild xAsChild = (XChild)UnoRuntime.queryInterface( XChild.class, aForm );
        XIndexContainer xContainer = (XIndexContainer)UnoRuntime.queryInterface(
            XIndexContainer.class, xAsChild.getParent() );;
        // append a new form to this parent container
        return createSubForm( xContainer, sInitialName );
    }

    /* ------------------------------------------------------------------ */
    /** retrieves the document model which a given form component belongs to
    */
    static public DocumentHelper getDocumentForComponent( Object aFormComponent, XComponentContext xCtx )
    {
        XChild xChild = (XChild)UnoRuntime.queryInterface( XChild.class, aFormComponent );
        XModel xModel = null;
        while ( ( null != xChild ) && ( null == xModel ) )
        {
            XInterface xParent = (XInterface)xChild.getParent();
            xModel = (XModel)UnoRuntime.queryInterface( XModel.class, xParent );
            xChild = (XChild)UnoRuntime.queryInterface( XChild.class, xParent );
        }

        return new DocumentHelper( xCtx, xModel );
    }

    /* ------------------------------------------------------------------ */
    /** returns a URL which can be used to create a document of a certain type
    */
    public static String getDocumentFactoryURL( DocumentType eType )
    {
        if ( eType == DocumentType.WRITER )
            return "private:factory/swriter";
        if ( eType == DocumentType.CALC )
            return "private:factory/scalc";
        if ( eType == DocumentType.DRAWING )
            return "private:factory/sdraw";
        return "private:factory/swriter";
    }

    /* ------------------------------------------------------------------ */
    /** classifies a document
    */
    public DocumentType classify( )
    {
        XServiceInfo xSI = (XServiceInfo)UnoRuntime.queryInterface(
            XServiceInfo.class, m_documentComponent );

        if ( xSI.supportsService( "com.sun.star.text.TextDocument" ) )
            return DocumentType.WRITER;
        else if ( xSI.supportsService( "com.sun.star.sheet.SpreadsheetDocument" ) )
            return DocumentType.CALC;
        else if ( xSI.supportsService( "com.sun.star.drawing.DrawingDocument" ) )
            return DocumentType.DRAWING;

        return DocumentType.UNKNOWN;
    }
    /* ------------------------------------------------------------------ */
    /** retrieves a com.sun.star.drawing.DrawPage of the document, denoted by index
     *  @param index
     *      the index of the draw page<br/>
     *  @throws
     *      com.sun.star.lang.IndexOutOfBoundsException
     *      com.sun.star.lang.WrappedTargetException
     */
    protected XDrawPage getDrawPage( int index ) throws com.sun.star.lang.IndexOutOfBoundsException, com.sun.star.lang.WrappedTargetException
    {
        XDrawPagesSupplier xSuppPages = (XDrawPagesSupplier)UnoRuntime.queryInterface(
            XDrawPagesSupplier.class, getDocument() );
        XDrawPages xPages = xSuppPages.getDrawPages();

        return (XDrawPage)UnoRuntime.queryInterface( XDrawPage.class, xPages.getByIndex( index ) );
    }

    /* ------------------------------------------------------------------ */
    /** retrieves the <type scope="com.sun.star.drawing">DrawPage</type> of the document
    */
    protected XDrawPage getMainDrawPage( ) throws com.sun.star.uno.Exception
    {
        XDrawPage xReturn;

        // in case of a Writer document, this is rather easy: simply ask the XDrawPageSupplier
        XDrawPageSupplier xSuppPage = (XDrawPageSupplier)UnoRuntime.queryInterface(
            XDrawPageSupplier.class, getDocument() );
        if ( null != xSuppPage )
            xReturn = xSuppPage.getDrawPage();
        else
        {   // the model itself is no draw page supplier - okay, it may be a Writer or Calc document
            // (or any other multi-page document)
            XDrawPagesSupplier xSuppPages = (XDrawPagesSupplier)UnoRuntime.queryInterface(
                XDrawPagesSupplier.class, getDocument() );
            XDrawPages xPages = xSuppPages.getDrawPages();

            xReturn = (XDrawPage)UnoRuntime.queryInterface( XDrawPage.class, xPages.getByIndex( 0 ) );

            // Note that this is no really error-proof code: If the document model does not support the
            // XDrawPagesSupplier interface, or if the pages collection returned is empty, this will break.
        }

        return xReturn;
    }

    /* ------------------------------------------------------------------ */
    /** retrieves the root of the hierarchy of form components
    */
    protected XNameContainer getFormComponentTreeRoot( ) throws com.sun.star.uno.Exception
    {
        XFormsSupplier xSuppForms = (XFormsSupplier)UnoRuntime.queryInterface(
            XFormsSupplier.class, getMainDrawPage( ) );

        XNameContainer xFormsCollection = null;
        if ( null != xSuppForms )
        {
            xFormsCollection = xSuppForms.getForms();
        }
        return xFormsCollection;
    }

    /* ------------------------------------------------------------------ */
    /** creates a component at the service factory provided by the document
    */
    public XInterface createInstance( String serviceSpecifier ) throws com.sun.star.uno.Exception
    {
        XMultiServiceFactory xORB = (XMultiServiceFactory)UnoRuntime.queryInterface( XMultiServiceFactory.class,
            m_documentComponent );
        return (XInterface)xORB.createInstance( serviceSpecifier );
    }

    /* ------------------------------------------------------------------ */
    /** creates a component at the service factory provided by the document
    */
    public XInterface createInstanceWithArguments( String serviceSpecifier, Object[] arguments ) throws com.sun.star.uno.Exception
    {
        XMultiServiceFactory xORB = (XMultiServiceFactory)UnoRuntime.queryInterface( XMultiServiceFactory.class,
            m_documentComponent );
        return (XInterface) xORB.createInstanceWithArguments( serviceSpecifier, arguments );
    }
};

