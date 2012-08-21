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

package com.sun.star.wizards.common;

import com.sun.star.beans.PropertyState;
import com.sun.star.beans.PropertyValue;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;

/**
 *
 * @author frank.schoenheit@sun.com
 */
public class NamedValueCollection
{
    final private HashMap< String, Object >    m_values = new HashMap< String, Object >();

    public NamedValueCollection()
    {
    }

    public NamedValueCollection( final PropertyValue[] i_values )
    {
        for ( int i = 0; i < i_values.length; ++i )
            m_values.put( i_values[i].Name, i_values[i].Value );
    }

    public final void put( final String i_name, final Object i_value )
    {
        m_values.put( i_name, i_value );
    }

    @SuppressWarnings("unchecked")
    public final < T  > T getOrDefault( final String i_key, final T i_default )
    {
        if ( m_values.containsKey( i_key ) )
        {
            final Object value = m_values.get( i_key );
            try
            {
                return (T)value;
            }
            catch ( ClassCastException e ) { }
        }
        return i_default;
    }

    public final < T extends XInterface > T queryOrDefault( final String i_key, final T i_default, Class<T> i_interfaceClass )
    {
        if ( m_values.containsKey( i_key ) )
        {
            final Object value = m_values.get( i_key );
            return UnoRuntime.queryInterface( i_interfaceClass, value );
        }
        return i_default;
    }

    public final boolean has( final String i_key )
    {
        return m_values.containsKey( i_key );
    }

    public final PropertyValue[] getPropertyValues()
    {
        PropertyValue[] values = new PropertyValue[ m_values.size() ];

        Iterator< Entry< String, Object > > iter = m_values.entrySet().iterator();
        int i = 0;
        while ( iter.hasNext() )
        {
            Entry< String, Object > entry = iter.next();
            values[i++] = new PropertyValue(
                entry.getKey(),
                0,
                entry.getValue(),
                PropertyState.DIRECT_VALUE
            );
        }

        return values;
    }
}
