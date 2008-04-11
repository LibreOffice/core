/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WeakBase.java,v $
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
import com.sun.star.uno.XWeak;
import com.sun.star.uno.XAdapter;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import java.lang.reflect.Field;
import java.util.Vector;
import com.sun.star.uno.UnoRuntime;
import java.util.Map;
import java.util.Hashtable;


/** This class can be used as the base class for UNO components. It implements the capability
 *  to be kept weak (com.sun.star.uno.XWeak) and it implements com.sun.star.lang.XTypeProvider
 *  which is necessary for using the component with StarBasic.
 */
public class WeakBase implements XWeak, XTypeProvider
{
    private final boolean DEBUG= false;

    // Contains all WeakAdapter which have been created in this class
    // They have to be notified when this object dies
    private WeakAdapter m_adapter;

    protected static Map _mapImplementationIds= new Hashtable();
    protected static Map _mapTypes= new Hashtable();

    /** Method of XWeak. The returned XAdapter implementation can be used to keap
     * a weak reference to this object.
     * @return a com.sun.star.uno.XAdapter implementation.
     */
    synchronized public XAdapter queryAdapter()
    {
        if (m_adapter == null)
            m_adapter= new WeakAdapter(this);
        return m_adapter;
    }

    /** Override of Object.finalize. When there are no references to this object anymore
     * then the garbage collector calls this method. Thereby causing the adapter object
     * to be notified. The adapter, in turn, notifies all listeners (com.sun.star.uno.XReference)
     */
    protected void finalize() throws java.lang.Throwable
    {
        if (m_adapter != null)
            m_adapter.referentDying();
        super.finalize();
    }

    /** Method of XTypeProvider. It returns an array of Type objects which represent
     * all implemented UNO interfaces of this object.
     * @return Type objects of all implemented interfaces.
     */
    public Type[] getTypes()
    {
        Type[] arTypes= (Type[]) _mapTypes.get( getClass());
        if (arTypes == null)
        {
            Vector vec= new Vector();
            Class currentClass= getClass();
            do
            {
                Class interfaces[]= currentClass.getInterfaces();
                for(int i = 0; i < interfaces.length; ++ i)
                {
                    // Test if it is a UNO interface
                    if (com.sun.star.uno.XInterface.class.isAssignableFrom((interfaces[i])))
                        vec.add(new Type(interfaces[i]));
                }
                // get the superclass the currentClass inherits from
                currentClass= currentClass.getSuperclass();
            } while (currentClass != null);

            Type types[]= new Type[vec.size()];
            for( int i= 0; i < types.length; i++)
                types[i]= (Type) vec.elementAt(i);
            _mapTypes.put(getClass(), types);
            arTypes= types;
        }
        return arTypes;
    }

    /** Method of XTypeProvider. It provides an identifier that represents the set of UNO
     * interfaces implemented by this class. All instances of this class
     * which run in the same Java Virtual Machine return the same array. (This only works as long
     * the ClassLoader preserves the class even if no instance exist.)
     *@return identifier as array of bytes
     */
    public byte[] getImplementationId()
    {
        byte[] id= null;
        synchronized (_mapImplementationIds)
        {
            id= (byte[]) _mapImplementationIds.get(getClass());

            if (id == null)
            {
                int hash = hashCode();
                String sName= getClass().getName();
                byte[] arName= sName.getBytes();
                int nNameLength= arName.length;

                id= new byte[ 4 + nNameLength];
                id[0]= (byte)(hash & 0xff);
                id[1]= (byte)((hash >>> 8) & 0xff);
                id[2]= (byte)((hash >>> 16) & 0xff);
                id[3]= (byte)((hash >>>24) & 0xff);

                for (int i= 0; i < nNameLength; i++)
                {
                    id[4 + i]= arName[i];
                }
                _mapImplementationIds.put(getClass(), id);
            }
        }
        return id;
    }
}
