/*************************************************************************
*
*  $RCSfile: ScriptDescriptor.java,v $
*
*  $Revision: 1.1 $
*
*  last change: $Author: toconnor $ $Date: 2003-09-10 10:44:31 $
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

package com.sun.star.script.framework.provider.java;

import java.util.Vector;
import java.util.StringTokenizer;

/**
 * The <code>ScriptDescriptor</code> object is used to store the search
 * criteria that should be used for finding a particular script
 *
 * @author     Tomas O'Connor
 * @created    August 2, 2002
 */
public class ScriptDescriptor
{
    private String m_name;
    private String m_methodName;
    private String m_className;
    private Vector m_classpath;
    private Vector m_argumentTypes = new Vector( 11 );


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
    public void setClasspath( Vector classpath )
    {
        this.m_classpath = classpath;
    }


    /**
     * Gets the classpath value stored by this <code>ScriptDescriptor</code>
     *
     * @return    The classpath value
     */
    public Vector getClasspath()
    {
        return m_classpath;
    }


    /**
     * Adds the given <code>Class</code> to the list of argument types stored in
     * this ScriptDescriptor
     *
     * @param  clazz  The feature to be added to the ArgumentType attribute
     */
    public synchronized void addArgumentType( Class clazz )
    {
        m_argumentTypes.addElement( clazz );
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
    public synchronized Class[]
    getArgumentTypes()
    {
        if ( m_argumentTypes.size() > 0 )
            return ( Class[] ) m_argumentTypes.toArray( new Class[ 0 ] );
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

