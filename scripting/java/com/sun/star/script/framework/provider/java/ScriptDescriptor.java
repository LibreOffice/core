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

package com.sun.star.script.framework.provider.java;

import java.util.List;
import java.util.ArrayList;
import java.util.StringTokenizer;

/**
 * The <code>ScriptDescriptor</code> object is used to store the search
 * criteria that should be used for finding a particular script
 *
 * @author     Tomas O'Connor
 */
public class ScriptDescriptor
{
    private String m_name;
    private String m_methodName;
    private String m_className;
    private List<String> m_classpath;
    private ArrayList<Class<?>> m_argumentTypes = new ArrayList<Class<?>>( 11 );


    /**
     * Constructs a ScriptDescriptor for the given name
     *
     * @param  name                          Script Name
     * @exception  IllegalArgumentException  if the given name does not contain a "."
     */
    public ScriptDescriptor( String name )
    throws IllegalArgumentException
    {
        int idx = name.lastIndexOf( '.' );

        if ( idx == -1 )
        {
            throw new IllegalArgumentException( "Invalid method name" );
        }

        this.m_name = name;
        this.m_methodName = name.substring( idx + 1 );
        this.m_className = name.substring( 0, idx );
    }


    /**
     * Gets the fully qualified name of this <code>ScriptDescriptor</code>
     *
     * @return    The Script Name value
     */
    public String getName()
    {
        return m_name;
    }

    /**
     * Gets the fully qualified name of this <code>ScriptDescriptor</code>
     *
     * @return    The Script Name value
     */
    public String getClassName()
    {
        return m_className;
    }


    /**
     * Gets the method name of this <code>ScriptDescriptor</code>
     *
     * @return    The methodName value
     */
    public String getMethodName()
    {
        return m_methodName;
    }


    /**
     * Sets the classpath value stored by this <code>ScriptDescriptor</code>
     *
     * @param  classpath  The new classpath value
     */
    public void setClasspath( String classpath )
    {
        StringTokenizer stk = new StringTokenizer( classpath, ":" );
        while( stk.hasMoreElements() )
        {
            this.m_classpath.add( (String) stk.nextElement() );
        }
    }

    /**
     * Sets the classpath value stored by this <code>ScriptDescriptor</code>
     *
     * @param  classpath  The new classpath value
     */
    public void setClasspath( List<String> classpath )
    {
        this.m_classpath = classpath;
    }


    /**
     * Gets the classpath value stored by this <code>ScriptDescriptor</code>
     *
     * @return    The classpath value
     */
    public List<String> getClasspath()
    {
        return m_classpath;
    }


    /**
     * Adds the given <code>Class</code> to the list of argument types stored in
     * this ScriptDescriptor
     *
     * @param  clazz  The feature to be added to the ArgumentType attribute
     */
    public synchronized void addArgumentType( Class<?> clazz )
    {
        m_argumentTypes.add( clazz );
    }


    /**
     * Adds the given array of <code>Class</code> to the list of argument types
     * stored in this ScriptDescriptor
     *
     * @param  classes  The feature to be added to the ArgumentTypes attribute
     */
    public synchronized void addArgumentTypes( Class[] classes )
    {
        for ( int i = 0; i < classes.length; i++ )
        {
            addArgumentType( classes[ i ] );
        }
    }


    /**
     * Gets a list of the types of the arguments stored in this
     * <code>ScriptDescriptor</code>
     *
     * return the argument types as an array of Class
     *
     * @return    The argumentTypes value
     */
    public synchronized Class<?>[]
    getArgumentTypes()
    {
        if ( m_argumentTypes.size() > 0 )
            return m_argumentTypes.toArray( new Class[ m_argumentTypes.size() ] );
        else
            return null;
    }


    /**
     * Returns a <code>String</code> representation of this
     * <code>ScriptDescriptor</code>
     *
     * @return    The scriptName including the parameters.
     */
    public String toString()
    {
        StringBuffer description = new StringBuffer( m_name );
        Class[] types = getArgumentTypes();

        description.append( " (" );

        if ( types != null )
        {
            for ( int i = 0; i < types.length - 1; i++ )
            {
                description.append( types[ i ].getName() );
                description.append( ", " );
            }

            description.append( types[ types.length - 1 ].getName() );
        }
        description.append( ")" );

        return description.toString();
    }
}

