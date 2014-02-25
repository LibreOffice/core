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

package com.sun.star.lib.uno.helper;
import com.sun.star.uno.Type;
import com.sun.star.lib.uno.typedesc.TypeDescription;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XEventListener;
import com.sun.star.uno.IQueryInterface;
//import com.sun.star.lib.uno.environments.java.Proxy;
import com.sun.star.lib.uno.environments.java.java_environment;
//import com.sun.star.lib.uno.environments.java.IRequester;


public class ProxyProvider
{
    static java_environment env= new java_environment(null);

    /** Creates a new instance of ProxyProvider */
    public ProxyProvider()
    {
    }
    /** returns Holder proxy objects for the specified interface. If the method is called
     * several times with the same arguments then each time a new HolderProxy is returned.
     * Then all HolderProxy s refer to the same Proxy object.
     * The proxy can be queried for XEventListener. On the returned proxy disposing can be called
     *
     */
    public static Object createProxy(Object obj, Class iface)
    {

        Object retVal= null;
        if (obj == null || iface == null || iface.isInstance(obj) == false )
            return retVal;

        Type type= new Type(TypeDescription.getTypeDescription(iface));
        Type evtType= new Type(TypeDescription.getTypeDescription(com.sun.star.lang.XEventListener.class));
        // find the object identifier
        String sOid= UnoRuntime.generateOid(obj);
        retVal= env.getRegisteredInterface(sOid, type);
        // if retVal == null then probably not registered
        if (retVal == null)
        {
            Object aProxy = new Proxy(sOid, type);
            String[] arOid = new String[]
            {sOid};
            retVal= env.registerInterface(aProxy, arOid, type);
        }
        return retVal;
    }
}

class Proxy implements IQueryInterface, XEventListener
{
    String oid;
    Type type;
    Proxy(String oid, Type t) {
        this.oid = oid;
        this.type = t;
    }

    public String getOid() {
        return oid;
    }

    public boolean isSame(Object object) {
        if (object instanceof IQueryInterface)
        {
            IQueryInterface iquery = (IQueryInterface) object;
            if (iquery != null)
            {
                if (iquery.getOid().equals(oid))
                    return true;
                else
                    return false;
            }
        }

        String oidObj = UnoRuntime.generateOid(object);
        if (oidObj.equals(oid))
            return true;
        else
            return false;
    }

    public Object queryInterface(Type type) {
        return null;
    }

    public void disposing(com.sun.star.lang.EventObject eventObject) {
    }

}


//class Requester //implements IRequester
//{
//    int _modus;
//    boolean _virtual;
//    boolean _forceSynchronous;
//    boolean _passed = true;

//    Object _xEventListenerProxy;
//    int nDisposingCalled= 0;

//    Requester(boolean virtual, boolean forceSynchronous, Object evtListener)
//    {
//        _virtual = virtual;
//        _forceSynchronous = forceSynchronous;
//        _xEventListenerProxy= evtListener;

//    }

//    public Object sendRequest(Object object,
//    Type type,
//    String operation,
//    Object params[],
//    Boolean synchron[],
//    Boolean mustReply[]) throws Throwable
//    {

//        Object result = null;
//        if (operation.equals("disposing"))
//        {
//            System.out.println("Disposing called on XEventListener proxy");
//            nDisposingCalled++;
//        }
//        else if (operation.equals("queryInterface"))
//        {
//            if (params[0] instanceof Type)
//            {
//                Type t= (Type) params[0];
//                if (t.equals( new Type("com.sun.star.lang.XEventListener")))
//                    result= _xEventListenerProxy;
//            }
//        }
//        return result;
//    }
//}


