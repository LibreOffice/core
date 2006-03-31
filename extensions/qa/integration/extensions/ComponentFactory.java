/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ComponentFactory.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2006-03-31 12:17:09 $
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

package integration.extensions;

import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XSingleComponentFactory;
import java.lang.reflect.Constructor;

/**
 *
 * @author fs93730
 */
public class ComponentFactory implements XSingleComponentFactory
{
    private Class       m_handlerClass;
    private Constructor m_defaultConstructor;
    private Constructor m_initConstructor;

    public ComponentFactory( Class _handlerClass )
    {
        m_handlerClass = _handlerClass;

        Class objectArrayClass = null;
        try
        {
            objectArrayClass = Class.forName("[Ljava.lang.Object;");
        }
        catch ( java.lang.ClassNotFoundException e ) { }

        Constructor ctors[] = _handlerClass.getConstructors();
        for ( int i = 0; i < ctors.length && ctors != null; ++i)
        {
            Class ctorParams[] = ctors[i].getParameterTypes();
            if ( ( ctorParams.length == 1 ) && ( ctorParams[0].equals( XComponentContext.class ) ) )
                m_defaultConstructor = ctors[i];
            if  (   ( ctorParams.length == 2 )
                &&  ( ctorParams[0].equals( XComponentContext.class ) )
                &&  ( ctorParams[1].equals( objectArrayClass ) )
                )
                m_initConstructor = ctors[i];
        }
        if ( m_defaultConstructor == null )
            throw new java.lang.IllegalArgumentException();
    }

    private Object ipml_createInstance( Constructor _ctor, Object[] _arguments )
    {
        Object newInstance = null;
        try
        {
            newInstance = _ctor.newInstance( _arguments );
        }
        catch( InstantiationException e )
        {
            System.err.println( "InstantiationException: Could not instantiate an instance of " + m_handlerClass.getName() );
        }
        catch( IllegalAccessException e )
        {
            System.err.println( "IllegalAccessException: Could not instantiate an instance of " + m_handlerClass.getName() );
        }
        catch( java.lang.reflect.InvocationTargetException e )
        {
            System.err.println( "InvocationTargetException: Could not instantiate an instance of " + m_handlerClass.getName() );
        }
        return newInstance;
    }

    public Object createInstanceWithArgumentsAndContext(Object[] _arguments, XComponentContext _componentContext) throws com.sun.star.uno.Exception
    {
        if ( m_initConstructor != null )
            return ipml_createInstance( m_initConstructor, new Object[] { _componentContext, _arguments } );
        else
            return createInstanceWithContext( _componentContext );
    }

    public Object createInstanceWithContext(XComponentContext _componentContext) throws com.sun.star.uno.Exception
    {
        return ipml_createInstance( m_defaultConstructor, new Object[] { _componentContext } );
    }
}

