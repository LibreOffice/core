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
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.table.XCell;
import com.sun.star.util.XModifyListener;
import com.sun.star.text.XTextRange;

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
