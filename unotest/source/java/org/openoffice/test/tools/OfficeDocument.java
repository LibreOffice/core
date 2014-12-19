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
package org.openoffice.test.tools;

import com.sun.star.beans.PropertyState;
import com.sun.star.beans.PropertyValue;
import com.sun.star.document.MacroExecMode;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.CloseVetoException;
import com.sun.star.util.XCloseable;
import java.util.logging.Level;
import java.util.logging.Logger;

/**************************************************************************/

/**************************************************************************/
/** provides a small wrapper around a document
*/
public class OfficeDocument
{
    /* ================================================================== */
    /* ------------------------------------------------------------------ */
    public OfficeDocument( XMultiServiceFactory orb, XComponent document )
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
    private static XComponent implLoadAsComponent( XMultiServiceFactory orb, String documentOrFactoryURL, final PropertyValue[] i_args ) throws com.sun.star.uno.Exception
    {
        XComponentLoader aLoader = UnoRuntime.queryInterface( XComponentLoader.class,
            orb.createInstance( "com.sun.star.frame.Desktop" ) );

        XComponent document = UnoRuntime.queryInterface( XComponent.class,
            aLoader.loadComponentFromURL( documentOrFactoryURL, "_blank", 0, i_args )
        );
        return document;
    }

    /* ------------------------------------------------------------------ */
    private static OfficeDocument implLoadDocument( XMultiServiceFactory orb, String documentOrFactoryURL, final PropertyValue[] i_args ) throws com.sun.star.uno.Exception
    {
        XComponent document = implLoadAsComponent( orb, documentOrFactoryURL, i_args );

        XServiceInfo xSI = UnoRuntime.queryInterface( XServiceInfo.class, document );
        if ( xSI.supportsService( "com.sun.star.sheet.SpreadsheetDocument" ) )
            return new SpreadsheetDocument( orb, document );
        return new OfficeDocument( orb, document );
    }

    /* ------------------------------------------------------------------ */
    public static OfficeDocument blankTextDocument( XMultiServiceFactory orb ) throws com.sun.star.uno.Exception
    {
        return blankDocument( orb, DocumentType.WRITER );
    }

    /* ------------------------------------------------------------------ */
    public static OfficeDocument blankDocument( XMultiServiceFactory orb, DocumentType eType ) throws com.sun.star.uno.Exception
    {
        final PropertyValue[] args = new PropertyValue[] {
            new PropertyValue( "MacroExecutionMode", -1, MacroExecMode.ALWAYS_EXECUTE, PropertyState.DIRECT_VALUE )
        };
        return implLoadDocument( orb, getDocumentFactoryURL( eType ), args );
    }

    /* ------------------------------------------------------------------ */
    public boolean close()
    {
        try
        {
            XCloseable closeDoc = UnoRuntime.queryInterface( XCloseable.class, m_documentComponent );
            closeDoc.close( true );
            return true;
        }
        catch ( CloseVetoException e )
        {
            Logger.getLogger( OfficeDocument.class.getName() ).log( Level.SEVERE, "closing the document was vetoed", e );
        }
        return false;
    }

    /* ================================================================== */
    /* ------------------------------------------------------------------ */
    public XComponent getDocument( )
    {
        return m_documentComponent;
    }

    /* ------------------------------------------------------------------ */
    /** retrieves the current view of the document
        @return
            the view component, queried for the interface described by aInterfaceClass
    */
    public OfficeDocumentView getCurrentView( )
    {
        // get the model interface for the document
        XModel xDocModel = UnoRuntime.queryInterface( XModel.class, m_documentComponent );
        // get the current controller for the document - as a controller is tied to a view,
        // this gives us the currently active view for the document.
        XController xController = xDocModel.getCurrentController();

        if ( classify() == DocumentType.CALC )
            return new SpreadsheetView( m_orb, this, xController );

        return new OfficeDocumentView( m_orb, this, xController );
    }

    /* ------------------------------------------------------------------ */
    /** returns a URL which can be used to create a document of a certain type
    */
    private static String getDocumentFactoryURL( DocumentType eType )
    {
        if ( eType == DocumentType.WRITER )
            return "private:factory/swriter";
        if ( eType == DocumentType.CALC )
            return "private:factory/scalc";
        if ( eType == DocumentType.DRAWING )
            return "private:factory/sdraw";
        if ( eType == DocumentType.XMLFORM )
            return "private:factory/swriter?slot=21053";
        if ( eType == DocumentType.PRESENTATION )
            return "private:factory/simpress";
        if ( eType == DocumentType.FORMULA )
            return "private:factory/smath";
        return "private:factory/swriter";
    }

    /* ------------------------------------------------------------------ */
    /** classifies a document
    */
    private DocumentType classify( )
    {
        XServiceInfo xSI = UnoRuntime.queryInterface( XServiceInfo.class, m_documentComponent );

        if ( xSI.supportsService( "com.sun.star.text.TextDocument" ) )
            return DocumentType.WRITER;
        else if ( xSI.supportsService( "com.sun.star.sheet.SpreadsheetDocument" ) )
            return DocumentType.CALC;
        else if ( xSI.supportsService( "com.sun.star.drawing.DrawingDocument" ) )
            return DocumentType.DRAWING;
        else if ( xSI.supportsService( "com.sun.star.presentation.PresentationDocument" ) )
            return DocumentType.PRESENTATION;
        else if ( xSI.supportsService( "com.sun.star.formula.FormulaProperties" ) )
            return DocumentType.FORMULA;

        return DocumentType.UNKNOWN;
    }

    /* ------------------------------------------------------------------ */
    /** creates a component at the service factory provided by the document
    */
    public XInterface createInstance( String serviceSpecifier ) throws com.sun.star.uno.Exception
    {
        XMultiServiceFactory xORB = UnoRuntime.queryInterface( XMultiServiceFactory.class, m_documentComponent );
        return (XInterface)xORB.createInstance( serviceSpecifier );
    }

    /* ------------------------------------------------------------------ */
    /** creates a component at the service factory provided by the document, queried for a given interface type
    */
    public <T> T createInstance( String i_serviceSpecifier, Class<T> i_interfaceClass ) throws com.sun.star.uno.Exception
    {
        return UnoRuntime.queryInterface( i_interfaceClass, createInstance( i_serviceSpecifier ) );
    }

    /* ------------------------------------------------------------------ */
    /** creates a component at the service factory provided by the document
    */
    public XInterface createInstanceWithArguments( String serviceSpecifier, Object[] arguments ) throws com.sun.star.uno.Exception
    {
        XMultiServiceFactory xORB = UnoRuntime.queryInterface( XMultiServiceFactory.class, m_documentComponent );
        return (XInterface) xORB.createInstanceWithArguments( serviceSpecifier, arguments );
    }

    private final XMultiServiceFactory    m_orb;
    private XComponent              m_documentComponent;
}

