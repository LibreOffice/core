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



using System;
using System.Collections;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.lang;

namespace uno.util
{

/** This class can be used as a base class for UNO objects.
    It implements the capability to be kept weakly
    (unoidl.com.sun.star.uno.XWeak) and it implements
    unoidl.com.sun.star.lang.XTypeProvider which is necessary for
    using the object from StarBasic.
*/
public class WeakBase : XWeak, XTypeProvider
{
    // Contains all WeakAdapter which have been created in this class
    // They have to be notified when this object dies
    private WeakAdapter m_adapter = null;
    
    protected static Hashtable s_types = new Hashtable();
    protected static Hashtable s_impl_ids = new Hashtable();
    
    // XWeak impl
    /** The returned XAdapter implementation can be used to keap a
        weak reference to this object.
        
        @return a weak adapter
    */
    public XAdapter queryAdapter()
    {
        if (null == m_adapter)
        {
            lock (this)
            {
                if (null == m_adapter)
                    m_adapter = new WeakAdapter( this );
            }
        }
        return m_adapter;
    }
    
    /** Overrides of Object.Finalize method.
        When there are no references to this object anymore, then the
        garbage collector calls this method, thereby causing the adapter
        object to be notified.  The adapter, in turn, notifies all
        listeners (unoidl.com.sun.star.uno.XReference).
    */
    ~WeakBase()
    {
        if (null != m_adapter)
            m_adapter.referentDying();
    }
    
    // XTypeProvider impl
    
    /** Returns an array of Type objects which represent all implemented
        UNO interfaces of this object.
       
       @return Type objects of all implemented interfaces.
    */
    public Type [] getTypes()
    {
        Type [] types;
        Type type = GetType();
        lock (s_types)
        {
            types = (Type []) s_types[ type ];
            if (null == types)
            {
                Type [] interfaces = type.GetInterfaces();
                ArrayList list = new ArrayList( interfaces.Length );
                for ( Int32 pos = 0; pos < interfaces.Length; ++pos )
                {
                    Type iface = interfaces[ pos ];
                    // xxx todo: as long as the bridge cannot introduce
                    // native CTS types into UNO on the fly
                    if (iface.FullName.StartsWith( "unoidl." ))
                    {
                        list.Add( iface );
                    }
                }
                Int32 len = list.Count;
                Type [] ar = new Type [ len ];
                for ( Int32 pos = 0; pos < len; ++pos )
                    ar[ pos ] = (Type) list[ pos ];
                s_types[ type ] = ar;
                types = ar;
            }
        }
        return types;
    }
    
    /** Provides an identifier that represents the set of UNO interfaces
        implemented by this class.  All instances of this class which run
        in the same CLR return the same array.
        
        @return identifier as array of bytes
    */
    public byte [] getImplementationId()
    {
        byte [] id;
        Type type = GetType();
        lock (s_impl_ids)
        {
            id = (byte []) s_impl_ids[ type ];
            if (null == id)
            {
                Int32 hash = GetHashCode();
                String name = type.FullName;
                Int32 len= name.Length;
                
                id = new byte[ 4 + (2 * len) ];
                id[ 0 ]= (byte) (hash & 0xff);
                id[ 1 ]= (byte) ((hash >> 8) & 0xff);
                id[ 2 ]= (byte) ((hash >> 16) & 0xff);
                id[ 3 ]= (byte) ((hash >> 24) & 0xff);
                
                for ( Int32 pos = 0; pos < len; ++pos )
                {
                    UInt16 c = Convert.ToUInt16( name[ pos ] );
                    id[ 4 + (2 * pos) ] = (byte) (c & 0xff);
                    id[ 4 + (2 * pos) +1 ] = (byte) ((c >> 8) & 0xff);
                }
                s_impl_ids[ type ] = id;
            }
        }
        return id;
    }
    
    // System.Object
    public override String ToString()
    {
        System.Text.StringBuilder buf =
            new System.Text.StringBuilder( base.ToString(), 256 );
        buf.Append( "\nUNO Object Implementation:\n\tImplementationId: " );
        buf.Append( getImplementationId() );
        buf.Append( "\n\tInterfaces: " );
        Type [] types = getTypes();
        for ( Int32 pos = 0; pos < types.Length; ++pos )
        {
            buf.Append( types[ pos ].FullName );
            if (pos < (types.Length -1))
                buf.Append( ", " );
        }
        return buf.ToString();
    }
}

}

