/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

namespace cli_uno
{

using System;
using System.Collections;
using System.Runtime.Remoting;
using System.Runtime.Remoting.Proxies;
using System.Text;

using com.sun.star.bridges.mono_uno;

public class Cli_environment
{
    static string sOidPart = ";cli[0];";

    static Hashtable m_objects = Hashtable.Synchronized(new Hashtable());

    static string createKey(string oid, Type t)
    {
        return oid + t.FullName;
    }

//     FIXME setup debugging info here
//     public Cli_environment()
//     {
//     }

//     FIXME assert there are no more registered objects
//     public ~Cli_environment()
//     {
//     }

    /**
       Registers an UNO object as being mapped by this bridge. The resulting
       cli object is represents all interfaces of the UNO object. Therefore the
       object can be registered only with its OID; a type is not necessary.
    */
    public object registerInterface(object obj, string oid)
    {
        // FIXME debugging stuff
        m_objects.Add(oid, obj); // new WeakReference(obj));
        return obj;
    }

    /**
       Registers a CLI object as being mapped by this bridge. The resulting
       object represents exactly one UNO interface.
     */
    public object registerInterface(object obj, string oid, Type type)
    {
        // FIXME debugging stuff
        string key = createKey(oid, type);
        m_objects.Add(key, obj); // new WeakReference(obj));
        return obj;
    }

    /**
       By revoking an interface it is declared that the respective interface has
       not been mapped. The proxy implementations call revoke interface in their
       destructors.
     */
    public void revokeInterface(string oid)
    {
        revokeInterface(oid, null);
    }

    public void revokeInterface(string oid, Type type)
    {
        // FIXME debugging stuff
        string key = type != null ? createKey(oid, type) : oid;
        m_objects.Remove(key);
    }

    /**
     * Retrieves an interface identified by its object id and type from this
     * environment.
     *
     * @param oid object id of interface to be retrieved
     * @param type the type description of the interface to be retrieved
     * @see com.sun.star.uno.IEnvironment#getRegisteredInterface
     */
    public object getRegisteredInterface(string oid, Type type)
    {
        // try if it is a UNO interface
        object ret = null;
        ret = m_objects[oid];
        if (ret == null)
        {
            // try if it is a proxy for a cli object
            oid = createKey(oid, type);
            ret = m_objects[oid];
        }
/*        if (ret != null)
        {
            WeakReference weakIface = (WeakReference)ret;
            ret = weakIface.Target;
        } */

        if (ret == null)
            m_objects.Remove(oid);

        return ret;
    }

        /**
         * Generates a worldwide unique object identifier (oid) for the given object. It is
     * guaranteed, that subsequent calls to the method with the same object
     * will give the same id.
         * <p>
         * @return     the generated oid.
         * @param      object     the object for which a Oid should be generated.
         */
    public static string getObjectIdentifier(object obj)
    {
        string oid = null;
        RealProxy realProxy = null;

        if (RemotingServices.IsTransparentProxy(obj))
            realProxy = RemotingServices.GetRealProxy(obj);

        if (realProxy != null)
        {
            UnoInterfaceProxy proxyImpl = realProxy as UnoInterfaceProxy;
            if (proxyImpl != null)
                oid = proxyImpl.Oid;
        }

        if (oid == null)
        {
            Guid gd = typeof(Cli_environment).GUID; // FIXME apparently not a good idea with mono
            StringBuilder buf = new StringBuilder(128);
            buf.Append(obj.GetHashCode());
            buf.Append(sOidPart);
            buf.Append(gd);
            oid = buf.ToString();
        }

        return oid;
    }

}

}
