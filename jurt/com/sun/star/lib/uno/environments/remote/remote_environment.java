/*************************************************************************
 *
 *  $RCSfile: remote_environment.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:27:53 $
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

package com.sun.star.lib.uno.environments.remote;


import java.util.Enumeration;


import com.sun.star.lib.util.WeakTable;

import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.UnoRuntime;


public class remote_environment implements IEnvironment {
    public static final boolean DEBUG = false;

    protected    String _name;
    protected WeakTable _objects = new WeakTable();
    protected    Object _context; // free context pointer, that can be used for specific classes of environments,

    protected remote_environment(String name, Object context) {
        if(DEBUG) System.err.println("#### AbstractEnvironment.<init>");

        _name    = name;
        _context = context;
    }

    public remote_environment(Object context) {
        this("remote", context);
    }

    /**
     * the context
     */
    public Object getContext() {
        return _context;
    }

    /**
     * a name for this environment
     */
    public String getName() {
        return _name;
    }

    /**
     * Tests if two environments are equal.
     *<BR>
     * @param environment       one environment
     */
    public boolean equals(Object object) {
        return false;
    }

    /**
     * You register internal and external interfaces via this method. Internal interfaces are
     * proxies that are used in an environment. External interfaces are interfaces that are
     * exported to another environment, thus providing an object identifier for this task.
     * This can be called an external reference.
     * Interfaces are held weakly at an environment; they demand a final revokeInterface()
     * call for each interface that has been registered.
     *<BR>
     * @param object      inout parameter for the registered object
     * @param oId[]       inout parameter for the corresponding object id
     * @param xtypeDescr  type description of interface
     */
    public Object registerInterface(Object object, String oId[], Class zInterface) {
        if(oId[0] == null)
            oId[0] = UnoRuntime.generateOid(object);

        Object p_object = (Object)_objects.get(oId[0] + zInterface, zInterface);

        if(DEBUG)
            System.err.println("#### AbstractEnvironment.registerInterface:" + object + " " + UnoRuntime.generateOid(object) + " " + p_object);

        if(p_object == null)
            object = _objects.put(oId[0] + zInterface, object, zInterface);
        else
            object = p_object;

        return object;
    }

    /**
     * You have to revoke ANY interface that has been registered via this method.
     *<BR>
     * @param oId         object id of interface to be revoked
     * @param xtypeDescr  type description of interface to be revoked
     */
    public void revokeInterface(String oId, Class zInterface) {
        _objects.remove(oId);
    }

    /**
     * Retrieves an interface identified by its object id and type from this environment.
     *<BR>
     * @param oId        object id of interface to be retrieved
     * @param xtypeDescr description of interface to be retrieved
     */
    public Object getRegisteredInterface(String oId, Class zInterface)  {
        Object object = _objects.get(oId + zInterface, zInterface);

        if(DEBUG) System.err.println("#### AbstractEnvironment(" + getName() + ").getRegisteredInterface:>" + oId + "< " + zInterface +" " + object);

        return object;
    }

    /**
     * Retrieves the object identifier for a registered interface from this environment.
     *<BR>
     * @param object      a registered interface
     */
    public String getRegisteredObjectIdentifier(Object object) {
        return UnoRuntime.generateOid(object);
    }

    public void list() {
        System.err.println("#### AbstractEnvironment.list(" + getName() + " " + getContext() + "):");

        Enumeration elements = _objects.keys();
        while(elements.hasMoreElements()) {
            System.err.println("#### key:" + (String)elements.nextElement());
        }
    }

    public void dispose() {
        boolean result = _objects.reset();
        if(!result)
            System.err.println(getClass().getName() + ".reset - " + getName() + " " + result);

//          return result;
    }
}

