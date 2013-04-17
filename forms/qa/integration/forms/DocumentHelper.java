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
package integration.forms;

import com.sun.star.beans.PropertyState;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XChild;
import com.sun.star.container.XIndexContainer;
import com.sun.star.container.XNameContainer;
import com.sun.star.document.MacroExecMode;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPageSupplier;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.form.XFormsSupplier;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XController;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XModifiable;

/**************************************************************************/

/**************************************************************************/
/** provides a small wrapper around a document
*/
public class DocumentHelper
{
    private XMultiServiceFactory    m_orb;
    private XComponent              m_documentComponent;

    /* ================================================================== */
    /* ------------------------------------------------------------------ */
    public DocumentHelper( XMultiServiceFactory orb, XComponent document )
    {
        m_orb = orb;
        m_documentComponent = document;
    }

    /* ------------------------------------------------------------------ */
    protected static XComponent implLoadAsComponent( XMultiServiceFactory orb, String documentOrFactoryURL ) throws com.sun.star.uno.Exception
    {
        return implLoadAsComponent( orb, documentOrFactoryURL, new PropertyValue[0] );
    }

    /* ------------------------------------------------------------------ */
    protected static XComponent implLoadAsComponent( XMultiServiceFactory orb, String documentOrFactoryURL, final PropertyValue[] i_args ) throws com.sun.star.uno.Exception
    {
        XComponentLoader aLoader = UnoRuntime.queryInterface(
            XComponentLoader.class,
            orb.createInstance( "com.sun.star.frame.Desktop" )
        );

        XComponent document = dbfTools.queryComponent(
            aLoader.loadComponentFromURL( documentOrFactoryURL, "_blank", 0, i_args )
        );
        return document;
    }

    /* ------------------------------------------------------------------ */
    private static DocumentHelper implLoadDocument( XMultiServiceFactory orb, String documentOrFactoryURL ) throws com.sun.star.uno.Exception
    {
        return implLoadDocument( orb, documentOrFactoryURL, new PropertyValue[0] );
    }

    /* ------------------------------------------------------------------ */
    private static DocumentHelper implLoadDocument( XMultiServiceFactory orb, String documentOrFactoryURL, final PropertyValue[] i_args ) throws com.sun.star.uno.Exception
    {
        XComponent document = implLoadAsComponent( orb, documentOrFactoryURL, i_args );

        XServiceInfo xSI = UnoRuntime.queryInterface( XServiceInfo.class,
            document );
        if ( xSI.supportsService( "com.sun.star.sheet.SpreadsheetDocument" ) )
            return new SpreadsheetDocument( orb, document );
        return new DocumentHelper( orb, document );
    }

    /* ------------------------------------------------------------------ */
    public static DocumentHelper loadDocument( XMultiServiceFactory orb, String documentURL ) throws com.sun.star.uno.Exception
    {
        return implLoadDocument( orb, documentURL );
    }

    /* ------------------------------------------------------------------ */
    public static DocumentHelper blankTextDocument( XMultiServiceFactory orb ) throws com.sun.star.uno.Exception
    {
        return blankDocument( orb, DocumentType.WRITER );
    }

    /* ------------------------------------------------------------------ */
    public static DocumentHelper blankXMLForm( XMultiServiceFactory orb ) throws com.sun.star.uno.Exception
    {
        return blankDocument( orb, DocumentType.XMLFORM );
    }

    /* ------------------------------------------------------------------ */
    public static DocumentHelper blankDocument( XMultiServiceFactory orb, DocumentType eType ) throws com.sun.star.uno.Exception
    {
        final PropertyValue[] args = new PropertyValue[] {
            new PropertyValue( "MacroExecutionMode", -1, MacroExecMode.ALWAYS_EXECUTE, PropertyState.DIRECT_VALUE )
        };
        return implLoadDocument( orb, getDocumentFactoryURL( eType ), args );
    }

    /* ================================================================== */
    /* ------------------------------------------------------------------ */
    public XComponent getDocument( )
    {
        return m_documentComponent;
    }

    /* ------------------------------------------------------------------ */
    public boolean isModified()
    {
        XModifiable modify = (XModifiable)query( XModifiable.class );
        return modify.isModified();
    }

    /* ------------------------------------------------------------------ */
    public Object query( Class aInterfaceClass )
    {
        return UnoRuntime.queryInterface( aInterfaceClass, m_documentComponent );
    }

    /* ------------------------------------------------------------------ */
    public XMultiServiceFactory getOrb( )
    {
        return m_orb;
    }

    /* ------------------------------------------------------------------ */
    /** retrieves the current view of the document
        @return
            the view component, queried for the interface described by aInterfaceClass
    */
    public DocumentViewHelper getCurrentView( )
    {
        // get the model interface for the document
        XModel xDocModel = UnoRuntime.queryInterface(XModel.class, m_documentComponent );
        // get the current controller for the document - as a controller is tied to a view,
        // this gives us the currently active view for the document.
        XController xController = xDocModel.getCurrentController();

        if ( classify() == DocumentType.CALC )
            return new SpreadsheetView( m_orb, this, xController );

        return new DocumentViewHelper( m_orb, this, xController );
    }

