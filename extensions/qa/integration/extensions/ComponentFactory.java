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

import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XSingleComponentFactory;
import java.lang.reflect.Constructor;

public class ComponentFactory implements XSingleComponentFactory
{
    private Class       m_handlerClass;
    private Constructor m_defaultConstructor;
    private Constructor m_initConstructor;

    public ComponentFactory( Class _handlerClass )
    {
        m_handlerClass = _handlerClass;

        Class objectArrayClass = null;
        try
        {
            objectArrayClass = Class.forName("[Ljava.lang.Object;");
        }
        catch ( java.lang.ClassNotFoundException e ) { }

        Constructor ctors[] = _handlerClass.getConstructors();
        for ( int i = 0; i < ctors.length && ctors != null; ++i)
        {
            Class ctorParams[] = ctors[i].getParameterTypes();
            if ( ( ctorParams.length == 1 ) && ( ctorParams[0].equals( XComponentContext.class ) ) )
                m_defaultConstructor = ctors[i];
            if  (   ( ctorParams.length == 2 )
                &&  ( ctorParams[0].equals( XComponentContext.class ) )
                &&  ( ctorParams[1].equals( objectArrayClass ) )
                )
                m_initConstructor = ctors[i];
        }
        if ( m_defaultConstructor == null )
            throw new java.lang.IllegalArgumentException();
    }

    private Object ipml_createInstance( Constructor _ctor, Object[] _arguments )
    {
        Object newInstance = null;
        try
        {
            newInstance = _ctor.newInstance( _arguments );
        }
        catch( InstantiationException e )
        {
            System.err.println( "InstantiationException: Could not instantiate an instance of " + m_handlerClass.getName() );
        }
        catch( IllegalAccessException e )
        {
            System.err.println( "IllegalAccessException: Could not instantiate an instance of " + m_handlerClass.getName() );
        }
        catch( java.lang.reflect.InvocationTargetException e )
        {
            System.err.println( "InvocationTargetException: Could not instantiate an instance of " + m_handlerClass.getName() );
        }
        return newInstance;
    }

    public Object createInstanceWithArgumentsAndContext(Object[] _arguments, XComponentContext _componentContext) throws com.sun.star.uno.Exception
    {
        if ( m_initConstructor != null )
            return ipml_createInstance( m_initConstructor, new Object[] { _componentContext, _arguments } );
        else
            return createInstanceWithContext( _componentContext );
    }

    public Object createInstanceWithContext(XComponentContext _componentContext) throws com.sun.star.uno.Exception
    {
        return ipml_createInstance( m_defaultConstructor, new Object[] { _componentContext } );
    }
}

