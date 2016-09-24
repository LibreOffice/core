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
    @Override
    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkSomething"
        };
    }

    /* ------------------------------------------------------------------ */
    @Override
    public String getTestObjectName()
    {
        return "Form Component Property Bag Test";
    }

    /* ------------------------------------------------------------------ */
    public void before() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        m_orb = param.getMSF();
        m_document = DocumentHelper.blankTextDocument( m_orb );
        m_formLayer = new FormLayer( m_document );
    }

    /* ------------------------------------------------------------------ */
    private void impl_closeDoc() throws com.sun.star.uno.Exception
    {
        if ( m_document != null )
        {
            XCloseable closeDoc = UnoRuntime.queryInterface( XCloseable.class, m_document.getDocument() );
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
        XPropertyContainer propContainer = UnoRuntime.queryInterface(
            XPropertyContainer.class, textFieldModel );
        assure("XPropertyContainer not supported!", propContainer != null );

        propContainer.addProperty( "SomeBoundText", PropertyAttribute.BOUND, "InitialBoundText" );
        propContainer.addProperty( "SomeTransientText", PropertyAttribute.TRANSIENT, "InitialTransientProperty" );
        propContainer.addProperty( "SomeReadonlyText", PropertyAttribute.READONLY, "InitialReadonlyText" );
        propContainer.addProperty( "SomeNumericValue", PropertyAttribute.BOUND, Integer.valueOf( 42 ) );

        XPropertySetInfo propertyInfo = textFieldModel.getPropertySetInfo();
        assure( "Per service definition, dynamic properties are expected to be forced to be removable",
            ( propertyInfo.getPropertyByName("SomeBoundText").Attributes & PropertyAttribute.REMOVABLE ) != 0 );

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
        XStorable store = UnoRuntime.queryInterface( XStorable.class, m_document.getDocument() );
        String documentURL = util.utils.getOfficeTemp( m_orb ) + "document.odt";
        PropertyValue[] storeArguments = new PropertyValue[] { new PropertyValue() };
        storeArguments[0].Name = "FilterName";
        storeArguments[0].Value = "writer8";
        store.storeAsURL( documentURL, storeArguments );

        // close and re-load it
        impl_closeDoc();

        m_document = DocumentHelper.loadDocument( m_orb, documentURL );
        m_formLayer = new FormLayer( m_document );

        XPropertySet textFieldModel = m_formLayer.getControlModel( new int[] { 0, 0 } );

        // all persistent properties should have the expected values
        assure( "persistent properties did not survive reload (1)!", ((String)textFieldModel.getPropertyValue( "SomeBoundText" )).equals( "ChangedBoundText" ) );
        assure( "persistent properties did not survive reload (2)!", ((String)textFieldModel.getPropertyValue( "SomeReadonlyText" )).equals( "InitialReadonlyText" ) );
//        assure( "persistent properties did not survive reload (3)!", ((Integer)textFieldModel.getPropertyValue( "SomeNumericValue" )).equals( Integer.valueOf( 42 ) ) );
            // cannot check this until the types really survive - at the moment, integers are converted to doubles...

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