    /* ------------------------------------------------------------------ */
    /** reloads the document
     *
     *  The reload is done by dispatching the respective URL at a frame of the document.
     *  As a consequence, if you have references to a view of the document, or any interface
     *  of the document, they will become invalid.
     *  The Model instance itself, at which you called reload, will still be valid, it will
     *  automatically update its internal state after the reload.
     *
     *  Another consequence is that if the document does not have a view at all, it cannot
     *  be reloaded.
     */
    public void reload() throws Exception
    {
        DocumentViewHelper view = getCurrentView();
        XFrame frame = view.getController().getFrame();
        XModel oldModel = frame.getController().getModel();

        getCurrentView().dispatch( ".uno:Reload" );

        m_documentComponent = UnoRuntime.queryInterface( XComponent.class,
            frame.getController().getModel() );

        XModel newModel = getCurrentView().getController().getModel();
        if ( UnoRuntime.areSame( oldModel, newModel ) )
            throw new java.lang.IllegalStateException( "reload failed" );
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
            XPropertySet xFormProps = dbfTools.queryPropertySet( xNewForm );
            xFormProps.setPropertyValue( "Name", sInitialName );
        }

        // outta here
        return UnoRuntime.queryInterface( XIndexContainer.class, xNewForm );
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
        XIndexContainer xParentContainer = UnoRuntime.queryInterface(
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
        XIndexContainer xContainer = (XIndexContainer)dbfTools.getParent(
            aForm, XIndexContainer.class );
        // append a new form to this parent container
        return createSubForm( xContainer, sInitialName );
    }

    /* ------------------------------------------------------------------ */
    /** retrieves the document model which a given form component belongs to
    */
    static public DocumentHelper getDocumentForComponent( Object aFormComponent, XMultiServiceFactory orb )
    {
        XChild xChild = UnoRuntime.queryInterface( XChild.class, aFormComponent );
        XModel xModel = null;
        while ( ( null != xChild ) && ( null == xModel ) )
        {
            XInterface xParent = (XInterface)xChild.getParent();
            xModel = UnoRuntime.queryInterface( XModel.class, xParent );
            xChild = UnoRuntime.queryInterface( XChild.class, xParent );
        }

        return new DocumentHelper( orb, xModel );
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
        if ( eType == DocumentType.XMLFORM )
            return "private:factory/swriter?slot=21053";
        return "private:factory/swriter";
    }

    /* ------------------------------------------------------------------ */
    /** classifies a document
    */
    public DocumentType classify( )
    {
        XServiceInfo xSI = UnoRuntime.queryInterface(
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
     *      the index of the draw page
     *  @throws
     *      com.sun.star.lang.IndexOutOfBoundsException
     *      com.sun.star.lang.WrappedTargetException
     */
    protected XDrawPage getDrawPage( int index ) throws com.sun.star.lang.IndexOutOfBoundsException, com.sun.star.lang.WrappedTargetException
    {
        XDrawPagesSupplier xSuppPages = UnoRuntime.queryInterface(
            XDrawPagesSupplier.class, getDocument() );
        XDrawPages xPages = xSuppPages.getDrawPages();

        return UnoRuntime.queryInterface( XDrawPage.class, xPages.getByIndex( index ) );
    }

    /* ------------------------------------------------------------------ */
    /** retrieves the <type scope="com.sun.star.drawing">DrawPage</type> of the document
    */
    protected XDrawPage getMainDrawPage( ) throws com.sun.star.uno.Exception
    {
        XDrawPage xReturn;

        // in case of a Writer document, this is rather easy: simply ask the XDrawPageSupplier
        XDrawPageSupplier xSuppPage = UnoRuntime.queryInterface(
            XDrawPageSupplier.class, getDocument() );
        if ( null != xSuppPage )
            xReturn = xSuppPage.getDrawPage();
        else
        {   // the model itself is no draw page supplier - okay, it may be a Writer or Calc document
            // (or any other multi-page document)
            XDrawPagesSupplier xSuppPages = UnoRuntime.queryInterface(
                XDrawPagesSupplier.class, getDocument() );
            XDrawPages xPages = xSuppPages.getDrawPages();

            xReturn = UnoRuntime.queryInterface( XDrawPage.class, xPages.getByIndex( 0 ) );

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
        XFormsSupplier xSuppForms = UnoRuntime.queryInterface(
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
        XMultiServiceFactory xORB = UnoRuntime.queryInterface( XMultiServiceFactory.class,
            m_documentComponent );
        return (XInterface)xORB.createInstance( serviceSpecifier );
    }

    /* ------------------------------------------------------------------ */
    /** creates a component at the service factory provided by the document
    */
    public XInterface createInstanceWithArguments( String serviceSpecifier, Object[] arguments ) throws com.sun.star.uno.Exception
    {
        XMultiServiceFactory xORB = UnoRuntime.queryInterface( XMultiServiceFactory.class,
            m_documentComponent );
        return (XInterface) xORB.createInstanceWithArguments( serviceSpecifier, arguments );
    }
};

