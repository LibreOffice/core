/*************************************************************************
 *
 *  $RCSfile: DocumentHelper.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:52:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
package integration.forms;

/**************************************************************************/
import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.util.*;
import com.sun.star.awt.*;
import com.sun.star.drawing.*;
import com.sun.star.frame.*;
import com.sun.star.form.*;
import com.sun.star.beans.*;
import com.sun.star.container.*;
import com.sun.star.container.*;

import integration.forms.dbfTools;
import integration.forms.DocumentViewHelper;
import integration.forms.SpreadsheetView;

/**************************************************************************/
/** provides a small wrapper around a document
*/
public class DocumentHelper
{
    protected XMultiServiceFactory  m_orb;
    protected XComponent                m_documentComponent;

    /* ------------------------------------------------------------------ */
    public XComponent getDocument( )
    {
        return m_documentComponent;
    }

    /* ------------------------------------------------------------------ */
    public XMultiServiceFactory getOrb( )
    {
        return m_orb;
    }

    /* ------------------------------------------------------------------ */
    public DocumentHelper( XMultiServiceFactory orb, XComponent xDocument )
    {
        m_orb = orb;
        m_documentComponent = xDocument;
    }

    /* ------------------------------------------------------------------ */
    private static DocumentHelper implCreateBlankDocument( XMultiServiceFactory orb, String factoryURL ) throws com.sun.star.uno.Exception
    {
        XComponentLoader aLoader = (XComponentLoader)UnoRuntime.queryInterface(
            XComponentLoader.class,
            orb.createInstance( "com.sun.star.frame.Desktop" ) );

        XComponent xDocument = dbfTools.queryXComponent(
            aLoader.loadComponentFromURL( factoryURL, "_blank", 0, new PropertyValue[ 0 ] ) );
        return new DocumentHelper( orb, xDocument );
    }

    /* ------------------------------------------------------------------ */
    public static DocumentHelper blankTextDocument( XMultiServiceFactory orb ) throws com.sun.star.uno.Exception
    {
        return implCreateBlankDocument( orb, "private:factory/swriter" );
    }

    /* ------------------------------------------------------------------ */
    public static DocumentHelper blankSpreadsheetDocument( XMultiServiceFactory orb ) throws com.sun.star.uno.Exception
    {
        return implCreateBlankDocument( orb, "private:factory/scalc" );
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
            XPropertySet xFormProps = dbfTools.queryXPropertySet( xNewForm );
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
        XChild xChild = (XChild)UnoRuntime.queryInterface( XChild.class, aFormComponent );
        XModel xModel = null;
        while ( ( null != xChild ) && ( null == xModel ) )
        {
            XInterface xParent = (XInterface)xChild.getParent();
            xModel = (XModel)UnoRuntime.queryInterface( XModel.class, xParent );
            xChild = (XChild)UnoRuntime.queryInterface( XChild.class, xParent );
        }

        return new DocumentHelper( orb, xModel );
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
};

