/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PropertyBag.java,v $
 * $Revision: 1.5 $
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

package complex.dbaccess;

import complexlib.ComplexTestCase;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.*;

public class PropertyBag extends ComplexTestCase
{
    private static final String VALUE = "Value";
    private XPropertyContainer      m_bag;
    private XPropertySet            m_set;
    private XPropertyAccess         m_access;
    private XMultiServiceFactory    m_orb = null;

    public String[] getTestMethodNames()
    {
        return new String[]
        {
            "checkBasics",
            "checkSequenceAccess",
            "checkDynamicSet"
        };
    }

    public String getTestObjectName()
    {
        return "PropertyBag";
    }

    public void before()
    {
        m_orb = (XMultiServiceFactory)param.getMSF();
    }

    public void checkBasics()
    {
        createEmptyBag();
        log.println("testing the basics");

        // check whether empty property names are rejected
        boolean caughtExpected = false;
        try
        {
            m_bag.addProperty( "", PropertyAttribute.BOUND, Integer.valueOf( 3 ) );
        }
        catch(com.sun.star.lang.IllegalArgumentException e) { caughtExpected = true; }
        catch(com.sun.star.uno.Exception e) { }
        if ( !caughtExpected )
            failed( "empty property names are not rejected by XPropertyContainer::addProperty" );

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
            failed( "insertion of duplicate property names is not rejected" );

        // try removing the property we just added - this should fail, as it does not have
        // the REMOVEABLE attribute
        caughtExpected = false;
        try
        {
            m_bag.removeProperty( VALUE);
        }
        catch(com.sun.star.beans.NotRemoveableException e) { caughtExpected = true; }
        catch(com.sun.star.uno.Exception e) { }
        if ( !caughtExpected )
            failed( "removing non-removeable properties is expected to fail - but it didn't" );

        // try removing a non-existent property
        caughtExpected = false;
        try
        {
            m_bag.removeProperty( "NonExistent" );
        }
        catch(com.sun.star.beans.UnknownPropertyException e) { caughtExpected = true; }
        catch(com.sun.star.uno.Exception e) { }
        if ( !caughtExpected )
            failed( "removing non-existent properties is expected to fail - but it didn't" );

        // try writing and reading a value for the one property we have so far
        try
        {
            final String testValue = "someArbitraryValue";
            m_set.setPropertyValue(  VALUE , testValue);
            final String currentValue = (String)m_set.getPropertyValue( VALUE);
            if ( !currentValue.equals( testValue ) )
                failed( "set property is not remembered" );
        }
        catch(com.sun.star.uno.Exception e)
        {
            failed( "setting or getting a property value failed" );
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
            failed( "the bag does not respect the property type we declared for the property" );
    }

    public void checkSequenceAccess() throws com.sun.star.uno.Exception
    {
        log.println( "checking PropertySetAccess via sequences" );
        createStandardBag( false );

        // ---------------------------------
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
                log.println( "property name : " + expectedValues[i].Name );
                log.println( "expected value: " + expectedValues[i].Value.toString() );
                log.println( "current value : " + value.toString() );
                failed( "retrieving a previously set property (" + expectedValues[i].Value.getClass().toString() + ") failed" );
            }
        }

        // ---------------------------------
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
                        log.println( "property name : " + expectedValues[j].Name );
                        log.println( "expected value: " + expectedValues[j].Value.toString() );
                        log.println( "current value : " + value.toString() );
                        failed( "getPropertyValues failed for property '" + name + "' failed" );
                    }
                    break;
                }
            }

            if ( !m_set.getPropertyValue( name ).equals( value ) )
                failed( "XPropertyAccess::getPropertyValues() and XPropertyset::getPropertyValue results are inconsistent" );
        }
    }

    public void checkDynamicSet() throws com.sun.star.uno.Exception
    {
        log.println( "checking proper dynamic of the set" );
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
            failed( "the set shouldn't accept unknown property values, if not explicitly told to do so" );

        // re-create the bag, this time allow it to implicitly add properties
        createStandardBag( true );
        boolean success = false;
        try { m_access.setPropertyValues( props ); success = true; }
        catch( com.sun.star.uno.Exception e ) { }
        if ( !success )
            failed( "property bag failed to implicitly add unknown properties" );

        // see whether this property was really added, and not just ignored
        final PropertyValue newlyAdded = props[ props.length - 1 ];
        try
        {
            if ( !m_set.getPropertyValue( newlyAdded.Name ).equals( newlyAdded.Value ) )
                failed( "the new property was not really added, or not added with the proper value" );
        }
        catch( com.sun.star.uno.Exception e ) { }
    }

    private void createEmptyBag()
    {
        try
        {
            m_bag = null;
            final String serviceName = "com.sun.star.beans.PropertyBag";
            m_bag = (XPropertyContainer)UnoRuntime.queryInterface( XPropertyContainer.class,
                m_orb.createInstance( serviceName )
            );
            if ( m_bag == null )
                failed( "could not create a " + serviceName + " instance" );
            m_set = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, m_bag );
            m_access = (XPropertyAccess)UnoRuntime.queryInterface( XPropertyAccess.class, m_bag );
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
            m_bag = (XPropertyContainer)UnoRuntime.queryInterface( XPropertyContainer.class,
                m_orb.createInstanceWithArguments( serviceName, initArgs )
            );
            if ( m_bag == null )
                failed( "could not create a " + serviceName + " instance" );
            m_set = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, m_bag );
            m_access = (XPropertyAccess)UnoRuntime.queryInterface( XPropertyAccess.class, m_bag );

            final Object properties[][] =
            {
                { "BoolValue", Boolean.TRUE },
                { "StringValue", "" },
                { "IntegerValue", Integer.valueOf( 3 ) },
                { "InterfaceValue", (XInterface)m_bag }
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
