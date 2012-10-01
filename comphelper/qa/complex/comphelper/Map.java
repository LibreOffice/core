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

package complex.comphelper;

import com.sun.star.beans.Pair;
import com.sun.star.container.ContainerEvent;
import com.sun.star.container.XContainer;
import com.sun.star.container.XContainerListener;
import com.sun.star.container.XElementAccess;
import com.sun.star.container.XEnumerableMap;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XIdentifierAccess;
import com.sun.star.container.XMap;
import com.sun.star.container.XSet;
import com.sun.star.form.XFormComponent;
// import com.sun.star.lang.DisposedException;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.Locale;
// import com.sun.star.lang.NoSupportException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.util.HashSet;
import java.util.Set;

// import org.junit.After;
import org.junit.AfterClass;
// import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/** complex test case for the css.container.Map implementation
 */
public class Map /* extends complexlib.ComplexTestCase */
{
//    @Override
//    public String[] getTestMethodNames()
//    {
//        return new String[] {
//            "testSimpleKeyTypes",
//            "testComplexKeyTypes",
//            "testValueTypes",
//            "testEnumerations",
//            "testSpecialValues"
//        };
//    }

//    public static String getShortTestDescription()
//    {
//        return "tests the css.container.Map implementation from comphelper/source/misc/map.cxx";
//    }

    private static String impl_getNth( int n )
    {
        switch ( n % 10 )
        {
            case 1: return n + "st";
            case 2: return n + "nd";
            default: return n + "th";
        }
    }

    private static void impl_putAll( XMap _map, Object[] _keys, Object[] _values ) throws com.sun.star.uno.Exception
    {
        for ( int i=0; i<_keys.length; ++i )
        {
            _map.put( _keys[i], _values[i] );
        }
    }

    private static void impl_checkContent( XMap _map, Object[] _keys, Object[] _values, String _context ) throws com.sun.star.uno.Exception
    {
        for ( int i=0; i<_keys.length; ++i )
        {
            assertTrue( _context + ": " + impl_getNth(i) + " key (" + _keys[i].toString() + ") not found in map",
                _map.containsKey( _keys[i] ) );
            assertTrue( _context + ": " + impl_getNth(i) + " value (" + _values[i].toString() + ") not found in map",
                _map.containsValue( _values[i] ) );
            assertEquals( _context + ": wrong value for " + impl_getNth(i) + " key (" + _keys[i] + ")",
                _values[i], _map.get( _keys[i] ) );
        }
    }

    @SuppressWarnings("unchecked")
    private static void impl_checkMappings( Object[] _keys, Object[] _values, String _context ) throws com.sun.star.uno.Exception
    {
        System.out.println( "checking mapping " + _context + "..." );

        Type keyType = AnyConverter.getType( _keys[0] );
        Type valueType = AnyConverter.getType( _values[0] );

        // create a map for the given types
        XMap map = com.sun.star.container.EnumerableMap.create( connection.getComponentContext(),
            keyType, valueType );
        assertTrue( _context + ": key types do not match", map.getKeyType().equals( keyType ) );
        assertTrue( _context + ": value types do not match", map.getValueType().equals( valueType ) );

        // insert all values
        assertTrue( _context + ": initially created map is not empty", map.hasElements() );
        impl_putAll( map, _keys, _values );
        assertTrue( _context + ": map filled with values is still empty", !map.hasElements() );
        // and verify them
        impl_checkContent( map, _keys, _values, _context );

        // remove all values
        for ( int i=_keys.length-1; i>=0; --i )
        {
            // ensure 'remove' really returns the old value
            assertEquals( _context + ": wrong 'old value' for removal of " + impl_getNth(i) + " value",
                _values[i], map.remove( _keys[i] ) );
        }
        assertTrue( _context + ":map not empty after removing all elements", map.hasElements() );

        // insert again, and check whether 'clear' does what it should do
        impl_putAll( map, _keys, _values );
        map.clear();
        assertTrue( _context + ": 'clear' does not empty the map", map.hasElements() );

        // try the constructor which creates an immutable version
        Pair< ?, ? >[] initialMappings = new Pair< ?, ? >[ _keys.length ];
        for ( int i=0; i<_keys.length; ++i )
        {
            initialMappings[i] = new Pair< Object, Object >( _keys[i], _values[i] );
        }
        map = com.sun.star.container.EnumerableMap.createImmutable(
            connection.getComponentContext(), keyType, valueType, (Pair< Object, Object >[])initialMappings );
        impl_checkContent( map, _keys, _values, _context );

        // check the thing is actually immutable
        //? assureException( map, "clear", new Object[] {}, NoSupportException.class );
        //? assureException( map, "remove", new Class[] { Object.class }, new Object[] { _keys[0] }, NoSupportException.class );
        //? assureException( map, "put", new Class[] { Object.class, Object.class }, new Object[] { _keys[0], _values[0] }, NoSupportException.class );
    }

