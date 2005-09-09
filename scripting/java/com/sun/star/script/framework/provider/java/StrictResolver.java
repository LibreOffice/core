/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StrictResolver.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:04:29 $
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

