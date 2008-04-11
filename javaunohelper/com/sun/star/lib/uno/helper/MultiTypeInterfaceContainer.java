/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MultiTypeInterfaceContainer.java,v $
 * $Revision: 1.4 $
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

package com.sun.star.lib.uno.helper;
import com.sun.star.uno.Type;
import com.sun.star.lang.EventObject;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;

public class MultiTypeInterfaceContainer
{

    private Map map= new HashMap();

    /** Creates a new instance of MultiTypeInterfaceContainer */
    public MultiTypeInterfaceContainer()
    {
    }

    /** only returns types which have at least one value in InterfaceContainer
     *  return value can contain an element null, if someone called
     *  addInterface (null, interf)
     */
    synchronized public Type[] getContainedTypes()
    {
        int size;
        Type[] retVal= null;

        if ( (size=map.size()) > 0)
        {
            Type [] arTypes= new Type[size];
            Iterator it= map.keySet().iterator();

            int countTypes= 0;
            while (it.hasNext())
            {
                Object key= it.next();
                InterfaceContainer cont= (InterfaceContainer) map.get(key);
                if (cont != null && cont.size() > 0)
                {
                    if (key == null)
                        arTypes[countTypes++]= new Type();
                    else if (key instanceof Type)
                        arTypes[countTypes++]= (Type) key;
                    else if (key instanceof Class)
                        arTypes[countTypes++]= new Type((Class) key);
                    else
                        arTypes[countTypes++]= new Type(key.getClass());
                }
            }

            if (countTypes != size)
            {
                retVal= new Type[countTypes];
                System.arraycopy(arTypes, 0, retVal, 0, countTypes);
            }
            else
                retVal= arTypes;
        }
        if (retVal == null)
            retVal= new Type[0];
        return retVal;
    }

    /** param key can be null */
    synchronized public InterfaceContainer getContainer(Object key)
    {
        InterfaceContainer retVal= null;
        Iterator it= map.keySet().iterator();
        while (it.hasNext())
        {
            Object obj= it.next();
            if (obj == null && key == null)
            {
                retVal= (InterfaceContainer) map.get(null);
                break;
            }
            else if( obj != null && obj.equals(key))
            {
                retVal= (InterfaceContainer) map.get(obj);
                break;
            }
        }
        return retVal;
    }


    synchronized public int addInterface(Object ckey, Object iface)
    {
        //If the key is a Type then it does not matter if the objects are different
        // if they represent the same type. This is because Types overrides hashCode and
        // equals. For example:
        // Type a= new Type(XInterface.class);
        // Type b= new Type(XInterface.class);
        // Allthough a != b , the map interprets both as being the same.
        InterfaceContainer cont= (InterfaceContainer) map.get(ckey);
        if (cont != null)
        {
            cont.add(iface);
        }
        else
        {
            cont= new InterfaceContainer();
            cont.add(iface);
            map.put(ckey, cont);
        }
        return cont.size();
    }


    synchronized public int removeInterface(Object key, Object iface)
    {
        int retVal= 0;
        InterfaceContainer cont= (InterfaceContainer) map.get(key);
        if (cont != null)
        {
            cont.remove(iface);
            retVal= cont.size();
        }
        return retVal;
    }

    public void disposeAndClear(EventObject evt)
    {
        Iterator it= null;
        synchronized(this)
        {
            it= map.values().iterator();
        }
        while (it.hasNext() )
            ((InterfaceContainer) it.next()).disposeAndClear(evt);
    }

    synchronized public void clear()
    {
        Iterator it= map.values().iterator();
        while (it.hasNext())
            ((InterfaceContainer) it.next()).clear();
    }
}