    @Test public void testSimpleKeyTypes() throws com.sun.star.uno.Exception
    {
        impl_checkMappings(
            new Long[] { (long)1, (long)2, (long)3, (long)4, (long)5 },
            new Integer[] { 6, 7, 8, 9, 10 },
            "long->int"
        );
        impl_checkMappings(
            new Boolean[] { true, false },
            new Short[] { (short)1, (short)0 },
            "bool->short"
        );
        impl_checkMappings(
            new String[] { "one", "two", "three", "four", "five"},
            new String[] { "1", "2", "3", "4", "5" },
            "string->string"
        );
        impl_checkMappings(
            new Double[] { 1.2, 3.4, 5.6, 7.8, 9.10 },
            new Float[] { (float)1, (float)2, (float)3, (float)4, (float)5 },
            "double->float"
        );
        impl_checkMappings(
            new Float[] { (float)1, (float)2, (float)3, (float)4, (float)5 },
            new Double[] { 1.2, 3.4, 5.6, 7.8, 9.10 },
            "float->double"
        );
        impl_checkMappings(
            new Integer[] { 2, 9, 2005, 20, 11, 1970, 26, 3, 1974 },
            new String[] { "2nd", "September", "2005", "20th", "November", "1970", "26th", "March", "1974" },
            "int->string"
        );
    }

    @SuppressWarnings("unchecked")
    @Test public void testComplexKeyTypes() throws com.sun.star.uno.Exception
    {
        Type intType = new Type( Integer.class );
        Type longType = new Type( Long.class );
        Type msfType = new Type ( XMultiServiceFactory.class );
        // ....................................................................
        // css.uno.Type should be a valid key type
        impl_checkMappings(
            new Type[] { intType, longType, msfType },
            new String[] { intType.getTypeName(), longType.getTypeName(), msfType.getTypeName() },
            "type->string"
        );

        // ....................................................................
        // any UNO interface type should be a valid key type.
        // Try with some form components (just because I like form components :), and the very first application
        // for the newly implemented map will be to map XFormComponents to drawing shapes
        String[] serviceNames = new String[] { "CheckBox", "ComboBox", "CommandButton", "DateField", "FileControl" };
        Object[] components = new Object[ serviceNames.length ];
        for ( int i=0; i<serviceNames.length; ++i )
        {
            components[i] = getMSF().createInstance( "com.sun.star.form.component." + serviceNames[i] );
        }
        // "normalize" the first component, so it has the property type
        Type formComponentType = new Type( XFormComponent.class );
        components[0] = UnoRuntime.queryInterface( formComponentType.getZClass(), components[0] );
        impl_checkMappings( components, serviceNames, "XFormComponent->string" );

        // ....................................................................
        // any UNO enum type should be a valid key type
        impl_checkMappings(
            new TypeClass[] { intType.getTypeClass(), longType.getTypeClass(), msfType.getTypeClass() },
            new Object[] { "foo", "bar", "42" },
            "enum->string"
        );
    }

    private static Class<?> impl_getValueClassByPos( int _pos )
    {
        Class<?> valueClass = null;
        switch ( _pos )
        {
            case 0: valueClass = Boolean.class; break;
            case 1: valueClass = Short.class; break;
            case 2: valueClass = Integer.class; break;
            case 3: valueClass = Long.class; break;
            case 4: valueClass = XInterface.class; break;
            case 5: valueClass = XSet.class; break;
            case 6: valueClass = XContainer.class; break;
            case 7: valueClass = XIdentifierAccess.class; break;
            case 8: valueClass = XElementAccess.class; break;
            case 9: valueClass = com.sun.star.uno.Exception.class; break;
            case 10: valueClass = com.sun.star.uno.RuntimeException.class; break;
            case 11: valueClass = EventObject.class; break;
            case 12: valueClass = ContainerEvent.class; break;
            case 13: valueClass = Object.class; break;
            default:
                fail( "internal error: wrong position for getValueClass" );
        }
        return valueClass;
    }

