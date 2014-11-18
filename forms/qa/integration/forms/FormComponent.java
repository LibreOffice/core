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
package integration.forms;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.form.XFormsSupplier;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XChild;
import com.sun.star.container.XNamed;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.lang.XServiceInfo;

public class FormComponent
{
    private final Object          m_component;
    private final XNameAccess     m_nameAccess;
    private final XIndexAccess    m_indexAccess;
    private final XChild          m_child;
    private final XNamed          m_named;

    /* ------------------------------------------------------------------ */
    private FormComponent()
    {
        m_component = null;
        m_nameAccess = null;
        m_indexAccess = null;
        m_child = null;
        m_named = null;
    }

    /* ------------------------------------------------------------------ */
    public FormComponent( XDrawPage drawPage )
    {
        XFormsSupplier supp = UnoRuntime.queryInterface(
            XFormsSupplier.class, drawPage );
        m_component = supp.getForms();

        m_nameAccess = (XNameAccess)m_component;
        m_indexAccess = UnoRuntime.queryInterface(
            XIndexAccess.class, m_component );
        m_child = UnoRuntime.queryInterface(
            XChild.class, m_component );
        m_named = UnoRuntime.queryInterface(
            XNamed.class, m_component );
    }

    /* ------------------------------------------------------------------ */
    private FormComponent( Object element )
    {
        m_component = element;
        m_nameAccess = UnoRuntime.queryInterface(
            XNameAccess.class, m_component );
        m_indexAccess = UnoRuntime.queryInterface(
            XIndexAccess.class, m_component );
        m_child = UnoRuntime.queryInterface(
            XChild.class, m_component );
        m_named = UnoRuntime.queryInterface(
            XNamed.class, m_component );
    }

    /* ------------------------------------------------------------------ */
    /** Quick access to a given interface of the view
        @param aInterfaceClass
                the class of the interface which shall be returned
    */
    public <T> T query( Class<T> aInterfaceClass )
    {
        return UnoRuntime.queryInterface( aInterfaceClass, m_component );
    }

    /* ------------------------------------------------------------------ */
    public FormComponent getByName( String name )
    {
        try
        {
            if ( m_nameAccess != null )
                return new FormComponent( m_nameAccess.getByName( name ) );
        }
        catch( com.sun.star.uno.Exception e )
        {
            System.err.println( e );
            e.printStackTrace( System.err );
        }
        return new FormComponent();
    }

    /* ------------------------------------------------------------------ */
    public String[] getElementNames()
    {
        return ( m_nameAccess != null ) ? m_nameAccess.getElementNames() : new String[]{};
    }



    /* ------------------------------------------------------------------ */
    public int getCount()
    {
        return ( m_indexAccess != null ) ? m_indexAccess.getCount() : 0;
    }

    /* ------------------------------------------------------------------ */
    public FormComponent getByIndex( int index )
    {
        try
        {
            if ( m_indexAccess != null )
                return new FormComponent( m_indexAccess.getByIndex( index ) );
        }
        catch( com.sun.star.uno.Exception e )
        {
            System.err.println( e );
            e.printStackTrace( System.err );
        }
        return new FormComponent();
    }

    /* ------------------------------------------------------------------ */
    public com.sun.star.uno.Type getElementType(  )
    {
        if ( m_indexAccess != null )
            return m_indexAccess.getElementType();
        else if ( m_nameAccess != null )
            return m_nameAccess.getElementType();
        return new com.sun.star.uno.Type( String.class );
    }



    /* ------------------------------------------------------------------ */
    public FormComponent getParent()
    {
        return ( m_child != null ) ? new FormComponent( m_child.getParent() ) : new FormComponent();
    }

    /* ------------------------------------------------------------------ */
    public String getName()
    {
        return ( m_named != null ) ? m_named.getName() : "";
    }

    /* ------------------------------------------------------------------ */
    public String getImplementationName()
    {
        XServiceInfo si = UnoRuntime.queryInterface(
            XServiceInfo.class, m_component );
        return ( si != null ) ? si.getImplementationName() : "";
    }
}
