/*************************************************************************
*
*  $RCSfile: ScriptContext.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: toconnor $ $Date: 2003-10-29 15:01:13 $
*
*  The Contents of this file are made available subject to the terms of
*  either of the following licenses
*
*         - GNU Lesser General Public License Version 2.1
*         - Sun Industry Standards Source License Version 1.1
*
*  Sun Microsystems Inc., October, 2000
*
*  GNU Lesser General Public License Version 2.1
*  =============================================
*  Copyright 2000 by Sun Microsystems, Inc.
*  901 San Antonio Road, Palo Alto, CA 94303, USA
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License version 2.1, as published by the Free Software Foundation.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*  MA  02111-1307  USA
*
*
*  Sun Industry Standards Source License Version 1.1
*  =================================================
*  The contents of this file are subject to the Sun Industry Standards
*  Source License Version 1.1 (the "License"); You may not use this file
*  except in compliance with the License. You may obtain a copy of the
*  License at http://www.openoffice.org/license.html.
*
*  Software provided under this License is provided on an "AS IS" basis,
*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
*  See the License for the specific provisions governing your rights and
*  obligations concerning the Software.
*
*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
*
*  Copyright: 2000 by Sun Microsystems, Inc.
*
*  All Rights Reserved.
*
*  Contributor(s): _______________________________________
*
*
************************************************************************/
package com.sun.star.script.framework.provider;

import com.sun.star.frame.XModel;
import com.sun.star.frame.XDesktop;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;

import java.util.HashMap;

import drafts.com.sun.star.script.provider.XScriptContext;

import com.sun.star.script.framework.log.LogUtils;

/**
 *  Description of the Class
 *
 * @author     Noel Power
 * @created    August 2, 2002
 */
public class ScriptContext implements XScriptContext, XPropertySet
{
    /**
     *  Description of the Class
     *
     * @author     John Rice
     * @created    18/09/02
     */

    public final static String HM_DOC_REF = "DocumentReference";
    public final static String HM_DESKTOP = "Desktop";
    public final static String HM_COMPONENT_CONTEXT = "ComponentContext";
    private HashMap m_properties = new HashMap();

    private final static String DOC_REF = "SCRIPTING_DOC_REF";
    private final static String DOC_STORAGE_ID = "SCRIPTING_DOC_STORAGE_ID";
    private final static String DOC_URI = "SCRIPTING_DOC_URI";

    public ScriptContext( XComponentContext xmComponentContext,
        XDesktop xDesktop, XPropertySet invocationCtxPropSet)
    {
        XModel xmDocRef = null;
        try
        {
            xmDocRef = ( XModel ) UnoRuntime.queryInterface(
                XModel.class,
                invocationCtxPropSet.getPropertyValue( DOC_REF ) );
            int iDocStorageID =
                AnyConverter.toInt( invocationCtxPropSet.getPropertyValue(
                    DOC_STORAGE_ID ) );
            String sDocUri = ( String )
                invocationCtxPropSet.getPropertyValue( DOC_URI );

            LogUtils.DEBUG( "DOC_REF query for URL = " + xmDocRef.getURL() );
            LogUtils.DEBUG( "DOC_STORAGE_ID = " + iDocStorageID );
            LogUtils.DEBUG( "DOC_URI query for URL = " + sDocUri );

            m_properties.put( DOC_STORAGE_ID, new Integer( iDocStorageID ) );
            m_properties.put( DOC_URI, sDocUri );
        }
        catch ( UnknownPropertyException upe )
        {
            upe.printStackTrace();
        }
        catch ( WrappedTargetException wte )
        {
            wte.printStackTrace();
        }
        catch ( IllegalArgumentException iae )
        {
            iae.printStackTrace();
        }
        m_properties.put( HM_DOC_REF, xmDocRef );
        m_properties.put( HM_DESKTOP, xDesktop );
        m_properties.put( HM_COMPONENT_CONTEXT, xmComponentContext );
    }

    public static ScriptContext createContext(Object invocationCtx,
        XComponentContext xCtxt, XMultiComponentFactory xMCF)
    {
        ScriptContext sc = null;

        try {
            XPropertySet invocationCtxPropSet = (XPropertySet)
                UnoRuntime.queryInterface( XPropertySet.class, invocationCtx);

            Object xInterface = null;
            XDesktop xDesktop = null;

            xInterface = xMCF.createInstanceWithContext(
                "com.sun.star.frame.Desktop", xCtxt);
            xDesktop = (XDesktop)
                UnoRuntime.queryInterface(XDesktop.class, xInterface);

            sc = new ScriptContext(xCtxt, xDesktop, invocationCtxPropSet);
        }
        catch ( Exception e ) {
            e.printStackTrace();
        }
        return sc;
    }