    private Object impl_getSomeValueByTypePos( int _pos )
    {
        Object someValue = null;
        switch ( _pos )
        {
            case 0: someValue = new Boolean( false ); break;
            case 1: someValue = new Short( (short)0 ); break;
            case 2: someValue = new Integer( 0 ); break;
            case 3: someValue = new Long( 0 ); break;
            case 4: someValue = UnoRuntime.queryInterface( XInterface.class, new DummyInterface() ); break;
            case 5: someValue = UnoRuntime.queryInterface( XSet.class, new DummySet() ); break;
            case 6: someValue = UnoRuntime.queryInterface( XContainer.class, new DummyContainer() ); break;
            case 7: someValue = UnoRuntime.queryInterface( XIdentifierAccess.class, new DummyIdentifierAccess() ); break;
            case 8: someValue = UnoRuntime.queryInterface( XElementAccess.class, new DummyElementAccess() ); break;
            case 9: someValue = new com.sun.star.uno.Exception(); break;
            case 10: someValue = new com.sun.star.uno.RuntimeException(); break;
            case 11: someValue = new EventObject(); break;
            case 12: someValue = new ContainerEvent(); break;
            case 13: someValue = new Locale(); break;   // just use *any* value which does not conflict with the others
            default:
                fail( "internal error: wrong position for getSomeValue" );
        }
        return someValue;
    }

    private class DummyInterface implements XInterface
    {
    }

    private class DummySet implements XSet
    {
        public boolean has( Object arg0 )       { throw new UnsupportedOperationException( "Not implemented." ); }
        public void insert( Object arg0 )       { throw new UnsupportedOperationException( "Not implemented." ); }
        public void remove( Object arg0 )       { throw new UnsupportedOperationException( "Not implemented." ); }
        public XEnumeration createEnumeration() { throw new UnsupportedOperationException( "Not implemented." ); }
        public Type getElementType()            { throw new UnsupportedOperationException( "Not implemented." ); }
        public boolean hasElements()            { throw new UnsupportedOperationException( "Not implemented." ); }
    }

    private class DummyContainer implements XContainer
    {
        public void addContainerListener( XContainerListener arg0 ) { throw new UnsupportedOperationException( "Not implemented." ); }
        public void removeContainerListener( XContainerListener arg0 ) { throw new UnsupportedOperationException( "Not implemented." ); }
    }

    private class DummyIdentifierAccess implements XIdentifierAccess
    {
        public Object getByIdentifier( int arg0 ) { throw new UnsupportedOperationException( "Not implemented." ); }
        public int[] getIdentifiers() { throw new UnsupportedOperationException( "Not implemented." ); }
        public Type getElementType() { throw new UnsupportedOperationException( "Not implemented." ); }
        public boolean hasElements() { throw new UnsupportedOperationException( "Not implemented." ); }
    }

    private class DummyElementAccess implements XElementAccess
    {
        public Type getElementType() { throw new UnsupportedOperationException( "Not implemented." ); }
        public boolean hasElements() { throw new UnsupportedOperationException( "Not implemented." ); }
    }

