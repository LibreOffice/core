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
    private Method resolveArguments( ScriptDescriptor sd, Class<?> c )
    throws ClassNotFoundException, NoSuchMethodException
    {
        return c.getMethod( sd.getMethodName(), sd.getArgumentTypes() );
    }
}

