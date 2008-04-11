/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FormComponent.java,v $
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
    private Object          m_component;
    private XNameAccess     m_nameAccess;
    private XIndexAccess    m_indexAccess;
    private XChild          m_child;
    private XNamed          m_named;

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
        XFormsSupplier supp = (XFormsSupplier)UnoRuntime.queryInterface(
            XFormsSupplier.class, drawPage );
        m_component = supp.getForms();

        m_nameAccess = (XNameAccess)m_component;
        m_indexAccess = (XIndexAccess)UnoRuntime.queryInterface(
            XIndexAccess.class, m_component );
        m_child = (XChild)UnoRuntime.queryInterface(
            XChild.class, m_component );
        m_named = (XNamed)UnoRuntime.queryInterface(
            XNamed.class, m_component );
    }

    /* ------------------------------------------------------------------ */
    public FormComponent( Object element )
    {
        m_component = element;
        m_nameAccess = (XNameAccess)UnoRuntime.queryInterface(
            XNameAccess.class, m_component );
        m_indexAccess = (XIndexAccess)UnoRuntime.queryInterface(
            XIndexAccess.class, m_component );
        m_child = (XChild)UnoRuntime.queryInterface(
            XChild.class, m_component );
        m_named = (XNamed)UnoRuntime.queryInterface(
            XNamed.class, m_component );
    }

    /* ------------------------------------------------------------------ */
    /** Quick access to a given interface of the view
        @param aInterfaceClass
                the class of the interface which shall be returned
    */
    public Object query( Class aInterfaceClass )
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
        if ( m_nameAccess != null )
            return m_nameAccess.getElementNames();
        return new String[]{};
    }

    /* ------------------------------------------------------------------ */
    public boolean hasByName( String name )
    {
        if ( m_nameAccess != null )
            return m_nameAccess.hasByName( name );
        return false;
    }

    /* ------------------------------------------------------------------ */
    public int getCount()
    {
        if ( m_indexAccess != null )
            return m_indexAccess.getCount();
        return 0;
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
    public boolean hasElements()
    {
        if ( m_indexAccess != null )
            return m_indexAccess.hasElements();
        else if ( m_nameAccess != null )
            return m_nameAccess.hasElements();
        return false;
    }

    /* ------------------------------------------------------------------ */
    public FormComponent getParent()
    {
        if ( m_child != null )
            return new FormComponent( m_child.getParent() );
        return new FormComponent();
    }

    /* ------------------------------------------------------------------ */
    public String getName()
    {
        if ( m_named != null )
            return m_named.getName();
        return "";
    }

    /* ------------------------------------------------------------------ */
    public String getImplementationName()
    {
        XServiceInfo si = (XServiceInfo)UnoRuntime.queryInterface(
            XServiceInfo.class, m_component );
        if ( si != null )
            return si.getImplementationName();
        return "";
    }
}