    //----------------------------------------------------------------------
    /**
        Obtain the document reference on which the script can operate

        @returns
          XModel interface
    */
    public XModel getDocument()
    {
        return ( XModel )m_properties.get( HM_DOC_REF );
    }

    /**
        Obtain the desktop reference on which the script can operate

        @returns
          XDesktop interface
    */
    public XDesktop getDesktop()
    {
        return ( XDesktop )m_properties.get( HM_DESKTOP );
    }

    /**
        Obtain the component context which the script can use to create other uno components

        @returns
          XComponentContext interface
    */
    public XComponentContext getComponentContext()
    {
        return ( XComponentContext )m_properties.get( HM_COMPONENT_CONTEXT );
    }


    /** @returns
            the <type>XPropertySetInfo</type> interface, which
            describes all properties of the object which supplies this
            interface.

        @returns
            <const>NULL</const> if the implementation cannot or will
            not provide information about the properties; otherwise the
            interface <type>XPropertySetInfo</type> is returned.
     */
    public com.sun.star.beans.XPropertySetInfo getPropertySetInfo()
    {
        return null;
    }

    /** sets the value of the property with the specified name.

        <p>If it is a bound property the value will be changed before
        the change event is fired.  If it is a constrained property
        a vetoable event is fired before the property value can be
        changed.  </p>

        @raises com::sun::star::beans::PropertyVetoException
            if the property is read-only or vetoable
            and one of the listeners throws this exception
            because of an unaccepted new value.
     */
    public void setPropertyValue( String aPropertyName,
        java.lang.Object aValue )
        throws UnknownPropertyException, PropertyVetoException,
            com.sun.star.lang.IllegalArgumentException,
            com.sun.star.lang.WrappedTargetException
    {
        throw new PropertyVetoException("No changes to ScriptContext allowed");
    }

    /** @returns
            the value of the property with the specified name.

        @param PropertyName
            This parameter specifies the name of the property.

        @throws UnknownPropertyException
            if the property does not exist.

        @throws com::sun::star::lang::WrappedTargetException
            if the implementation has an internal reason for the exception.
            In this case the original exception is wrapped into that
            <type scope="com::sun::star::lang">WrappedTargetException</type>.
     */
    public java.lang.Object getPropertyValue( String PropertyName )
        throws UnknownPropertyException,
            com.sun.star.lang.WrappedTargetException
    {
        if( !m_properties.containsKey( PropertyName ) )
        {
            throw new UnknownPropertyException( "No property called " + PropertyName + " in ScriptContext" );
        }
        return m_properties.get( PropertyName );
    }

    /** adds an <type>XPropertyChangeListener</type> to the specified property.

        <p>An empty name ("") registers the listener to all bound
        properties. If the property is not bound, the behavior is
        not specified.  </p>

        @see removePropertyChangeListener
     */
    public void addPropertyChangeListener( String aPropertyName,
        XPropertyChangeListener xListener )
        throws UnknownPropertyException,
        com.sun.star.lang.WrappedTargetException
    {
        throw new RuntimeException("ScriptContext::addPropertyChangeListener not supported.");
    }

    /** removes an <type>XPropertyChangeListener</type> from
        the listener list.

        <p>It is a "noop" if the listener is not registered.  </p>

        @see addPropertyChangeListener
     */
    public void removePropertyChangeListener( String aPropertyName,
        XPropertyChangeListener aListener )
        throws UnknownPropertyException,
            com.sun.star.lang.WrappedTargetException
    {
        throw new RuntimeException("ScriptContext::removePropertyChangeListener not supported.");
    }

    /** adds an <type>XVetoableChangeListener</type> to the specified
        property with the name PropertyName.

        <p>An empty name ("") registers the listener to all
        constrained properties. If the property is not constrained,
        the behavior is not specified.  </p>

        @see removeVetoableChangeListener
     */
    public void addVetoableChangeListener( String PropertyName,
        XVetoableChangeListener aListener )
        throws UnknownPropertyException,
            com.sun.star.lang.WrappedTargetException
    {
        throw new RuntimeException("ScriptContext::addVetoableChangeListener not supported.");
    }

    /** removes an <type>XVetoableChangeListener</type> from the
        listener list.

        <p>It is a "noop" if the listener is not registered.  </p>

        @see addVetoableChangeListener
     */
    public void removeVetoableChangeListener( String PropertyName,
        XVetoableChangeListener aListener )
        throws UnknownPropertyException,
            com.sun.star.lang.WrappedTargetException
    {
        throw new RuntimeException("ScriptContext::removeVetoableChangeListener not supported.");
    }

}
