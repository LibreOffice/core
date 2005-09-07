/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestBridge.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:15:33 $
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
package com.sun.star.lib.uno.protocols.urp;

import java.io.IOException;

import java.util.Hashtable;


import com.sun.star.uno.IBridge;
import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.Type;


class TestBridge implements IBridge {
    static public final boolean DEBUG = false;

    Hashtable _hashtable = new Hashtable();

    IEnvironment _source ;//= new com.sun.star.lib.uno.environments.java.java_environment(null);


    class MyEnv implements IEnvironment {
        public Object getContext() {
            return null;
        }

        public String getName() {
            return null;
        }

        public Object registerInterface(Object object, String oId[], Type type) {
            return null;
        }

        public void revokeInterface(String oId, Type type) {
        }

        public Object getRegisteredInterface(String oid, Type type) {
            Object object = _hashtable.get(oid);

            if(DEBUG) System.err.println("##### " + getClass().getName() + ".getRegisteredInterface:" + oid + " " + object);

            return object;
        }

        public String getRegisteredObjectIdentifier(Object object) {
            return null;
        }

        public void list() {
        }
    }

    TestBridge() {
        _source = new MyEnv();
    }

    public Object mapInterfaceTo(Object object, Type type) {
        if (object == null) {
            return null;
        } else {
            String oid = ">" + object.toString() + type.toString() + "<";
            _hashtable.put(oid, object);
            return oid;
        }
    }

    public Object mapInterfaceFrom(Object object, Type type) {
        String oid = (String)object;

        return _hashtable.get(oid);
    }

    public IEnvironment getSourceEnvironment() {
        return _source;
    }

    public IEnvironment getTargetEnvironment() {
        return null;
    }

    public void acquire() {}

    public void release() {}

    public void reset() throws IOException {}

    public void dispose() throws InterruptedException, IOException {}
}

