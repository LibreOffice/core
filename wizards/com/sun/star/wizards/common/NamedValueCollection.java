/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.sun.star.wizards.common;

import com.sun.star.beans.PropertyAttribute;
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

    @SuppressWarnings("unchecked")
    public final < T extends XInterface > T queryOrDefault( final String i_key, final T i_default, Class i_interfaceClass )
    {
        if ( m_values.containsKey( i_key ) )
        {
            final Object value = m_values.get( i_key );
            return (T)UnoRuntime.queryInterface( i_interfaceClass, value );
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
