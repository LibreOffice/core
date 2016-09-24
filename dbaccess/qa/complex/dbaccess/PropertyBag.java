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

package complex.dbaccess;

import com.sun.star.beans.NamedValue;
import com.sun.star.beans.PropertyState;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.XPropertyAccess;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertyContainer;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;

// ---------- junit imports -----------------
import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;


public class PropertyBag extends TestCase
{
    private static final String VALUE = "Value";
    private XPropertyContainer      m_bag;
    private XPropertySet            m_set;
    private XPropertyAccess         m_access;
    private XMultiServiceFactory    m_orb = null;

    public String getTestObjectName()
    {
        return "PropertyBag";
    }

    @Before
    @Override
    public void before()
    {
        m_orb = getMSF();
    }

    @Test
    public void checkBasics() throws Exception
    {
        createEmptyBag();
        System.out.println("testing the basics");

        // check whether empty property names are rejected
        boolean caughtExpected = false;
        try
        {
            m_bag.addProperty( "", PropertyAttribute.BOUND, Integer.valueOf( 3 ) );
        }
        catch(com.sun.star.lang.IllegalArgumentException e) { caughtExpected = true; }
        catch(com.sun.star.uno.Exception e) { }
        if ( !caughtExpected )
        {
            fail("empty property names are not rejected by XPropertyContainer::addProperty");
        }

        // check whether duplicate insertions are rejected
        caughtExpected = false;
        try
        {
            m_bag.addProperty( VALUE, PropertyAttribute.BOUND, "" );
            m_bag.addProperty( VALUE, PropertyAttribute.BOUND, "" );
        }
        catch(com.sun.star.beans.PropertyExistException e) { caughtExpected = true; }
        catch(com.sun.star.uno.Exception e) { }
        if ( !caughtExpected )
        {
            fail("insertion of duplicate property names is not rejected");
        }

        // try removing the property we just added - this should fail, as it does not have
        // the REMOVABLE attribute
        caughtExpected = false;
        try
        {
            m_bag.removeProperty( VALUE);
        }
        catch(com.sun.star.beans.NotRemoveableException e) { caughtExpected = true; }
        catch(com.sun.star.uno.Exception e) { }
        if ( !caughtExpected )
        {
            fail("removing non-removable properties is expected to fail - but it didn't");
        }

        // try removing a non-existent property
        caughtExpected = false;
        try
        {
            m_bag.removeProperty( "NonExistent" );
        }
        catch(com.sun.star.beans.UnknownPropertyException e) { caughtExpected = true; }
        catch(com.sun.star.uno.Exception e) { }
        if ( !caughtExpected )
        {
            fail("removing non-existent properties is expected to fail - but it didn't");
        }

        // try writing and reading a value for the one property we have so far
        final String testValue = "someArbitraryValue";
        m_set.setPropertyValue(  VALUE , testValue);
        final String currentValue = (String)m_set.getPropertyValue( VALUE);
        if ( !currentValue.equals( testValue ) )
        {
            fail("set property is not remembered");
        }

        // try setting an illegal value for the property
        caughtExpected = false;
        try
        {
            m_set.setPropertyValue( VALUE, Integer.valueOf( 3 ) );
        }
        catch(com.sun.star.lang.IllegalArgumentException e) { caughtExpected = true; }
        catch(com.sun.star.uno.Exception e) { }
        if ( !caughtExpected )
        {
            fail("the bag does not respect the property type we declared for the property");
        }
    }

    @Test
    public void checkSequenceAccess() throws com.sun.star.uno.Exception
    {
        System.out.println( "checking PropertySetAccess via sequences" );
        createStandardBag( false );


        // XPropertyAccess.setPropertyValues
        final PropertyValue expectedValues[] =
        {
            new PropertyValue( "BoolValue", -1, Boolean.FALSE, PropertyState.DIRECT_VALUE ),
            new PropertyValue( "StringValue", -1, "some text", PropertyState.DIRECT_VALUE ),
            new PropertyValue( "IntegerValue", -1, Integer.valueOf( 3 ), PropertyState.DIRECT_VALUE ),
            new PropertyValue( "InterfaceValue", -1, m_bag, PropertyState.DIRECT_VALUE )
        };
        m_access.setPropertyValues( expectedValues );

        for ( int i=0; i<expectedValues.length; ++i )
        {
            final Object value = m_set.getPropertyValue( expectedValues[i].Name );
            if ( !value.equals( expectedValues[i].Value ) )
            {
                System.out.println( "property name : " + expectedValues[i].Name );
                System.out.println( "expected value: " + expectedValues[i].Value.toString() );
                System.out.println( "current value : " + value.toString() );
                fail( "retrieving a previously set property (" + expectedValues[i].Value.getClass().toString() + ") failed" );
            }
        }


        // XPropertyAccess.getPropertyValues
        final PropertyValue currentValues[] = m_access.getPropertyValues();
        for ( int i=0; i<currentValues.length; ++i )
        {
            final String name = currentValues[i].Name;
            final Object value = currentValues[i].Value;
            for ( int j=0; j<expectedValues.length; ++j )
            {
                if ( expectedValues[j].Name.equals( name ) )
                {
                    if ( !expectedValues[j].Value.equals( value ) )
                    {
                        System.out.println( "property name : " + expectedValues[j].Name );
                        System.out.println( "expected value: " + expectedValues[j].Value.toString() );
                        System.out.println( "current value : " + value.toString() );
                        fail( "getPropertyValues failed for property '" + name + "' failed" );
                    }
                    break;
                }
            }

            if ( !m_set.getPropertyValue( name ).equals( value ) )
            {
                fail("XPropertyAccess::getPropertyValues() and XPropertyset::getPropertyValue results are inconsistent");
            }
        }
    }

