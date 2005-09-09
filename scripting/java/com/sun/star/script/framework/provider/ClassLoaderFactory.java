/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ClassLoaderFactory.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:00:03 $
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

package com.sun.star.script.framework.provider;

import java.util.ArrayList;
import java.util.Vector;
import java.io.File;
import java.io.IOException;
import java.net.*;
import com.sun.star.frame.XModel;
import com.sun.star.uno.XComponentContext;

import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.container.ScriptMetaData;

/**
 *  Class Loader Factory
 *
 * @author     Noel Power
 * @created    August 2, 2002
 */
public class ClassLoaderFactory
{
    private ClassLoaderFactory() {}

    public static ClassLoader getURLClassLoader( ScriptMetaData scriptData )
        throws NoSuitableClassLoaderException, MalformedURLException
    {
        ClassLoader parent = scriptData.getClass().getClassLoader();
        URL[] classPath = scriptData.getClassPath();
        LogUtils.DEBUG("Classpath has length " + classPath.length );
        for ( int i=0; i < classPath.length; i++ )
        {
            LogUtils.DEBUG("ClassPath " + i + "} is " + classPath[ i ].toString() );
        }
        return getURLClassLoader( parent, classPath );
    }
    public static ClassLoader getURLClassLoader( ClassLoader parent, URL[] classpath)
        throws NoSuitableClassLoaderException
    {
        ClassLoader loader =
            new URLClassLoader( classpath, parent);

        if (loader != null)
        {
            return loader;
        }
        else
        {
            throw new NoSuitableClassLoaderException(
                "Unable to create URLClassLoader");
        }
    }

}
