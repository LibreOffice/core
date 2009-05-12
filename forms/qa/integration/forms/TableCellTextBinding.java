/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TableCellTextBinding.java,v $
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

package integration.forms;

import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.table.XCell;
import com.sun.star.util.XModifyListener;
import com.sun.star.beans.XPropertySet;
import com.sun.star.text.XTextRange;
import com.sun.star.form.binding.IncompatibleTypesException;

/** a value binding to be connected to a form control

    This binding synchronizes the text contained in a table cell (which you must
    pass upon construction) to the text in an XBindableValue.

    Well, in real it does not synchronize both directions. The ValueBinding
    service has not much room for own activity: It allows notification of changes
    in the own value, and it allows external instances to set the current value.

    Note that we implement this binding as a separate thread, which is (more or
    less permanently) polling for a new text at the cell. This is unfortunate, but
    sadly the Writer table cells do not support actively notifying changes in their
    content to other interested parties.
*/
public class TableCellTextBinding
                extends     java.lang.Thread
                implements  com.sun.star.form.binding.XValueBinding,
                            com.sun.star.util.XModifyBroadcaster
{
    private XTextRange  m_cellText;
    private Object      m_writeSignal;
    private String      m_newCellText;
    private String      m_lastKnownCellText;
    private boolean     m_haveNewCellText;
    private java.util.List  m_listeners;

    /** Creates a new instance of TableCellTextBinding */
    public TableCellTextBinding( XCell cell )
    {
        m_cellText = (XTextRange)UnoRuntime.queryInterface( XTextRange.class, cell );

        m_newCellText = new String();
        m_listeners = new java.util.LinkedList();

        start();
    }

    /** retrieves the list of data types which this binding can exchange
    */
    public com.sun.star.uno.Type[] getSupportedValueTypes()
    {
        try
        {
            // well, only strings here ...
            return new Type[] {
                getStringType()
            };
        }
        catch( java.lang.Exception e )
        {
        }
        return new Type[] { };
    }

    /** retrieves the current value
    */
    public Object getValue(com.sun.star.uno.Type type) throws com.sun.star.form.binding.IncompatibleTypesException
    {
        if ( !type.equals( getStringType() ) )
            throw new com.sun.star.form.binding.IncompatibleTypesException();

        return m_cellText.getString();
    }

    /** sets a new value
    */
    public void setValue(Object obj) throws com.sun.star.form.binding.IncompatibleTypesException
    {
        String text;
        try
        {
            text = (String)obj;
        }
        catch( java.lang.ClassCastException e )
        {
            throw new com.sun.star.form.binding.IncompatibleTypesException();
        }
        // remember the new text
        synchronized( m_newCellText )
        {
            m_newCellText = text;
            m_haveNewCellText = true;
        }
        // and wake up the thread which is waiting for it
        synchronized( m_writeSignal )
        {
            m_writeSignal.notify();
        }
    }

    /** determines whether a given value type is supported
    */
    public boolean supportsType(com.sun.star.uno.Type type)
    {
        return type.equals( getStringType() );
    }

    /** retrieves the UNO type for the string class
    */
    private static final Type getStringType()
    {
        return new com.sun.star.uno.Type( String.class );
    }

    /** runs the thread
    */
    public void run()
    {
        try
        {
            m_writeSignal = new Object();
            while ( true )
            {
                // go sleep a while
                synchronized( m_writeSignal )
                {
                    m_writeSignal.wait( 200 );
                }

                // if there's new text in the control, propagate it to the cell
                synchronized ( m_newCellText )
                {
                    if ( m_haveNewCellText )
                    {
                        m_cellText.setString( m_newCellText );
                        m_lastKnownCellText = m_newCellText;
                    }
                    m_haveNewCellText = false;
                }

                // if there's new text in the cell, propagate it to the control
                String currentCellText = m_cellText.getString();
                if ( !currentCellText.equals( m_lastKnownCellText ) )
                {
                    m_lastKnownCellText = currentCellText;
                    // notify the modification
                    synchronized( m_listeners )
                    {
                        com.sun.star.lang.EventObject eventSource = new com.sun.star.lang.EventObject( this );

                        java.util.Iterator loop = m_listeners.iterator();
                        while ( loop.hasNext() )
                        {
                            ((XModifyListener)loop.next()).modified( eventSource );
                        }
                    }
                }
            }
        }
        catch( java.lang.Exception e )
        {
            e.printStackTrace(System.err);
        }
    }

    public void addModifyListener(com.sun.star.util.XModifyListener xModifyListener)
    {
        synchronized( m_listeners )
        {
            m_listeners.add( xModifyListener );
        }
    }

    public void removeModifyListener(com.sun.star.util.XModifyListener xModifyListener)
    {
        synchronized( m_listeners )
        {
            m_listeners.remove( xModifyListener );
        }
    }

    public void disposing(com.sun.star.lang.EventObject eventObject)
    {
        // not interested in
    }
}
