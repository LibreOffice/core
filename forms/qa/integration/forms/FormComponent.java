/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FormComponent.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:28:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
