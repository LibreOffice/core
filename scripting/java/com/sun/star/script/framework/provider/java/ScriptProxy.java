/*************************************************************************
*
*  $RCSfile: ScriptProxy.java,v $
*
*  $Revision: 1.1 $
*
*  last change: $Author: toconnor $ $Date: 2003-09-10 10:44:32 $
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
                IllegalArgumentException, Exception
    {
        return m_method.invoke( m_targetObject, args );
    }
}