    @Test public void testValueTypes() throws com.sun.star.uno.Exception
    {
        // type compatibility matrix: rows are the value types used to create the map,
        // columns are the value types fed into the map. A value "1" means the respective type
        // should be accepted.
        Integer[][] typeCompatibility = new Integer[][] {
            /* boolean           */ new Integer[] { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
            /* short             */ new Integer[] { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
            /* int               */ new Integer[] { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
            /* long              */ new Integer[] { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
            /* XInterface        */ new Integer[] { 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },
            /* XSet              */ new Integer[] { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
            /* XContainer        */ new Integer[] { 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
            /* XIdentifierAccess */ new Integer[] { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
            /* XElementAccess    */ new Integer[] { 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0 },
            /* Exception         */ new Integer[] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0 },
            /* RuntimeException  */ new Integer[] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
            /* EventObject       */ new Integer[] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0 },
            /* ContainerEvent    */ new Integer[] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },
            /* any               */ new Integer[] { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        };
        // several asects are checked with this compatibility matrix:
        // - if a map's value type is a scalar type, or a string, then nothing but this
        //   type should be accepted
        // - if a map's value type is an interface type, then values should be accepted if
        //   they contain a derived interface, or the interrface itself, or if they can be
        //   queried for this interface (actually, the latter rule is not tested with the
        //   above matrix)
        // - if a map's value type is a struct or exception, then values should be accepted
        //   if they are of the given type, or of a derived type.
        // - if a map's value type is "any", then, well, any value should be accepted

        for ( int valueTypePos = 0; valueTypePos != typeCompatibility.length; ++valueTypePos )
        {
            com.sun.star.container.EnumerableMap.create( connection.getComponentContext(),
                new Type( Integer.class ), new Type( impl_getValueClassByPos( valueTypePos ) ) );

            for ( int checkTypePos = 0; checkTypePos != typeCompatibility[valueTypePos].length; ++checkTypePos )
            {
                impl_getSomeValueByTypePos( checkTypePos );
                if ( typeCompatibility[valueTypePos][checkTypePos] != 0 )
                {
                    // expected to succeed
//?                    assureException(
//?                        "(" + valueTypePos + "," + checkTypePos + ") putting an " +
//?                            AnyConverter.getType( value ).getTypeName() + ", where " +
//?                            map.getValueType().getTypeName() + " is expected, should succeed",
//?                        map, "put", new Class[] { Object.class, Object.class }, new Object[] { key, value },
//?                        null );
                }
                else
                {
                    // expected to fail
//?                    assureException(
//?                        "(" + valueTypePos + "," + checkTypePos + ") putting an " +
//?                            AnyConverter.getType( value ).getTypeName() + ", where " +
//?                            map.getValueType().getTypeName() + " is expected, should not succeed",
//?                        map, "put", new Class[] { Object.class, Object.class }, new Object[] { key, value },
//?                        IllegalTypeException.class );
                }
            }
        }
    }

    private interface CompareEqual
    {
        public boolean areEqual( Object _lhs, Object _rhs );
    }

    private class DefaultCompareEqual implements CompareEqual
    {
        public boolean areEqual( Object _lhs, Object _rhs )
        {
            return _lhs.equals( _rhs );
        }
    }

    private class PairCompareEqual implements CompareEqual
    {
        public boolean areEqual( Object _lhs, Object _rhs )
        {
            Pair< ?, ? > lhs = (Pair< ?, ? >)_lhs;
            Pair< ?, ? > rhs = (Pair< ?, ? >)_rhs;
            return lhs.First.equals( rhs.First ) && lhs.Second.equals( rhs.Second );
        }
    }

    private void impl_verifyEnumerationContent( XEnumeration _enum, final Object[] _expectedElements, final String _context )
        throws com.sun.star.uno.Exception
    {
        // since we cannot assume the map to preserve the ordering in which the elements where inserted,
        // we can only verify that all elements exist as expected, plus *no more* elements than expected
        // are provided by the enumeration
        Set<Integer> set = new HashSet<Integer>();
        for ( int i=0; i<_expectedElements.length; ++i )
        {
            set.add( i );
        }

        CompareEqual comparator = _expectedElements[0].getClass().equals( Pair.class )
                                ? new PairCompareEqual()
                                : new DefaultCompareEqual();

        for ( int i=0; i<_expectedElements.length; ++i )
        {
            assertTrue( _context + ": too few elements in the enumeration (still " + ( _expectedElements.length - i ) + " to go)",
                _enum.hasMoreElements() );

            Object nextElement = _enum.nextElement();
            if ( nextElement.getClass().equals( Any.class ) )
            {
                nextElement = ((Any)nextElement).getObject();
            }

            int foundPos = -1;
            for ( int j=0; j<_expectedElements.length; ++j )
            {
                if ( comparator.areEqual( _expectedElements[j], nextElement ) )
                {
                    foundPos = j;
                    break;
                }
            }

            assertTrue( _context + ": '" + nextElement.toString() + "' is not expected in the enumeration",
                set.contains( foundPos ) );
            set.remove( foundPos );
        }
        assertTrue( _context + ": too many elements returned by the enumeration", set.isEmpty() );
    }

    @Test public void testEnumerations() throws com.sun.star.uno.Exception
    {
        // fill a map
        final String[] keys = new String[] { "This", "is", "an", "enumeration", "test" };
        final String[] values = new String[] { "for", "the", "map", "implementation", "." };
        XEnumerableMap map = com.sun.star.container.EnumerableMap.create( connection.getComponentContext(), new Type( String.class ), new Type( String.class ) );
        impl_putAll( map, keys, values );

        final Pair< ?, ? >[] paired = new Pair< ?, ? >[ keys.length ];
        for ( int i=0; i<keys.length; ++i )
        {
            paired[i] = new Pair< Object, Object >( keys[i], values[i] );
        }

        // create non-isolated enumerators, and check their content
        XEnumeration enumerateKeys = map.createKeyEnumeration( false );
        XEnumeration enumerateValues = map.createValueEnumeration( false );
        XEnumeration enumerateAll = map.createElementEnumeration( false );
        impl_verifyEnumerationContent( enumerateKeys, keys, "key enumeration" );
        impl_verifyEnumerationContent( enumerateValues, values, "value enumeration" );
        impl_verifyEnumerationContent( enumerateAll, paired, "content enumeration" );

        // all enumerators above have been created as non-isolated iterators, so they're expected to die when
        // the underlying map changes
        map.remove( keys[0] );
//?        assureException( enumerateKeys, "hasMoreElements", new Object[] {}, DisposedException.class );
//?        assureException( enumerateValues, "hasMoreElements", new Object[] {}, DisposedException.class );
//?        assureException( enumerateAll, "hasMoreElements", new Object[] {}, DisposedException.class );

        // now try with isolated iterators
        map.put( keys[0], values[0] );
        enumerateKeys = map.createKeyEnumeration( true );
        enumerateValues = map.createValueEnumeration( true );
        enumerateAll = map.createElementEnumeration( true );
        map.put( "additional", "value" );
        impl_verifyEnumerationContent( enumerateKeys, keys, "key enumeration" );
        impl_verifyEnumerationContent( enumerateValues, values, "value enumeration" );
        impl_verifyEnumerationContent( enumerateAll, paired, "content enumeration" );
    }

    @Test public void testSpecialValues() throws com.sun.star.uno.Exception
    {
        final Double[] keys = new Double[] { new Double( 0 ), Double.POSITIVE_INFINITY, Double.NEGATIVE_INFINITY };
        final Double[] values = new Double[] { Double.NEGATIVE_INFINITY, Double.POSITIVE_INFINITY, new Double( 0 ) };

        XEnumerableMap map = com.sun.star.container.EnumerableMap.create( connection.getComponentContext(), new Type( Double.class ), new Type( Double.class ) );
        impl_putAll( map, keys, values );

        assertTrue( "containsKey( Double.+INF failed", map.containsKey( Double.POSITIVE_INFINITY ) );
        assertTrue( "containsKey( Double.-INF failed", map.containsKey( Double.NEGATIVE_INFINITY ) );
        assertTrue( "containsKey( 0 ) failed", map.containsKey( new Double( 0 ) ) );

        assertTrue( "containsValue( Double.+INF ) failed", map.containsValue( Double.POSITIVE_INFINITY ) );
        assertTrue( "containsValue( Double.-INF ) failed", map.containsValue( Double.NEGATIVE_INFINITY ) );
        assertTrue( "containsValue( 0 ) failed", map.containsValue( new Double( 0 ) ) );

        // put and containsKey should reject Double.NaN as key
//?        assureException( "Double.NaN should not be allowed as key in a call to 'put'", map, "put",
//?            new Class[] { Object.class, Object.class }, new Object[] { Double.NaN, new Double( 0 ) },
//?            com.sun.star.lang.IllegalArgumentException.class );
//?        assureException( "Double.NaN should not be allowed as key in a call to 'containsKey'", map, "containsKey",
//?            new Class[] { Object.class }, new Object[] { Double.NaN },
//?            com.sun.star.lang.IllegalArgumentException.class );

        // ditto for put and containsValue
//?        assureException( "Double.NaN should not be allowed as value in a call to 'put'", map, "put",
//?            new Class[] { Object.class, Object.class }, new Object[] { new Double( 0 ), Double.NaN },
//?            com.sun.star.lang.IllegalArgumentException.class );
//?        assureException( "Double.NaN should not be allowed as key in a call to 'containsValue'", map, "containsValue",
//?            new Class[] { Object.class }, new Object[] { Double.NaN },
//?            com.sun.star.lang.IllegalArgumentException.class );
    }


    private static XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();
}
