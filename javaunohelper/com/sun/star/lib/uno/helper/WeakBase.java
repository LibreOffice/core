/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package com.sun.star.lib.uno.helper;
import com.sun.star.uno.XWeak;
import com.sun.star.uno.XAdapter;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import java.util.Vector;
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