    @Test
    public void checkDynamicSet()
    {
        System.out.println( "checking proper dynamic of the set" );
        createStandardBag( false );

        final PropertyValue props[] =
        {
            new PropertyValue( "BoolValue", -1, Boolean.FALSE, PropertyState.DIRECT_VALUE),
            new PropertyValue( "StringValue", -1, "test", PropertyState.DIRECT_VALUE ),
            new PropertyValue( "SomeOtherStringValue", -1, "string value", PropertyState.DIRECT_VALUE )
        };

        // try setting some property values which are not existent
        boolean caughtExpected = false;
        try
        {
            m_access.setPropertyValues( props );
        }
        catch( com.sun.star.beans.UnknownPropertyException e ) { caughtExpected = true; }
        catch( com.sun.star.uno.Exception e ) { }
        if ( !caughtExpected )
        {
            fail("the set shouldn't accept unknown property values, if not explicitly told to do so");
        }

        // re-create the bag, this time allow it to implicitly add properties
        createStandardBag( true );
        boolean success = false;
        try { m_access.setPropertyValues( props ); success = true; }
        catch( com.sun.star.uno.Exception e ) { }
        if ( !success )
        {
            fail("property bag failed to implicitly add unknown properties");
        }

        // see whether this property was really added, and not just ignored
        final PropertyValue newlyAdded = props[ props.length - 1 ];
        try
        {
            if ( !m_set.getPropertyValue( newlyAdded.Name ).equals( newlyAdded.Value ) )
            {
                fail("the new property was not really added, or not added with the proper value");
            }
        }
        catch( com.sun.star.uno.Exception e ) { }
    }

    private void createEmptyBag()
    {
        try
        {
            m_bag = null;
            final String serviceName = "com.sun.star.beans.PropertyBag";
            m_bag = UnoRuntime.queryInterface(XPropertyContainer.class, m_orb.createInstance(serviceName));
            if ( m_bag == null )
            {
                fail("could not create a " + serviceName + " instance");
            }
            m_set = UnoRuntime.queryInterface(XPropertySet.class, m_bag);
            m_access = UnoRuntime.queryInterface(XPropertyAccess.class, m_bag);
        }
        catch( com.sun.star.uno.Exception e )
        {
        }
    }

    private void createStandardBag( boolean allowLazyAdding )
    {
        try
        {
            m_bag = null;

            final Object initArgs[] = { new NamedValue( "AutomaticAddition", Boolean.valueOf( allowLazyAdding ) ) };

            final String serviceName = "com.sun.star.beans.PropertyBag";
            m_bag = UnoRuntime.queryInterface(XPropertyContainer.class, m_orb.createInstanceWithArguments(serviceName, initArgs));
            if ( m_bag == null )
            {
                fail("could not create a " + serviceName + " instance");
            }
            m_set = UnoRuntime.queryInterface(XPropertySet.class, m_bag);
            m_access = UnoRuntime.queryInterface(XPropertyAccess.class, m_bag);

            final Object properties[][] =
            {
                { "BoolValue", Boolean.TRUE },
                { "StringValue", "" },
                { "IntegerValue", Integer.valueOf( 3 ) },
                { "InterfaceValue", m_bag }
            };
            for ( int i=0; i<properties.length; ++i )
            {
                m_bag.addProperty(
                    (String)properties[i][0],
                    PropertyAttribute.MAYBEVOID,
                    properties[i][1]
                );
            }
        }
        catch( com.sun.star.uno.Exception e )
        {
        }
    }
}
