/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScriptProxy.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:04:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
************************************************************************/

package com.sun.star.script.framework.provider.java;

import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

/**
 * A ScriptProxy object acts as a proxy for a Java <code>Method</code>
 *
 * @author     Tomas O'Connor
 * @created    August 2, 2002
 * @see        java.lang.reflect.Method
 */
public class ScriptProxy
{
    private Object m_targetObject;
    private Method m_method;


    /**
     * Constructs a <code>ScriptProxy</code> object for the given
     * <code>Method</code>
     *
     * @param  method  Description of the Parameter
     */
    public ScriptProxy( Method method )
    {
        this.m_method = method;
    }


    /**
     * Sets the <code>Object</code> on which the ScriptProxy should invoke
     * the method
     *
     * @param  obj  The new targetObject value
     */
    public void setTargetObject( Object obj )
    {
        m_targetObject = obj;
    }


    /**
     * Invokes the method contained in this <code>ScriptProxy</code>,
     * any exceptions resulting from the invocation will be thrown
     *
     * @param  args                           the arguments to be passed when invoking
     *                                          the method
     * @return                                the Object returned from the method
     *                                          invocation, may be null
     * @exception  IllegalAccessException     Description of the Exception
     * @exception  InvocationTargetException  Description of the Exception
     * @exception  IllegalArgumentException   Description of the Exception
     * @exception  Exception                  Description of the Exception
     * @see                                   java.lang.reflect.Method for the exceptions
     *                                          that may be thrown
     */
    public  Object invoke( Object[] args )
    throws IllegalAccessException, InvocationTargetException,
                IllegalArgumentException
    {
        return m_method.invoke( m_targetObject, args );
    }
}

