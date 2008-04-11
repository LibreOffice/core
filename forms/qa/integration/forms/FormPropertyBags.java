/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FormPropertyBags.java,v $
 * $Revision: 1.4 $
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
package integration.forms;

import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyChangeEvent;
import com.sun.star.beans.PropertyExistException;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XPropertyContainer;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.lang.XMultiServiceFactory;

import com.sun.star.util.XCloseable;

import integration.forms.DocumentHelper;

public class FormPropertyBags extends complexlib.ComplexTestCase implements XPropertyChangeListener
{
    private DocumentHelper          m_document;
    private FormLayer               m_formLayer;
    private XMultiServiceFactory    m_orb;

    private PropertyChangeEvent     m_propertyChangeEvent;

    /** Creates a new instance of FormPropertyBags */
    public FormPropertyBags()
    {
        m_propertyChangeEvent = new PropertyChangeEvent();
    }

    /* ------------------------------------------------------------------ */
    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkSomething"
        };
    }

    /* ------------------------------------------------------------------ */
    public String getTestObjectName()
    {
        return "Form Component Property Bag Test";
    }

    /* ------------------------------------------------------------------ */
    public void before() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        m_orb = (XMultiServiceFactory)param.getMSF();
        m_document = DocumentHelper.blankTextDocument( m_orb );
        m_formLayer = new FormLayer( m_document );
    }

    /* ------------------------------------------------------------------ */
    private void impl_closeDoc() throws com.sun.star.uno.Exception
    {
        if ( m_document != null )
        {
            XCloseable closeDoc = (XCloseable)UnoRuntime.queryInterface( XCloseable.class,
                m_document.getDocument() );
            closeDoc.close( true );
        }
    }

    /* ------------------------------------------------------------------ */
    public void after() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        impl_closeDoc();
    }

    /* ------------------------------------------------------------------ */
    public void checkSomething() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XPropertySet textFieldModel = m_formLayer.createControlAndShape( "DatabaseTextField", 10, 10, 25, 6 );

        // check whether adding new properties is successful
        XPropertyContainer propContainer = (XPropertyContainer)UnoRuntime.queryInterface(
            XPropertyContainer.class, textFieldModel );
        assure("XPropertyContainer not supported!", propContainer != null );

        propContainer.addProperty( "SomeBoundText", PropertyAttribute.BOUND, "InitialBoundText" );
        propContainer.addProperty( "SomeTransientText", PropertyAttribute.TRANSIENT, "InitialTransientProperty" );
        propContainer.addProperty( "SomeReadonlyText", PropertyAttribute.READONLY, "InitialReadonlyText" );
        propContainer.addProperty( "SomeNumericValue", PropertyAttribute.BOUND, new Integer( 42 ) );

        XPropertySetInfo propertyInfo = textFieldModel.getPropertySetInfo();
        assure( "Per service definition, dynamic properties are expected to be forced to be removeable",
            ( propertyInfo.getPropertyByName("SomeBoundText").Attributes & PropertyAttribute.REMOVEABLE ) != 0 );

        // a second addition of a property with an existent name should be rejected
        boolean caughtExpected = false;
        try { propContainer.addProperty( "SomeBoundText", PropertyAttribute.BOUND, "InitialBoundText" ); }
        catch( PropertyExistException e ) { caughtExpected = true; }
        catch( Exception e ) { }
        assure( "repeated additions of a property with the same name should be rejected",
            caughtExpected );

        // check whether the properties are bound as expected
        impl_checkPropertyValueNotification( textFieldModel );

        // check property value persistence
        impl_checkPropertyPersistence();
    }

    /* ------------------------------------------------------------------ */
    private void impl_checkPropertyValueNotification( XPropertySet _controlModel ) throws com.sun.star.uno.Exception
    {
        _controlModel.addPropertyChangeListener( "", this );

        _controlModel.setPropertyValue( "SomeBoundText", "ChangedBoundText" );
        assure( "changes in the bound property are not properly notified",
                m_propertyChangeEvent.PropertyName.equals( "SomeBoundText" )
            &&  m_propertyChangeEvent.OldValue.equals( "InitialBoundText" )
            &&  m_propertyChangeEvent.NewValue.equals( "ChangedBoundText" ) );

        m_propertyChangeEvent = null;
        _controlModel.setPropertyValue( "SomeTransientText", "ChangedTransientText" );
        assure( "changes in non-bound properties should not be notified",
            m_propertyChangeEvent == null );

        boolean caughtExpected = false;
        try { _controlModel.setPropertyValue( "SomeReadonlyText", "ChangedReadonlyText" ); }
        catch( PropertyVetoException e ) { caughtExpected = true; }
        catch( Exception e ) { }
        assure( "trying to write a read-only property did not give the expected result",
            caughtExpected );

        _controlModel.removePropertyChangeListener( "", this );
    }

    /* ------------------------------------------------------------------ */
    private void impl_checkPropertyPersistence() throws com.sun.star.uno.Exception
    {
        // store the document
        XStorable store = (XStorable)UnoRuntime.queryInterface( XStorable.class,
            m_document.getDocument() );
        String documentURL = util.utils.getOfficeTemp( m_orb ) + "document.odt";
        PropertyValue[] storeArguments = new PropertyValue[] { new PropertyValue() };
        storeArguments[0].Name = "FilterName";
        storeArguments[0].Value = "writer8";
        store.storeAsURL( documentURL, storeArguments );

        // close and re-load it
        impl_closeDoc();

        m_document = DocumentHelper.loadDocument( m_orb, documentURL );
        m_formLayer = new FormLayer( m_document );

        XPropertySet textFieldModel = m_formLayer.getControlModel( new String[] { "Standard", "TextBox" } );

        // all persistent properties should have the expected values
        assure( "persistent properties did not survive reload (1)!", ((String)textFieldModel.getPropertyValue( "SomeBoundText" )).equals( "ChangedBoundText" ) );
        assure( "persistent properties did not survive reload (2)!", ((String)textFieldModel.getPropertyValue( "SomeReadonlyText" )).equals( "InitialReadonlyText" ) );
//        assure( "persistent properties did not survive reload (3)!", ((Integer)textFieldModel.getPropertyValue( "SomeNumericValue" )).equals( new Integer( 42 ) ) );
            // cannot check this until the types really survice - at the moment, integers are converted to doubles ...

        // the transient property should not have survived
        boolean caughtExpected = false;
        try { textFieldModel.getPropertyValue( "SomeTransientText" ); }
        catch( UnknownPropertyException e ) { caughtExpected = true; }
        assure( "transient property did survive reload!", caughtExpected );

        // There would be more things to check.
        // For instance, it would be desirable of the property attributes would have survived
        // the reload, and the property defaults (XPropertyState).
        // However, the file format currently doesn't allow for this, so those information
        // is lost when saving the document.
    }

    /* ------------------------------------------------------------------ */
    public void propertyChange(PropertyChangeEvent _propertyChangeEvent)
    {
        m_propertyChangeEvent = _propertyChangeEvent;
    }

    /* ------------------------------------------------------------------ */
    public void disposing(EventObject eventObject)
    {
        // not interested in
    }
}
