/*************************************************************************
 *
 *  $RCSfile: ProxyProvider.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-02 09:45:07 $
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
//
//    Object _xEventListenerProxy;
//    int nDisposingCalled= 0;
//
//    Requester(boolean virtual, boolean forceSynchronous, Object evtListener)
//    {
//        _virtual = virtual;
//        _forceSynchronous = forceSynchronous;
//        _xEventListenerProxy= evtListener;
//
//    }
//
//    public Object sendRequest(Object object,
//    Type type,
//    String operation,
//    Object params[],
//    Boolean synchron[],
//    Boolean mustReply[]) throws Throwable
//    {
//
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


