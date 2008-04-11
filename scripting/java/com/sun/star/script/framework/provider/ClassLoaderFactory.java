/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ClassLoaderFactory.java,v $
 * $Revision: 1.5 $
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
