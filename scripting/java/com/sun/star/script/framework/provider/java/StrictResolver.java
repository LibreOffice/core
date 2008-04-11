/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StrictResolver.java,v $
 * $Revision: 1.3 $
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

package com.sun.star.script.framework.provider.java;

import java.lang.reflect.*;
import com.sun.star.script.framework.log.LogUtils;

/**
 * A StrictResolver can be used to get a ScriptProxy object for a given
 * ScriptDescriptor and Class. The StrictResolver is an implementation of
 * the Resolver strategy. It will only return a ScriptProxy object if a
 * method accepting all of the arguments specified in the ScriptDescriptor
 * can be found in the Class.
 *
 * @author     Tomas O'Connor
 * @created    August 2, 2002
 */
public class StrictResolver implements Resolver
{
    /**
     *Constructor for the StrictResolver object
     */
    public StrictResolver()
    {
        LogUtils.DEBUG( this.getClass().getName() + " created" );
    }


    /**
     * Returns a ScriptProxy object for the given ScriptDescriptor and Class.
     * Only a strict match will be returned ie. where all of the arguments in
     * the given ScriptDescriptor match the types of the
     *
     * @param  sd  the ScriptDescriptor for which a ScriptProxy is required
     * @param  c   the Class file in which to search for the method
     * @return     the ScriptProxy matching the criteria, or null if no match is found
     */
    public ScriptProxy getProxy( ScriptDescriptor sd, Class c )
    throws NoSuchMethodException
    {
        Method m = null;
        ScriptProxy sp = null;

        LogUtils.DEBUG( "StrictResolver.getProxy() for: " + sd.toString() );

        try
        {
            m = resolveArguments( sd, c );
        }
        catch ( ClassNotFoundException e )
        {
            throw new NoSuchMethodException( "StrictResolver.getProxy: Cant find method: "
            + sd.getMethodName() + ":" + e.getMessage() );
        }
        catch ( NoSuchMethodException e )
        {
            throw new NoSuchMethodException( "StrictResolver.getProxy: Cant find method: "
                + sd.getMethodName() + ":" + e.getMessage() );
        }

        sp = new ScriptProxy( m );

        int modifiers = m.getModifiers();
        if ( !Modifier.isStatic( modifiers ) )
        {
            Object o;
            try
            {
                o = c.newInstance();
            }
            catch ( InstantiationException ie )
            {
                throw new NoSuchMethodException( "getScriptProxy: Cant instantiate: " +
                    c.getName() );
            }
            catch ( IllegalAccessException iae )
            {
                throw new NoSuchMethodException( "getScriptProxy: Cant access: "
                    + c.getName() );
            }
            sp.setTargetObject( o );
        }

        return sp;
    }


    /**
     *  Description of the Method
     *
     * @param  sd                          Description of the Parameter
     * @param  c                           Description of the Parameter
     * @return                             Description of the Return Value
     * @exception  ClassNotFoundException
     * @exception  NoSuchMethodException
     */
    private Method resolveArguments( ScriptDescriptor sd, Class c )
    throws ClassNotFoundException, NoSuchMethodException
    {
        return c.getMethod( sd.getMethodName(), sd.getArgumentTypes() );
    }
}

