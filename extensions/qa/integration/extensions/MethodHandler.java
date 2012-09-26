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

package integration.extensions;

import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.beans.*;
import com.sun.star.reflection.*;
import com.sun.star.inspection.*;

public class MethodHandler implements XPropertyHandler
{
    private XComponentContext       m_context;
    private XIntrospection          m_introspection;
    private XIntrospectionAccess    m_introspectionAccess;
    private XIdlClass               m_idlClass;
    private XIdlMethod[]            m_methods;
    private java.util.HashMap       m_methodsHash;

    /** Creates a new instance of MethodHandler */
    public MethodHandler( XComponentContext _context )
    {
        m_context = _context;
        m_methodsHash = new java.util.HashMap();

        try
        {
            m_introspection = (XIntrospection)UnoRuntime.queryInterface( XIntrospection.class,
                m_context.getServiceManager().createInstanceWithContext( "com.sun.star.beans.Introspection", m_context )
            );
        }
        catch( com.sun.star.uno.Exception e )
        {
            System.err.println( "MethodHandler: could not create a Introspection service, not much functionality will be available." );
        }
    }

    static public XSingleComponentFactory getFactory()
    {
        return new ComponentFactory( MethodHandler.class );
    }

    public void actuatingPropertyChanged(String _propertyName, Object _newValue, Object _oldValue, com.sun.star.inspection.XObjectInspectorUI _objectInspectorUI, boolean _firstTimeInit) throws com.sun.star.lang.NullPointerException
    {
        // not interested in
    }

    public void addEventListener(com.sun.star.lang.XEventListener _eventListener)
    {
        // ingnoring this
    }

    public void addPropertyChangeListener(com.sun.star.beans.XPropertyChangeListener _propertyChangeListener) throws com.sun.star.lang.NullPointerException
    {
        // ingnoring this
    }

    public Object convertToControlValue(String _propertyName, Object _propertyValue, com.sun.star.uno.Type type) throws com.sun.star.beans.UnknownPropertyException
    {
        return _propertyValue;
    }

    public Object convertToPropertyValue(String _propertyName, Object _controlValue) throws com.sun.star.beans.UnknownPropertyException
    {
        return _controlValue;
    }

    public com.sun.star.inspection.LineDescriptor describePropertyLine(String _propertyName, com.sun.star.inspection.XPropertyControlFactory _propertyControlFactory) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.lang.NullPointerException
    {
        com.sun.star.inspection.LineDescriptor descriptor = new com.sun.star.inspection.LineDescriptor();

        descriptor = new LineDescriptor();
        descriptor.Category = "Methods";
        descriptor.DisplayName = "has method";
        descriptor.HasPrimaryButton = descriptor.HasSecondaryButton = false;
        descriptor.IndentLevel = 0;
        try
        {
            XPropertyControl control = (XPropertyControl)UnoRuntime.queryInterface(
                    XPropertyControl.class, _propertyControlFactory.createPropertyControl(
                    PropertyControlType.TextField, true ) );

            descriptor.Control = control;
        }
        catch( com.sun.star.lang.IllegalArgumentException e )
        {
        }
        return descriptor;
    }

    public void dispose()
    {
        // nothing to do
    }

    public String[] getActuatingProperties()
    {
        // none
        return new String[] { };
    }

    public com.sun.star.beans.PropertyState getPropertyState(String _propertyName) throws com.sun.star.beans.UnknownPropertyException
    {
        return com.sun.star.beans.PropertyState.DIRECT_VALUE;
    }

    public Object getPropertyValue(String _propertyName) throws com.sun.star.beans.UnknownPropertyException
    {
        XIdlMethod method = impl_getMethod( _propertyName );

        String signature = new String();
        signature += method.getReturnType().getName();
        signature += " ";
        signature += method.getName();

        signature += "(";

        XIdlClass[] parameterTypes = method.getParameterTypes();
        for ( int param = 0; param<parameterTypes.length; ++param )
        {
            signature += ( param == 0 ) ? " " : ", ";
            signature += parameterTypes[param].getName();
        }

        signature += " )";
        return signature;
    }

    public String[] getSupersededProperties()
    {
        return new String[] {  };
    }

    public com.sun.star.beans.Property[] getSupportedProperties()
    {
        Property[] properties = new Property[] { };
        if ( m_methods != null )
        {
            properties = new Property[ m_methods.length ];
            for ( int i=0; i<m_methods.length; ++i )
            {
                properties[i] = new Property( m_methods[i].getName(), 0, new Type( String.class ), (short)0 );
                m_methodsHash.put( m_methods[i].getName(), m_methods[i] );
            }
        }
        return properties;
    }

    public void inspect(Object _component) throws com.sun.star.lang.NullPointerException
    {
        if ( m_introspection == null )
            return;

        m_introspectionAccess = null;
        m_methods = null;
        m_methodsHash = new java.util.HashMap();

        m_introspectionAccess = m_introspection.inspect( _component );
        if ( m_introspectionAccess == null )
            return;

        m_methods = m_introspectionAccess.getMethods( MethodConcept.ALL );
    }

    public boolean isComposable(String _propertyName) throws com.sun.star.beans.UnknownPropertyException
    {
        return true;
    }

    public com.sun.star.inspection.InteractiveSelectionResult onInteractivePropertySelection(String str, boolean param, Object[] obj, com.sun.star.inspection.XObjectInspectorUI xObjectInspectorUI) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.lang.NullPointerException
    {
        return InteractiveSelectionResult.Cancelled;
    }

    public void removeEventListener(com.sun.star.lang.XEventListener _eventListener)
    {
        // ignoring this
    }

    public void removePropertyChangeListener(com.sun.star.beans.XPropertyChangeListener _propertyChangeListener)
    {
        // ignoring this
    }

    public void setPropertyValue(String str, Object obj) throws com.sun.star.beans.UnknownPropertyException
    {
        // we declared our properties as readonly
        throw new java.lang.RuntimeException();
    }

    public boolean suspend(boolean param)
    {
        return true;
    }

    /** returns the descriptor for the method with the given name
     *  @param _propertyName
     *      the name of the method whose descriptor should be obtained
     *  @throws com.sun.star.beans.UnknownPropertyException
     *      if we don't have a method hash, or the given property name does not denote a method of our inspectee
     */
    private XIdlMethod impl_getMethod( String _methodName ) throws UnknownPropertyException
    {
        XIdlMethod method = (XIdlMethod)m_methodsHash.get( _methodName );
        if ( method == null )
            throw new com.sun.star.beans.UnknownPropertyException();

        return method;
    }
}
