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
    
    public byte [] getImplementationId()
    {
        return new byte[0];
    }
    
    // System.Object
    public override String ToString()
    {
        System.Text.StringBuilder buf =
            new System.Text.StringBuilder( base.ToString(), 256 );
        buf.Append( "\nUNO Object Implementation:\n\tInterfaces: " );
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

