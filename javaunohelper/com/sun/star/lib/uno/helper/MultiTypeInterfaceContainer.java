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
import com.sun.star.lang.EventObject;
import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;

public class MultiTypeInterfaceContainer
{

    private final Map<Object,InterfaceContainer> map= new HashMap<Object,InterfaceContainer>();

    /** only returns types which have at least one value in InterfaceContainer
     *  return value can contain an element null, if someone called
     *  addInterface (null, interf)
     *  @return an array of types in this container.
     */
    synchronized public Type[] getContainedTypes()
    {
        int size;
        Type[] retVal= null;

        if ( (size=map.size()) > 0)
        {
            Type [] arTypes= new Type[size];
            Iterator<Object> it= map.keySet().iterator();

            int countTypes= 0;
            while (it.hasNext())
            {
                Object key= it.next();
                InterfaceContainer cont= map.get(key);
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

    /** param key can be null
     *  @param key the object for which the container should be retrieved.
     *  @return the container that contains the object key, if any.
     */
    synchronized public InterfaceContainer getContainer(Object key)
    {
        InterfaceContainer retVal= null;
        Iterator<Object> it= map.keySet().iterator();
        while (it.hasNext())
        {
            Object obj= it.next();
            if (obj == null && key == null)
            {
                retVal= map.get(null);
                break;
            }
            else if( obj != null && obj.equals(key))
            {
                retVal= map.get(obj);
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
        // Although a != b , the map interprets both as being the same.
        InterfaceContainer cont= map.get(ckey);
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
        InterfaceContainer cont= map.get(key);
        if (cont != null)
        {
            cont.remove(iface);
            retVal= cont.size();
        }
        return retVal;
    }

    public void disposeAndClear(EventObject evt)
    {
        Iterator<InterfaceContainer> it= null;
        synchronized(this)
        {
            it= map.values().iterator();
        }
        while (it.hasNext() ) {
            it.next().disposeAndClear(evt);
        }
    }

    synchronized public void clear()
    {
        Iterator<InterfaceContainer> it= map.values().iterator();
        while (it.hasNext()) {
            it.next().clear();
        }
    }
}
